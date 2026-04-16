#include "log_printer.h"

#include "../common/thread_affinity.h"

#include <cstdio>
#include <stdexcept>

namespace {

const char* readOrderActionLabel(OrderIntentAction action) {
    switch (action) {
        case OrderIntentAction::Buy:
            return "BUY";
        case OrderIntentAction::Sell:
            return "SELL";
        case OrderIntentAction::None:
        default:
            return "NONE";
    }
}

const char* readTxEventLabel(TxEventKind event) {
    switch (event) {
        case TxEventKind::ConnectionEstablished:
            return "ConnectionEstablished";
        case TxEventKind::ConnectionLost:
            return "ConnectionLost";
        case TxEventKind::OrderSent:
            return "OrderSent";
        case TxEventKind::OrderAccepted:
            return "OrderAccepted";
        case TxEventKind::OrderRejected:
            return "OrderRejected";
        case TxEventKind::OrderFilled:
            return "OrderFilled";
        case TxEventKind::OrderDropped:
            return "OrderDropped";
        default:
            return "Unknown";
    }
}

} // namespace

LogPrinter::LogPrinter(uint16_t queue_num, std::size_t capacity)
    : m_queue_num(queue_num) {
    if (queue_num == 0) {
        throw std::invalid_argument("LogPrinter queue_num must be non-zero");
    }
    if (capacity == 0 || (capacity & (capacity - 1)) != 0) {
        throw std::invalid_argument("LogPrinter capacity must be a non-zero power of two");
    }

    m_latency_log_queues.reserve(queue_num);
    m_execution_log_queues.reserve(queue_num);
    m_tx_log_queues.reserve(queue_num);
    for (uint16_t idx = 0; idx < queue_num; ++idx) {
        m_latency_log_queues.push_back(
            std::make_unique<SpscRingQueue<LatencyLogRecord>>(capacity));
        m_execution_log_queues.push_back(
            std::make_unique<SpscRingQueue<ExecutionLogRecord>>(capacity));
        m_tx_log_queues.push_back(
            std::make_unique<SpscRingQueue<TxLogRecord>>(capacity));
    }
}

LogPrinter::~LogPrinter() {
    stop();
}

bool LogPrinter::pushLatencyLog(const LatencyLogRecord& record) {
    return _pushLatencyLogRecord(record);
}

bool LogPrinter::pushExecutionLog(const ExecutionLogRecord& execution) {
    return _pushExecutionLogRecord(execution);
}

bool LogPrinter::pushTxLog(const TxLogRecord& record) {
    return _pushTxLogRecord(record);
}

void LogPrinter::setWorkerCpu(int cpu_id) {
    m_worker_cpu = cpu_id;
}

