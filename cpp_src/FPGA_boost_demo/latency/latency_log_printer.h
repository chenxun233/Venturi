#pragma once

#include "../common/shared_types.h"

#include <atomic>
#include <cstddef>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

class LatencyLogPrinter {
public:
    explicit LatencyLogPrinter(std::size_t capacity = 1024);
    ~LatencyLogPrinter();

    bool pushLatency(const LatencyLogRecord& record);
    bool pushSnapshot(const FpgaSyncSnapshot& snapshot);
    bool pushExecution(const ExecutionLogRecord& record);
    void start();
    void stop();
    uint64_t readDropCount() const;

private:
    void _run();
    bool _pushRecord(const AsyncLogRecord& record);
    void _handleRecord(const AsyncLogRecord& record);
    std::size_t _slotIndex(std::size_t idx) const;

    std::vector<AsyncLogRecord> m_records;
    std::size_t m_capacity_mask {0};
    std::mutex m_push_mutex;
    std::mutex m_wait_mutex;
    std::condition_variable m_record_cv;
    alignas(64) std::atomic<std::size_t> m_head {0};
    alignas(64) std::atomic<std::size_t> m_tail {0};
    std::atomic<uint64_t> m_drop_count {0};
    std::atomic<bool> m_running {false};
    std::thread m_thread;
};
