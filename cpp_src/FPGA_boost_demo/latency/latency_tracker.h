#pragma once

#include "trace_buffer.h"
#include "../common/shared_types.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>
#include <chrono>

class Regression;
class Tuner;

class LatencyTracker {
public:
    explicit        LatencyTracker(uint16_t producer_num, std::size_t buffer_capacity = 1024);
    void            attachRegression(Regression* regression);
    TraceBuffer&    readBuffer(uint16_t producer_idx);
    void            setMeasurementEnabled(bool enabled);
    void            setPrintInterval(std::chrono::seconds interval);
    void            run();
    // void            attachTuner(Tuner* tuner);

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

    struct LatencyStatsState {
        LatencyStats stats {};
        __int128 sum_ns {0};
        std::vector<uint64_t> samples_ns;
    };

    struct PendingEventState {
        uint64_t frame_start_host_ns {0};
        uint64_t dma_emit_host_ns {0};
        bool has_frame_start {false};
        bool has_dma_emit {false};
    };

    void _processRecord(const TimeRecord& record);
    void _updateStats(const StageLatency& latency);
    void _incrementDrop(uint16_t que_idx, stage prev_stage, stage curr_stage);
    void _refreshPercentiles();
    void _printStats() const;
    void _printRecords() const;
    bool _convertRecordTimeToHostNs(const TimeRecord& record, uint64_t& host_time_ns) const;
    void _resetStats();

    std::vector<std::unique_ptr<TraceBuffer>> m_trace_buffer;
    uint16_t m_capacity {0};
    uint16_t m_next_buffer_idx {0};
    Regression* m_regressions {nullptr};
    bool m_measurement_enabled {false};
    std::chrono::seconds m_print_interval {std::chrono::seconds(1)};
    std::chrono::steady_clock::time_point m_last_print_time {std::chrono::steady_clock::now()};
    std::vector<TimeRecord> m_trace_records;
    std::unordered_map<EventKey, PendingEventState, EventKeyHash> m_pending_records;
    std::unordered_map<StageKey, LatencyStatsState, StageKeyHash> m_latency_stats;
    // Tuner* m_tuner {nullptr};
};
