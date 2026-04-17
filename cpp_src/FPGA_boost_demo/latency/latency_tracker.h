#pragma once

#include "../common/spsc_ring_queue.h"
#include "../common/shared_types.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

class LogPrinter;
class Tuner;

class LatencyTracker {
public:
    explicit        LatencyTracker(uint16_t producer_num,
                                   std::size_t buffer_capacity = 1024,
                                   std::size_t pending_capacity = 1024);
    std::size_t     run();
    void            pushRecord(const TimeRecord& record) noexcept;
    void            attachLogPrinter(LogPrinter* log_printer);
    

private:
    struct PendingEventState {
        uint64_t frame_start_tick {0};
        uint64_t dma_emit_tick {0};
        uint64_t batch_start_tick {0};
        uint64_t batch_end_tick {0};
        uint64_t strategy_start_tick {0};
        uint64_t tx_execution_accepted_tick {0};
        uint64_t tx_enqueue_tick {0};
        uint64_t tx_send_enter_tick {0};
        uint64_t tx_send_syscall_enter_tick {0};
        uint64_t frame_start_to_dma_emit_ns {0};
        int64_t batch_duration_ns {0};
        int64_t batch_end_to_strategy_start_ns {0};
        int64_t strategy_start_to_tx_execution_accepted_ns {0};
        int64_t tx_execution_accepted_to_tx_enqueue_ns {0};
        int64_t tx_enqueue_to_tx_send_enter_ns {0};
        int64_t tx_send_enter_to_tx_send_syscall_enter_ns {0};
        int64_t tx_send_syscall_enter_to_tx_send_ns {0};
        uint32_t tx_enqueue_backlog_depth {0};
        uint32_t tx_send_enter_backlog_depth {0};
        uint32_t tx_send_call_count {0};
        uint32_t tx_send_bytes_total {0};
        uint32_t tx_send_eintr_retry_count {0};
        uint32_t tx_send_had_partial_write {0};
        bool has_dma_emit {false};
        bool has_batch_start {false};
        bool has_batch_end {false};
        bool has_strategy_start {false};
        bool has_tx_execution_accepted {false};
        bool has_tx_enqueue {false};
        bool has_tx_send_enter {false};
        bool has_tx_send_syscall_enter {false};
    };

    struct PendingSlot {
        bool occupied {false};
        uint64_t event_tag {0};
        uint32_t trace_id {0};
        uint64_t insertion_sequence {0};
        PendingEventState state {};
    };

    struct PendingTable {
        std::vector<PendingSlot> slots {};
        uint64_t next_insertion_sequence {0};
        std::size_t live_count {0};
    };

    std::size_t _drainFairPass();
    bool _hasPendingRecord(uint16_t que_idx, uint64_t event_tag) const noexcept;
    PendingSlot* _findPendingSlot(uint16_t que_idx, uint64_t event_tag) noexcept;
    PendingSlot& _upsertPendingSlot(uint16_t que_idx, uint64_t event_tag, uint32_t trace_id);
    void _erasePendingSlot(uint16_t que_idx, PendingSlot& slot) noexcept;
    void _processRecord(const TimeRecord& record);
    void _handleFrameStart(const TimeRecord& record);
    void _handleMissingPendingRecord(const TimeRecord& record);
    void _handleDmaEmit(const TimeRecord& record, PendingSlot& slot);
    void _handleBatchStart(const TimeRecord& record, PendingSlot& slot);
    void _handleBatchEnd(const TimeRecord& record, PendingSlot& slot);
    void _handleStrategyStart(const TimeRecord& record, PendingSlot& slot);
    void _handleTxExecutionAccepted(const TimeRecord& record, PendingSlot& slot);
    void _handleTxEnqueue(const TimeRecord& record, PendingSlot& slot);
    void _handleTxSendEnter(const TimeRecord& record, PendingSlot& slot);
    void _handleTxSendSyscallEnter(const TimeRecord& record, PendingSlot& slot);
    void _handleTxSend(const TimeRecord& record, PendingSlot& slot);
    static int64_t _readSignedDelta(uint64_t later_tick, uint64_t earlier_tick);
    static int64_t _readSignedHostDeltaNs(uint64_t later_tick, uint64_t earlier_tick);
    void _updateStats(const StageLatency& latency);
    void _incrementDrop(uint16_t que_idx, stage prev_stage, stage curr_stage);
    LatencyStats& _readStats(uint16_t que_idx, stage prev_stage, stage curr_stage);
    static bool _isPowerOfTwo(std::size_t value) noexcept;

    std::vector<std::unique_ptr<SpscRingQueue<TimeRecord>>> m_latency_queues;
    uint16_t m_queue_num {0};
    uint16_t m_next_queue_idx {0};
    LogPrinter* m_log_printer {nullptr};
    std::size_t m_pending_capacity {0};
    std::vector<PendingTable> m_pending_tables;
    std::vector<std::vector<LatencyStats>> m_latency_stats;
    // Tuner* m_tuner {nullptr};
};
