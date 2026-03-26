#pragma once

#include "../common/shared_types.h"

#include <cstdint>
#include <mutex>

class Regression {
public:
    Regression() = default;

    void updateSnapshot(const FpgaSyncSnapshot& snapshot);
    FpgaSyncSnapshot readSnapshot() const;
    RegressionPara returnParaSnapshot() const;
    bool isFrozen() const;
    bool convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const;

private:
    static constexpr uint64_t kTickNumer = 32ULL;
    static constexpr uint64_t kTickDenom = 5ULL;
    static constexpr uint64_t kFixedAQ32 = 27487790694ULL; // floor((32/5) * 2^32)

    void _updateModel(const FpgaSyncSnapshot& snapshot);

    mutable std::mutex      m_regression_mutex {};
    RegressionPara          m_regression_para {};
    uint32_t                m_snapshot_count {0};
    bool                    m_is_frozen {false};
    FpgaSyncSnapshot        m_latest_snapshot {};
};
