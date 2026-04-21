#include "log_printer.h"

#include "../common/thread_affinity.h"

#include <algorithm>
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
        case TxEventKind::ConnectionIssue:
            return "ConnectionIssue";
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

int64_t readPercentileValue(std::vector<int64_t> samples,
                            std::size_t numerator,
                            std::size_t denominator) {
    if (samples.empty()) {
        return 0;
    }

    std::sort(samples.begin(), samples.end());
    const std::size_t last_idx = samples.size() - 1U;
    const std::size_t idx = (last_idx * numerator) / denominator;
    return samples[idx];
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
    m_latency_seen_counts.assign(queue_num, 0U);
    m_latency_summary_stats.resize(queue_num);
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

void LogPrinter::setLatencyWarmupRecords(uint64_t record_count) {
    m_latency_warmup_records = record_count;
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
    const uint16_t queue_idx = execution.queue_idx;
    if (queue_idx >= m_queue_num) {
        m_drop_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    const bool pushed = m_execution_log_queues[queue_idx]->push(execution);
    if (!pushed) {
        m_drop_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    return true;
}

bool LogPrinter::_pushTxLogRecord(const TxLogRecord& record) {
    const uint16_t queue_idx = record.queue_idx;
    if (queue_idx >= m_queue_num) {
        m_drop_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

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

    m_latency_summary_printed = false;
    m_thread = std::thread(&LogPrinter::_run, this);
}

void LogPrinter::stop() {
    m_running.store(false, std::memory_order_release);
    if (m_thread.joinable()) {
        m_thread.join();
    }

    _drainRemaining();
    const bool has_latency_records =
        std::any_of(m_latency_seen_counts.begin(),
                    m_latency_seen_counts.end(),
                    [](uint64_t seen_count) { return seen_count > 0U; });
    if (has_latency_records && !m_latency_summary_printed) {
        _printLatencySummary();
        m_latency_summary_printed = true;
    }
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
        _recordLatencySamples(record);
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

void LogPrinter::_recordLatencySamples(const LatencyLogRecord& record) {
    if (record.que_idx >= m_queue_num) {
        return;
    }

    uint64_t& seen_count = m_latency_seen_counts[record.que_idx];
    seen_count += 1U;
    if (seen_count <= m_latency_warmup_records) {
        return;
    }

    auto& stats = m_latency_summary_stats[record.que_idx];
    stats[static_cast<std::size_t>(LatencyField::FrameStartToDmaEmit)].samples.push_back(
        static_cast<int64_t>(record.FRAME_START_to_DMA_EMIT));
    stats[static_cast<std::size_t>(LatencyField::BatchDuration)].samples.push_back(
        record.BATCH_DURATION);
    stats[static_cast<std::size_t>(LatencyField::BatchEndToStrategyStart)].samples.push_back(
        record.BATCH_END_to_STRATEGY_START);
    stats[static_cast<std::size_t>(LatencyField::StrategyStartToTxExecutionAccepted)]
        .samples.push_back(record.STRATEGY_START_to_TX_SEND_ACCEPTED);
    stats[static_cast<std::size_t>(LatencyField::TxExecutionAcceptedToTxEnqueue)]
        .samples.push_back(record.TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE);
    stats[static_cast<std::size_t>(LatencyField::TxEnqueueToTxSendEnter)].samples.push_back(
        record.TX_SEND_ENQUEUE_to_TX_SEND_ENTER);
    stats[static_cast<std::size_t>(LatencyField::TxSendEnterToTxSendSyscallEnter)]
        .samples.push_back(record.TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER);
    stats[static_cast<std::size_t>(LatencyField::TxSendSyscallEnterToTxSend)]
        .samples.push_back(record.TX_SEND_SYSCALL_ENTER_to_TX_SEND);
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

void LogPrinter::_printLatencySummary() {
    for (uint16_t queue_idx = 0; queue_idx < m_queue_num; ++queue_idx) {
        _printLatencyQueueSummary(queue_idx);
    }
    std::fflush(stdout);
}

void LogPrinter::_printLatencyQueueSummary(uint16_t queue_idx) {
    constexpr int kLatencyLabelWidth = 48;
    static constexpr std::array<const char*, static_cast<std::size_t>(LatencyField::Count)>
        kLatencyLabels = {
            "FRAME_START_to_DMA_EMIT",
            "BATCH_DURATION",
            "BATCH_END_to_STRATEGY_START",
            "STRATEGY_START_to_TX_SEND_ACCEPTED",
            "TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE",
            "TX_SEND_ENQUEUE_to_TX_SEND_ENTER",
            "TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER",
            "TX_SEND_SYSCALL_ENTER_to_TX_SEND",
        };

    const auto& queue_stats = m_latency_summary_stats[queue_idx];
    const auto& reference_samples =
        queue_stats[static_cast<std::size_t>(LatencyField::FrameStartToDmaEmit)].samples;
    if (reference_samples.empty()) {
        std::printf("\nLatencySummary queue=%u insufficient post-warmup samples\n",
                    static_cast<unsigned int>(queue_idx));
        return;
    }

    std::printf("\nLatencySummary queue=%u\n", static_cast<unsigned int>(queue_idx));
    for (std::size_t field_idx = 0; field_idx < kLatencyLabels.size(); ++field_idx) {
        const auto& samples = queue_stats[field_idx].samples;
        const int64_t min_value = readPercentileValue(samples, 0U, 1U);
        const int64_t p50_value = readPercentileValue(samples, 50U, 100U);
        const int64_t p99_value = readPercentileValue(samples, 99U, 100U);
        const int64_t max_value = readPercentileValue(samples, 1U, 1U);
        std::printf("%-*s count=%zu min=%lld p50=%lld p99=%lld max=%lld\n",
                    kLatencyLabelWidth,
                    kLatencyLabels[field_idx],
                    samples.size(),
                    static_cast<long long>(min_value),
                    static_cast<long long>(p50_value),
                    static_cast<long long>(p99_value),
                    static_cast<long long>(max_value));
    }
}

void LogPrinter::_printLatencyRecord(const LatencyLogRecord& record) {
    constexpr int kLatencyLabelWidth = 48;
    const bool has_negative = (record.BATCH_DURATION < 0) ||
                              (record.BATCH_END_to_STRATEGY_START < 0) ||
                              (record.STRATEGY_START_to_TX_SEND_ACCEPTED < 0) ||
                              (record.TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE < 0) ||
                              (record.TX_SEND_ENQUEUE_to_TX_SEND_ENTER < 0) ||
                              (record.TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER < 0) ||
                              (record.TX_SEND_SYSCALL_ENTER_to_TX_SEND < 0);
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
                         static_cast<unsigned long long>(record.FRAME_START_to_DMA_EMIT));
    printSignedLatency("BATCH_DURATION", static_cast<long long>(record.BATCH_DURATION));
    printSignedLatency("batch_end -> strategy_start_ns",
                       static_cast<long long>(record.BATCH_END_to_STRATEGY_START));
    printSignedLatency("strategy_start -> tx_execution_accepted_ns",
                       static_cast<long long>(record.STRATEGY_START_to_TX_SEND_ACCEPTED));
    printSignedLatency("tx_execution_accepted -> tx_enqueue_ns",
                       static_cast<long long>(record.TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE));
    printSignedLatency("tx_enqueue -> tx_send_enter_ns",
                       static_cast<long long>(record.TX_SEND_ENQUEUE_to_TX_SEND_ENTER));
    printSignedLatency("tx_send_enter -> tx_send_syscall_enter_ns",
                       static_cast<long long>(record.TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER));
    printSignedLatency("tx_send_syscall_enter -> tx_send_ns",
                       static_cast<long long>(record.TX_SEND_SYSCALL_ENTER_to_TX_SEND));
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
