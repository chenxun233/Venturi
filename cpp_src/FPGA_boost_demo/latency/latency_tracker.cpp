#include "latency_tracker.h"

#include "latency_analyzer.h"
#include "../common/time_utils.h"

#include <array>
#include <thread>
#include <utility>

namespace {

constexpr std::array<stage, 10> kRequiredStages = {{
    stage::FRAME_START,
    stage::DMA_EMIT,
    stage::BATCH_START,
    stage::BATCH_END,
    stage::STRATEGY_START,
    stage::TX_EXECUTION_ACCEPTED,
    stage::TX_ENQUEUE,
    stage::TX_SEND_ENTER,
    stage::TX_SEND_SYSCALL_ENTER,
    stage::TX_SEND,
}};

constexpr std::size_t kMaxCommandsPerQueuePerPass = 4U;

} // namespace

LatencyTracker::LatencyTracker(uint16_t producer_num,
                               std::size_t buffer_capacity,
                               std::size_t command_capacity)
    : m_trace_command_overflow_slots(producer_num),
      m_queue_num(producer_num),
      m_active_trace_ids(producer_num),
      m_started_trace_counts(producer_num),
      m_finalize_request_counts(producer_num),
      m_completed_trace_counts(producer_num),
      m_drop_request_counts(producer_num),
      m_missing_trace_record_counts(producer_num),
      m_stage_mismatch_drop_counts(producer_num) {
    m_latency_queues.reserve(producer_num);
    m_trace_command_queues.reserve(producer_num);
    for (uint16_t producer_idx = 0; producer_idx < producer_num; ++producer_idx) {
        m_latency_queues.push_back(
            std::make_unique<SpscRingQueue<TimeRecord>>(buffer_capacity));
        m_trace_command_queues.push_back(
            std::make_unique<SpscRingQueue<TraceCommand>>(command_capacity));
        m_trace_command_overflow_slots[producer_idx].store(0ULL, std::memory_order_relaxed);
        m_active_trace_ids[producer_idx].store(0U, std::memory_order_relaxed);
        m_started_trace_counts[producer_idx].store(0ULL, std::memory_order_relaxed);
        m_finalize_request_counts[producer_idx].store(0ULL, std::memory_order_relaxed);
        m_completed_trace_counts[producer_idx].store(0ULL, std::memory_order_relaxed);
        m_drop_request_counts[producer_idx].store(0ULL, std::memory_order_relaxed);
        m_missing_trace_record_counts[producer_idx].store(0ULL, std::memory_order_relaxed);
        m_stage_mismatch_drop_counts[producer_idx].store(0ULL, std::memory_order_relaxed);
    }
}

uint32_t LatencyTracker::tryAllocateTraceId(uint16_t que_idx, bool is_first_event) noexcept {
    if (!is_first_event || que_idx >= m_active_trace_ids.size()) {
        return 0U;
    }

    const uint32_t trace_id = _allocateTraceId();
    if (trace_id == 0U) {
        return 0U;
    }

    uint32_t expected_trace_id = 0U;
    if (!m_active_trace_ids[que_idx].compare_exchange_strong(expected_trace_id,
                                                             trace_id,
                                                             std::memory_order_acq_rel,
                                                             std::memory_order_acquire)) {
        return 0U;
    }

    m_started_trace_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
    return trace_id;
}

void LatencyTracker::attachAnalyzer(LatencyAnalyzer* latency_analyzer) noexcept {
    m_latency_analyzer = latency_analyzer;
}

void LatencyTracker::pushRecord(const TimeRecord& record) noexcept {
    if (record.que_idx >= m_latency_queues.size()) {
        return;
    }

    (void)m_latency_queues[record.que_idx]->push(record);
}

bool LatencyTracker::requestFinalize(uint16_t que_idx, uint32_t trace_id) noexcept {
    if (que_idx >= m_trace_command_queues.size() || trace_id == 0U) {
        return false;
    }
    m_finalize_request_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
    return _enqueueCommand(que_idx, que_idx, TraceCommandOp::Finalize, trace_id);
}

