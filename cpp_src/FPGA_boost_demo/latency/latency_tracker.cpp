#include "latency_tracker.h"

#include "latency_analyzer.h"
#include "../common/time_utils.h"

#include <array>
#include <thread>
#include <utility>

namespace {

constexpr std::array<stage, 8> kRequiredStages = {{
    stage::FRAME_START,
    stage::DMA_EMIT,
    stage::BATCH_START,
    stage::BATCH_END,
    stage::STRATEGY_START,
    stage::TX_SENDER_EXECUTION_ACCEPTED,
    stage::TX_SEND_SYSCALL_ENTER,
    stage::TX_SEND,
}};

constexpr std::size_t kMaxCommandsPerQueuePerPass = 4U;

} // namespace

LatencyTracker::LatencyTracker(uint16_t producer_num,
                               std::size_t buffer_capacity,
                               std::size_t command_capacity)
    : m_queue_num(producer_num),
      m_next_trace_ids(producer_num, 1U),
      m_active_trace_ids(producer_num),
      m_completed_trace_counts(producer_num) {
    m_host_tick_scale = calibrateHostTickScale();
    if (m_host_tick_scale.use_clock_fallback || m_host_tick_scale.tsc_hz == 0U) {
        throw std::runtime_error("Failed to calibrate host tick scale");
    }

    m_latency_queues.reserve(producer_num);
    m_command_queues.reserve(producer_num);
    for (uint16_t producer_idx = 0; producer_idx < producer_num; ++producer_idx) {
        m_latency_queues.push_back(
            std::make_unique<SpscRingBuffer<TimeRecord>>(buffer_capacity));
        m_command_queues.push_back(
            std::make_unique<SpscRingBuffer<TraceCommand>>(command_capacity));
        m_active_trace_ids[producer_idx].store(0U, std::memory_order_relaxed);
        m_completed_trace_counts[producer_idx].store(0ULL, std::memory_order_relaxed);
    }
}

uint32_t LatencyTracker::tryAllocateTraceId(uint16_t que_idx, bool is_first_event) noexcept {
    if (!is_first_event || que_idx >= m_active_trace_ids.size()) {
        return 0U;
    }

    const uint32_t trace_id = m_next_trace_ids[que_idx];
    uint32_t next_trace_id = trace_id + 1U;
    if (next_trace_id == 0U) {
        next_trace_id = 1U;
    }

    uint32_t expected_trace_id = 0U;
    if (!m_active_trace_ids[que_idx].compare_exchange_strong(expected_trace_id, // if current active trace_id is 0, set to new trace_id
                                                             trace_id,          // if current active trace_id is not 0, return false, assign current active trace_id to expected_trace_id.
                                                             std::memory_order_acq_rel,
                                                             std::memory_order_acquire)) {
        return 0U;
    }

    m_next_trace_ids[que_idx] = next_trace_id;
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
    if (que_idx >= m_command_queues.size() || trace_id == 0U) {
        return false;
    }
    TraceCommand command {
        .que_idx = que_idx,
        .trace_id = trace_id,
        .op = TraceCommandOp::Finalize,
    };
    if (m_command_queues[que_idx]->push(command)) {
        return true;
    }

    return false;
}



bool LatencyTracker::requestDrop(uint16_t que_idx,
                                 uint32_t trace_id) noexcept {
    if (que_idx >= m_command_queues.size() ||
        trace_id == 0U) {
        return false;
    }
    TraceCommand command {
        .que_idx = que_idx,
        .trace_id = trace_id,
        .op = TraceCommandOp::Drop,
    };
    if (m_command_queues[que_idx]->push(command)) {
        return true;
    }
    return false;
}

void LatencyTracker::run() noexcept {
    while (!m_should_stop.load(std::memory_order_acquire)) {
        if (!_drainCommand()) {
            std::this_thread::yield();
        }
    }
    _drainCommand();
}

void LatencyTracker::stop() noexcept {
    m_should_stop.store(true, std::memory_order_release);
}

uint64_t LatencyTracker::readCompletedTraceCount(uint16_t que_idx) const noexcept {
    if (que_idx >= m_completed_trace_counts.size()) {
        return 0ULL;
    }
    return m_completed_trace_counts[que_idx].load(std::memory_order_relaxed);
}

