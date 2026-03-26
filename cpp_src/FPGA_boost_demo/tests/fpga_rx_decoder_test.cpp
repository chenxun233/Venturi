#include "../decoder/fpga_rx_decoder.h"
#include "../driver/fake_fpga_dev.h"

#include <gtest/gtest.h>

#include <array>

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
    FakeFPGADev fake_device;
    std::vector<FakeFPGADev::RawSlot> raw_slots;
    raw_slots.push_back(makeRawSlot(0x000d,
                                    0x000102030405ULL,
                                    0x00060708090aULL,
                                    1234U,
                                    5678U,
                                    4321U,
                                    8765U,
                                    1U));
    fake_device.setRawSlots(0, raw_slots);
    fake_device.setProdPtr(0, 1);

    FPGARxDecoder decoder(fake_device, 0);
    std::array<FPGAEventDesc, 4> out {};
    FirstEventMask mask {};

    const std::size_t count = decoder.decodeRawBatch(mask, nullptr, out.data(), out.size());

    ASSERT_EQ(count, 1U);
    EXPECT_EQ(mask.count, 1U);
    EXPECT_EQ(mask.first_event_mask, 0x1U);
    EXPECT_EQ(out[0].stock_locate, 0x000d);
    EXPECT_EQ(out[0].event_tk, 0x000102030405ULL);
    EXPECT_EQ(out[0].frame_start_tk, 0x00060708090aULL);
    EXPECT_EQ(out[0].bid_shares, 1234U);
    EXPECT_EQ(out[0].bid_price, 5678U);
    EXPECT_EQ(out[0].ask_shares, 4321U);
    EXPECT_EQ(out[0].ask_price, 8765U);
    EXPECT_EQ(out[0].is_first_event, 1U);
    EXPECT_EQ(fake_device.lastWrittenConsPtr(0), 1U);
}

TEST(FpgaRxDecoderTest, decodesSyncSnapshotAndPublishesConsumerProgress) {
    FakeFPGADev fake_device;
    std::vector<FakeFPGADev::RawSlot> raw_slots;
    raw_slots.push_back(makeRawSlot(0x0ee8,
                                    0x000000000011ULL,
                                    0x000000000022ULL,
                                    10U,
                                    20U,
                                    30U,
                                    40U,
                                    0U));
    fake_device.setRawSlots(0, raw_slots);
    fake_device.setSyncSnapshot(0, 1U, 111U, 222U, 333U);

    FPGARxDecoder decoder(fake_device, 0);
    std::array<FPGAEventDesc, 2> out {};
    FpgaSyncSnapshot snapshot {};
    FirstEventMask mask {};

    const std::size_t count = decoder.decodeRawBatchSync(mask, nullptr, out.data(), snapshot, true, out.size());

    ASSERT_EQ(count, 1U);
    EXPECT_EQ(mask.count, 0U);
    EXPECT_EQ(mask.first_event_mask, 0U);
    EXPECT_EQ(snapshot.fpga_tick, 111U);
    EXPECT_EQ(snapshot.host_time_ns, 222U);
    EXPECT_EQ(snapshot.interval_ns, 333U);
    EXPECT_EQ(out[0].stock_locate, 0x0ee8);
    EXPECT_EQ(out[0].is_first_event, 0U);
    EXPECT_EQ(fake_device.lastWrittenConsPtr(0), 1U);
}

TEST(FpgaRxDecoderTest, constructsDecoderWithFakeSourcePattern) {
    FakeFPGADev fake_device(1);
    fake_device.setRawSlots(0, {makeRawSlot(0x0011, 1U, 2U, 3U, 4U, 5U, 6U, 0U)});
    fake_device.setProdPtr(0, 1U);

    FPGARxDecoder decoder(fake_device, 0);
    std::array<FPGAEventDesc, 1> out {};
    FirstEventMask mask {};

    EXPECT_EQ(decoder.decodeRawBatch(mask, nullptr, out.data(), out.size()), 1U);
}
