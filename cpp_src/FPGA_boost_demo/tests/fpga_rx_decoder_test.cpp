#include "../decoder/fpga_rx_decoder.h"
#include "../driver/fake_fpga_dev.h"

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

TEST(FpgaRxDecoderTest, decodesKnownRawBytesIntoExpectedFields) {
    FPGARxDecoder decoder;
    const FakeFPGADev::RawSlot raw = makeRawSlot(0x000d,
                                                 0x000102030405ULL,
                                                 0x00060708090aULL,
                                                 1234U,
                                                 5678U,
                                                 4321U,
                                                 8765U,
                                                 1U);
    FPGAEventDesc event {};

    decoder.decodeRawRecord(raw.data(), event);

    EXPECT_EQ(event.stock_locate, 0x000d);
    EXPECT_EQ(event.event_tk, 0x000102030405ULL);
    EXPECT_EQ(event.frame_start_tk, 0x00060708090aULL);
    EXPECT_EQ(event.bid_shares, 1234U);
    EXPECT_EQ(event.ask_price, 8765U);
    EXPECT_EQ(event.is_first_event, 1U);
}
