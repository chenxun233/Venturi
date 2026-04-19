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

std::size_t FPGARxEngine::pollDecodedBatchImpl(std::size_t max_count,
                                               bool get_snapshot,
                                               bool emit_batch_start,
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
        out[record_count].trace_id = 0U;
        m_decoded_count.fetch_add(1ULL, std::memory_order_relaxed);
        if (out[record_count].is_first_event != 0U) {
            m_first_event_count.fetch_add(1ULL, std::memory_order_relaxed);
        }

        if (emit_batch_start && m_latency_tracker != nullptr) {
            out[record_count].trace_id = m_latency_tracker->tryAllocateTraceId(
                m_que_idx,
                out[record_count].is_first_event);

            if (out[record_count].trace_id != 0U) {
                const uint64_t decode_time_ns = readMonotonicRawNs();
                m_latency_tracker->pushRecord(TimeRecord {
                    .que_idx = m_que_idx,
                    .event_tag = out[record_count].event_tk,
                    .trace_id = out[record_count].trace_id,
                    .event_stage = stage::FRAME_START,
                    .time_captured = out[record_count].frame_start_tk,
                });
                m_latency_tracker->pushRecord(TimeRecord {
                    .que_idx = m_que_idx,
                    .event_tag = out[record_count].event_tk,
                    .trace_id = out[record_count].trace_id,
                    .event_stage = stage::DMA_EMIT,
                    .time_captured = out[record_count].event_tk,
                });
                m_latency_tracker->pushRecord(TimeRecord {
                    .que_idx = m_que_idx,
                    .event_tag = out[record_count].event_tk,
                    .trace_id = out[record_count].trace_id,
                    .event_stage = stage::BATCH_START,
                    .time_captured = decode_time_ns,
                });
            }
        }

        ++record_count;
        ++cons_ptr;
    }

    m_cons_ptr = cons_ptr;
    m_device._writeConsPtr(m_que_idx, cons_ptr);

    return record_count;
}

std::size_t FPGARxEngine::pollDecodedBatch(std::size_t max_count, FPGAEventDesc* out) {
    const std::size_t count = pollDecodedBatchImpl(max_count, false, true, nullptr, out);
    if (m_latency_tracker == nullptr || count == 0U) {
        return count;
    }

    for (std::size_t idx = 0; idx < count; ++idx) {
        if (out[idx].trace_id == 0U) {
            continue;
        }
        m_latency_tracker->pushRecord(TimeRecord {
            .que_idx = m_que_idx,
            .event_tag = out[idx].event_tk,
            .trace_id = out[idx].trace_id,
            .event_stage = stage::BATCH_END,
            .time_captured = readMonotonicRawNs(),
        });
    }

    return count;
}

std::size_t FPGARxEngine::pollDecodedBatchSync(std::size_t max_count,
                                               bool get_snapshot,
                                               FpgaSyncSnapshot* snapshot,
                                               FPGAEventDesc* out) {
    return pollDecodedBatchImpl(max_count, get_snapshot, false, snapshot, out);
}

void FPGARxEngine::attachLatenyTracker(LatencyTracker* latency_tracker) {
    m_latency_tracker = latency_tracker;
}

uint64_t FPGARxEngine::readDecodedCount() const noexcept {
    return m_decoded_count.load(std::memory_order_relaxed);
}

uint64_t FPGARxEngine::readFirstEventCount() const noexcept {
    return m_first_event_count.load(std::memory_order_relaxed);
}

uint16_t FPGARxEngine::readQueueIdx() const noexcept {
    return m_que_idx;
}
