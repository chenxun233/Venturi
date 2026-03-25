#include "latency_tracker.h"

#include "../../common/log.h"
#include "../sync/regression.h"

#include <algorithm>
#include <cstdio>
#include <limits>
#include <stdexcept>

namespace {

const char* stageName(stage value) {
    switch (value) {
        case stage::FRAME_START:
            return "FRAME_START";
        case stage::DMA_EMIT:
            return "DMA_EMIT";
        case stage::DECODE:
            return "DECODE";
        case stage::ANALYSIS:
            return "ANALYSIS";
    }

    return "UNKNOWN";
}

} // namespace

LatencyTracker::LatencyTracker(uint16_t producer_num, std::size_t buffer_capacity):
m_capacity(producer_num) {
    if (m_capacity & (m_capacity - 1)) {
        throw std::invalid_argument("Producer number must be a power of two");
    }
    m_trace_buffer.reserve(producer_num);
    for (uint16_t producer_idx = 0; producer_idx < producer_num; ++producer_idx) {
        m_trace_buffer.push_back(std::make_unique<TraceBuffer>(buffer_capacity));
    }
}

void LatencyTracker::attachRegression(Regression* regression) {
    m_regressions = regression;
}

TraceBuffer& LatencyTracker::readBuffer(uint16_t producer_idx) {
    if (producer_idx >= m_trace_buffer.size()) {
        throw std::out_of_range("LatencyTracker producer index out of range");
    }

    return *m_trace_buffer[producer_idx];
}

void LatencyTracker::setMeasurementEnabled(bool enabled) {
    if (m_measurement_enabled == enabled) {
        return;
    }

    m_measurement_enabled = enabled;
    m_trace_records.clear();
    m_pending_records.clear();
    _resetStats();
    m_last_print_time = std::chrono::steady_clock::now();
}

void LatencyTracker::setPrintInterval(std::chrono::seconds interval) {
    m_print_interval = interval;
}

// void LatencyTracker::attachTuner(Tuner* tuner) {
//     m_tuner = tuner;
// }

void LatencyTracker::run() {
    TimeRecord record {};
    for (uint16_t offset = 0; offset < m_capacity; ++offset) {
        const uint16_t que_idx = (m_next_buffer_idx + offset) & (m_capacity - 1);
        while (m_trace_buffer[que_idx]->pop(record)) {
            if (m_measurement_enabled) {
                m_trace_records.push_back(record);
            }
        }
    }

    m_next_buffer_idx = (m_next_buffer_idx + 1) & (m_capacity - 1);

    if (!m_measurement_enabled) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (now - m_last_print_time < m_print_interval) {
        return;
    }

    _printRecords();
    m_trace_records.clear();
    m_last_print_time = now;
}

void LatencyTracker::_printRecords() const {
    struct PrintedEventState {
        uint64_t dma_emit_host_ns {0};
        uint64_t decode_host_ns {0};
        bool has_dma_emit {false};
        bool has_decode {false};
    };

    std::unordered_map<EventKey, PrintedEventState, EventKeyHash> printed_events;

    for (const TimeRecord& record : m_trace_records) {
        uint64_t host_time_ns = 0;
        const bool has_host_time = _convertRecordTimeToHostNs(record, host_time_ns);

        if (has_host_time) {
            std::printf("TraceRecord queue=%u event_ts=%llu stage=%s raw_time=%llu host_time_ns=%llu\n",
                        record.que_idx,
                        static_cast<unsigned long long>(record.event_ts),
                        stageName(record.event_stage),
                        static_cast<unsigned long long>(record.time_captured),
                        static_cast<unsigned long long>(host_time_ns));
        } else {
            std::printf("TraceRecord queue=%u event_ts=%llu stage=%s raw_time=%llu host_time_ns=NA\n",
                        record.que_idx,
                        static_cast<unsigned long long>(record.event_ts),
                        stageName(record.event_stage),
                        static_cast<unsigned long long>(record.time_captured));
        }

        const EventKey event_key {
            .que_idx = record.que_idx,
            .event_ts = record.event_ts
        };
        PrintedEventState& event_state = printed_events[event_key];
        if (record.event_stage == stage::DMA_EMIT && has_host_time) {
            event_state.dma_emit_host_ns = host_time_ns;
            event_state.has_dma_emit = true;
        } else if (record.event_stage == stage::DECODE) {
            event_state.decode_host_ns = record.time_captured;
            event_state.has_decode = true;
        }
    }

    for (const auto& [event_key, event_state] : printed_events) {
        if (!event_state.has_dma_emit || !event_state.has_decode) {
            continue;
        }

        const int64_t dma_emit_to_decode_ns =
            static_cast<int64_t>(event_state.decode_host_ns) -
            static_cast<int64_t>(event_state.dma_emit_host_ns);
        std::printf("TraceLatency queue=%u event_ts=%llu dma_emit_to_decode_ns=%lld\n",
                    event_key.que_idx,
                    static_cast<unsigned long long>(event_key.event_ts),
                    static_cast<long long>(dma_emit_to_decode_ns));
    }
    std::fflush(stdout);
}

