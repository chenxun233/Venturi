#include "../driver/fake_fpga_dev.h"
#include "../decoder/fpga_rx_decoder.h"
#include "../latency/latency_tracker.h"
#include "../rx_engine/fpga_rx_engine.h"
#include "../sync/regression.h"

#include <gtest/gtest.h>

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

TEST(FpgaRxEngineTest, pollsDecodedBatchAndPreservesRxSideEffects) {
    FakeFPGADev device(1);
    device.setRawSlots(0, {
        makeRawSlot(0x000d, 0x11ULL, 0x01ULL, 100U, 101U, 200U, 201U, 1U),
        makeRawSlot(0x0ee8, 0x22ULL, 0x02ULL, 300U, 301U, 400U, 401U, 0U),
    });
    device.setSyncSnapshot(0, 2U, 111U, 222U, 333U);

    FPGARxDecoder decoder;
    FPGARxEngine engine(device, decoder, 0);
    LatencyTracker latency_tracker(1, 8);
    Regression regression;
    engine.attachLatencyTracker(latency_tracker);
    engine.attachRegression(regression);

    FpgaSyncSnapshot snapshot {};
    FirstEventMask mask {};
    const std::size_t count = engine.pollDecodedBatchSync(mask, MAX_POLL_RECORDS, true, snapshot);

    EXPECT_EQ(count, 2U);
    EXPECT_EQ(mask.count, 1U);
    EXPECT_EQ(mask.first_event_mask, 0x1U);
    EXPECT_EQ(snapshot.fpga_tick, 111U);
    EXPECT_EQ(snapshot.host_time_ns, 222U);
    EXPECT_EQ(snapshot.interval_ns, 333U);
    EXPECT_EQ(device.lastWrittenConsPtr(0), 2U);

    const auto& events = engine.readEventBuffer();
    EXPECT_EQ(events[0].stock_locate, 0x000d);
    EXPECT_EQ(events[0].bid_price, 101U);
    EXPECT_EQ(events[0].ask_shares, 200U);
    EXPECT_EQ(events[1].stock_locate, 0x0ee8);
    EXPECT_EQ(events[1].ask_price, 401U);

    const FpgaSyncSnapshot regression_snapshot = regression.readSnapshot();
    EXPECT_EQ(regression_snapshot.fpga_tick, 111U);
    EXPECT_EQ(regression_snapshot.host_time_ns, 222U);
    EXPECT_EQ(regression_snapshot.interval_ns, 333U);

    EXPECT_EQ(latency_tracker.run(), 3U);
}