bool LogPrinter::_pushLatencyLogRecord(const LatencyLogRecord& record) {
    if (record.que_idx >= m_queue_num) {
        m_drop_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    const bool pushed = m_latency_log_queues[record.que_idx]->push(record);
    if (!pushed) {
        m_drop_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    return true;
}

bool LogPrinter::_pushExecutionLogRecord(const ExecutionLogRecord& execution) {
    const uint16_t queue_idx = 0;
    const bool pushed = m_execution_log_queues[queue_idx]->push(execution);
    if (!pushed) {
        m_drop_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    return true;
}

bool LogPrinter::_pushTxLogRecord(const TxLogRecord& record) {
    const uint16_t queue_idx = 0;
    const bool pushed = m_tx_log_queues[queue_idx]->push(record);
    if (!pushed) {
        m_drop_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    return true;
}

void LogPrinter::start() {
    bool expected = false;
    if (!m_running.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return;
    }

    m_thread = std::thread(&LogPrinter::_run, this);
}

void LogPrinter::stop() {
    m_running.store(false, std::memory_order_release);
    if (m_thread.joinable()) {
        m_thread.join();
    }

    _drainRemaining();
}

uint64_t LogPrinter::readDropCount() const {
    return m_drop_count.load(std::memory_order_relaxed);
}

bool LogPrinter::_drainLatencyRecord() {
    if (m_latency_log_queues.empty()) {
        return false;
    }

    LatencyLogRecord record {};
    const std::size_t queue_num = m_queue_num;
    for (std::size_t offset = 0; offset < queue_num; ++offset) {
        const std::size_t queue_idx =
            (static_cast<std::size_t>(m_next_latency_queue_idx) + offset) % queue_num;
        if (!m_latency_log_queues[queue_idx]->pop(record)) {
            continue;
        }

        m_next_latency_queue_idx = static_cast<uint16_t>((queue_idx + 1U) % queue_num);
        _printLatencyRecord(record);
        return true;
    }

    return false;
}

bool LogPrinter::_drainExecutionLogRecord() {
    if (m_execution_log_queues.empty()) {
        return false;
    }

    ExecutionLogRecord execution {};
    const std::size_t queue_num = m_queue_num;
    for (std::size_t offset = 0; offset < queue_num; ++offset) {
        const std::size_t queue_idx =
            (static_cast<std::size_t>(m_next_execution_log_queue_idx) + offset) % queue_num;
        if (!m_execution_log_queues[queue_idx]->pop(execution)) {
            continue;
        }

        m_next_execution_log_queue_idx = static_cast<uint16_t>((queue_idx + 1U) % queue_num);
        _printExecutionLogRecord(execution);
        return true;
    }

    return false;
}

bool LogPrinter::_drainTxLogRecord() {
    if (m_tx_log_queues.empty()) {
        return false;
    }

    TxLogRecord record {};
    const std::size_t queue_num = m_queue_num;
    for (std::size_t offset = 0; offset < queue_num; ++offset) {
        const std::size_t queue_idx =
            (static_cast<std::size_t>(m_next_tx_log_queue_idx) + offset) % queue_num;
        if (!m_tx_log_queues[queue_idx]->pop(record)) {
            continue;
        }

        m_next_tx_log_queue_idx = static_cast<uint16_t>((queue_idx + 1U) % queue_num);
        _printTxLogRecord(record);
        return true;
    }

    return false;
}

void LogPrinter::_drainRemaining() {
    while (_drainLatencyRecord() || _drainExecutionLogRecord() || _drainTxLogRecord()) {
    }
}

void LogPrinter::_run() {
    if (m_worker_cpu >= 0) {
        pinCurrentThreadToCpu(m_worker_cpu);
    }

    while (m_running.load(std::memory_order_acquire)) {
        bool drained = false;
        drained |= _drainLatencyRecord();
        drained |= _drainExecutionLogRecord();
        drained |= _drainTxLogRecord();
        (void)drained;
    }
}

void LogPrinter::_printLatencyRecord(const LatencyLogRecord& record) {
    constexpr int kLatencyLabelWidth = 48;
    const bool has_negative = (record.batch_duration_ns < 0) ||
                              (record.batch_end_to_strategy_start_ns < 0) ||
                              (record.strategy_start_to_tx_execution_accepted_ns < 0) ||
                              (record.tx_execution_accepted_to_tx_enqueue_ns < 0) ||
                              (record.tx_enqueue_to_tx_send_enter_ns < 0) ||
                              (record.tx_send_enter_to_tx_send_syscall_enter_ns < 0) ||
                              (record.tx_send_syscall_enter_to_tx_send_ns < 0);
    auto printSignedLatency = [kLatencyLabelWidth](const char* label, long long value) {
        std::printf("%-*s = %lld\n", kLatencyLabelWidth, label, value);
    };
    auto printUnsignedLatency = [kLatencyLabelWidth](const char* label,
                                                     unsigned long long value) {
        std::printf("%-*s = %llu\n", kLatencyLabelWidth, label, value);
    };
    std::printf("\nLatencyNs%s queue=%u event_tag=%llu\n",
                has_negative ? "[NEG]" : "",
                static_cast<unsigned int>(record.que_idx),
                static_cast<unsigned long long>(record.event_tag));
    printUnsignedLatency("frame_start -> dma_emit_ns",
                         static_cast<unsigned long long>(record.frame_start_to_dma_emit_ns));
    printSignedLatency("batch_duration_ns", static_cast<long long>(record.batch_duration_ns));
    printSignedLatency("batch_end -> strategy_start_ns",
                       static_cast<long long>(record.batch_end_to_strategy_start_ns));
    printSignedLatency("strategy_start -> tx_execution_accepted_ns",
                       static_cast<long long>(record.strategy_start_to_tx_execution_accepted_ns));
    printSignedLatency("tx_execution_accepted -> tx_enqueue_ns",
                       static_cast<long long>(record.tx_execution_accepted_to_tx_enqueue_ns));
    printSignedLatency("tx_enqueue -> tx_send_enter_ns",
                       static_cast<long long>(record.tx_enqueue_to_tx_send_enter_ns));
    printSignedLatency("tx_send_enter -> tx_send_syscall_enter_ns",
                       static_cast<long long>(record.tx_send_enter_to_tx_send_syscall_enter_ns));
    printSignedLatency("tx_send_syscall_enter -> tx_send_ns",
                       static_cast<long long>(record.tx_send_syscall_enter_to_tx_send_ns));
    printUnsignedLatency("tx_enqueue_backlog_depth",
                         static_cast<unsigned long long>(record.tx_enqueue_backlog_depth));
    printUnsignedLatency("tx_send_enter_backlog_depth",
                         static_cast<unsigned long long>(record.tx_send_enter_backlog_depth));
    printUnsignedLatency("tx_send_call_count",
                         static_cast<unsigned long long>(record.tx_send_call_count));
    printUnsignedLatency("tx_send_bytes_total",
                         static_cast<unsigned long long>(record.tx_send_bytes_total));
    printUnsignedLatency("tx_send_eintr_retry_count",
                         static_cast<unsigned long long>(record.tx_send_eintr_retry_count));
    printUnsignedLatency("tx_send_had_partial_write",
                         static_cast<unsigned long long>(record.tx_send_had_partial_write));
    std::fflush(stdout);
}

void LogPrinter::_printExecutionLogRecord(const ExecutionLogRecord& execution) {
    std::printf("Execution action=%s symbol=%s stock_locate=0x%04x price=%u shares=%u\n",
                readOrderActionLabel(execution.intent.action),
                "UNKNOWN",
                static_cast<unsigned int>(execution.stock_locate),
                static_cast<unsigned int>(execution.intent.price),
                static_cast<unsigned int>(execution.intent.shares));
    std::fflush(stdout);
}

void LogPrinter::_printTxLogRecord(const TxLogRecord& record) {
    std::printf("TxEvent event=%s user_ref=%u symbol=%s stock_locate=0x%04x price=%u shares=%u reason=0x%04x match=%llu\n",
                readTxEventLabel(record.event),
                static_cast<unsigned int>(record.user_ref_num),
                "UNKNOWN",
                static_cast<unsigned int>(record.stock_locate),
                static_cast<unsigned int>(record.price),
                static_cast<unsigned int>(record.shares),
                static_cast<unsigned int>(record.reason),
                static_cast<unsigned long long>(record.match_number));
    std::fflush(stdout);
}
