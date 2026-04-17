#include "../driver/fake_fpga_dev.h"
#include "../rx_engine/fpga_rx_engine.h"

#define private public
#include "../latency/latency_tracker.h"
#undef private

#include "../common/time_utils.h"

#include <gtest/gtest.h>
#include <vector>

namespace {

void writeLe16(FakeFPGADev::RawSlot& slot, std::size_t offset, uint16_t value) {
    slot[offset] = static_cast<uint8_t>(value & 0xffU);
    slot[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xffU);
}

void writeLe32(FakeFPGADev::RawSlot& slot, std::size_t offset, uint32_t value) {
    for (std::size_t byte_idx = 0; byte_idx < 4; ++byte_idx) {
        slot[offset + byte_idx] = static_cast<uint8_t>((value >> (8 * byte_idx)) & 0xffU);
    }
}

void writeLe48(FakeFPGADev::RawSlot& slot, std::size_t offset, uint64_t value) {
    for (std::size_t byte_idx = 0; byte_idx < 6; ++byte_idx) {
        slot[offset + byte_idx] = static_cast<uint8_t>((value >> (8 * byte_idx)) & 0xffU);
    }
}

FakeFPGADev::RawSlot makeRawSlot(uint16_t stock_locate,
                                 uint64_t event_tk,
                                 uint64_t frame_start_tk,
                                 uint32_t bid_shares,
                                 uint32_t bid_price,
                                 uint32_t ask_shares,
                                 uint32_t ask_price,
                                 uint8_t first_event) {
    FakeFPGADev::RawSlot slot {};
    writeLe16(slot, 0, stock_locate);
    writeLe48(slot, 2, event_tk);
    writeLe48(slot, 8, frame_start_tk);
    writeLe32(slot, 14, bid_shares);
    writeLe32(slot, 18, bid_price);
    writeLe32(slot, 22, ask_shares);
    writeLe32(slot, 26, ask_price);
    slot[30] = first_event;
    return slot;
}

} // namespace

TEST(FpgaRxEngineTest, pollDecodedBatchSyncDecodesFirstEventFlagAndSnapshot) {
    FakeFPGADev dev(1);
    dev.setSyncSnapshot(0, 2U, 12345U, 67890U, 222U);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 1000ULL, 900ULL, 10U, 100U, 20U, 105U, 0U),
        makeRawSlot(0x000d, 1001ULL, 900ULL, 11U, 101U, 21U, 106U, 1U),
    });

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    FPGAEventDesc out[2] {};
    FpgaSyncSnapshot snapshot {};

    const std::size_t count = engine.pollDecodedBatchSync(2, true, &snapshot, out);

    ASSERT_EQ(count, 2U);
    EXPECT_EQ(snapshot.fpga_tick, 12345U);
    EXPECT_EQ(snapshot.host_time_ns, 67890U);
    EXPECT_EQ(snapshot.interval_ns, 222U);
    EXPECT_EQ(out[0].is_first_event, 0U);
    EXPECT_EQ(out[1].is_first_event, 1U);
    EXPECT_EQ(out[1].event_tk, 1001U);
    EXPECT_EQ(out[0].stock_locate, 0x000d);
    EXPECT_EQ(out[0].frame_start_tk, 900U);
    EXPECT_EQ(out[0].bid_price, 100U);
    EXPECT_EQ(out[0].ask_price, 105U);
    EXPECT_EQ(out[1].bid_shares, 11U);
    EXPECT_EQ(out[1].ask_shares, 21U);
}

TEST(FpgaRxEngineTest, pollDecodedBatchReturnsPlainEventsWithoutDecodedWrapper) {
    FakeFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 2000ULL, 1900ULL, 12U, 102U, 22U, 107U, 1U),
        makeRawSlot(0x000d, 2001ULL, 1900ULL, 13U, 103U, 23U, 108U, 0U),
        makeRawSlot(0x000d, 2002ULL, 1900ULL, 14U, 104U, 24U, 109U, 0U),
    });
    dev.setProdPtr(0, 3U);

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    FPGAEventDesc out[3] {};

    const std::size_t count = engine.pollDecodedBatch(3, out);

    ASSERT_EQ(count, 3U);
    EXPECT_EQ(out[0].is_first_event, 1U);
    EXPECT_EQ(out[1].is_first_event, 0U);
    EXPECT_EQ(out[2].is_first_event, 0U);
    EXPECT_EQ(out[0].frame_start_tk, 1900U);
    EXPECT_EQ(out[0].bid_price, 102U);
    EXPECT_EQ(out[1].ask_price, 108U);
    EXPECT_EQ(out[2].bid_shares, 14U);
    EXPECT_EQ(out[2].ask_shares, 24U);
}

