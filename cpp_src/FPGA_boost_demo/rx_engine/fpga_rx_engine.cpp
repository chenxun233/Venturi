#include "fpga_rx_engine.h"

FPGARxEngine::FPGARxEngine(BasicRxSource& source,
                           const FPGARxDecoder& decoder,
                           uint16_t que_idx)
    : m_source(source),
      m_decoder(decoder),
      m_que_idx(que_idx) {
}

const std::array<FPGAEventDesc, MAX_POLL_RECORDS>& FPGARxEngine::readEventBuffer() const {
    return m_event_buffer;
}

std::size_t FPGARxEngine::pollDecodedBatchImpl(FirstEventMask& mask,
                                               std::size_t max_count,
                                               bool get_time,
                                               FpgaSyncSnapshot* snapshot) {
    mask.count = 0;
    mask.first_event_mask = 0;

    uint64_t prod_ptr = 0;
    if (get_time) {
        uint64_t fpga_tick = 0;
        uint64_t host_time_ns = 0;
        uint64_t interval_ns = 0;
        m_source._readProdPtrSnapshot(m_que_idx,
                                      prod_ptr,
                                      fpga_tick,
                                      host_time_ns,
                                      interval_ns,
                                      true);
        if (snapshot != nullptr) {
            snapshot->fpga_tick = fpga_tick;
            snapshot->host_time_ns = host_time_ns;
            snapshot->interval_ns = interval_ns;
        }
    } else {
        m_source._readProdPtr(m_que_idx, prod_ptr);
    }

    std::size_t record_count = 0;
    uint64_t cons_ptr = m_cons_ptr;
    while (record_count < max_count && cons_ptr < prod_ptr) {
        const uint8_t* raw = m_source._pollOneRaw(m_que_idx, cons_ptr);
        if (raw == nullptr) {
            break;
        }
        m_decoder.decodeRawRecord(raw, m_event_buffer[record_count]);
        if (m_event_buffer[record_count].is_first_event) {
            mask.first_event_mask |= (1U << record_count);
            ++mask.count;
        }
        ++record_count;
        ++cons_ptr;
    }

    m_cons_ptr = cons_ptr;
    m_source._writeConsPtr(m_que_idx, cons_ptr);
    return record_count;
}

std::size_t FPGARxEngine::pollDecodedBatch(FirstEventMask& mask,
                                           std::size_t max_count) {
    return pollDecodedBatchImpl(mask, max_count, false, nullptr);
}

std::size_t FPGARxEngine::pollDecodedBatchSync(FirstEventMask& mask,
                                               std::size_t max_count,
                                               bool get_time,
                                               FpgaSyncSnapshot& snapshot) {
    return pollDecodedBatchImpl(mask, max_count, get_time, &snapshot);
}
