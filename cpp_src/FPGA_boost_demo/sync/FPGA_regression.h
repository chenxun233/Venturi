#pragma once

#include "../common/shared_types.h"
#include "../driver/fpga_dev.h"
#include <cstddef>
#include <cstdint>
#include <mutex>

class FPGARegression {
public:
    explicit FPGARegression(uint64_t trigger_period = 0);

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
    static uint64_t         _scaleTicksToNs(uint64_t tick_delta, uint64_t a_q32);
    mutable std::mutex      m_regression_mutex {};
    RegressionPara          m_pre_regression_para {};
    RegressionPara          m_cur_regression_para {};
    uint32_t                m_snapshot_count {0};
    bool                    m_is_frozen {false};

    uint64_t                m_trigger_period {0};
    uint64_t                m_trigger_countdown {0};
    FpgaSyncSnapshot        m_candidate_snapshot {};
    FpgaSyncSnapshot        m_pre_snapshot {};
    FpgaSyncSnapshot        m_cur_snapshot {};
};