bool LatencyTracker::requestDrop(uint16_t que_idx, uint32_t trace_id) noexcept {
    return requestDrop(que_idx, que_idx, trace_id);
}

bool LatencyTracker::requestDrop(uint16_t command_queue_idx,
                                 uint16_t target_queue_idx,
                                 uint32_t trace_id) noexcept {
    if (command_queue_idx >= m_trace_command_queues.size() ||
        target_queue_idx >= m_latency_queues.size() ||
        trace_id == 0U) {
        return false;
    }
    m_drop_request_counts[target_queue_idx].fetch_add(1ULL, std::memory_order_relaxed);
    return _enqueueCommand(command_queue_idx, target_queue_idx, TraceCommandOp::Drop, trace_id);
}

void LatencyTracker::run() noexcept {
    while (!m_should_stop.load(std::memory_order_acquire)) {
        if (!_drainCommandPass()) {
            std::this_thread::yield();
        }
    }

    _drainAllCommands();
}

void LatencyTracker::stop() noexcept {
    m_should_stop.store(true, std::memory_order_release);
}

void LatencyTracker::printDebugSummary() const noexcept {
    std::printf("Latency Tracker Debug Summary\n");
    for (uint16_t que_idx = 0; que_idx < m_queue_num; ++que_idx) {
        std::printf("queue=%u started=%llu finalize_requested=%llu completed=%llu drop_requested=%llu missing_trace=%llu stage_mismatch=%llu active_trace=%u\n",
                    static_cast<unsigned int>(que_idx),
                    static_cast<unsigned long long>(
                        m_started_trace_counts[que_idx].load(std::memory_order_relaxed)),
                    static_cast<unsigned long long>(
                        m_finalize_request_counts[que_idx].load(std::memory_order_relaxed)),
                    static_cast<unsigned long long>(
                        m_completed_trace_counts[que_idx].load(std::memory_order_relaxed)),
                    static_cast<unsigned long long>(
                        m_drop_request_counts[que_idx].load(std::memory_order_relaxed)),
                    static_cast<unsigned long long>(
                        m_missing_trace_record_counts[que_idx].load(std::memory_order_relaxed)),
                    static_cast<unsigned long long>(
                        m_stage_mismatch_drop_counts[que_idx].load(std::memory_order_relaxed)),
                    static_cast<unsigned int>(
                        m_active_trace_ids[que_idx].load(std::memory_order_relaxed)));
    }
    std::fflush(stdout);
}