TEST(FpgaRxEngineTest, firstEventPushesFrameStartDmaEmitBatchStartAndBatchEndRecords) {
    FakeFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 2000ULL, 1900ULL, 12U, 102U, 22U, 107U, 1U),
    });
    dev.setProdPtr(0, 1U);

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    LatencyTracker tracker(1, 8);
    engine.attachLatenyTracker(&tracker);
    FPGAEventDesc out[1] {};

    const uint64_t before_ns = readMonotonicRawNs();
    ASSERT_EQ(engine.pollDecodedBatch(1, out), 1U);
    const uint64_t after_ns = readMonotonicRawNs();

    TimeRecord first {};
    TimeRecord second {};
    TimeRecord third {};
    TimeRecord fourth {};
    ASSERT_TRUE(tracker.m_latency_queues[0]->pop(first));
    ASSERT_TRUE(tracker.m_latency_queues[0]->pop(second));
    ASSERT_TRUE(tracker.m_latency_queues[0]->pop(third));
    ASSERT_TRUE(tracker.m_latency_queues[0]->pop(fourth));
    EXPECT_EQ(first.que_idx, 0U);
    EXPECT_EQ(first.event_tag, 2000U);
    EXPECT_EQ(first.event_stage, stage::FRAME_START);
    EXPECT_EQ(first.time_captured, 1900U);
    EXPECT_EQ(second.que_idx, 0U);
    EXPECT_EQ(second.event_tag, 2000U);
    EXPECT_EQ(second.event_stage, stage::DMA_EMIT);
    EXPECT_EQ(second.time_captured, 2000U);
    EXPECT_EQ(third.que_idx, 0U);
    EXPECT_EQ(third.event_tag, 2000U);
    EXPECT_EQ(third.event_stage, stage::BATCH_START);
    EXPECT_GT(third.time_captured, 0U);
    EXPECT_GE(third.time_captured, before_ns);
    EXPECT_LE(third.time_captured, after_ns);
    EXPECT_EQ(fourth.que_idx, 0U);
    EXPECT_EQ(fourth.event_tag, 2000U);
    EXPECT_EQ(fourth.event_stage, stage::BATCH_END);
    EXPECT_GT(fourth.time_captured, 0U);
    EXPECT_GE(fourth.time_captured, before_ns);
    EXPECT_LE(fourth.time_captured, after_ns);
    TimeRecord extra {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(extra));
}

TEST(FpgaRxEngineTest, pollDecodedBatchPushesBatchEndForEachFirstEvent) {
    FakeFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 2000ULL, 1900ULL, 12U, 102U, 22U, 107U, 1U),
        makeRawSlot(0x000d, 2001ULL, 1900ULL, 13U, 103U, 23U, 108U, 0U),
        makeRawSlot(0x000d, 2002ULL, 1900ULL, 14U, 104U, 24U, 109U, 1U),
    });
    dev.setProdPtr(0, 3U);

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    LatencyTracker tracker(1, 16);
    engine.attachLatenyTracker(&tracker);
    FPGAEventDesc out[3] {};

    const uint64_t before_ns = readMonotonicRawNs();
    ASSERT_EQ(engine.pollDecodedBatch(3, out), 3U);
    const uint64_t after_ns = readMonotonicRawNs();

    std::vector<TimeRecord> records;
    TimeRecord record {};
    while (tracker.m_latency_queues[0]->pop(record)) {
        records.push_back(record);
    }

    ASSERT_EQ(records.size(), 8U);
    EXPECT_EQ(records[0].event_stage, stage::FRAME_START);
    EXPECT_EQ(records[1].event_stage, stage::DMA_EMIT);
    EXPECT_EQ(records[2].event_stage, stage::BATCH_START);
    EXPECT_EQ(records[3].event_stage, stage::FRAME_START);
    EXPECT_EQ(records[4].event_stage, stage::DMA_EMIT);
    EXPECT_EQ(records[5].event_stage, stage::BATCH_START);
    EXPECT_EQ(records[6].event_stage, stage::BATCH_END);
    EXPECT_EQ(records[7].event_stage, stage::BATCH_END);
    EXPECT_EQ(records[2].event_tag, 2000U);
    EXPECT_EQ(records[5].event_tag, 2002U);
    EXPECT_EQ(records[6].event_tag, 2000U);
    EXPECT_EQ(records[7].event_tag, 2002U);
    EXPECT_GE(records[6].time_captured, before_ns);
    EXPECT_LE(records[6].time_captured, after_ns);
    EXPECT_GE(records[7].time_captured, before_ns);
    EXPECT_LE(records[7].time_captured, after_ns);
}

TEST(FpgaRxEngineTest, firstEventRecordsShareSameNonZeroTraceId) {
    FakeFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 2000ULL, 1900ULL, 12U, 102U, 22U, 107U, 1U),
    });
    dev.setProdPtr(0, 1U);

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    LatencyTracker tracker(1, 8, 8);
    engine.attachLatenyTracker(&tracker);
    FPGAEventDesc out[1] {};

    ASSERT_EQ(engine.pollDecodedBatch(1, out), 1U);

    std::vector<TimeRecord> records;
    TimeRecord record {};
    while (tracker.m_latency_queues[0]->pop(record)) {
        records.push_back(record);
    }

    ASSERT_EQ(records.size(), 4U);
    EXPECT_NE(records[0].trace_id, 0U);
    EXPECT_EQ(records[0].trace_id, records[1].trace_id);
    EXPECT_EQ(records[1].trace_id, records[2].trace_id);
    EXPECT_EQ(records[2].trace_id, records[3].trace_id);
}

