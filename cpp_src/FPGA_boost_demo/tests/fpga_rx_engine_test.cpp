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
