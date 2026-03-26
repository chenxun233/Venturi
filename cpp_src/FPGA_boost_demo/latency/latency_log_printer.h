#pragma once

#include "../common/shared_types.h"

#include <atomic>
#include <cstddef>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

class FPGADev;

class LatencyLogPrinter {
public:
    explicit LatencyLogPrinter(std::size_t capacity = 1024);
    ~LatencyLogPrinter();

    void attachDebugDevice(FPGADev* device);
    bool pushLatency(const LatencyLogRecord& record);
    bool pushSnapshot(const FpgaSyncSnapshot& snapshot);
    bool pushQueuePoll(const QueuePollLogRecord& record);
    void start();
    void stop();
    uint64_t readDropCount() const;

private:
    void _run();
    bool _pushRecord(const AsyncLogRecord& record);
    void _handleRecord(const AsyncLogRecord& record);
    std::size_t _slotIndex(std::size_t idx) const;
    void _ensureQueuePollCapacity(std::size_t queue_idx);
    void _printDebugCounters();

    std::vector<AsyncLogRecord> m_records;
    std::vector<QueuePollLogRecord> m_latest_queue_polls;
    std::vector<bool> m_has_queue_poll;
    std::size_t m_capacity_mask {0};
    FPGADev* m_debug_device {nullptr};
    std::mutex m_push_mutex;
    std::mutex m_wait_mutex;
    std::condition_variable m_record_cv;
    alignas(64) std::atomic<std::size_t> m_head {0};
    alignas(64) std::atomic<std::size_t> m_tail {0};
    std::atomic<uint64_t> m_drop_count {0};
    std::atomic<bool> m_running {false};
    std::thread m_thread;
};
