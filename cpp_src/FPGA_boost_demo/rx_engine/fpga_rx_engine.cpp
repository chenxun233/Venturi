#include "fpga_rx_engine.h"

#include "../latency/latency_tracker.h"
#include "../sync/regression.h"

#include <ctime>
#include <stdexcept>

namespace {

void pushTraceRecords(LatencyTracker* latency_tracker,
                      const std::array<FPGAEventDesc, MAX_POLL_RECORDS>& event_buffer,
                      uint16_t que_idx,
                      uint32_t first_event_mask,
                      std::size_t count) {
    timespec ts {};
    for (std::size_t record_idx = 0; record_idx < count; ++record_idx) {
        if ((first_event_mask & (1u << record_idx)) == 0U) {
            continue;
        }

        const FPGAEventDesc& event = event_buffer[record_idx];
        TimeRecord record {
            .que_idx = que_idx,
            .event_ts = event.event_tk,
            .event_stage = stage::FRAME_START,
            .time_captured = event.frame_start_tk
        };
        latency_tracker->pushRecord(record);

        record.event_stage = stage::DMA_EMIT;
        record.time_captured = event.event_tk;
        latency_tracker->pushRecord(record);

        record.event_stage = stage::DECODE;
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        record.time_captured =
            static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL +
            static_cast<uint64_t>(ts.tv_nsec);
        latency_tracker->pushRecord(record);
    }
}

} // namespace

FPGARxEngine::FPGARxEngine(BasicRxSource& source,
                           const FPGARxDecoder& decoder,
                           uint16_t que_idx)
    : m_source(source),
      m_decoder(decoder),
      m_que_idx(que_idx) {
    if (!m_source.isValid()) {
        throw std::runtime_error("Failed to initialize FPGARxEngine with invalid source");
    }
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

    if (get_time && snapshot != nullptr && m_regression != nullptr) {
        m_regression->updateSnapshot(*snapshot);
        *snapshot = m_regression->readSnapshot();
    }

    if (mask.count > 0 && m_latency_tracker != nullptr) {
        pushTraceRecords(m_latency_tracker,
                         m_event_buffer,
                         m_que_idx,
                         mask.first_event_mask,
                         record_count);
    }

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