void LatencyTracker::printDebugSummary() const noexcept {
    std::printf("Latency Tracker Debug Summary\n");
    for (uint16_t que_idx = 0; que_idx < m_queue_num; ++que_idx) {
        const uint64_t latency_record_drops =
            m_latency_queues[que_idx]->readDropCount();
        const uint64_t command_drops =
            m_command_queues[que_idx]->readDropCount();
        const uint32_t active_trace_id =
            m_active_trace_ids[que_idx].load(std::memory_order_relaxed);
        std::printf("queue=%u traced_packets=%llu latency_record_drops=%llu command_drops=%llu active_trace=%u\n",
                    static_cast<unsigned int>(que_idx),
                    static_cast<unsigned long long>(
                        m_completed_trace_counts[que_idx].load(std::memory_order_relaxed)),
                    static_cast<unsigned long long>(latency_record_drops),
                    static_cast<unsigned long long>(command_drops),
                    static_cast<unsigned int>(active_trace_id));
    }
    std::fflush(stdout);
}

void LatencyTracker::_finalizeTrace(uint16_t que_idx, uint32_t trace_id) noexcept {
    if (que_idx >= m_latency_queues.size() ||
        trace_id == 0U ||
        m_active_trace_ids[que_idx].load(std::memory_order_relaxed) != trace_id) {
        _cleanLatencyQueue(que_idx);
        _clearActiveTraceID(que_idx, trace_id);
        return;
    }

    SpscRingBuffer<TimeRecord>& queue = *m_latency_queues[que_idx];
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
        _clearActiveTraceID(que_idx, trace_id);
        return;
    }

    LatencyLogRecord completed_record {.que_idx = que_idx};
    uint64_t frame_start_tick = 0U;
    uint64_t batch_start_tick = 0U;
    uint64_t batch_end_tick = 0U;
    uint64_t strategy_start_tick = 0U;
    uint64_t tx_execution_accepted_tick = 0U;
    uint64_t tx_send_syscall_enter_tick = 0U;

    for (std::size_t stage_idx = 0; stage_idx < kRequiredStages.size(); ++stage_idx) {
        if (record.trace_id != trace_id || record.event_stage != kRequiredStages[stage_idx]) {
            _cleanLatencyQueue(que_idx);
            _clearActiveTraceID(que_idx, trace_id);
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
                    _FPGATick2ns(record.time_captured, frame_start_tick);
                if (delta_ns < 0) {
                    _cleanLatencyQueue(que_idx);
                    _clearActiveTraceID(que_idx, trace_id);
                    return;
                }
                completed_record.FRAME_START_to_DMA_EMIT =
                    static_cast<uint64_t>(delta_ns);
                break;
            }
            case stage::BATCH_START:
                batch_start_tick = record.time_captured;
                break;
            case stage::BATCH_END: {
                batch_end_tick = record.time_captured;
                const int64_t delta_ns =
                    _hostTick2ns(record.time_captured, batch_start_tick);
                if (delta_ns < 0) {
                    _cleanLatencyQueue(que_idx);
                    _clearActiveTraceID(que_idx, trace_id);
                    return;
                }
                completed_record.BATCH_DURATION = delta_ns;
                break;
            }
            case stage::STRATEGY_START: {
                strategy_start_tick = record.time_captured;
                const int64_t delta_ns =
                    _hostTick2ns(record.time_captured, batch_end_tick);
                if (delta_ns < 0) {
                    _cleanLatencyQueue(que_idx);
                    _clearActiveTraceID(que_idx, trace_id);
                    return;
                }
                completed_record.BATCH_END_to_STRATEGY_START = delta_ns;
                break;
            }
            case stage::TX_SENDER_EXECUTION_ACCEPTED: {
                tx_execution_accepted_tick = record.time_captured;
                const int64_t delta_ns =
                    _hostTick2ns(record.time_captured, strategy_start_tick);
                if (delta_ns < 0) {
                    _cleanLatencyQueue(que_idx);
                    _clearActiveTraceID(que_idx, trace_id);
                    return;
                }
                completed_record.STRATEGY_START_to_TX_SEND_ACCEPTED = delta_ns;
                break;
            }
            case stage::TX_SEND_SYSCALL_ENTER: {
                tx_send_syscall_enter_tick = record.time_captured;
                const int64_t delta_ns =
                    _hostTick2ns(record.time_captured, tx_execution_accepted_tick);
                if (delta_ns < 0) {
                    _cleanLatencyQueue(que_idx);
                    _clearActiveTraceID(que_idx, trace_id);
                    return;
                }
                completed_record.TX_SEND_ACCEPTED_to_TX_SEND_SYSCALL_ENTER = delta_ns;
                break;
            }
            case stage::TX_SEND: {
                const int64_t delta_ns =
                    _hostTick2ns(record.time_captured, tx_send_syscall_enter_tick);
                if (delta_ns < 0) {
                    _cleanLatencyQueue(que_idx);
                    _clearActiveTraceID(que_idx, trace_id);
                    return;
                }
                completed_record.TX_SEND_SYSCALL_ENTER_to_TX_SEND = delta_ns;
                break;
            }
            default:
                _cleanLatencyQueue(que_idx);
                _clearActiveTraceID(que_idx, trace_id);
                return;
        }

        if (stage_idx + 1U == kRequiredStages.size()) {
            break;
        }

        if (!queue.pop(record)) {
            _clearActiveTraceID(que_idx, trace_id);
            return;
        }
    }

    if (m_latency_analyzer != nullptr) {
        m_latency_analyzer->pushCompletedRecord(completed_record);
    }
    m_completed_trace_counts[que_idx].fetch_add(1ULL, std::memory_order_relaxed);
    _cleanLatencyQueue(que_idx);
    _clearActiveTraceID(que_idx, trace_id);
}

