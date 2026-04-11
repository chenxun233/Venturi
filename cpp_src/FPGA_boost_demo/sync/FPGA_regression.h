#pragma once

#include "../common/publisher.h"
#include "../common/shared_types.h"
#include "../driver/fpga_dev.h"
#include <cstddef>
#include <cstdint>

struct RegressionPublishedState {
    RegressionPara      regression_para {};
    bool                is_frozen {false};
    FpgaSyncSnapshot    anchor_snapshot {};
};

class FPGARegression {
public:
    explicit FPGARegression(uint64_t trigger_period = 100);

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
            _update_a();
            if (isFrozen()) {
                return true;
            }
        }
        return false;
    }

    void run(CapSignal& cap_signal);
    bool tryAcceptSnapshot(const FpgaSyncSnapshot& snapshot, uint64_t accepted_interval_ns);

    RegressionStatusLogRecord readStatusLogRecord() const;
    bool isFrozen() const;
    bool convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const;

private:
    void                    _update_a();
    void                    _publishState();
    static uint64_t         _scaleTicksToNs(uint64_t tick_delta, uint64_t a_q32);
    Publisher<RegressionPublishedState>
                            m_published_state {RegressionPublishedState {}};
    RegressionPara          m_regression_para {};
    uint32_t                m_converge_count {0};
    bool                    m_is_frozen {false};

    uint64_t                m_trigger_period {0};
    uint64_t                m_trigger_countdown {0};
    FpgaSyncSnapshot        m_candidate_snapshot {};
    FpgaSyncSnapshot        m_pre_snapshot {};
    FpgaSyncSnapshot        m_cur_snapshot {};

    std::size_t             m_sample_count {0};
    long double             m_sum_fpga_tick {0.0L};
    long double             m_sum_host_time_ns {0.0L};
    long double             m_sum_fpga_tick_sq {0.0L};
    long double             m_sum_fpga_host_product {0.0L};
    bool                    m_has_prev_fitted_a {false};
    uint64_t                m_prev_fitted_a_q32 {0};
    long double             m_fitted_b_ns {0.0L};
};
