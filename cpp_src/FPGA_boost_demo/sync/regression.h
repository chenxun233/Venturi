#pragma once

#include "../common/shared_types.h"

#include <cstdint>
#include <mutex>

class Regression {
public:
    Regression() = default;

    void run(std::atomic<bool>& snap_ready, const FpgaSyncSnapshot& snapshot);
    RegressionPara readParaSnapshot() const;
    bool isFrozen() const;
    bool convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const;

private:
    static constexpr uint64_t kMinDeltaSamples = 1;
    static constexpr uint64_t kStableAQ32Delta = 500000;
    static constexpr int64_t kStableBDeltaNs = 200000000;
    static constexpr uint32_t kStableUpdatesRequired = 3;

    typedef struct  {
        uint64_t sample_count {0};
        uint64_t delta_sample_count {0};
        __int128 sum_fpga_tick {0};
        __int128 sum_host_time_ns {0};
        __int128 sum_fpga_delta {0};
        __int128 sum_host_delta_ns {0};
        uint64_t prev_fpga_tick {0};
        uint64_t prev_host_time_ns {0};
        bool has_prev_sample {false};
    }SyncSampleAcc;

    typedef struct {
        RegressionPara prev_candidate {};
        uint32_t stable_update_count {0};
        bool has_prev_candidate {false};
        bool is_frozen {false};
    } ConvergenceState;

    void _updateRegression(const FpgaSyncSnapshot& snapshot);

    SyncSampleAcc           m_sync_acc {};
    ConvergenceState        m_convergence {};
    mutable std::mutex      m_regression_mutex {};
    RegressionPara          m_regression_para {};
};