void LatencyTracker::_dropTrace(uint16_t que_idx, uint32_t trace_id) noexcept {
    if (que_idx >= m_latency_queues.size() ||
        trace_id == 0U ||
        m_active_trace_ids[que_idx].load(std::memory_order_acquire) != trace_id) {
        return;
    }
    _cleanLatencyQueue(que_idx);
    _clearActiveTraceID(que_idx, trace_id);
}

bool LatencyTracker::_drainCommand() noexcept {
    if (m_command_queues.empty()) {
        return false;
    }

    bool did_work = false;
    const uint16_t start_queue_idx =
        static_cast<uint16_t>(m_next_command_queue_idx % m_queue_num);

    for (uint16_t queue_offset = 0; queue_offset < m_queue_num; ++queue_offset) {
        const uint16_t que_idx =
            static_cast<uint16_t>((start_queue_idx + queue_offset) % m_queue_num);
        TraceCommand command {};
        std::size_t commands_processed = 0U;

        while (commands_processed < kMaxCommandsPerQueuePerPass &&
               m_command_queues[que_idx]->pop(command)) {
            did_work = true;
            _processCommand(command);
            ++commands_processed;
        }
    }
    m_next_command_queue_idx =
        static_cast<uint16_t>((start_queue_idx + 1U) % m_queue_num);
    return did_work;
}



void LatencyTracker::_processCommand(const TraceCommand& command) noexcept {
    switch (command.op) {
        case TraceCommandOp::Finalize:
            _finalizeTrace(command.que_idx, command.trace_id);
            break;
        case TraceCommandOp::Drop:
            _dropTrace(command.que_idx, command.trace_id);
            break;
    }
}

int64_t LatencyTracker::_FPGATick2ns(uint64_t later_tick, uint64_t earlier_tick) noexcept {
    if (later_tick < earlier_tick) {
        return -1;
    }

    const uint64_t delta_tick = later_tick - earlier_tick;
    const uint64_t delta_ns = (delta_tick * 64ULL) / 10ULL;
    return static_cast<int64_t>(delta_ns);
}

int64_t LatencyTracker::_hostTick2ns(uint64_t later_tick, uint64_t earlier_tick) noexcept {
    if (later_tick < earlier_tick) {
        return -1;
    }

    const uint64_t delta_tick = later_tick - earlier_tick;
    return static_cast<int64_t>(
        (static_cast<__uint128_t>(delta_tick) * 1000000000ULL) /
        m_host_tick_scale.tsc_hz);
}

void LatencyTracker::_clearActiveTraceID(uint16_t que_idx, uint32_t trace_id) noexcept {
    if (que_idx >= m_active_trace_ids.size()) {
        return;
    }

    uint32_t expected_trace_id = trace_id;
    (void)m_active_trace_ids[que_idx].compare_exchange_strong(expected_trace_id,
                                                              0U,
                                                              std::memory_order_acq_rel,
                                                              std::memory_order_acquire);
}

void LatencyTracker::_cleanLatencyQueue(uint16_t que_idx) noexcept {
    if (que_idx >= m_latency_queues.size()) {
        return;
    }

    TimeRecord dropped_record {};
    while (m_latency_queues[que_idx]->pop(dropped_record)) {
    }
}
