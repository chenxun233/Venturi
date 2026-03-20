#include "fpga_rx_adapter.h"

#include <array>

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
constexpr std::size_t kRawDecodeBatchSize = 32;

} // namespace

FPGARxDataAdaptor::FPGARxDataAdaptor(FPGADev& device)
    : m_device(device),
      m_queue_states(device.rxQueueCount()) {
}

uint16_t FPGARxDataAdaptor::queueCount() const {
    return m_device.rxQueueCount();
}

bool FPGARxDataAdaptor::pollOne(uint16_t queue_id, FPGAEventDesc& out) {
    if (!_prepareQueue(queue_id)) {
        return false;
    }

    FPGADev::RawRxRecordView raw_record;
    QueueState& queue_state = m_queue_states[queue_id];
    if (m_device.pollRawRecordsHotPath(queue_state.queue_ctx, queue_state.cons_ptr, &raw_record, 1) != 1) {
        return false;
    }
    out = _decodeRawRecord(raw_record);
    ++queue_state.cons_ptr;
    m_device.writeConsPtrHotPath(queue_state.queue_ctx, queue_state.cons_ptr);
    return true;
}

std::size_t FPGARxDataAdaptor::pollBatch(uint16_t queue_id, FPGAEventDesc* out, std::size_t max_count) {
    if (out == nullptr || max_count == 0) {
        return 0;
    }

    if (!_prepareQueue(queue_id)) {
        return 0;
    }

    std::array<FPGADev::RawRxRecordView, kRawDecodeBatchSize> raw_batch {};
    QueueState& queue_state = m_queue_states[queue_id];
    std::size_t total_decoded = 0;
    while (total_decoded < max_count) {
        const std::size_t batch_limit = std::min<std::size_t>(raw_batch.size(), max_count - total_decoded);
        const std::size_t raw_count = m_device.pollRawRecordsHotPath(queue_state.queue_ctx,
                                                                     queue_state.cons_ptr,
                                                                     raw_batch.data(),
                                                                     batch_limit);
        for (std::size_t idx = 0; idx < raw_count; ++idx) {
            out[total_decoded + idx] = _decodeRawRecord(raw_batch[idx]);
        }
        queue_state.cons_ptr += raw_count;
        if (raw_count != 0) {
            m_device.writeConsPtrHotPath(queue_state.queue_ctx, queue_state.cons_ptr);
        }
        total_decoded += raw_count;
        if (raw_count < batch_limit) {
            break;
        }
    }
    return total_decoded;
}

bool FPGARxDataAdaptor::readSyncSnapshot(uint16_t queue_id, FpgaSyncSnapshot& out) {
    FPGADev::RawSyncSnapshot raw_snapshot;
    if (!m_device.readRawSyncSnapshot(queue_id, raw_snapshot)) {
        return false;
    }
    out = _decodeRawSyncSnapshot(raw_snapshot);
    return true;
}

bool FPGARxDataAdaptor::_prepareQueue(uint16_t queue_id) {
    if (queue_id >= m_queue_states.size()) {
        m_queue_states.resize(static_cast<std::size_t>(queue_id) + 1);
    }

    QueueState& queue_state = m_queue_states[queue_id];
    if (queue_state.validated) {
        return true;
    }

    if (!m_device.validateRxQueue(queue_id, queue_state.queue_ctx)) {
        return false;
    }

    queue_state.cons_ptr = 0;
    queue_state.validated = true;
    return true;
}

FPGAEventDesc FPGARxDataAdaptor::_decodeRawRecord(const FPGADev::RawRxRecordView& record) {
    FPGAEventDesc event;
    event.queue_id = record.queue_id;
    event.stock_locate = read_le16(record.bytes, 0);
    event.frame_latency = read_le48(record.bytes, 2);
    event.frame_start_ts = read_le48(record.bytes, 8);
    event.bid_shares = read_le32(record.bytes, 14);
    event.bid_price = read_le32(record.bytes, 18);
    event.ask_shares = read_le32(record.bytes, 22);
    event.ask_price = read_le32(record.bytes, 26);
    return event;
}

FpgaSyncSnapshot FPGARxDataAdaptor::_decodeRawSyncSnapshot(const FPGADev::RawSyncSnapshot& snapshot) {
    FpgaSyncSnapshot decoded;
    decoded.queue_id = snapshot.queue_id;
    decoded.prod_ptr = snapshot.prod_ptr_qword;
    decoded.dma_timestamp = snapshot.timestamp_qword & kTimestampMask48;
    return decoded;
}
