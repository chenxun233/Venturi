#include "fpga_rx_decoder.h"
#include <stdexcept>
#include "../../common/log.h"


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

constexpr uint64_t kTimestampMask48 = (1ULL << 48) - 1ULL;

} // namespace

FPGARxDecoder::FPGARxDecoder(BasicRxSource& source, uint16_t que_idx)
    : m_source(source),
      m_cons_ptr(0),
      m_que_idx(que_idx) {

    if (!m_source.isValid()) {
        warn("configure the RX source before creating FPGARxDecoder");
    }
}


void FPGARxDecoder::_decodeRawRecord(const uint8_t* record, FPGAEventDesc& event) {
    event.is_first_event        = record[30];
    event.ask_price             = read_le32(record, 26);
    event.ask_shares            = read_le32(record, 22);
    event.bid_price             = read_le32(record, 18);
    event.bid_shares            = read_le32(record, 14);
    event.frame_start_tk        = read_le48(record, 8);
    event.event_tk              = read_le48(record, 2);
    event.stock_locate          = read_le16(record, 0);
    
    
}




std::size_t FPGARxDecoder::decodeRawBatch(FirstEventMask& mask,
                                        FPGAEventDesc* out,
                                        std::size_t max_count) {
    uint64_t cons_ptr = m_cons_ptr;
    uint64_t prod_ptr = 0;
    m_source._readProdPtr(m_que_idx, prod_ptr);
    std::size_t record_count = 0;
    const uint8_t* raw_byte = nullptr;
    while (record_count < max_count && cons_ptr < prod_ptr) {
        raw_byte = m_source._pollOneRaw(m_que_idx, cons_ptr);
        _decodeRawRecord(raw_byte, out[record_count]);
        if (out[record_count].is_first_event) {
            mask.first_event_mask |= (1ULL <<record_count);
            ++mask.count;
        }
        ++record_count;
        ++cons_ptr;
    }
    m_cons_ptr = cons_ptr;
    m_source._writeConsPtr(m_que_idx, cons_ptr);
    return record_count;
}


std::size_t FPGARxDecoder::decodeRawBatchSync(FirstEventMask& mask,
                                             FPGAEventDesc* out, 
                                             FpgaSyncSnapshot& snapshot,
                                             bool get_time,
                                             std::size_t max_count){

    uint64_t cons_ptr = m_cons_ptr;
    uint64_t prod_ptr = 0;
    m_source._readProdPtrAndTime(m_que_idx,
                                prod_ptr,
                                snapshot.fpga_tick,
                                snapshot.host_time_ns,
                                snapshot.interval_ns,
                                get_time);
    std::size_t record_count = 0;
    const uint8_t* raw_byte = nullptr;
    while (record_count < max_count && cons_ptr < prod_ptr) {
        raw_byte = m_source._pollOneRaw(m_que_idx, cons_ptr);
        _decodeRawRecord(raw_byte, out[record_count]);
        if (out[record_count].is_first_event) {
            mask.first_event_mask |= (1ULL <<record_count);
            ++mask.count;
        }
        ++record_count;
        ++cons_ptr;
    }
    m_cons_ptr = cons_ptr;
    m_source._writeConsPtr(m_que_idx, cons_ptr);
    return record_count;

}
