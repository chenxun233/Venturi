#include "latency_log_printer.h"

#include <chrono>
#include <cstdio>
#include <stdexcept>

LatencyLogPrinter::LatencyLogPrinter(std::size_t capacity)
    : m_records(capacity),
      m_capacity_mask(capacity - 1) {
    if (capacity == 0 || (capacity & (capacity - 1)) != 0) {
        throw std::invalid_argument("LatencyLogPrinter capacity must be a non-zero power of two");
    }
}

LatencyLogPrinter::~LatencyLogPrinter() {
    stop();
}

bool LatencyLogPrinter::pushLatency(const LatencyLogRecord& record) {
    return _pushRecord(AsyncLogRecord {
        .kind = AsyncLogKind::Latency,
        .latency = record,
        .snapshot = {}
    });
}

bool LatencyLogPrinter::pushSnapshot(const FpgaSyncSnapshot& snapshot) {
    return _pushRecord(AsyncLogRecord {
        .kind = AsyncLogKind::Snapshot,
        .latency = {},
        .snapshot = snapshot
    });
}

bool LatencyLogPrinter::_pushRecord(const AsyncLogRecord& record) {
    const std::lock_guard<std::mutex> lock(m_push_mutex);
    const std::size_t tail = m_tail.load(std::memory_order_relaxed);
    const std::size_t head = m_head.load(std::memory_order_acquire);
    if (tail - head == m_records.size()) {
        m_drop_count.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    m_records[_slotIndex(tail)] = record;
    m_tail.store(tail + 1, std::memory_order_release);
    return true;
}

void LatencyLogPrinter::start() {
    bool expected = false;
    if (!m_running.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return;
    }

    m_thread = std::thread(&LatencyLogPrinter::_run, this);
}

void LatencyLogPrinter::stop() {
    const bool was_running = m_running.exchange(false, std::memory_order_acq_rel);
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (!was_running) {
        return;
    }

    AsyncLogRecord record {};
    while (m_head.load(std::memory_order_acquire) != m_tail.load(std::memory_order_acquire)) {
        const std::size_t head = m_head.load(std::memory_order_relaxed);
        record = m_records[_slotIndex(head)];
        m_head.store(head + 1, std::memory_order_release);
        _printRecord(record);
    }
}

uint64_t LatencyLogPrinter::readDropCount() const {
    return m_drop_count.load(std::memory_order_relaxed);
}

void LatencyLogPrinter::_run() {
    AsyncLogRecord record {};
    while (m_running.load(std::memory_order_acquire)) {
        const std::size_t head = m_head.load(std::memory_order_relaxed);
        if (head == m_tail.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            continue;
        }

        record = m_records[_slotIndex(head)];
        m_head.store(head + 1, std::memory_order_release);
        _printRecord(record);
    }
}

void LatencyLogPrinter::_printRecord(const AsyncLogRecord& record) const {
    if (record.kind == AsyncLogKind::Snapshot) {
        std::printf("SyncSnapshot fpga_tick=%llu host_time_ns=%llu interval_ns=%llu\n",
                    static_cast<unsigned long long>(record.snapshot.fpga_tick),
                    static_cast<unsigned long long>(record.snapshot.host_time_ns),
                    static_cast<unsigned long long>(record.snapshot.interval_ns));
        std::fflush(stdout);
        return;
    }

    std::printf("LatencyNs queue=%u event_ts=%llu frame_start_to_dma_emit_ns=%llu dma_emit_to_decode_ns=%llu\n",
                record.latency.que_idx,
                static_cast<unsigned long long>(record.latency.event_ts),
                static_cast<unsigned long long>(record.latency.frame_start_to_dma_emit_ns),
                static_cast<unsigned long long>(record.latency.dma_emit_to_decode_ns));
    std::fflush(stdout);
}

std::size_t LatencyLogPrinter::_slotIndex(std::size_t idx) const {
    return idx & m_capacity_mask;
}
