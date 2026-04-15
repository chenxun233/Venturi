#include "latency_tracker.h"
#include "log_printer.h"

#include <cstdio>
#include <limits>
#include <stdexcept>
#include <utility>

LatencyTracker::LatencyTracker(uint16_t producer_num, std::size_t buffer_capacity):
m_queue_num(producer_num) {
    m_latency_queues.reserve(producer_num);
    for (uint16_t producer_idx = 0; producer_idx < producer_num; ++producer_idx) {
        m_latency_queues.push_back(std::make_unique<SpscRingQueue<TimeRecord>>(buffer_capacity));
    }
}

void LatencyTracker::attachLogPrinter(LogPrinter* log_printer) {
    m_log_printer = log_printer;
}

void LatencyTracker::pushRecord(const TimeRecord& record) noexcept {
    if (record.que_idx >= m_latency_queues.size()) {
        return;
    }
    m_latency_queues[record.que_idx]->push(record);
}

std::size_t LatencyTracker::run() {
    std::size_t processed_count = 0;
    TimeRecord record {};
    for (uint16_t offset = 0; offset < m_queue_num; ++offset) {
        const uint16_t producer_idx = (m_next_queue_idx + offset) >= m_queue_num ? (m_next_queue_idx + offset - m_queue_num) : (m_next_queue_idx + offset);
        while (m_latency_queues[producer_idx]->pop(record)) {
            _processRecord(record);
            ++processed_count;
        }
    }

    m_next_queue_idx = (m_next_queue_idx + 1) >= m_queue_num ? (m_next_queue_idx + 1 - m_queue_num) : (m_next_queue_idx + 1);
    return processed_count;
}

void LatencyTracker::_processRecord(const TimeRecord& record) {
    const EventKey event_key {
        .que_idx = record.que_idx,
        .event_tag = record.event_tag
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
        case stage::BATCH_START:
            _handleBatchStart(record, it);
            return;
        case stage::BATCH_END:
            _handleBatchEnd(record, it);
            return;
        case stage::STRATEGY_START:
            _handleStrategyStart(record, it);
            return;
        case stage::TX_EXECUTION_ACCEPTED:
            _handleTxExecutionAccepted(record, it);
            return;
        case stage::EXECUTION_TAKEN:
            return;
        case stage::TX_EXECUTION_DEQUEUE:
            _handleTxExecutionDequeue(record, it);
            return;
        case stage::TX_ORDER_FRAME_BUILT:
            _handleTxOrderFrameBuilt(record, it);
            return;
        case stage::TX_PENDING_RECORDED:
            _handleTxPendingRecorded(record, it);
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
    m_pending_records[event_key] = PendingEventState {
        .frame_start_tick = record.time_captured,
    };
}

void LatencyTracker::_handleMissingPendingRecord(const TimeRecord& record) {
    switch (record.event_stage) {
        case stage::DMA_EMIT:
            _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
            return;
        case stage::BATCH_START:
            _incrementDrop(record.que_idx, stage::DMA_EMIT, stage::BATCH_START);
            return;
        case stage::BATCH_END:
            _incrementDrop(record.que_idx, stage::BATCH_START, stage::BATCH_END);
            return;
        case stage::STRATEGY_START:
            _incrementDrop(record.que_idx, stage::BATCH_END, stage::STRATEGY_START);
            return;
        case stage::TX_EXECUTION_ACCEPTED:
            _incrementDrop(record.que_idx, stage::STRATEGY_START, stage::TX_EXECUTION_ACCEPTED);
            return;
        case stage::TX_EXECUTION_DEQUEUE:
            _incrementDrop(record.que_idx, stage::TX_EXECUTION_ACCEPTED, stage::TX_EXECUTION_DEQUEUE);
            return;
        case stage::TX_ORDER_FRAME_BUILT:
            _incrementDrop(record.que_idx, stage::TX_EXECUTION_DEQUEUE, stage::TX_ORDER_FRAME_BUILT);
            return;
        case stage::TX_PENDING_RECORDED:
            _incrementDrop(record.que_idx, stage::TX_ORDER_FRAME_BUILT, stage::TX_PENDING_RECORDED);
            return;
        case stage::TX_ENQUEUE:
            _incrementDrop(record.que_idx, stage::TX_PENDING_RECORDED, stage::TX_ENQUEUE);
            return;
        case stage::TX_SEND:
            _incrementDrop(record.que_idx, stage::TX_ENQUEUE, stage::TX_SEND);
            return;
        default:
            return;
    }
}

void LatencyTracker::_handleDmaEmit(const TimeRecord& record, PendingIterator it) {
    PendingEventState& state = it->second;
    if (record.time_captured < state.frame_start_tick) {
        _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
        m_pending_records.erase(it);
        return;
    }

    const uint64_t frame_start_to_dma_emit_ns =
        ((record.time_captured - state.frame_start_tick) * 64ULL) / 10ULL;
    const StageLatency latency {
        .que_idx = record.que_idx,
        .event_tag = record.event_tag,
        .prev_stage = stage::FRAME_START,
        .curr_stage = stage::DMA_EMIT,
        .latency = frame_start_to_dma_emit_ns
    };
    _updateStats(latency);

    state.dma_emit_tick = record.time_captured;
    state.frame_start_to_dma_emit_ns = frame_start_to_dma_emit_ns;
    state.has_dma_emit = true;
}

void LatencyTracker::_handleBatchStart(const TimeRecord& record, PendingIterator it) {
    PendingEventState& state = it->second;
    if (!state.has_dma_emit) {
        _incrementDrop(record.que_idx, stage::DMA_EMIT, stage::BATCH_START);
        m_pending_records.erase(it);
        return;
    }
    if (state.has_batch_start) {
        _incrementDrop(record.que_idx, stage::BATCH_START, stage::BATCH_START);
        m_pending_records.erase(it);
        return;
    }
    if (state.has_batch_end) {
        _incrementDrop(record.que_idx, stage::BATCH_END, stage::BATCH_START);
        m_pending_records.erase(it);
        return;
    }

    state.batch_start_ns = record.time_captured;
    state.has_batch_start = true;
}

void LatencyTracker::_handleBatchEnd(const TimeRecord& record, PendingIterator it) {
    PendingEventState& state = it->second;
    if (!state.has_batch_start) {
        _incrementDrop(record.que_idx, stage::BATCH_START, stage::BATCH_END);
        m_pending_records.erase(it);
        return;
    }
    if (state.has_batch_end) {
        _incrementDrop(record.que_idx, stage::BATCH_END, stage::BATCH_END);
        m_pending_records.erase(it);
        return;
    }
    if (record.time_captured < state.batch_start_ns) {
        _incrementDrop(record.que_idx, stage::BATCH_START, stage::BATCH_END);
        m_pending_records.erase(it);
        return;
    }

    const int64_t batch_duration_ns =
        _readSignedDelta(record.time_captured, state.batch_start_ns);
    if (batch_duration_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::BATCH_START,
            .curr_stage = stage::BATCH_END,
            .latency = static_cast<uint64_t>(batch_duration_ns)
        });
    }

    state.batch_end_ns = record.time_captured;
    state.batch_duration_ns = batch_duration_ns;
    state.has_batch_end = true;
}

