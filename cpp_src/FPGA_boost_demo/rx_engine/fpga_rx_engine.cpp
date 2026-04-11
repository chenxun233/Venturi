#include "fpga_rx_engine.h"

#include "../common/time_utils.h"
#include "../latency/latency_tracker.h"
#include "../sync/FPGA_regression.h"

#include <ctime>
#include <stdexcept>


FPGARxEngine::FPGARxEngine(BasicRxDev& source,
                           const FPGARxDecoder& decoder,
                           uint16_t que_idx)
    : m_dev(source),
      m_decoder(decoder),
      m_que_idx(que_idx) {
    if (!m_dev.isValid()) {
        throw std::runtime_error("Failed to initialize FPGARxEngine with invalid source");
    }
}

std::size_t FPGARxEngine::pollDecodedBatchImpl(FirstEventMask& mask,
                                               std::size_t max_count,
                                               bool get_snapshot,
                                               FpgaSyncSnapshot* snapshot,
                                               DecodedEvent* out) {
    mask.count = 0;
    mask.first_event_mask = 0;

    uint64_t prod_ptr = 0;
    if (get_snapshot) {
        uint64_t fpga_tick = 0;
        uint64_t host_time_ns = 0;
        uint64_t interval_ns = 0;
        m_dev._readProdPtrSnapshot(m_que_idx,
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
        m_dev._readProdPtr(m_que_idx, prod_ptr);
    }

    std::size_t record_count = 0;
    uint64_t cons_ptr = m_cons_ptr;
    while (record_count < max_count && cons_ptr < prod_ptr) {
        const uint8_t* raw = m_dev._pollDataRaw(m_que_idx, cons_ptr);
        if (raw == nullptr) {
            break;
        }
        out[record_count].captured_time_ns = 0;
        m_decoder.decodeRawRecord(raw, out[record_count].event);
        if (out[record_count].event.is_first_event) {
            out[record_count].captured_time_ns = readMonotonicRawNs();
            mask.first_event_mask |= (1U << record_count);
            ++mask.count;
        }
        ++record_count;
        ++cons_ptr;
    }

    m_cons_ptr = cons_ptr;
    m_dev._writeConsPtr(m_que_idx, cons_ptr);
    return record_count;
}

std::size_t FPGARxEngine::pollDecodedBatch(FirstEventMask& mask,
                                           std::size_t max_count,
                                           DecodedEvent* out) {
    return pollDecodedBatchImpl(mask, max_count, false, nullptr, out);
}

std::size_t FPGARxEngine::pollDecodedBatchSync(FirstEventMask& mask,
                                               std::size_t max_count,
                                               bool get_snapshot,
                                               FpgaSyncSnapshot* snapshot,
                                               DecodedEvent* out) {
    return pollDecodedBatchImpl(mask, max_count, get_snapshot, snapshot, out);
}
