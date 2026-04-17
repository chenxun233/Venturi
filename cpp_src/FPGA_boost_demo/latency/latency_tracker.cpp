#include "latency_tracker.h"
#include "log_printer.h"

#include "../common/time_utils.h"

#include <cstdio>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {
constexpr std::size_t kStageCount =
    static_cast<std::size_t>(stage::BATCH_END) + 1U;
} // namespace

LatencyTracker::LatencyTracker(uint16_t producer_num,
                               std::size_t buffer_capacity,
                               std::size_t pending_capacity):
m_queue_num(producer_num),
m_pending_capacity(pending_capacity) {
    if (!_isPowerOfTwo(pending_capacity)) {
        throw std::invalid_argument("pending_capacity must be a non-zero power-of-two");
    }

    m_latency_queues.reserve(producer_num);
    for (uint16_t producer_idx = 0; producer_idx < producer_num; ++producer_idx) {
        m_latency_queues.push_back(std::make_unique<SpscRingQueue<TimeRecord>>(buffer_capacity));
    }

    m_pending_tables.resize(producer_num);
    m_latency_stats.resize(producer_num);
    for (uint16_t producer_idx = 0; producer_idx < producer_num; ++producer_idx) {
        PendingTable& table = m_pending_tables[producer_idx];
        table.slots.resize(m_pending_capacity);

        std::vector<LatencyStats>& queue_stats = m_latency_stats[producer_idx];
        queue_stats.resize(kStageCount * kStageCount);
        for (std::size_t prev_idx = 0; prev_idx < kStageCount; ++prev_idx) {
            for (std::size_t curr_idx = 0; curr_idx < kStageCount; ++curr_idx) {
                LatencyStats& stats = queue_stats[prev_idx * kStageCount + curr_idx];
                stats.que_idx = producer_idx;
                stats.prev_stage = static_cast<stage>(prev_idx);
                stats.curr_stage = static_cast<stage>(curr_idx);
                stats.min_ns = std::numeric_limits<uint64_t>::max();
            }
        }
    }
}