void LatencyTracker::_handleStrategyStart(const TimeRecord& record, PendingIterator it) {
    PendingEventState& state = it->second;
    if (!state.has_batch_end) {
        _incrementDrop(record.que_idx, stage::BATCH_END, stage::STRATEGY_START);
        m_pending_records.erase(it);
        return;
    }
    if (record.time_captured < state.batch_end_ns) {
        _incrementDrop(record.que_idx, stage::BATCH_END, stage::STRATEGY_START);
        m_pending_records.erase(it);
        return;
    }

    state.strategy_start_ns = record.time_captured;
    state.batch_end_to_strategy_start_ns =
        _readSignedDelta(record.time_captured, state.batch_end_ns);
    if (state.batch_end_to_strategy_start_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::BATCH_END,
            .curr_stage = stage::STRATEGY_START,
            .latency = static_cast<uint64_t>(state.batch_end_to_strategy_start_ns)
        });
    }

    state.has_strategy_start = true;
}

void LatencyTracker::_handleTxExecutionAccepted(const TimeRecord& record, PendingIterator it) {
    PendingEventState& state = it->second;
    if (!state.has_strategy_start) {
        _incrementDrop(record.que_idx, stage::STRATEGY_START, stage::TX_EXECUTION_ACCEPTED);
        m_pending_records.erase(it);
        return;
    }

    state.tx_execution_accepted_ns = record.time_captured;
    state.strategy_start_to_tx_execution_accepted_ns =
        _readSignedDelta(record.time_captured, state.strategy_start_ns);
    if (state.strategy_start_to_tx_execution_accepted_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::STRATEGY_START,
            .curr_stage = stage::TX_EXECUTION_ACCEPTED,
            .latency = static_cast<uint64_t>(state.strategy_start_to_tx_execution_accepted_ns)
        });
    }

    state.has_tx_execution_accepted = true;
}

void LatencyTracker::_handleTxExecutionDequeue(const TimeRecord& record, PendingIterator it) {
    PendingEventState& state = it->second;
    if (!state.has_tx_execution_accepted) {
        _incrementDrop(record.que_idx, stage::TX_EXECUTION_ACCEPTED, stage::TX_EXECUTION_DEQUEUE);
        m_pending_records.erase(it);
        return;
    }

    state.tx_execution_dequeue_ns = record.time_captured;
    state.tx_execution_accepted_to_tx_execution_dequeue_ns =
        _readSignedDelta(record.time_captured, state.tx_execution_accepted_ns);
    if (state.tx_execution_accepted_to_tx_execution_dequeue_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::TX_EXECUTION_ACCEPTED,
            .curr_stage = stage::TX_EXECUTION_DEQUEUE,
            .latency = static_cast<uint64_t>(state.tx_execution_accepted_to_tx_execution_dequeue_ns)
        });
    }

    state.has_tx_execution_dequeue = true;
}

