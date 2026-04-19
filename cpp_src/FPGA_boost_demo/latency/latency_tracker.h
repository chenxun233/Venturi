#pragma once

#include "../common/shared_types.h"
#include "../common/spsc_ring_queue.h"

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
                            std::size_t command_capacity = 8);

    uint32_t tryAllocateTraceId(uint16_t que_idx, bool is_first_event) noexcept;
    void attachAnalyzer(LatencyAnalyzer* latency_analyzer) noexcept;
    void pushRecord(const TimeRecord& record) noexcept;
    bool requestFinalize(uint16_t que_idx, uint32_t trace_id) noexcept;
    bool requestDrop(uint16_t que_idx, uint32_t trace_id) noexcept;
    bool requestDrop(uint16_t command_queue_idx,
                     uint16_t target_queue_idx,
                     uint32_t trace_id) noexcept;
    void run() noexcept;
    void stop() noexcept;
    void printDebugSummary() const noexcept;

private:
    static int64_t _readSignedDelta(uint64_t later_tick, uint64_t earlier_tick) noexcept;
    static int64_t _readSignedHostDeltaNs(uint64_t later_tick, uint64_t earlier_tick) noexcept;
    static uint64_t _encodeOverflowCommand(const TraceCommand& command) noexcept;
    static TraceCommand _decodeOverflowCommand(uint64_t encoded_command) noexcept;
    uint32_t _allocateTraceId() noexcept;
    void finalizeTrace(uint16_t que_idx, uint32_t trace_id) noexcept;
    void dropTrace(uint16_t que_idx, uint32_t trace_id) noexcept;
    bool _drainCommandPass() noexcept;
    void _drainAllCommands() noexcept;
    bool _enqueueCommand(uint16_t command_queue_idx,
                         uint16_t target_queue_idx,
                         TraceCommandOp op,
                         uint32_t trace_id) noexcept;
    bool _tryStoreOverflowCommand(uint16_t command_queue_idx,
                                  const TraceCommand& command) noexcept;
    bool _tryConsumeOverflowCommand(uint16_t command_queue_idx,
                                    TraceCommand& command) noexcept;
    void _processCommand(const TraceCommand& command) noexcept;
    void _clearActiveTrace(uint16_t que_idx, uint32_t trace_id) noexcept;
    void _dropQueueUntilEmpty(uint16_t que_idx) noexcept;

    std::vector<std::unique_ptr<SpscRingQueue<TimeRecord>>> m_latency_queues;
    std::vector<std::unique_ptr<SpscRingQueue<TraceCommand>>> m_trace_command_queues;
    std::vector<std::atomic<uint64_t>> m_trace_command_overflow_slots;
    uint16_t m_queue_num {0};
    std::atomic<uint32_t> m_next_trace_id {1U};
    std::vector<std::atomic<uint32_t>> m_active_trace_ids;
    std::vector<std::atomic<uint64_t>> m_started_trace_counts;
    std::vector<std::atomic<uint64_t>> m_finalize_request_counts;
    std::vector<std::atomic<uint64_t>> m_completed_trace_counts;
    std::vector<std::atomic<uint64_t>> m_drop_request_counts;
    std::vector<std::atomic<uint64_t>> m_missing_trace_record_counts;
    std::vector<std::atomic<uint64_t>> m_stage_mismatch_drop_counts;
    LatencyAnalyzer* m_latency_analyzer {nullptr};
    std::atomic<bool> m_should_stop {false};
    uint16_t m_next_command_queue_idx {0};
};
