#include "latency_tracker.h"
#include "log_printer.h"

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

void LatencyTracker::attachLogPrinter(LogPrinter* log_printer) {
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

std::size_t LatencyTracker::run() {
    const std::lock_guard<std::mutex> lock(m_run_mutex);
    std::size_t processed_count = 0;
    TimeRecord record {};
    for (uint16_t offset = 0; offset < m_capacity; ++offset) {
        const uint16_t producer_idx = (m_next_buffer_idx + offset) & (m_capacity - 1);
        while (m_trace_buffer[producer_idx]->pop(record)) {
            _processRecord(record);
            ++processed_count;
        }
    }

    m_next_buffer_idx = (m_next_buffer_idx + 1) & (m_capacity - 1);
    return processed_count;
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

    switch (record.event_stage) {
        case stage::DMA_EMIT:
            _handleDmaEmit(record, it);
            return;
        case stage::DECODE:
            _handleDecode(record, it);
            return;
        case stage::STRATEGY:
            _handleStrategy(record, it);
            return;
        case stage::EXECUTOR:
            _handleExecutor(record, it);
            return;
        case stage::TX_ENQUEUE:
            _handleTxEnqueue(record, it);
            return;
        case stage::TX_SEND:
            _handleTxSend(record, it);
            return;
        default:
            return;
    }
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
    switch (record.event_stage) {
        case stage::DMA_EMIT:
            _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
            return;
        case stage::DECODE:
            _incrementDrop(record.que_idx, stage::DMA_EMIT, stage::DECODE);
            return;
        case stage::STRATEGY:
            _incrementDrop(record.que_idx, stage::DECODE, stage::STRATEGY);
            return;
        case stage::EXECUTOR:
            _incrementDrop(record.que_idx, stage::STRATEGY, stage::EXECUTOR);
            return;
        case stage::TX_ENQUEUE:
            _incrementDrop(record.que_idx, stage::EXECUTOR, stage::TX_ENQUEUE);
            return;
        case stage::TX_SEND:
            _incrementDrop(record.que_idx, stage::TX_ENQUEUE, stage::TX_SEND);
            return;
        default:
            return;
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

    state.decode_host_ns = record.time_captured;
    state.dma_emit_to_decode_ns = _readSignedDelta(record.time_captured, state.dma_emit_host_ns);

    if (state.dma_emit_to_decode_ns >= 0) {
        const StageLatency latency {
            .que_idx = record.que_idx,
            .event_ts = record.event_ts,
            .prev_stage = stage::DMA_EMIT,
            .curr_stage = stage::DECODE,
            .latency = static_cast<uint64_t>(state.dma_emit_to_decode_ns)
        };
        _updateStats(latency);
    }

    state.has_decode = true;
}

void LatencyTracker::_handleStrategy(
    const TimeRecord& record,
    std::unordered_map<EventKey, PendingEventState, EventKeyHash>::iterator it) {
    PendingEventState& state = it->second;
    if (!state.has_decode) {
        _incrementDrop(record.que_idx, stage::DECODE, stage::STRATEGY);
        m_pending_records.erase(it);
        return;
    }

    state.strategy_host_ns = record.time_captured;
    state.decode_to_strategy_ns = _readSignedDelta(record.time_captured, state.decode_host_ns);
    if (state.decode_to_strategy_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_ts = record.event_ts,
            .prev_stage = stage::DECODE,
            .curr_stage = stage::STRATEGY,
            .latency = static_cast<uint64_t>(state.decode_to_strategy_ns)
        });
    }

    state.has_strategy = true;
}

void LatencyTracker::_handleExecutor(
    const TimeRecord& record,
    std::unordered_map<EventKey, PendingEventState, EventKeyHash>::iterator it) {
    PendingEventState& state = it->second;
    if (!state.has_strategy) {
        _incrementDrop(record.que_idx, stage::STRATEGY, stage::EXECUTOR);
        m_pending_records.erase(it);
        return;
    }

    state.executor_host_ns = record.time_captured;
    state.strategy_to_executor_ns = _readSignedDelta(record.time_captured, state.strategy_host_ns);
    if (state.strategy_to_executor_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_ts = record.event_ts,
            .prev_stage = stage::STRATEGY,
            .curr_stage = stage::EXECUTOR,
            .latency = static_cast<uint64_t>(state.strategy_to_executor_ns)
        });
    }

    state.has_executor = true;
}

void LatencyTracker::_handleTxEnqueue(
    const TimeRecord& record,
    std::unordered_map<EventKey, PendingEventState, EventKeyHash>::iterator it) {
    PendingEventState& state = it->second;
    if (!state.has_executor) {
        _incrementDrop(record.que_idx, stage::EXECUTOR, stage::TX_ENQUEUE);
        m_pending_records.erase(it);
        return;
    }

    state.tx_enqueue_host_ns = record.time_captured;
    state.executor_to_tx_enqueue_ns =
        _readSignedDelta(record.time_captured, state.executor_host_ns);
    if (state.executor_to_tx_enqueue_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_ts = record.event_ts,
            .prev_stage = stage::EXECUTOR,
            .curr_stage = stage::TX_ENQUEUE,
            .latency = static_cast<uint64_t>(state.executor_to_tx_enqueue_ns)
        });
    }

    state.has_tx_enqueue = true;
}

void LatencyTracker::_handleTxSend(
    const TimeRecord& record,
    std::unordered_map<EventKey, PendingEventState, EventKeyHash>::iterator it) {
    PendingEventState& state = it->second;
    if (!state.has_tx_enqueue) {
        _incrementDrop(record.que_idx, stage::TX_ENQUEUE, stage::TX_SEND);
        m_pending_records.erase(it);
        return;
    }

    state.tx_enqueue_to_tx_send_ns = _readSignedDelta(record.time_captured, state.tx_enqueue_host_ns);
    if (state.tx_enqueue_to_tx_send_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_ts = record.event_ts,
            .prev_stage = stage::TX_ENQUEUE,
            .curr_stage = stage::TX_SEND,
            .latency = static_cast<uint64_t>(state.tx_enqueue_to_tx_send_ns)
        });
    }

    if (m_log_printer != nullptr) {
        m_log_printer->pushLatency(LatencyLogRecord {
            .que_idx = record.que_idx,
            .event_ts = record.event_ts,
            .frame_start_to_dma_emit_ns = state.frame_start_to_dma_emit_ns,
            .dma_emit_to_decode_ns = state.dma_emit_to_decode_ns,
            .decode_to_strategy_ns = state.decode_to_strategy_ns,
            .strategy_to_executor_ns = state.strategy_to_executor_ns,
            .executor_to_tx_enqueue_ns = state.executor_to_tx_enqueue_ns,
            .tx_enqueue_to_tx_send_ns = state.tx_enqueue_to_tx_send_ns
        });
    }
    m_pending_records.erase(it);
}

int64_t LatencyTracker::_readSignedDelta(uint64_t later_ns, uint64_t earlier_ns) {
    return (later_ns >= earlier_ns)
        ? static_cast<int64_t>(later_ns - earlier_ns)
        : -static_cast<int64_t>(earlier_ns - later_ns);
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
