#include "../fpga_dev/fpga_dev.h"

#define private public
#include "../fpga_rx_engine/fpga_rx_engine.h"
#include "../latency/latency_tracker.h"
#undef private

#include "../common/time_utils.h"

#include <gtest/gtest.h>

#include <vector>

namespace {

class TestFPGADev : public FPGADev {
public:
    static constexpr std::size_t kSlotSizeBytes = 32;
    using RawSlot = std::array<uint8_t, kSlotSizeBytes>;

    explicit TestFPGADev(std::size_t queue_count = 1)
        : FPGADev("test"),
          m_queue_states(queue_count) {
    }

    void setRawSlots(uint16_t que_idx, const std::vector<RawSlot>& slots) {
        m_queue_states[que_idx].slots = slots;
    }

    void setProdPtr(uint16_t que_idx, uint64_t prod_ptr) {
        m_queue_states[que_idx].prod_ptr = prod_ptr;
    }

    uint64_t lastWrittenConsPtr(uint16_t que_idx) const {
        return m_queue_states[que_idx].last_written_cons_ptr;
    }

    bool isValid() const {
        return true;
    }

    void readProdPtr(uint16_t que_idx, uint64_t& prod_ptr) const {
        prod_ptr = m_queue_states[que_idx].prod_ptr;
    }

    uint64_t readDropCount(uint16_t que_idx) const {
        return m_queue_states[que_idx].drop_count;
    }

    const uint8_t* pollDataRaw(uint16_t que_idx, uint64_t cons_ptr) const {
        const QueueState& queue_state = m_queue_states[que_idx];
        const std::size_t slot_count = queue_state.slots.size();
        if (slot_count == 0) {
            return nullptr;
        }
        const std::size_t slot_idx = static_cast<std::size_t>(cons_ptr % slot_count);
        return queue_state.slots[slot_idx].data();
    }

    void writeConsPtr(uint16_t que_idx, uint64_t cons_ptr) {
        m_queue_states[que_idx].last_written_cons_ptr = cons_ptr;
    }

private:
    struct QueueState {
        std::vector<RawSlot> slots;
        uint64_t prod_ptr {0};
        uint64_t drop_count {0};
        uint64_t last_written_cons_ptr {0};
    };

    std::vector<QueueState> m_queue_states;
};

void writeLe16(TestFPGADev::RawSlot& slot, std::size_t offset, uint16_t value) {
    slot[offset] = static_cast<uint8_t>(value & 0xffU);
    slot[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xffU);
}

void writeLe32(TestFPGADev::RawSlot& slot, std::size_t offset, uint32_t value) {
    for (std::size_t byte_idx = 0; byte_idx < 4; ++byte_idx) {
        slot[offset + byte_idx] = static_cast<uint8_t>((value >> (8 * byte_idx)) & 0xffU);
    }
}

void writeLe48(TestFPGADev::RawSlot& slot, std::size_t offset, uint64_t value) {
    for (std::size_t byte_idx = 0; byte_idx < 6; ++byte_idx) {
        slot[offset + byte_idx] = static_cast<uint8_t>((value >> (8 * byte_idx)) & 0xffU);
    }
}

TestFPGADev::RawSlot makeRawSlot(uint16_t stock_locate,
                                 uint64_t event_tk,
                                 uint64_t frame_start_tk,
                                 uint32_t bid_shares,
                                 uint32_t bid_price,
                                 uint32_t ask_shares,
                                 uint32_t ask_price,
                                 uint8_t first_event) {
    TestFPGADev::RawSlot slot {};
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

TEST(FpgaRxEngineTest, decodeRawRecordMapsKnownBytesIntoExpectedFields) {
    const TestFPGADev::RawSlot raw = makeRawSlot(0x000d,
                                                 0x000102030405ULL,
                                                 0x00060708090aULL,
                                                 1234U,
                                                 5678U,
                                                 4321U,
                                                 8765U,
                                                 1U);
    FPGAEventDesc event {};

    FPGARxEngine::_decodeRawRecord(raw.data(), event);

    EXPECT_EQ(event.stock_locate, 0x000d);
    EXPECT_EQ(event.event_tk, 0x000102030405ULL);
    EXPECT_EQ(event.frame_start_tk, 0x00060708090aULL);
    EXPECT_EQ(event.bid_shares, 1234U);
    EXPECT_EQ(event.bid_price, 5678U);
    EXPECT_EQ(event.ask_shares, 4321U);
    EXPECT_EQ(event.ask_price, 8765U);
    EXPECT_EQ(event.is_first_event, 1U);
}

TEST(FpgaRxEngineTest, firstEventPushesTracingStagesWithOneSharedTraceId) {
    TestFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 2000ULL, 1900ULL, 12U, 102U, 22U, 107U, 1U),
    });
    dev.setProdPtr(0, 1U);

    FPGARxEngine engine(dev, 0);
    LatencyTracker tracker(1, 8);
    engine.attachLatencyTracker(&tracker);
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
    TestFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 2001ULL, 1900ULL, 13U, 103U, 23U, 108U, 0U),
    });
    dev.setProdPtr(0, 1U);

    FPGARxEngine engine(dev, 0);
    LatencyTracker tracker(1, 8);
    engine.attachLatencyTracker(&tracker);
    FPGAEventDesc out[1] {};

    ASSERT_EQ(engine.pollDecodedBatch(1, out), 1U);
    EXPECT_EQ(out[0].trace_id, 0U);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(FpgaRxEngineTest, activeTraceBlocksLaterFirstEventUntilFinalizeClearsIt) {
    TestFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 2000ULL, 1900ULL, 12U, 102U, 22U, 107U, 1U),
        makeRawSlot(0x000d, 2001ULL, 1900ULL, 13U, 103U, 23U, 108U, 1U),
    });
    dev.setProdPtr(0, 2U);

    FPGARxEngine engine(dev, 0);
    LatencyTracker tracker(1, 16);
    engine.attachLatencyTracker(&tracker);
    FPGAEventDesc out[2] {};

    ASSERT_EQ(engine.pollDecodedBatch(2, out), 2U);
    ASSERT_NE(out[0].trace_id, 0U);
    EXPECT_EQ(out[1].trace_id, 0U);
}
