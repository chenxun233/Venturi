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

TEST(FpgaRxEngineTest, pollDecodedBatchSyncDecodesSnapshotWithoutTracing) {
    FakeFPGADev dev(1);
    dev.setSyncSnapshot(0, 2U, 12345U, 67890U, 222U);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 1000ULL, 900ULL, 10U, 100U, 20U, 105U, 1U),
    });

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    LatencyTracker tracker(1, 8);
    engine.attachLatenyTracker(&tracker);
    FPGAEventDesc out[1] {};
    FpgaSyncSnapshot snapshot {};

    ASSERT_EQ(engine.pollDecodedBatchSync(1, true, &snapshot, out), 1U);
    EXPECT_EQ(snapshot.fpga_tick, 12345U);
    EXPECT_EQ(snapshot.host_time_ns, 67890U);
    EXPECT_EQ(snapshot.interval_ns, 222U);
    EXPECT_EQ(out[0].trace_id, 0U);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(FpgaRxEngineTest, firstEventPushesTracingStagesWithOneSharedTraceId) {
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

    ASSERT_NE(out[0].trace_id, 0U);

    std::vector<TimeRecord> records;
    TimeRecord record {};
    while (tracker.m_latency_queues[0]->pop(record)) {
        records.push_back(record);
    }

    ASSERT_EQ(records.size(), 4U);
    EXPECT_EQ(records[0].event_stage, stage::FRAME_START);
    EXPECT_EQ(records[1].event_stage, stage::DMA_EMIT);
    EXPECT_EQ(records[2].event_stage, stage::BATCH_START);
    EXPECT_EQ(records[3].event_stage, stage::BATCH_END);
    EXPECT_EQ(records[0].trace_id, out[0].trace_id);
    EXPECT_EQ(records[1].trace_id, out[0].trace_id);
    EXPECT_EQ(records[2].trace_id, out[0].trace_id);
    EXPECT_EQ(records[3].trace_id, out[0].trace_id);
    EXPECT_GE(records[2].time_captured, before_ns);
    EXPECT_LE(records[3].time_captured, after_ns);
}

TEST(FpgaRxEngineTest, nonFirstEventDoesNotAllocateTraceId) {
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
    EXPECT_EQ(out[0].trace_id, 0U);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(FpgaRxEngineTest, activeTraceBlocksLaterFirstEventUntilFinalizeClearsIt) {
    FakeFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 2000ULL, 1900ULL, 12U, 102U, 22U, 107U, 1U),
        makeRawSlot(0x000d, 2001ULL, 1900ULL, 13U, 103U, 23U, 108U, 1U),
    });
    dev.setProdPtr(0, 2U);

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    LatencyTracker tracker(1, 16);
    engine.attachLatenyTracker(&tracker);
    FPGAEventDesc out[2] {};

    ASSERT_EQ(engine.pollDecodedBatch(2, out), 2U);
    ASSERT_NE(out[0].trace_id, 0U);
    EXPECT_EQ(out[1].trace_id, 0U);
}