void LatencyTracker::_handleTxOrderFrameBuilt(const TimeRecord& record, PendingIterator it) {
    PendingEventState& state = it->second;
    if (!state.has_tx_execution_dequeue) {
        _incrementDrop(record.que_idx, stage::TX_EXECUTION_DEQUEUE, stage::TX_ORDER_FRAME_BUILT);
        m_pending_records.erase(it);
        return;
    }

    state.tx_order_frame_built_ns = record.time_captured;
    state.tx_execution_dequeue_to_tx_order_frame_built_ns =
        _readSignedDelta(record.time_captured, state.tx_execution_dequeue_ns);
    if (state.tx_execution_dequeue_to_tx_order_frame_built_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::TX_EXECUTION_DEQUEUE,
            .curr_stage = stage::TX_ORDER_FRAME_BUILT,
            .latency = static_cast<uint64_t>(state.tx_execution_dequeue_to_tx_order_frame_built_ns)
        });
    }

    state.has_tx_order_frame_built = true;
}

void LatencyTracker::_handleTxPendingRecorded(const TimeRecord& record, PendingIterator it) {
    PendingEventState& state = it->second;
    if (!state.has_tx_order_frame_built) {
        _incrementDrop(record.que_idx, stage::TX_ORDER_FRAME_BUILT, stage::TX_PENDING_RECORDED);
        m_pending_records.erase(it);
        return;
    }

    state.tx_pending_recorded_ns = record.time_captured;
    state.tx_order_frame_built_to_tx_pending_recorded_ns =
        _readSignedDelta(record.time_captured, state.tx_order_frame_built_ns);
    if (state.tx_order_frame_built_to_tx_pending_recorded_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::TX_ORDER_FRAME_BUILT,
            .curr_stage = stage::TX_PENDING_RECORDED,
            .latency = static_cast<uint64_t>(state.tx_order_frame_built_to_tx_pending_recorded_ns)
        });
    }

    state.has_tx_pending_recorded = true;
}

void LatencyTracker::_handleTxEnqueue(const TimeRecord& record, PendingIterator it) {
    PendingEventState& state = it->second;
    if (!state.has_tx_pending_recorded) {
        _incrementDrop(record.que_idx, stage::TX_PENDING_RECORDED, stage::TX_ENQUEUE);
        m_pending_records.erase(it);
        return;
    }

    state.tx_enqueue_ns = record.time_captured;
    state.tx_pending_recorded_to_tx_enqueue_ns =
        _readSignedDelta(record.time_captured, state.tx_pending_recorded_ns);
    if (state.tx_pending_recorded_to_tx_enqueue_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::TX_PENDING_RECORDED,
            .curr_stage = stage::TX_ENQUEUE,
            .latency = static_cast<uint64_t>(state.tx_pending_recorded_to_tx_enqueue_ns)
        });
    }

    state.has_tx_enqueue = true;
}

void LatencyTracker::_handleTxSend(const TimeRecord& record, PendingIterator it) {
    PendingEventState& state = it->second;
    if (!state.has_tx_enqueue) {
        _incrementDrop(record.que_idx, stage::TX_ENQUEUE, stage::TX_SEND);
        m_pending_records.erase(it);
        return;
    }

    state.tx_enqueue_to_tx_send_ns = _readSignedDelta(record.time_captured, state.tx_enqueue_ns);
    if (state.tx_enqueue_to_tx_send_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::TX_ENQUEUE,
            .curr_stage = stage::TX_SEND,
            .latency = static_cast<uint64_t>(state.tx_enqueue_to_tx_send_ns)
        });
    }

    if (record.event_tag != 0 && m_log_printer != nullptr) {
        (void)m_log_printer->pushLatencyLog(LatencyLogRecord {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .frame_start_to_dma_emit_ns = state.frame_start_to_dma_emit_ns,
            .batch_duration_ns = state.batch_duration_ns,
            .batch_end_to_strategy_start_ns = state.batch_end_to_strategy_start_ns,
            .strategy_start_to_tx_execution_accepted_ns = state.strategy_start_to_tx_execution_accepted_ns,
            .tx_execution_accepted_to_tx_execution_dequeue_ns = state.tx_execution_accepted_to_tx_execution_dequeue_ns,
            .tx_execution_dequeue_to_tx_order_frame_built_ns = state.tx_execution_dequeue_to_tx_order_frame_built_ns,
            .tx_order_frame_built_to_tx_pending_recorded_ns = state.tx_order_frame_built_to_tx_pending_recorded_ns,
            .tx_pending_recorded_to_tx_enqueue_ns = state.tx_pending_recorded_to_tx_enqueue_ns,
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
