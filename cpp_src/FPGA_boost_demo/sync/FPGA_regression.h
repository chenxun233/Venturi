#pragma once

#include "../common/shared_types.h"
#include "../driver/fpga_dev.h"
#include <cstddef>
#include <cstdint>
#include <mutex>

class FPGARegression {
public:
    explicit FPGARegression(uint64_t trigger_period = 0,
                            std::size_t required_stable_updates = 4,
                            long double convergence_threshold_ns_per_tick = 1e-8L);

    template <typename Device>
    bool initSync(Device& device,
                  std::size_t max_attempts,
                  uint64_t accepted_interval_ns) {
        for (std::size_t attempt = 0; attempt < max_attempts; ++attempt) {
            if (!device.readSyncTimestamp(m_candidate_snapshot)) {
                continue;
            }
            if (!tryAcceptSnapshot(m_candidate_snapshot, accepted_interval_ns)) {
                continue;
            }
            updateRegression();
            if (isFrozen()) {
                return true;
            }
        }
        return false;
    }

    void run(CapSignal& cap_signal);
    bool tryAcceptSnapshot(const FpgaSyncSnapshot& snapshot, uint64_t accepted_interval_ns);
    void updateRegression();

    RegressionStatusLogRecord readStatusLogRecord() const;
    bool isFrozen() const;
    bool convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const;

private:
    static uint64_t         _scaleTicksToNs(uint64_t tick_delta, uint64_t a_q32);
    static uint64_t         _computeRawAQ32(uint64_t tick_delta, uint64_t host_time_delta_ns);
    mutable std::mutex      m_regression_mutex {};
    std::size_t             m_required_stable_updates {4};
    long double             m_convergence_threshold_ns_per_tick {1e-8L};
    uint64_t                m_convergence_threshold_q32 {0};
    bool                    m_has_prev_raw_a {false};
    uint64_t                m_prev_raw_a_q32 {0};
    std::size_t             m_stable_update_count {0};
    RegressionPara          m_frozen_regression_para {};
    bool                    m_is_frozen {false};

    uint64_t                m_trigger_period {0};
    uint64_t                m_trigger_countdown {0};
    FpgaSyncSnapshot        m_candidate_snapshot {};
    FpgaSyncSnapshot        m_pre_snapshot {};
    FpgaSyncSnapshot        m_cur_snapshot {};
    FpgaSyncSnapshot        m_latest_snapshot {};

};