void LatencyTracker::finalizeTrace(uint16_t que_idx, uint32_t trace_id) noexcept {
    if (que_idx >= m_latency_queues.size() ||
        trace_id == 0U ||
        m_active_trace_ids[que_idx].load(std::memory_order_acquire) != trace_id) {
        return;
    }

    SpscRingQueue<TimeRecord>& queue = *m_latency_queues[que_idx];
    TimeRecord record {};
    bool found_matching_trace = false;
    while (queue.pop(record)) {
        if (record.trace_id != trace_id) {
            continue;
        }
        found_matching_trace = true;
        break;
    }

    if (!found_matching_trace) {
        m_missing_trace_record_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
        _clearActiveTrace(que_idx, trace_id);
        return;
    }

    LatencyLogRecord completed_record {.que_idx = que_idx};
    uint64_t frame_start_tick = 0U;
    uint64_t batch_start_tick = 0U;
    uint64_t batch_end_tick = 0U;
    uint64_t strategy_start_tick = 0U;
    uint64_t tx_execution_accepted_tick = 0U;
    uint64_t tx_enqueue_tick = 0U;
    uint64_t tx_send_enter_tick = 0U;
    uint64_t tx_send_syscall_enter_tick = 0U;

    for (std::size_t stage_idx = 0; stage_idx < kRequiredStages.size(); ++stage_idx) {
        if (record.trace_id != trace_id || record.event_stage != kRequiredStages[stage_idx]) {
            m_stage_mismatch_drop_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
            _dropQueueUntilEmpty(que_idx);
            _clearActiveTrace(que_idx, trace_id);
            return;
        }

        if (stage_idx == 0U) {
            completed_record.event_tag = record.event_tag;
        }

        switch (record.event_stage) {
            case stage::FRAME_START:
                frame_start_tick = record.time_captured;
                break;
            case stage::DMA_EMIT: {
                const int64_t delta_ns =
                    _readSignedDelta(record.time_captured, frame_start_tick);
                if (delta_ns < 0) {
                    m_stage_mismatch_drop_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
                    _dropQueueUntilEmpty(que_idx);
                    _clearActiveTrace(que_idx, trace_id);
                    return;
                }
                completed_record.frame_start_to_dma_emit_ns =
                    static_cast<uint64_t>(delta_ns);
                break;
            }
            case stage::BATCH_START:
                batch_start_tick = record.time_captured;
                break;
            case stage::BATCH_END: {
                batch_end_tick = record.time_captured;
                const int64_t delta_ns =
                    _readSignedHostDeltaNs(record.time_captured, batch_start_tick);
                if (delta_ns < 0) {
                    m_stage_mismatch_drop_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
                    _dropQueueUntilEmpty(que_idx);
                    _clearActiveTrace(que_idx, trace_id);
                    return;
                }
                completed_record.batch_duration_ns = delta_ns;
                break;
            }
            case stage::STRATEGY_START: {
                strategy_start_tick = record.time_captured;
                const int64_t delta_ns =
                    _readSignedHostDeltaNs(record.time_captured, batch_end_tick);
                if (delta_ns < 0) {
                    m_stage_mismatch_drop_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
                    _dropQueueUntilEmpty(que_idx);
                    _clearActiveTrace(que_idx, trace_id);
                    return;
                }
                completed_record.batch_end_to_strategy_start_ns = delta_ns;
                break;
            }
            case stage::TX_EXECUTION_ACCEPTED: {
                tx_execution_accepted_tick = record.time_captured;
                const int64_t delta_ns =
                    _readSignedHostDeltaNs(record.time_captured, strategy_start_tick);
                if (delta_ns < 0) {
                    m_stage_mismatch_drop_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
                    _dropQueueUntilEmpty(que_idx);
                    _clearActiveTrace(que_idx, trace_id);
                    return;
                }
                completed_record.strategy_start_to_tx_execution_accepted_ns = delta_ns;
                break;
            }
            case stage::TX_ENQUEUE: {
                tx_enqueue_tick = record.time_captured;
                const int64_t delta_ns =
                    _readSignedHostDeltaNs(record.time_captured, tx_execution_accepted_tick);
                if (delta_ns < 0) {
                    m_stage_mismatch_drop_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
                    _dropQueueUntilEmpty(que_idx);
                    _clearActiveTrace(que_idx, trace_id);
                    return;
                }
                completed_record.tx_execution_accepted_to_tx_enqueue_ns = delta_ns;
                break;
            }
            case stage::TX_SEND_ENTER: {
                tx_send_enter_tick = record.time_captured;
                const int64_t delta_ns =
                    _readSignedHostDeltaNs(record.time_captured, tx_enqueue_tick);
                if (delta_ns < 0) {
                    m_stage_mismatch_drop_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
                    _dropQueueUntilEmpty(que_idx);
                    _clearActiveTrace(que_idx, trace_id);
                    return;
                }
                completed_record.tx_enqueue_to_tx_send_enter_ns = delta_ns;
                break;
            }
            case stage::TX_SEND_SYSCALL_ENTER: {
                tx_send_syscall_enter_tick = record.time_captured;
                const int64_t delta_ns =
                    _readSignedHostDeltaNs(record.time_captured, tx_send_enter_tick);
                if (delta_ns < 0) {
                    m_stage_mismatch_drop_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
                    _dropQueueUntilEmpty(que_idx);
                    _clearActiveTrace(que_idx, trace_id);
                    return;
                }
                completed_record.tx_send_enter_to_tx_send_syscall_enter_ns = delta_ns;
                break;
            }
            case stage::TX_SEND: {
                const int64_t delta_ns =
                    _readSignedHostDeltaNs(record.time_captured, tx_send_syscall_enter_tick);
                if (delta_ns < 0) {
                    _dropQueueUntilEmpty(que_idx);
                    _clearActiveTrace(que_idx, trace_id);
                    return;
                }
                completed_record.tx_send_syscall_enter_to_tx_send_ns = delta_ns;
                break;
            }
            default:
                m_stage_mismatch_drop_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
                _dropQueueUntilEmpty(que_idx);
                _clearActiveTrace(que_idx, trace_id);
                return;
        }

        if (stage_idx + 1U == kRequiredStages.size()) {
            break;
        }

        if (!queue.pop(record)) {
            m_missing_trace_record_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
            _clearActiveTrace(que_idx, trace_id);
            return;
        }
    }

    if (m_latency_analyzer != nullptr) {
        m_latency_analyzer->pushCompletedRecord(completed_record);
    }
    m_completed_trace_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
    _clearActiveTrace(que_idx, trace_id);
}

