#include "log_printer.h"

#include "../common/thread_affinity.h"

#include <cstdio>
#include <stdexcept>

namespace {

const char* readTxEventLabel(TxEventKind event) {
    switch (event) {
        case TxEventKind::ConnectionEstablished:
            return "ConnectionEstablished";
        case TxEventKind::ConnectionLost:
            return "ConnectionLost";
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

    m_tx_log_queues.reserve(queue_num);
    for (uint16_t idx = 0; idx < queue_num; ++idx) {
        m_tx_log_queues.push_back(
            std::make_unique<SpscRingQueue<TxLogRecord>>(capacity));
    }
}

LogPrinter::~LogPrinter() {
    stop();
}

void LogPrinter::setWorkerCpu(int cpu_id) {
    m_worker_cpu = cpu_id;
}

bool LogPrinter::pushTxLog(const TxLogRecord& record) {
    const uint16_t queue_idx = record.queue_idx;

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
    while (_drainTxLogRecord()) {
    }
}

void LogPrinter::_run() {
    if (m_worker_cpu >= 0) {
        pinCurrentThreadToCpu(m_worker_cpu);
    }

    while (m_running.load(std::memory_order_acquire)) {
        (void)_drainTxLogRecord();
    }
}

void LogPrinter::_printTxLogRecord(const TxLogRecord& record) {
    std::printf("TxEvent queue=%u event=%s\n",
                static_cast<unsigned int>(record.queue_idx),
                readTxEventLabel(record.event));
    std::fflush(stdout);
}