TEST(FpgaRxEngineTest, laterFirstEventGetsLargerTraceId) {
    FakeFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 2000ULL, 1900ULL, 12U, 102U, 22U, 107U, 1U),
        makeRawSlot(0x000d, 2001ULL, 1900ULL, 13U, 103U, 23U, 108U, 0U),
        makeRawSlot(0x000d, 2002ULL, 1900ULL, 14U, 104U, 24U, 109U, 1U),
    });
    dev.setProdPtr(0, 3U);

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    LatencyTracker tracker(1, 16, 8);
    engine.attachLatenyTracker(&tracker);
    FPGAEventDesc out[3] {};

    ASSERT_EQ(engine.pollDecodedBatch(3, out), 3U);

    std::vector<TimeRecord> records;
    TimeRecord record {};
    while (tracker.m_latency_queues[0]->pop(record)) {
        if (record.event_stage == stage::FRAME_START) {
            records.push_back(record);
        }
    }

    ASSERT_EQ(records.size(), 2U);
    EXPECT_LT(records[0].trace_id, records[1].trace_id);
}

TEST(FpgaRxEngineTest, firstEventBeyondMaxPollRecordsKeepsBatchEndTraceId) {
    FakeFPGADev dev(1);
    std::vector<FakeFPGADev::RawSlot> raw_slots;
    raw_slots.reserve(MAX_POLL_RECORDS + 1U);
    for (std::size_t idx = 0; idx < MAX_POLL_RECORDS; ++idx) {
        raw_slots.push_back(
            makeRawSlot(0x000d,
                        2000ULL + static_cast<uint64_t>(idx),
                        1900ULL,
                        12U,
                        102U,
                        22U,
                        107U,
                        0U));
    }
    raw_slots.push_back(makeRawSlot(0x000d, 5000ULL, 4900ULL, 12U, 102U, 22U, 107U, 1U));
    dev.setRawSlots(0, raw_slots);
    dev.setProdPtr(0, static_cast<uint64_t>(MAX_POLL_RECORDS + 1U));

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    LatencyTracker tracker(1, 16, 8);
    engine.attachLatenyTracker(&tracker);
    std::vector<FPGAEventDesc> out(MAX_POLL_RECORDS + 1U);

    ASSERT_EQ(engine.pollDecodedBatch(out.size(), out.data()), MAX_POLL_RECORDS + 1U);

    std::vector<TimeRecord> records;
    TimeRecord record {};
    while (tracker.m_latency_queues[0]->pop(record)) {
        records.push_back(record);
    }

    ASSERT_EQ(records.size(), 4U);
    EXPECT_EQ(records[0].event_stage, stage::FRAME_START);
    EXPECT_EQ(records[3].event_stage, stage::BATCH_END);
    EXPECT_NE(records[0].trace_id, 0U);
    EXPECT_EQ(records[0].trace_id, records[3].trace_id);
}

TEST(FpgaRxEngineTest, pollDecodedBatchSyncDoesNotPushBatchBoundaryRecords) {
    FakeFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 2100ULL, 2000ULL, 12U, 102U, 22U, 107U, 1U),
    });
    dev.setProdPtr(0, 1U);

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    LatencyTracker tracker(1, 8);
    engine.attachLatenyTracker(&tracker);
    FPGAEventDesc out[1] {};

    ASSERT_EQ(engine.pollDecodedBatchSync(1, false, nullptr, out), 1U);

    std::vector<TimeRecord> records;
    TimeRecord record {};
    while (tracker.m_latency_queues[0]->pop(record)) {
        records.push_back(record);
    }

    ASSERT_EQ(records.size(), 2U);
    EXPECT_EQ(records[0].event_stage, stage::FRAME_START);
    EXPECT_EQ(records[1].event_stage, stage::DMA_EMIT);
    EXPECT_EQ(records[0].event_tag, 2100U);
    EXPECT_EQ(records[1].event_tag, 2100U);
}

TEST(FpgaRxEngineTest, firstEventWithoutTrackerDoesNotEmitLatencyRecords) {
    FakeFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 3000ULL, 2900ULL, 15U, 110U, 25U, 115U, 1U),
    });
    dev.setProdPtr(0, 1U);

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    LatencyTracker tracker(1, 8);
    FPGAEventDesc out[1] {};

    ASSERT_EQ(engine.pollDecodedBatch(1, out), 1U);
    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(FpgaRxEngineTest, nonFirstEventDoesNotpushLatencyLogRecords) {
    FakeFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 2001ULL, 1900ULL, 13U, 103U, 23U, 108U, 0U),
    });
    dev.setProdPtr(0, 1U);

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    LatencyTracker tracker(1, 8);
    engine.attachLatenyTracker(&tracker);
    FPGAEventDesc out[1] {};

    ASSERT_EQ(engine.pollDecodedBatch(1, out), 1U);
    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}