bool LatencyTracker::_isPowerOfTwo(std::size_t value) noexcept {
    return value != 0 && (value & (value - 1)) == 0;
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

std::size_t LatencyTracker::_drainFairPass() {
    if (m_queue_num == 0) {
        return 0;
    }

    std::size_t processed = 0;
    TimeRecord record {};
    for (uint16_t offset = 0; offset < m_queue_num; ++offset) {
        const uint16_t que_idx =
            static_cast<uint16_t>((m_next_queue_idx + offset) % m_queue_num);
        if (!m_latency_queues[que_idx]->pop(record)) {
            continue;
        }
        _processRecord(record);
        ++processed;
    }

    m_next_queue_idx = static_cast<uint16_t>((m_next_queue_idx + 1U) % m_queue_num);
    return processed;
}

std::size_t LatencyTracker::run() {
    std::size_t processed_count = 0;
    for (;;) {
        const std::size_t pass_count = _drainFairPass();
        if (pass_count == 0) {
            return processed_count;
        }
        processed_count += pass_count;
    }
}

bool LatencyTracker::_hasPendingRecord(uint16_t que_idx, uint64_t event_tag) const noexcept {
    if (que_idx >= m_pending_tables.size()) {
        return false;
    }

    const PendingTable& table = m_pending_tables[que_idx];
    for (const PendingSlot& slot : table.slots) {
        if (slot.occupied && slot.event_tag == event_tag) {
            return true;
        }
    }
    return false;
}

LatencyTracker::PendingSlot* LatencyTracker::_findPendingSlot(uint16_t que_idx,
                                                              uint64_t event_tag) noexcept {
    if (que_idx >= m_pending_tables.size()) {
        return nullptr;
    }

    PendingTable& table = m_pending_tables[que_idx];
    for (PendingSlot& slot : table.slots) {
        if (slot.occupied && slot.event_tag == event_tag) {
            return &slot;
        }
    }
    return nullptr;
}

LatencyTracker::PendingSlot& LatencyTracker::_upsertPendingSlot(uint16_t que_idx,
                                                                uint64_t event_tag,
                                                                uint32_t trace_id) {
    PendingTable& table = m_pending_tables[que_idx];
    if (PendingSlot* existing = _findPendingSlot(que_idx, event_tag)) {
        existing->trace_id = trace_id;
        return *existing;
    }

    for (PendingSlot& slot : table.slots) {
        if (slot.occupied) {
            continue;
        }
        slot.occupied = true;
        slot.event_tag = event_tag;
        slot.trace_id = trace_id;
        slot.insertion_sequence = table.next_insertion_sequence++;
        slot.state = PendingEventState {};
        ++table.live_count;
        return slot;
    }

    PendingSlot* oldest_slot = nullptr;
    uint64_t oldest_sequence = std::numeric_limits<uint64_t>::max();
    for (PendingSlot& candidate : table.slots) {
        if (!candidate.occupied) {
            continue;
        }
        if (candidate.insertion_sequence >= oldest_sequence) {
            continue;
        }
        oldest_sequence = candidate.insertion_sequence;
        oldest_slot = &candidate;
    }

    PendingSlot& slot = (oldest_slot != nullptr) ? *oldest_slot : table.slots.front();
    slot.occupied = true;
    slot.event_tag = event_tag;
    slot.trace_id = trace_id;
    slot.insertion_sequence = table.next_insertion_sequence++;
    slot.state = PendingEventState {};
    return slot;
}

void LatencyTracker::_erasePendingSlot(uint16_t que_idx, PendingSlot& slot) noexcept {
    if (que_idx >= m_pending_tables.size() || !slot.occupied) {
        return;
    }

    PendingTable& table = m_pending_tables[que_idx];
    slot = PendingSlot {};
    if (table.live_count > 0) {
        --table.live_count;
    }
}

void LatencyTracker::_processRecord(const TimeRecord& record) {
    if (record.event_stage == stage::FRAME_START) {
        _handleFrameStart(record);
        return;
    }

    PendingSlot* slot = _findPendingSlot(record.que_idx, record.event_tag);
    if (slot == nullptr) {
        _handleMissingPendingRecord(record);
        return;
    }

    switch (record.event_stage) {
        case stage::DMA_EMIT:
            _handleDmaEmit(record, *slot);
            return;
        case stage::BATCH_START:
            _handleBatchStart(record, *slot);
            return;
        case stage::BATCH_END:
            _handleBatchEnd(record, *slot);
            return;
        case stage::STRATEGY_START:
            _handleStrategyStart(record, *slot);
            return;
        case stage::TX_EXECUTION_ACCEPTED:
            _handleTxExecutionAccepted(record, *slot);
            return;
        case stage::EXECUTION_TAKEN:
            return;
        case stage::TX_ENQUEUE:
            _handleTxEnqueue(record, *slot);
            return;
        case stage::TX_SEND_ENTER:
            _handleTxSendEnter(record, *slot);
            return;
        case stage::TX_SEND_SYSCALL_ENTER:
            _handleTxSendSyscallEnter(record, *slot);
            return;
        case stage::TX_SEND:
            _handleTxSend(record, *slot);
            return;
        default:
            return;
    }
}

void LatencyTracker::_handleFrameStart(const TimeRecord& record) {
    PendingSlot& slot = _upsertPendingSlot(record.que_idx,
                                           record.event_tag,
                                           record.trace_id);
    slot.trace_id = record.trace_id;
    slot.state = PendingEventState {
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
        case stage::TX_ENQUEUE:
            _incrementDrop(record.que_idx, stage::TX_EXECUTION_ACCEPTED, stage::TX_ENQUEUE);
            return;
        case stage::TX_SEND_ENTER:
            _incrementDrop(record.que_idx, stage::TX_ENQUEUE, stage::TX_SEND_ENTER);
            return;
        case stage::TX_SEND_SYSCALL_ENTER:
            _incrementDrop(record.que_idx, stage::TX_SEND_ENTER, stage::TX_SEND_SYSCALL_ENTER);
            return;
        case stage::TX_SEND:
            _incrementDrop(record.que_idx, stage::TX_SEND_SYSCALL_ENTER, stage::TX_SEND);
            return;
        default:
            return;
    }
}

void LatencyTracker::_handleDmaEmit(const TimeRecord& record, PendingSlot& slot) {
    PendingEventState& state = slot.state;
    if (record.time_captured < state.frame_start_tick) {
        _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
        _erasePendingSlot(record.que_idx, slot);
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

void LatencyTracker::_handleBatchStart(const TimeRecord& record, PendingSlot& slot) {
    PendingEventState& state = slot.state;
    if (!state.has_dma_emit) {
        _incrementDrop(record.que_idx, stage::DMA_EMIT, stage::BATCH_START);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }
    if (state.has_batch_start) {
        _incrementDrop(record.que_idx, stage::BATCH_START, stage::BATCH_START);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }
    if (state.has_batch_end) {
        _incrementDrop(record.que_idx, stage::BATCH_END, stage::BATCH_START);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }

    state.batch_start_tick = record.time_captured;
    state.has_batch_start = true;
}

void LatencyTracker::_handleBatchEnd(const TimeRecord& record, PendingSlot& slot) {
    PendingEventState& state = slot.state;
    if (!state.has_batch_start) {
        _incrementDrop(record.que_idx, stage::BATCH_START, stage::BATCH_END);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }
    if (state.has_batch_end) {
        _incrementDrop(record.que_idx, stage::BATCH_END, stage::BATCH_END);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }
    if (record.time_captured < state.batch_start_tick) {
        _incrementDrop(record.que_idx, stage::BATCH_START, stage::BATCH_END);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }

    const int64_t batch_duration_ns =
        _readSignedHostDeltaNs(record.time_captured, state.batch_start_tick);
    if (batch_duration_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::BATCH_START,
            .curr_stage = stage::BATCH_END,
            .latency = static_cast<uint64_t>(batch_duration_ns)
        });
    }

    state.batch_end_tick = record.time_captured;
    state.batch_duration_ns = batch_duration_ns;
    state.has_batch_end = true;
}

void LatencyTracker::_handleStrategyStart(const TimeRecord& record, PendingSlot& slot) {
    PendingEventState& state = slot.state;
    if (!state.has_batch_end) {
        _incrementDrop(record.que_idx, stage::BATCH_END, stage::STRATEGY_START);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }
    if (record.time_captured < state.batch_end_tick) {
        _incrementDrop(record.que_idx, stage::BATCH_END, stage::STRATEGY_START);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }

    state.strategy_start_tick = record.time_captured;
    state.batch_end_to_strategy_start_ns =
        _readSignedHostDeltaNs(record.time_captured, state.batch_end_tick);
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

void LatencyTracker::_handleTxExecutionAccepted(const TimeRecord& record, PendingSlot& slot) {
    PendingEventState& state = slot.state;
    if (!state.has_strategy_start) {
        _incrementDrop(record.que_idx, stage::STRATEGY_START, stage::TX_EXECUTION_ACCEPTED);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }

    state.tx_execution_accepted_tick = record.time_captured;
    state.strategy_start_to_tx_execution_accepted_ns =
        _readSignedHostDeltaNs(record.time_captured, state.strategy_start_tick);
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

void LatencyTracker::_handleTxEnqueue(const TimeRecord& record, PendingSlot& slot) {
    PendingEventState& state = slot.state;
    if (!state.has_tx_execution_accepted) {
        _incrementDrop(record.que_idx, stage::TX_EXECUTION_ACCEPTED, stage::TX_ENQUEUE);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }

    state.tx_enqueue_tick = record.time_captured;
    state.tx_enqueue_backlog_depth = record.sender_backlog_depth;
    state.tx_execution_accepted_to_tx_enqueue_ns =
        _readSignedHostDeltaNs(record.time_captured, state.tx_execution_accepted_tick);
    if (state.tx_execution_accepted_to_tx_enqueue_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::TX_EXECUTION_ACCEPTED,
            .curr_stage = stage::TX_ENQUEUE,
            .latency = static_cast<uint64_t>(state.tx_execution_accepted_to_tx_enqueue_ns)
        });
    }

    state.has_tx_enqueue = true;
}

void LatencyTracker::_handleTxSendEnter(const TimeRecord& record, PendingSlot& slot) {
    PendingEventState& state = slot.state;
    if (!state.has_tx_enqueue) {
        _incrementDrop(record.que_idx, stage::TX_ENQUEUE, stage::TX_SEND_ENTER);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }

    state.tx_send_enter_tick = record.time_captured;
    state.tx_send_enter_backlog_depth = record.sender_backlog_depth;
    state.tx_enqueue_to_tx_send_enter_ns =
        _readSignedHostDeltaNs(record.time_captured, state.tx_enqueue_tick);
    if (state.tx_enqueue_to_tx_send_enter_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::TX_ENQUEUE,
            .curr_stage = stage::TX_SEND_ENTER,
            .latency = static_cast<uint64_t>(state.tx_enqueue_to_tx_send_enter_ns)
        });
    }

    state.has_tx_send_enter = true;
}

