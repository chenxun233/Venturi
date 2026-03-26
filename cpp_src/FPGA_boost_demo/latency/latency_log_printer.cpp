#include "latency_log_printer.h"

#include "../driver/fpga_dev.h"

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

void LatencyLogPrinter::attachDebugDevice(FPGADev* device) {
    m_debug_device = device;
}

bool LatencyLogPrinter::pushLatency(const LatencyLogRecord& record) {
    return _pushRecord(AsyncLogRecord {
        .kind = AsyncLogKind::Latency,
        .latency = record,
        .snapshot = {},
        .queue_poll = {}
    });
}

bool LatencyLogPrinter::pushSnapshot(const FpgaSyncSnapshot& snapshot) {
    return _pushRecord(AsyncLogRecord {
        .kind = AsyncLogKind::Snapshot,
        .latency = {},
        .snapshot = snapshot,
        .queue_poll = {}
    });
}

bool LatencyLogPrinter::pushQueuePoll(const QueuePollLogRecord& record) {
    return _pushRecord(AsyncLogRecord {
        .kind = AsyncLogKind::QueuePoll,
        .latency = {},
        .snapshot = {},
        .queue_poll = record
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
    m_record_cv.notify_one();
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
        record = m_records[_slotIndex(head)];
        m_head.store(head + 1, std::memory_order_release);
        _handleRecord(record);
    }
}

uint64_t LatencyLogPrinter::readDropCount() const {
    return m_drop_count.load(std::memory_order_relaxed);
}

void LatencyLogPrinter::_run() {
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

        record = m_records[_slotIndex(head)];
        m_head.store(head + 1, std::memory_order_release);
        wait_lock.unlock();
        _handleRecord(record);
        wait_lock.lock();
    }
}

void LatencyLogPrinter::_handleRecord(const AsyncLogRecord& record) {
    if (record.kind == AsyncLogKind::QueuePoll) {
        _ensureQueuePollCapacity(record.queue_poll.que_idx);
        m_latest_queue_polls[record.queue_poll.que_idx] = record.queue_poll;
        m_has_queue_poll[record.queue_poll.que_idx] = true;
        return;
    }

    if (record.kind == AsyncLogKind::Snapshot) {
        // std::printf("SyncSnapshot fpga_tick=%llu host_time_ns=%llu interval_ns=%llu\n",
        //             static_cast<unsigned long long>(record.snapshot.fpga_tick),
        //             static_cast<unsigned long long>(record.snapshot.host_time_ns),
        //             static_cast<unsigned long long>(record.snapshot.interval_ns));
        for (std::size_t queue_idx = 0; queue_idx < m_has_queue_poll.size(); ++queue_idx) {
            if (!m_has_queue_poll[queue_idx]) {
                continue;
            }
            const QueuePollLogRecord& queue_poll = m_latest_queue_polls[queue_idx];
            std::printf("que %u, record_count %llu, prod_ptr %llu, drop_count %llu\n",
                        queue_poll.que_idx,
                        static_cast<unsigned long long>(queue_poll.record_count),
                        static_cast<unsigned long long>(queue_poll.prod_ptr),
                        static_cast<unsigned long long>(queue_poll.drop_count));
        }
        _printDebugCounters();
        std::fflush(stdout);
        return;
    }

    // std::printf("LatencyNs queue=%u event_ts=%llu frame_start_to_dma_emit_ns=%llu dma_emit_to_decode_ns=%llu\n",
    //             record.latency.que_idx,
    //             static_cast<unsigned long long>(record.latency.event_ts),
    //             static_cast<unsigned long long>(record.latency.frame_start_to_dma_emit_ns),
    //             static_cast<unsigned long long>(record.latency.dma_emit_to_decode_ns));
    // std::fflush(stdout);
}

std::size_t LatencyLogPrinter::_slotIndex(std::size_t idx) const {
    return idx & m_capacity_mask;
}

void LatencyLogPrinter::_ensureQueuePollCapacity(std::size_t queue_idx) {
    if (queue_idx < m_latest_queue_polls.size()) {
        return;
    }
    m_latest_queue_polls.resize(queue_idx + 1);
    m_has_queue_poll.resize(queue_idx + 1, false);
}

void LatencyLogPrinter::_printDebugCounters() {
    if (m_debug_device == nullptr) {
        return;
    }

    uint64_t pcs_frame_count = 0;
    uint64_t frame_count = 0;
    uint64_t parser_msg_count = 0;
    std::vector<uint64_t> builder_event_counts;
    if (!m_debug_device->readRxDebugCounters(pcs_frame_count,
                                             frame_count,
                                             parser_msg_count,
                                             builder_event_counts)) {
        return;
    }

    std::printf("rx_dbg pcs_frame_count=%llu frame_count=%llu parser_msg_count=%llu",
                static_cast<unsigned long long>(pcs_frame_count),
                static_cast<unsigned long long>(frame_count),
                static_cast<unsigned long long>(parser_msg_count));
    for (std::size_t queue_idx = 0; queue_idx < builder_event_counts.size(); ++queue_idx) {
        std::printf(" builder_event_count[%zu]=%llu",
                    queue_idx,
                    static_cast<unsigned long long>(builder_event_counts[queue_idx]));
    }
    std::printf("\n");
}