void LatencyTracker::dropTrace(uint16_t que_idx, uint32_t trace_id) noexcept {
    if (que_idx >= m_latency_queues.size() ||
        trace_id == 0U ||
        m_active_trace_ids[que_idx].load(std::memory_order_acquire) != trace_id) {
        return;
    }

    _dropQueueUntilEmpty(que_idx);
    _clearActiveTrace(que_idx, trace_id);
}

bool LatencyTracker::_drainCommandPass() noexcept {
    if (m_trace_command_queues.empty()) {
        return false;
    }

    bool did_work = false;
    const uint16_t start_queue_idx =
        static_cast<uint16_t>(m_next_command_queue_idx % m_trace_command_queues.size());

    for (uint16_t queue_offset = 0; queue_offset < m_queue_num; ++queue_offset) {
        const uint16_t queue_idx =
            static_cast<uint16_t>((start_queue_idx + queue_offset) % m_queue_num);
        TraceCommand command {};
        std::size_t commands_processed = 0U;

        while (commands_processed < kMaxCommandsPerQueuePerPass &&
               m_trace_command_queues[queue_idx]->pop(command)) {
            did_work = true;
            _processCommand(command);
            ++commands_processed;
        }

        if (_tryConsumeOverflowCommand(queue_idx, command)) {
            did_work = true;
            _processCommand(command);
        }
    }

    m_next_command_queue_idx =
        static_cast<uint16_t>((start_queue_idx + 1U) % m_queue_num);
    return did_work;
}

void LatencyTracker::_drainAllCommands() noexcept {
    while (_drainCommandPass()) {
    }
}

bool LatencyTracker::_enqueueCommand(uint16_t command_queue_idx,
                                     uint16_t target_queue_idx,
                                     TraceCommandOp op,
                                     uint32_t trace_id) noexcept {
    TraceCommand command {
        .que_idx = target_queue_idx,
        .trace_id = trace_id,
        .op = op,
    };
    if (m_trace_command_queues[command_queue_idx]->push(command)) {
        return true;
    }

    return _tryStoreOverflowCommand(command_queue_idx, command);
}

bool LatencyTracker::_tryStoreOverflowCommand(uint16_t command_queue_idx,
                                              const TraceCommand& command) noexcept {
    if (command_queue_idx >= m_trace_command_overflow_slots.size()) {
        return false;
    }

    const uint64_t encoded_command = _encodeOverflowCommand(command);
    uint64_t expected_command = 0ULL;
    return m_trace_command_overflow_slots[command_queue_idx].compare_exchange_strong(
        expected_command,
        encoded_command,
        std::memory_order_acq_rel,
        std::memory_order_acquire);
}

