#include "log_printer.h"

#include <cstdio>
#include <stdexcept>

namespace {

const char* readIntentActionName(OrderIntentAction action) {
    switch (action) {
        case OrderIntentAction::Buy:
            return "BUY";
        case OrderIntentAction::Sell:
            return "SELL";
        default:
            return "NONE";
    }
}

const char* readSymbolName(uint16_t stock_locate) {
    switch (stock_locate) {
        case 0x000d:
            return "AAPL";
        case 0x0ee8:
            return "HSBC";
        default:
            return "UNKNOWN";
    }
}

} // namespace

LogPrinter::LogPrinter(std::size_t capacity)
    : m_records(capacity),
      m_capacity_mask(capacity - 1) {
    if (capacity == 0 || (capacity & (capacity - 1)) != 0) {
        throw std::invalid_argument("LogPrinter capacity must be a non-zero power of two");
    }
}

LogPrinter::~LogPrinter() {
    stop();
}

bool LogPrinter::pushLatency(const LatencyLogRecord& record) {
    return _pushRecord(AsyncLogRecord {
        .kind = AsyncLogKind::Latency,
        .latency = record,
        .snapshot = {},
        .regression_status = {},
        .execution = {},
        .tx = {}
    });
}

bool LogPrinter::pushSnapshot(const FpgaSyncSnapshot& snapshot) {
    return _pushRecord(AsyncLogRecord {
        .kind = AsyncLogKind::Snapshot,
        .latency = {},
        .snapshot = snapshot,
        .regression_status = {},
        .execution = {},
        .tx = {}
    });
}

bool LogPrinter::pushRegressionStatus(const RegressionStatusLogRecord& record) {
    return _pushRecord(AsyncLogRecord {
        .kind = AsyncLogKind::RegressionStatus,
        .latency = {},
        .snapshot = {},
        .regression_status = record,
        .execution = {},
        .tx = {}
    });
}

bool LogPrinter::pushExecution(const ExecutionLogRecord& record) {
    return _pushRecord(AsyncLogRecord {
        .kind = AsyncLogKind::Execution,
        .latency = {},
        .snapshot = {},
        .regression_status = {},
        .execution = record,
        .tx = {}
    });
}

bool LogPrinter::pushTxEvent(const TxLogRecord& record) {
    return _pushRecord(AsyncLogRecord {
        .kind = AsyncLogKind::Tx,
        .latency = {},
        .snapshot = {},
        .regression_status = {},
        .execution = {},
        .tx = record
    });
}

