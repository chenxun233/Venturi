#pragma once

#include "../common/shared_types.h"
#include "../common/spsc_ring_queue.h"
#include "../common/time_utils.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

class LatencyAnalyzer;

class LatencyTracker {
public:
    explicit LatencyTracker(uint16_t producer_num,
                            std::size_t buffer_capacity = 1024,
                            std::size_t command_capacity = 64);

    uint32_t tryAllocateTraceId(uint16_t que_idx, bool is_first_event) noexcept;
    void attachAnalyzer(LatencyAnalyzer* latency_analyzer) noexcept;
    void pushRecord(const TimeRecord& record) noexcept;
    bool requestFinalize(uint16_t que_idx, uint32_t trace_id) noexcept;
    bool requestDrop(uint16_t que_idx,
                     uint32_t trace_id) noexcept;
    void run() noexcept;
    void stop() noexcept;
    uint64_t readCompletedTraceCount(uint16_t que_idx) const noexcept;

private:
    static int64_t _FPGATick2ns(uint64_t later_tick, uint64_t earlier_tick) noexcept;
    int64_t _hostTick2ns(uint64_t later_tick, uint64_t earlier_tick) noexcept;
    void _finalizeTrace(uint16_t que_idx, uint32_t trace_id) noexcept;
    void _dropTrace(uint16_t que_idx, uint32_t trace_id) noexcept;
    bool _drainCommand() noexcept;

    void _processCommand(const TraceCommand& command) noexcept;
    void _clearActiveTraceID(uint16_t que_idx, uint32_t trace_id) noexcept;
    void _cleanLatencyQueue(uint16_t que_idx) noexcept;

    std::vector<std::unique_ptr<SpscRingBuffer<TimeRecord>>> m_latency_queues;
    std::vector<std::unique_ptr<SpscRingBuffer<TraceCommand>>> m_command_queues;
    uint16_t m_queue_num {0};
    HostTickScale m_host_tick_scale {};
    std::vector<uint32_t> m_next_trace_ids;
    LatencyAnalyzer* m_latency_analyzer {nullptr};
    std::atomic<bool> m_should_stop {false};
    uint16_t m_next_command_queue_idx {0};
    // for debug
    std::vector<std::atomic<uint32_t>> m_active_trace_ids;
    std::vector<std::atomic<uint64_t>> m_completed_trace_counts;

};