bool LatencyTracker::_convertRecordTimeToHostNs(const TimeRecord& record, uint64_t& host_time_ns) const {
    if (record.event_stage == stage::DECODE) {
        host_time_ns = record.time_captured;
        return true;
    }

    if (m_regressions == nullptr) {
        return false;
    }

    return m_regressions->convertFpgaToHostTime(record.time_captured, host_time_ns);
}

void LatencyTracker::_processRecord(const TimeRecord& record) {
    const EventKey event_key {
        .que_idx = record.que_idx,
        .event_ts = record.event_ts
    };

    if (record.event_stage == stage::FRAME_START) {
        uint64_t frame_start_host_ns = 0;
        if (m_regressions == nullptr) {
            warn("Regression is not attached");
            _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
            m_pending_records.erase(event_key);
            return;
        }
        if (!m_regressions->convertFpgaToHostTime(record.time_captured, frame_start_host_ns)) {
            _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
            m_pending_records.erase(event_key);
            return;
        }

        m_pending_records[event_key] = PendingEventState {
            .frame_start_host_ns = frame_start_host_ns,
            .dma_emit_host_ns = 0,
            .has_frame_start = true,
            .has_dma_emit = false
        };
        return;
    }

    const auto it = m_pending_records.find(event_key);
    if (it == m_pending_records.end()) {
        if (record.event_stage == stage::DMA_EMIT) {
            _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
        } else if (record.event_stage == stage::DECODE) {
            _incrementDrop(record.que_idx, stage::DMA_EMIT, stage::DECODE);
        }
        return;
    }

    PendingEventState& state = it->second;
    if (record.event_stage == stage::DMA_EMIT) {
        if (!state.has_frame_start) {
            _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
            m_pending_records.erase(it);
            return;
        }

        uint64_t dma_emit_host_ns = 0;
        if (!m_regressions->convertFpgaToHostTime(record.time_captured, dma_emit_host_ns)) {
            _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
            m_pending_records.erase(it);
            return;
        }

        if (dma_emit_host_ns < state.frame_start_host_ns) {
            _incrementDrop(record.que_idx, stage::FRAME_START, stage::DMA_EMIT);
            m_pending_records.erase(it);
            return;
        }

        const StageLatency latency {
            .que_idx = record.que_idx,
            .event_ts = record.event_ts,
            .prev_stage = stage::FRAME_START,
            .curr_stage = stage::DMA_EMIT,
            .latency = dma_emit_host_ns - state.frame_start_host_ns
        };
        _updateStats(latency);

        state.dma_emit_host_ns = dma_emit_host_ns;
        state.has_dma_emit = true;
        return;
    }

    if (record.event_stage != stage::DECODE) {
        return;
    }

    if (!state.has_dma_emit) {
        _incrementDrop(record.que_idx, stage::DMA_EMIT, stage::DECODE);
        m_pending_records.erase(it);
        return;
    }

    if (record.time_captured < state.dma_emit_host_ns) {
        _incrementDrop(record.que_idx, stage::DMA_EMIT, stage::DECODE);
        m_pending_records.erase(it);
        return;
    }

    const StageLatency latency {
        .que_idx = record.que_idx,
        .event_ts = record.event_ts,
        .prev_stage = stage::DMA_EMIT,
        .curr_stage = stage::DECODE,
        .latency = record.time_captured - state.dma_emit_host_ns
    };

    _updateStats(latency);
    m_pending_records.erase(it);
}