void LatencyTracker::_handleTxSendSyscallEnter(const TimeRecord& record, PendingSlot& slot) {
    PendingEventState& state = slot.state;
    if (!state.has_tx_send_enter) {
        _incrementDrop(record.que_idx, stage::TX_SEND_ENTER, stage::TX_SEND_SYSCALL_ENTER);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }

    state.tx_send_syscall_enter_tick = record.time_captured;
    state.tx_send_enter_to_tx_send_syscall_enter_ns =
        _readSignedHostDeltaNs(record.time_captured, state.tx_send_enter_tick);
    if (state.tx_send_enter_to_tx_send_syscall_enter_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::TX_SEND_ENTER,
            .curr_stage = stage::TX_SEND_SYSCALL_ENTER,
            .latency = static_cast<uint64_t>(state.tx_send_enter_to_tx_send_syscall_enter_ns)
        });
    }

    state.has_tx_send_syscall_enter = true;
}

void LatencyTracker::_handleTxSend(const TimeRecord& record, PendingSlot& slot) {
    PendingEventState& state = slot.state;
    if (!state.has_tx_send_syscall_enter) {
        _incrementDrop(record.que_idx, stage::TX_SEND_SYSCALL_ENTER, stage::TX_SEND);
        _erasePendingSlot(record.que_idx, slot);
        return;
    }

    state.tx_send_syscall_enter_to_tx_send_ns =
        _readSignedHostDeltaNs(record.time_captured, state.tx_send_syscall_enter_tick);
    state.tx_send_call_count = record.tx_send_call_count;
    state.tx_send_bytes_total = record.tx_send_bytes_total;
    state.tx_send_eintr_retry_count = record.tx_send_eintr_retry_count;
    state.tx_send_had_partial_write = record.tx_send_had_partial_write;
    if (state.tx_send_syscall_enter_to_tx_send_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::TX_SEND_SYSCALL_ENTER,
            .curr_stage = stage::TX_SEND,
            .latency = static_cast<uint64_t>(state.tx_send_syscall_enter_to_tx_send_ns)
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
            .tx_execution_accepted_to_tx_enqueue_ns = state.tx_execution_accepted_to_tx_enqueue_ns,
            .tx_enqueue_to_tx_send_enter_ns = state.tx_enqueue_to_tx_send_enter_ns,
            .tx_send_enter_to_tx_send_syscall_enter_ns = state.tx_send_enter_to_tx_send_syscall_enter_ns,
            .tx_send_syscall_enter_to_tx_send_ns = state.tx_send_syscall_enter_to_tx_send_ns,
            .tx_enqueue_backlog_depth = state.tx_enqueue_backlog_depth,
            .tx_send_enter_backlog_depth = state.tx_send_enter_backlog_depth,
            .tx_send_call_count = state.tx_send_call_count,
            .tx_send_bytes_total = state.tx_send_bytes_total,
            .tx_send_eintr_retry_count = state.tx_send_eintr_retry_count,
            .tx_send_had_partial_write = state.tx_send_had_partial_write
        });
    }
    _erasePendingSlot(record.que_idx, slot);
}

