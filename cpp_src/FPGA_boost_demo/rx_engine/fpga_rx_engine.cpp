#include "fpga_rx_engine.h"
#include "../sync/regression.h"
#include <stdexcept>
#include <algorithm>
#include <cstdio>
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
    //     printf("=====TraceBuffer push: que_idx: %u, frame_start_tk: %lu, event_tk: %lu, time_captured: %lu\n",
    //            record.que_idx, event.frame_start_tk, event.event_tk, record.time_captured);
    }
}

} // namespace

FPGARxEngine::FPGARxEngine(FPGADev& device, uint16_t que_idx):
      m_decoder(device, que_idx),
      m_que_idx(que_idx) {
    if (!m_decoder.isValid()) {
        throw std::runtime_error("Failed to initialize FPGARxDecoder in FPGARxEngine");
    }
}

const std::array<FPGAEventDesc, MAX_POLL_RECORDS>& FPGARxEngine::readEventBuffer() const {
    return m_event_buffer;
}


std::size_t FPGARxEngine::pollBatch(std::size_t batch_size, bool get_time ) {
    (void)get_time;
    FirstEventMask mask;
    const std::size_t count = m_decoder.decodeRawBatch(mask, m_event_buffer.data(), batch_size);

    if (mask.count > 0 && m_latency_tracker != nullptr) {
        pushTraceRecords(m_latency_tracker,
                         m_event_buffer,
                         m_que_idx,
                         mask.first_event_mask,
                         count);
    }
    return count;
}

std::size_t FPGARxEngine::pollBatchSync(std::size_t batch_size,
                                        const bool get_time,
                                        FpgaSyncSnapshot& snapshot) {
    FirstEventMask mask;
    const std::size_t count = m_decoder.decodeRawBatchSync(mask,
                                                           m_event_buffer.data(),
                                                           snapshot,
                                                           get_time,
                                                           batch_size);
    if (get_time) {
        FpgaSyncSnapshot snapshot_in_use = snapshot;
        if (m_regression != nullptr) {
            m_regression->updateSnapshot(snapshot);
            snapshot_in_use = m_regression->readSnapshot();
        }
        if (m_log_printer != nullptr) {
            m_log_printer->pushSnapshot(snapshot_in_use);
        }
    }
    if (mask.count > 0 && m_latency_tracker != nullptr) {
        pushTraceRecords(m_latency_tracker,
                         m_event_buffer,
                         m_que_idx,
                         mask.first_event_mask,
                         count);
    }
    return count;
}