void LatencyTracker::_updateStats(const StageLatency& latency) {
    const StageKey stage_key {
        .que_idx = latency.que_idx,
        .prev_stage = latency.prev_stage,
        .curr_stage = latency.curr_stage
    };

    auto [it, inserted] = m_latency_stats.try_emplace(stage_key);
    LatencyStatsState& state = it->second;
    if (inserted) {
        state.stats.que_idx = latency.que_idx;
        state.stats.prev_stage = latency.prev_stage;
        state.stats.curr_stage = latency.curr_stage;
        state.stats.min_ns = std::numeric_limits<uint64_t>::max();
    }

    ++state.stats.sample_count;
    state.sum_ns += static_cast<__int128>(latency.latency);
    state.stats.avg_ns = static_cast<uint64_t>(state.sum_ns / state.stats.sample_count);
    state.stats.min_ns = std::min(state.stats.min_ns, latency.latency);
    state.stats.max_ns = std::max(state.stats.max_ns, latency.latency);
    state.samples_ns.push_back(latency.latency);
}

void LatencyTracker::_incrementDrop(uint16_t que_idx, stage prev_stage, stage curr_stage) {
    const StageKey stage_key {
        .que_idx = que_idx,
        .prev_stage = prev_stage,
        .curr_stage = curr_stage
    };

    auto [it, inserted] = m_latency_stats.try_emplace(stage_key);
    LatencyStatsState& state = it->second;
    if (inserted) {
        state.stats.que_idx = que_idx;
        state.stats.prev_stage = prev_stage;
        state.stats.curr_stage = curr_stage;
        state.stats.min_ns = std::numeric_limits<uint64_t>::max();
    }

    ++state.stats.drop_count;
}

void LatencyTracker::_refreshPercentiles() {
    const auto percentile_index = [](std::size_t sample_count, std::size_t numer, std::size_t denom) {
        if (sample_count == 0) {
            return static_cast<std::size_t>(0);
        }

        const std::size_t rank = (sample_count * numer + denom - 1) / denom;
        return std::min(sample_count - 1, std::max<std::size_t>(1, rank) - 1);
    };

    for (auto& [stage_key, state] : m_latency_stats) {
        (void)stage_key;
        if (state.samples_ns.empty()) {
            continue;
        }

        std::vector<uint64_t> sorted_samples = state.samples_ns;
        std::sort(sorted_samples.begin(), sorted_samples.end());

        state.stats.p50_ns = sorted_samples[percentile_index(sorted_samples.size(), 50, 100)];
        state.stats.p99_ns = sorted_samples[percentile_index(sorted_samples.size(), 99, 100)];
        state.stats.p999_ns = sorted_samples[percentile_index(sorted_samples.size(), 999, 1000)];
    }
}

void LatencyTracker::_printStats() const {
    for (const auto& [stage_key, state] : m_latency_stats) {
        (void)stage_key;
        if (state.stats.sample_count == 0 && state.stats.drop_count == 0) {
            continue;
        }

        const uint64_t min_ns =
            (state.stats.sample_count == 0) ? 0 : state.stats.min_ns;
        const uint64_t max_ns =
            (state.stats.sample_count == 0) ? 0 : state.stats.max_ns;

        info("LatencyStats queue=%u stage=%s->%s samples=%llu drops=%llu p50=%llu p99=%llu p999=%llu avg=%llu min=%llu max=%llu",
             state.stats.que_idx,
             stageName(state.stats.prev_stage),
             stageName(state.stats.curr_stage),
             static_cast<unsigned long long>(state.stats.sample_count),
             static_cast<unsigned long long>(state.stats.drop_count),
             static_cast<unsigned long long>(state.stats.p50_ns),
             static_cast<unsigned long long>(state.stats.p99_ns),
             static_cast<unsigned long long>(state.stats.p999_ns),
             static_cast<unsigned long long>(state.stats.avg_ns),
             static_cast<unsigned long long>(min_ns),
             static_cast<unsigned long long>(max_ns));
    }

}

void LatencyTracker::_resetStats() {
    m_latency_stats.clear();
}