bool LogPrinter::_pushRecord(const AsyncLogRecord& record) {
    const std::lock_guard<std::mutex> lock(m_push_mutex);
    const std::size_t tail = m_tail.load(std::memory_order_relaxed);
    const std::size_t head = m_head.load(std::memory_order_acquire);
    if (tail - head == m_records.size()) {
        m_drop_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    m_records[_wrapIndexP1(tail)] = record;
    m_tail.store(tail + 1, std::memory_order_release);
    m_record_cv.notify_one();
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
    const bool was_running = m_running.exchange(false, std::memory_order_acq_rel);
    m_record_cv.notify_all();
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (!was_running) {
        return;
    }

    AsyncLogRecord record {};
    while (m_head.load(std::memory_order_acquire) != m_tail.load(std::memory_order_acquire)) {
        const std::size_t head = m_head.load(std::memory_order_relaxed);
        record = m_records[_wrapIndexP1(head)];
        m_head.store(head + 1, std::memory_order_release);
        _handleRecord(record);
    }
}

uint64_t LogPrinter::readDropCount() const {
    return m_drop_count.load(std::memory_order_relaxed);
}

void LogPrinter::_run() {
    AsyncLogRecord record {};
    std::unique_lock<std::mutex> wait_lock(m_wait_mutex);
    while (true) {
        m_record_cv.wait(wait_lock, [this]() {
            return !m_running.load(std::memory_order_acquire) ||
                   m_head.load(std::memory_order_acquire) != m_tail.load(std::memory_order_acquire);
        });

        const std::size_t head = m_head.load(std::memory_order_relaxed);
        if (head == m_tail.load(std::memory_order_acquire)) {
            if (!m_running.load(std::memory_order_acquire)) {
                break;
            }
            continue;
        }

        record = m_records[_wrapIndexP1(head)];
        m_head.store(head + 1, std::memory_order_release);
        wait_lock.unlock();
        _handleRecord(record);
        wait_lock.lock();
    }
}

void LogPrinter::_handleRecord(const AsyncLogRecord& record) {
    if (record.kind == AsyncLogKind::Snapshot) {
        std::printf("SyncSnapshot fpga_tick=%llu host_time_ns=%llu interval_ns=%llu\n",
                    static_cast<unsigned long long>(record.snapshot.fpga_tick),
                    static_cast<unsigned long long>(record.snapshot.host_time_ns),
                    static_cast<unsigned long long>(record.snapshot.interval_ns));
        std::fflush(stdout);
        return;
    }
    if (record.kind == AsyncLogKind::RegressionStatus) {
        if (record.regression_status.has_para) {
            std::printf("Regression a=%.9f\n", record.regression_status.a_ns_per_tick);
            std::fflush(stdout);
        }
        return;
    }
    if (record.kind == AsyncLogKind::Execution) {
        std::printf("Execution action=%s symbol=%s stock_locate=0x%04x price=%u shares=%u\n",
                    readIntentActionName(record.execution.intent.action),
                    readSymbolName(record.execution.stock_locate),
                    record.execution.stock_locate,
                    record.execution.intent.price,
                    record.execution.intent.shares);
        std::fflush(stdout);
        return;
    }
    if (record.kind == AsyncLogKind::Tx) {
        const char* event_name = "UNKNOWN";
        switch (record.tx.event) {
            case TxEventKind::ConnectionEstablished:
                event_name = "CONNECTION_ESTABLISHED";
                break;
            case TxEventKind::ConnectionLost:
                event_name = "CONNECTION_LOST";
                break;
            case TxEventKind::OrderSent:
                event_name = "ORDER_SENT";
                break;
            case TxEventKind::OrderAccepted:
                event_name = "ORDER_ACCEPTED";
                break;
            case TxEventKind::OrderRejected:
                event_name = "ORDER_REJECTED";
                break;
            case TxEventKind::OrderFilled:
                event_name = "ORDER_FILLED";
                break;
            case TxEventKind::OrderDropped:
                event_name = "ORDER_DROPPED";
                break;
        }
        std::printf("TxEvent event=%s user_ref=%u symbol=%s stock_locate=0x%04x price=%u shares=%u reason=0x%04x match=%llu\n",
                    event_name,
                    record.tx.user_ref_num,
                    readSymbolName(record.tx.stock_locate),
                    record.tx.stock_locate,
                    record.tx.price,
                    record.tx.shares,
                    record.tx.reason,
                    static_cast<unsigned long long>(record.tx.match_number));
        std::fflush(stdout);
        return;
    }

    std::printf("LatencyNs%s queue=%u event_ts=%llu frame_start_to_dma_emit_ns=%llu dma_emit_to_decode_ns=%lld decode_to_strategy_ns=%lld strategy_to_executor_ns=%lld executor_to_tx_enqueue_ns=%lld tx_enqueue_to_tx_send_ns=%lld\n",
                (record.latency.dma_emit_to_decode_ns < 0) ? "[NEG]" : "",
                record.latency.que_idx,
                static_cast<unsigned long long>(record.latency.event_ts),
                static_cast<unsigned long long>(record.latency.frame_start_to_dma_emit_ns),
                static_cast<long long>(record.latency.dma_emit_to_decode_ns),
                static_cast<long long>(record.latency.decode_to_strategy_ns),
                static_cast<long long>(record.latency.strategy_to_executor_ns),
                static_cast<long long>(record.latency.executor_to_tx_enqueue_ns),
                static_cast<long long>(record.latency.tx_enqueue_to_tx_send_ns));
    std::fflush(stdout);
    return;
}

std::size_t LogPrinter::_wrapIndexP1(std::size_t idx) const {
    return idx & m_capacity_mask;
}
