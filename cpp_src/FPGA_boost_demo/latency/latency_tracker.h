#pragma once

#include "trace_buffer.h"
#include "../common/shared_types.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

class Regression;
class LatencyLogPrinter;
class Tuner;

class LatencyTracker {
public:
    explicit        LatencyTracker(uint16_t producer_num, std::size_t buffer_capacity = 1024);
    void            run();
    bool            pushRecord(const TimeRecord& record);
    void            attachRegression(Regression* regression);
    void            attachLogPrinter(LatencyLogPrinter* log_printer);
    

private:
    struct EventKey {
        uint16_t que_idx {0};
        uint64_t event_ts {0};

        bool operator==(const EventKey& other) const {
            return que_idx == other.que_idx && event_ts == other.event_ts;
        }
    };

    struct StageKey {
        uint16_t que_idx {0};
        stage prev_stage {stage::DECODE};
        stage curr_stage {stage::DECODE};

        bool operator==(const StageKey& other) const {
            return que_idx == other.que_idx &&
                   prev_stage == other.prev_stage &&
                   curr_stage == other.curr_stage;
        }
    };

    struct EventKeyHash {
        std::size_t operator()(const EventKey& key) const {
            return (static_cast<std::size_t>(key.que_idx) << 1) ^
                   static_cast<std::size_t>(key.event_ts);
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
        uint64_t frame_start_host_ns {0};
        uint64_t dma_emit_host_ns {0};
        uint64_t frame_start_to_dma_emit_ns {0};
        bool has_dma_emit {false};
    };

    void _processRecord(const TimeRecord& record);
    void _handleFrameStart(const EventKey& event_key, const TimeRecord& record);
    void _handleMissingPendingRecord(const TimeRecord& record);
    void _handleDmaEmit(const TimeRecord& record,
                        std::unordered_map<EventKey, PendingEventState, EventKeyHash>::iterator it);
    void _handleDecode(const TimeRecord& record,
                       std::unordered_map<EventKey, PendingEventState, EventKeyHash>::iterator it);
    void _updateStats(const StageLatency& latency);
    void _incrementDrop(uint16_t que_idx, stage prev_stage, stage curr_stage);
    LatencyStats& _readOrCreateStats(uint16_t que_idx, stage prev_stage, stage curr_stage);

    std::vector<std::unique_ptr<TraceBuffer<TimeRecord>>> m_trace_buffer;
    uint16_t m_capacity {0};
    uint16_t m_next_buffer_idx {0};
    Regression* m_regressions {nullptr};
    LatencyLogPrinter* m_log_printer {nullptr};
    std::mutex m_run_mutex;
    std::unordered_map<EventKey, PendingEventState, EventKeyHash> m_pending_records;
    std::unordered_map<StageKey, LatencyStats, StageKeyHash> m_latency_stats;
    // Tuner* m_tuner {nullptr};
};
