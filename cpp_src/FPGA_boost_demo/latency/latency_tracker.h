#pragma once

#include "../common/spsc_ring_queue.h"
#include "../common/shared_types.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <thread>
#include <unordered_map>

class LogPrinter;
class Tuner;

class LatencyTracker {
public:
    explicit        LatencyTracker(uint16_t producer_num, std::size_t buffer_capacity = 1024);
    std::size_t     run();
    void            pushRecord(const TimeRecord& record) noexcept;
    void            attachLogPrinter(LogPrinter* log_printer);
    

private:
    struct EventKey {
        uint16_t que_idx {0};
        uint64_t event_tag {0};

        bool operator==(const EventKey& other) const {
            return que_idx == other.que_idx && event_tag == other.event_tag;
        }
    };

    struct StageKey {
        uint16_t que_idx {0};
        stage prev_stage {stage::FRAME_START};
        stage curr_stage {stage::FRAME_START};

        bool operator==(const StageKey& other) const {
            return que_idx == other.que_idx &&
                   prev_stage == other.prev_stage &&
                   curr_stage == other.curr_stage;
        }
    };

    struct EventKeyHash {
        std::size_t operator()(const EventKey& key) const {
            return (static_cast<std::size_t>(key.que_idx) << 1) ^
                   static_cast<std::size_t>(key.event_tag);
        }
    };

    struct StageKeyHash {
        std::size_t operator()(const StageKey& key) const {
            return static_cast<std::size_t>(key.que_idx) ^
                   (static_cast<std::size_t>(key.prev_stage) << 8) ^
                   (static_cast<std::size_t>(key.curr_stage) << 16);
        }
    };

    struct PendingEventState {
        uint64_t frame_start_tick {0};
        uint64_t dma_emit_tick {0};
        uint64_t batch_start_ns {0};
        uint64_t batch_end_ns {0};
        uint64_t strategy_start_ns {0};
        uint64_t tx_execution_accepted_ns {0};
        uint64_t tx_execution_dequeue_ns {0};
        uint64_t tx_order_frame_built_ns {0};
        uint64_t tx_pending_recorded_ns {0};
        uint64_t tx_enqueue_ns {0};
        uint64_t frame_start_to_dma_emit_ns {0};
        int64_t batch_duration_ns {0};
        int64_t batch_end_to_strategy_start_ns {0};
        int64_t strategy_start_to_tx_execution_accepted_ns {0};
        int64_t tx_execution_accepted_to_tx_execution_dequeue_ns {0};
        int64_t tx_execution_dequeue_to_tx_order_frame_built_ns {0};
        int64_t tx_order_frame_built_to_tx_pending_recorded_ns {0};
        int64_t tx_pending_recorded_to_tx_enqueue_ns {0};
        int64_t tx_enqueue_to_tx_send_ns {0};
        bool has_dma_emit {false};
        bool has_batch_start {false};
        bool has_batch_end {false};
        bool has_strategy_start {false};
        bool has_tx_execution_accepted {false};
        bool has_tx_execution_dequeue {false};
        bool has_tx_order_frame_built {false};
        bool has_tx_pending_recorded {false};
        bool has_tx_enqueue {false};
    };

    using PendingIterator = std::unordered_map<EventKey, PendingEventState, EventKeyHash>::iterator;

    void _processRecord(const TimeRecord& record);
    void _handleFrameStart(const EventKey& event_key, const TimeRecord& record);
    void _handleMissingPendingRecord(const TimeRecord& record);
    void _handleDmaEmit(const TimeRecord& record, PendingIterator it);
    void _handleBatchStart(const TimeRecord& record, PendingIterator it);
    void _handleBatchEnd(const TimeRecord& record, PendingIterator it);
    void _handleStrategyStart(const TimeRecord& record, PendingIterator it);
    void _handleTxExecutionAccepted(const TimeRecord& record, PendingIterator it);
    void _handleTxExecutionDequeue(const TimeRecord& record, PendingIterator it);
    void _handleTxOrderFrameBuilt(const TimeRecord& record, PendingIterator it);
    void _handleTxPendingRecorded(const TimeRecord& record, PendingIterator it);
    void _handleTxEnqueue(const TimeRecord& record, PendingIterator it);
    void _handleTxSend(const TimeRecord& record, PendingIterator it);
    static int64_t _readSignedDelta(uint64_t later_ns, uint64_t earlier_ns);
    void _updateStats(const StageLatency& latency);
    void _incrementDrop(uint16_t que_idx, stage prev_stage, stage curr_stage);
    LatencyStats& _readOrCreateStats(uint16_t que_idx, stage prev_stage, stage curr_stage);

    std::vector<std::unique_ptr<SpscRingQueue<TimeRecord>>> m_latency_queues;
    uint16_t m_queue_num {0};
    uint16_t m_next_queue_idx {0};
    LogPrinter* m_log_printer {nullptr};
    std::unordered_map<EventKey, PendingEventState, EventKeyHash> m_pending_records;
    std::unordered_map<StageKey, LatencyStats, StageKeyHash> m_latency_stats;
    // Tuner* m_tuner {nullptr};
};
