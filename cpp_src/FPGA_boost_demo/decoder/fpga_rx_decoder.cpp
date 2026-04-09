#include "fpga_rx_decoder.h"
#include <cstddef>

namespace {

uint16_t read_le16(const uint8_t* bytes, std::size_t offset) {
    return static_cast<uint16_t>(static_cast<uint16_t>(bytes[offset]) |
                                 (static_cast<uint16_t>(bytes[offset + 1]) << 8));
}

uint32_t read_le32(const uint8_t* bytes, std::size_t offset) {
    return static_cast<uint32_t>(bytes[offset]) |
           (static_cast<uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<uint32_t>(bytes[offset + 3]) << 24);
}

uint64_t read_le48(const uint8_t* bytes, std::size_t offset) {
    uint64_t value = 0;
    for (int byte_idx = 0; byte_idx < 6; ++byte_idx) {
        value |= (static_cast<uint64_t>(bytes[offset + byte_idx]) << (8 * byte_idx));
    }
    return value;
}


} // namespace

void FPGARxDecoder::decodeRawRecord(const uint8_t* record, FPGAEventDesc& event) const {
    event.is_first_event        = record[30];
    event.ask_price             = read_le32(record, 26);
    event.ask_shares            = read_le32(record, 22);
    event.bid_price             = read_le32(record, 18);
    event.bid_shares            = read_le32(record, 14);
    event.frame_start_tk        = read_le48(record, 8);
    event.event_tk              = read_le48(record, 2);
    event.stock_locate          = read_le16(record, 0);
}