int64_t LatencyTracker::_readSignedDelta(uint64_t later_ns, uint64_t earlier_ns) {
    return (later_ns >= earlier_ns)
        ? static_cast<int64_t>(later_ns - earlier_ns)
        : -static_cast<int64_t>(earlier_ns - later_ns);
}

int64_t LatencyTracker::_readSignedHostDeltaNs(uint64_t later_tick, uint64_t earlier_tick) {
    const int64_t signed_tick_delta = _readSignedDelta(later_tick, earlier_tick);
    const HostTickScale& scale = readHostTickScale();
    if (scale.use_clock_fallback || scale.tsc_hz == 0) {
        return signed_tick_delta;
    }

    const uint64_t abs_tick_delta = (signed_tick_delta >= 0)
        ? static_cast<uint64_t>(signed_tick_delta)
        : static_cast<uint64_t>(-signed_tick_delta);
    const uint64_t delta_ns = static_cast<uint64_t>(
        (static_cast<__uint128_t>(abs_tick_delta) * 1000000000ULL) / scale.tsc_hz);
    return (signed_tick_delta >= 0)
        ? static_cast<int64_t>(delta_ns)
        : -static_cast<int64_t>(delta_ns);
}

void LatencyTracker::_updateStats(const StageLatency& latency) {
    LatencyStats& stats =
        _readStats(latency.que_idx, latency.prev_stage, latency.curr_stage);

    ++stats.sample_count;
    if (latency.latency < stats.min_ns) {
        stats.min_ns = latency.latency;
    }
    if (latency.latency > stats.max_ns) {
        stats.max_ns = latency.latency;
    }
}

void LatencyTracker::_incrementDrop(uint16_t que_idx, stage prev_stage, stage curr_stage) {
    ++_readStats(que_idx, prev_stage, curr_stage).drop_count;
}

LatencyStats& LatencyTracker::_readStats(uint16_t que_idx,
                                         stage prev_stage,
                                         stage curr_stage) {
    const std::size_t stage_index =
        static_cast<std::size_t>(prev_stage) * kStageCount +
        static_cast<std::size_t>(curr_stage);
    return m_latency_stats[que_idx][stage_index];
}
