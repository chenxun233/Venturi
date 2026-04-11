#include "../driver/fake_fpga_dev.h"
#include "../rx_engine/fpga_rx_engine.h"

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

TEST(FpgaRxEngineTest, pollDecodedBatchSyncCapturesTimePerFirstEventItem) {
    FakeFPGADev dev(1);
    dev.setRawSlots(0, {
        makeRawSlot(0x000d, 1000ULL, 900ULL, 10U, 100U, 20U, 105U, 0U),
        makeRawSlot(0x000d, 1001ULL, 900ULL, 11U, 101U, 21U, 106U, 1U),
    });
    dev.setProdPtr(0, 2U);

    FPGARxDecoder decoder {};
    FPGARxEngine engine(dev, decoder, 0);
    FirstEventMask mask {};
    DecodedEvent out[2] {};

    const std::size_t count = engine.pollDecodedBatchSync(mask, 2, false, nullptr, out);

    ASSERT_EQ(count, 2U);
    EXPECT_EQ(mask.first_event_mask, 0b10U);
    EXPECT_EQ(out[0].captured_time_ns, 0U);
    EXPECT_GT(out[1].captured_time_ns, 0U);
    EXPECT_EQ(out[1].event.event_tk, 1001U);
}
