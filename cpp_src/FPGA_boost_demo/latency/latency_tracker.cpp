#include "latency_tracker.h"
#include "latency_log_printer.h"

#include "../sync/regression.h"

#include <cstdio>
#include <limits>
#include <stdexcept>

LatencyTracker::LatencyTracker(uint16_t producer_num, std::size_t buffer_capacity):
m_capacity(producer_num) {
    if (m_capacity & (m_capacity - 1)) {
        throw std::invalid_argument("Producer number must be a power of two");
    }
    m_trace_buffer.reserve(producer_num);
    for (uint16_t producer_idx = 0; producer_idx < producer_num; ++producer_idx) {
        m_trace_buffer.push_back(std::make_unique<TraceBuffer<TimeRecord>>(buffer_capacity));
    }
}

void LatencyTracker::attachRegression(Regression* regression) {
    m_regressions = regression;
}

void LatencyTracker::attachLogPrinter(LatencyLogPrinter* log_printer) {
    m_log_printer = log_printer;
}

bool LatencyTracker::pushRecord(const TimeRecord& record) {
    if (record.que_idx >= m_trace_buffer.size()) {
        throw std::out_of_range("LatencyTracker producer index out of range");
    }

    return m_trace_buffer[record.que_idx]->push(record);
}

// void LatencyTracker::attachTuner(Tuner* tuner) {
//     m_tuner = tuner;
// }

void LatencyTracker::run() {
    const std::lock_guard<std::mutex> lock(m_run_mutex);
    TimeRecord record {};
    for (uint16_t offset = 0; offset < m_capacity; ++offset) {
        const uint16_t producer_idx = (m_next_buffer_idx + offset) & (m_capacity - 1);
        while (m_trace_buffer[producer_idx]->pop(record)) {
            _processRecord(record);
        }
    }

    m_next_buffer_idx = (m_next_buffer_idx + 1) & (m_capacity - 1);
}

void LatencyTracker::_processRecord(const TimeRecord& record) {
    const EventKey event_key {
        .que_idx = record.que_idx,
        .event_ts = record.event_ts
    };

    if (record.event_stage == stage::FRAME_START) {
        _handleFrameStart(event_key, record);
        return;
    }

    const auto it = m_pending_records.find(event_key);
    if (it == m_pending_records.end()) {
        _handleMissingPendingRecord(record);
        return;
    }

    if (record.event_stage == stage::DMA_EMIT) {
        _handleDmaEmit(record, it);
        return;
    }

    if (record.event_stage != stage::DECODE) {
        return;
    }

    _handleDecode(record, it);
}

void LatencyTracker::_handleFrameStart(const EventKey& event_key, const TimeRecord& record) {
    uint64_t frame_start_host_ns = 0;
    if (m_regressions == nullptr) {
        _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
        m_pending_records.erase(event_key);
        return;
    }
    if (!m_regressions->convertFpgaToHostTime(record.time_captured, frame_start_host_ns)) {
        _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
        m_pending_records.erase(event_key);
        return;
    }

    m_pending_records[event_key] = PendingEventState {
        .frame_start_host_ns = frame_start_host_ns,
        .dma_emit_host_ns = 0,
        .frame_start_to_dma_emit_ns = 0,
        .has_dma_emit = false
    };
}

void LatencyTracker::_handleMissingPendingRecord(const TimeRecord& record) {
    if (record.event_stage == stage::DMA_EMIT) {
        _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
    } else if (record.event_stage == stage::DECODE) {
        _incrementDrop(record.que_idx, stage::DMA_EMIT, stage::DECODE);
    }
}

void LatencyTracker::_handleDmaEmit(
    const TimeRecord& record,
    std::unordered_map<EventKey, PendingEventState, EventKeyHash>::iterator it) {
    PendingEventState& state = it->second;
    uint64_t dma_emit_host_ns = 0;
    if (!m_regressions->convertFpgaToHostTime(record.time_captured, dma_emit_host_ns)) {
        _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
        m_pending_records.erase(it);
        return;
    }

    if (dma_emit_host_ns < state.frame_start_host_ns) {
        _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
        m_pending_records.erase(it);
        return;
    }

    const StageLatency latency {
        .que_idx = record.que_idx,
        .event_ts = record.event_ts,
        .prev_stage = stage::FRAME_START,
        .curr_stage = stage::DMA_EMIT,
        .latency = dma_emit_host_ns - state.frame_start_host_ns
    };
    _updateStats(latency);

    state.dma_emit_host_ns = dma_emit_host_ns;
    state.frame_start_to_dma_emit_ns = latency.latency;
    state.has_dma_emit = true;
}

void LatencyTracker::_handleDecode(
    const TimeRecord& record,
    std::unordered_map<EventKey, PendingEventState, EventKeyHash>::iterator it) {
    PendingEventState& state = it->second;
    if (!state.has_dma_emit) {
        _incrementDrop(record.que_idx, stage::DMA_EMIT, stage::DECODE);
        m_pending_records.erase(it);
        return;
    }

    const int64_t dma_emit_to_decode_ns =
        (record.time_captured >= state.dma_emit_host_ns)
            ? static_cast<int64_t>(record.time_captured - state.dma_emit_host_ns)
            : -static_cast<int64_t>(state.dma_emit_host_ns - record.time_captured);

    if (dma_emit_to_decode_ns >= 0) {
        const StageLatency latency {
            .que_idx = record.que_idx,
            .event_ts = record.event_ts,
            .prev_stage = stage::DMA_EMIT,
            .curr_stage = stage::DECODE,
            .latency = static_cast<uint64_t>(dma_emit_to_decode_ns)
        };
        _updateStats(latency);
    }

    if (m_log_printer != nullptr) {
        m_log_printer->pushLatency(LatencyLogRecord {
            .que_idx = record.que_idx,
            .event_ts = record.event_ts,
            .frame_start_to_dma_emit_ns = state.frame_start_to_dma_emit_ns,
            .dma_emit_to_decode_ns = dma_emit_to_decode_ns
        });
    }
    m_pending_records.erase(it);
}

void LatencyTracker::_updateStats(const StageLatency& latency) {
    LatencyStats& stats =
        _readOrCreateStats(latency.que_idx, latency.prev_stage, latency.curr_stage);

    ++stats.sample_count;
    if (latency.latency < stats.min_ns) {
        stats.min_ns = latency.latency;
    }
    if (latency.latency > stats.max_ns) {
        stats.max_ns = latency.latency;
    }
}

void LatencyTracker::_incrementDrop(uint16_t que_idx, stage prev_stage, stage curr_stage) {
    ++_readOrCreateStats(que_idx, prev_stage, curr_stage).drop_count;
}

LatencyStats& LatencyTracker::_readOrCreateStats(uint16_t que_idx, stage prev_stage, stage curr_stage) {
    const StageKey stage_key {
        .que_idx = que_idx,
        .prev_stage = prev_stage,
        .curr_stage = curr_stage
    };

    auto [it, inserted] = m_latency_stats.try_emplace(stage_key);
    LatencyStats& stats = it->second;
    if (inserted) {
        stats.que_idx = que_idx;
        stats.prev_stage = prev_stage;
        stats.curr_stage = curr_stage;
        stats.min_ns = std::numeric_limits<uint64_t>::max();
    }

    return stats;
}