bool LatencyTracker::_tryConsumeOverflowCommand(uint16_t command_queue_idx,
                                                TraceCommand& command) noexcept {
    if (command_queue_idx >= m_trace_command_overflow_slots.size()) {
        return false;
    }

    const uint64_t encoded_command =
        m_trace_command_overflow_slots[command_queue_idx].exchange(0ULL,
                                                                   std::memory_order_acq_rel);
    if (encoded_command == 0ULL) {
        return false;
    }

    command = _decodeOverflowCommand(encoded_command);
    return true;
}

void LatencyTracker::_processCommand(const TraceCommand& command) noexcept {
    switch (command.op) {
        case TraceCommandOp::Finalize:
            finalizeTrace(command.que_idx, command.trace_id);
            break;
        case TraceCommandOp::Drop:
            dropTrace(command.que_idx, command.trace_id);
            break;
    }
}

uint64_t LatencyTracker::_encodeOverflowCommand(const TraceCommand& command) noexcept {
    return (static_cast<uint64_t>(command.que_idx) << 48U) |
           (static_cast<uint64_t>(command.trace_id) << 16U) |
           static_cast<uint64_t>(static_cast<uint8_t>(command.op));
}

TraceCommand LatencyTracker::_decodeOverflowCommand(uint64_t encoded_command) noexcept {
    return TraceCommand {
        .que_idx = static_cast<uint16_t>((encoded_command >> 48U) & 0xFFFFU),
        .trace_id = static_cast<uint32_t>((encoded_command >> 16U) & 0xFFFFFFFFULL),
        .op = static_cast<TraceCommandOp>(encoded_command & 0xFFU),
    };
}

int64_t LatencyTracker::_readSignedDelta(uint64_t later_tick, uint64_t earlier_tick) noexcept {
    if (later_tick < earlier_tick) {
        return -1;
    }

    const uint64_t delta_tick = later_tick - earlier_tick;
    const uint64_t delta_ns = (delta_tick * 64ULL) / 10ULL;
    return static_cast<int64_t>(delta_ns);
}

int64_t LatencyTracker::_readSignedHostDeltaNs(uint64_t later_tick,
                                               uint64_t earlier_tick) noexcept {
    if (later_tick < earlier_tick) {
        return -1;
    }

    const uint64_t delta_tick = later_tick - earlier_tick;
    const HostTickScale& host_scale = readHostTickScale();
    if (host_scale.use_clock_fallback || host_scale.tsc_hz == 0U) {
        return static_cast<int64_t>(delta_tick);
    }

    return static_cast<int64_t>(
        (static_cast<__uint128_t>(delta_tick) * 1000000000ULL) / host_scale.tsc_hz);
}

uint32_t LatencyTracker::_allocateTraceId() noexcept {
    uint32_t current = m_next_trace_id.load(std::memory_order_relaxed);
    for (;;) {
        uint32_t next = current + 1U;
        if (next == 0U) {
            next = 1U;
        }

        if (m_next_trace_id.compare_exchange_weak(current,
                                                  next,
                                                  std::memory_order_relaxed,
                                                  std::memory_order_relaxed)) {
            return current;
        }
    }
}

void LatencyTracker::_clearActiveTrace(uint16_t que_idx, uint32_t trace_id) noexcept {
    if (que_idx >= m_active_trace_ids.size()) {
        return;
    }

    uint32_t expected_trace_id = trace_id;
    (void)m_active_trace_ids[que_idx].compare_exchange_strong(expected_trace_id,
                                                              0U,
                                                              std::memory_order_acq_rel,
                                                              std::memory_order_acquire);
}

void LatencyTracker::_dropQueueUntilEmpty(uint16_t que_idx) noexcept {
    if (que_idx >= m_latency_queues.size()) {
        return;
    }

    TimeRecord dropped_record {};
    while (m_latency_queues[que_idx]->pop(dropped_record)) {
    }
}
