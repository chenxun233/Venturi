#include "fpga_rx_engine.h"
#include <stdexcept>
#include <algorithm>

namespace {

void pushTraceRecords(TraceBuffer& trace_buffer,
                      const std::array<FPGAEventDesc, MAX_POLL_RECORDS>& event_buffer,
                      uint16_t que_idx,
                      uint32_t first_event_mask,
                      std::size_t count,
                      uint64_t decode_time_ns) {
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
        trace_buffer.push(record);

        record.event_stage = stage::DMA_EMIT;
        record.time_captured = event.event_tk;
        trace_buffer.push(record);

        record.event_stage = stage::DECODE;
        record.time_captured = decode_time_ns;
        trace_buffer.push(record);
    }
}

} // namespace

FPGARxEngine::FPGARxEngine(FPGADev& device, uint16_t que_idx)
    : m_decoder(device, que_idx),
      m_que_idx(que_idx) {
    if (!m_decoder.isValid()) {
        throw std::runtime_error("Failed to initialize FPGARxDecoder in FPGARxEngine");
    }
}

const std::array<FPGAEventDesc, MAX_POLL_RECORDS>& FPGARxEngine::readEventBuffer() const {
    return m_event_buffer;
}


std::size_t FPGARxEngine::pollBatch(std::size_t batch_size, bool get_time ) {
    FirstEventMask mask;
    const std::size_t count = m_decoder.decodeRawBatch(mask, m_event_buffer.data(), batch_size);

    if (get_time && mask.count > 0 && m_trace_buffer != nullptr) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &m_ts_captured);
        const uint64_t time_ns =
            static_cast<uint64_t>(m_ts_captured.tv_sec) * 1000000000ULL +
            static_cast<uint64_t>(m_ts_captured.tv_nsec);

        pushTraceRecords(*m_trace_buffer,
                         m_event_buffer,
                         m_que_idx,
                         mask.first_event_mask,
                         count,
                         time_ns);
    }
    return count;
}

std::size_t FPGARxEngine::pollBatchSync(std::size_t batch_size,
                                        const bool get_time,
                                        std::atomic<bool>& ready,
                                        FpgaSyncSnapshot& snapshot) {
    FirstEventMask mask;                                        
    const std::size_t count = m_decoder.decodeRawBatchSync(mask, m_event_buffer.data(),
                                                           snapshot,
                                                           get_time,
                                                           batch_size);
    if (get_time) {
        if (mask.count > 0 && m_trace_buffer != nullptr) {
            pushTraceRecords(*m_trace_buffer,
                             m_event_buffer,
                             m_que_idx,
                             mask.first_event_mask,
                             count,
                             snapshot.host_time_ns);
        }
        ready.store(true, std::memory_order_release);
    }
    return count;
}
