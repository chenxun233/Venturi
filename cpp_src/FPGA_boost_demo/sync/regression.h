#pragma once

#include "../common/shared_types.h"

#include <cstddef>
#include <cstdint>
#include <mutex>

class Regression {
public:
    Regression() = default;

    void updateSnapshot(const FpgaSyncSnapshot& snapshot);
    FpgaSyncSnapshot readSnapshot() const;
    RegressionPara returnParaSnapshot() const;
    RegressionStatusLogRecord readStatusLogRecord() const;
    bool isFrozen() const;
    bool convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const;

private:
    void _updateModel(const FpgaSyncSnapshot& snapshot);
    static uint64_t _scaleTicksToNs(uint64_t tick_delta, uint64_t a_q32);

    mutable std::mutex      m_regression_mutex {};
    RegressionPara          m_regression_para {};
    uint32_t                m_snapshot_count {0};
    bool                    m_is_frozen {false};
    FpgaSyncSnapshot        m_latest_snapshot {};
    long double             m_sum_fpga_ticks {0.0L};
    long double             m_sum_host_time_ns {0.0L};
    long double             m_sum_fpga_ticks_sq {0.0L};
    long double             m_sum_fpga_host_product {0.0L};
    long double             m_last_a_ns_per_tick {0.0L};
    std::size_t             m_stable_count {0};
};
