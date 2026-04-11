#include "fpga_rx_engine.h"

#include "../common/time_utils.h"
#include "../latency/latency_tracker.h"

#include <stdexcept>

FPGARxEngine::FPGARxEngine(BasicRxDev& source,
                           const FPGARxDecoder& decoder,
                           uint16_t que_idx)
    : m_device(source),
      m_decoder(decoder),
      m_que_idx(que_idx) {
    if (!m_device.isValid()) {
        throw std::runtime_error("Failed to initialize FPGARxEngine with invalid source");
    }
}

std::size_t FPGARxEngine::pollDecodedBatchImpl(
                                               std::size_t max_count,
                                               bool get_snapshot,
                                               FpgaSyncSnapshot* snapshot,
                                               FPGAEventDesc* out) {

    uint64_t prod_ptr = 0;
    if (get_snapshot) {
        uint64_t fpga_tick = 0;
        uint64_t host_time_ns = 0;
        uint64_t interval_ns = 0;
        m_device._readProdPtrSnapshot(m_que_idx,
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
        m_device._readProdPtr(m_que_idx, prod_ptr);
    }

    std::size_t record_count = 0;
    uint64_t cons_ptr = m_cons_ptr;
    while (record_count < max_count && cons_ptr < prod_ptr) {
        const uint8_t* raw = m_device._pollDataRaw(m_que_idx, cons_ptr);
        if (raw == nullptr) {
            break;
        }
        m_decoder.decodeRawRecord(raw, out[record_count]);
        if (out[record_count].is_first_event != 0 && m_latency_tracker != nullptr) {
            m_latency_tracker->pushRecord(TimeRecord {
                .que_idx = m_que_idx,
                .event_ts = out[record_count].event_tk,
                .event_stage = stage::FRAME_START,
                .time_captured = out[record_count].frame_start_tk,
            });
            m_latency_tracker->pushRecord(TimeRecord {
                .que_idx = m_que_idx,
                .event_ts = out[record_count].event_tk,
                .event_stage = stage::DMA_EMIT,
                .time_captured = out[record_count].event_tk,
            });
            m_latency_tracker->pushRecord(TimeRecord {
                .que_idx = m_que_idx,
                .event_ts = out[record_count].event_tk,
                .event_stage = stage::DECODE,
                .time_captured = readMonotonicRawNs(),
            });
        }
        ++record_count;
        ++cons_ptr;
    }

    m_cons_ptr = cons_ptr;
    m_device._writeConsPtr(m_que_idx, cons_ptr);

    return record_count;
}

std::size_t FPGARxEngine::pollDecodedBatch(
                                           std::size_t max_count,
                                           FPGAEventDesc* out) {
    return pollDecodedBatchImpl(max_count, false, nullptr, out);
}

std::size_t FPGARxEngine::pollDecodedBatchSync(
                                               std::size_t max_count,
                                               bool get_snapshot,
                                               FpgaSyncSnapshot* snapshot,
                                               FPGAEventDesc* out) {
    return pollDecodedBatchImpl(max_count, get_snapshot, snapshot, out);
}

void FPGARxEngine::attachLatenyTracker(LatencyTracker* latency_tracker) {
    m_latency_tracker = latency_tracker;
}
