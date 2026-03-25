#include "regression.h"

namespace {

uint64_t absDiffUint64(uint64_t lhs, uint64_t rhs) {
    return (lhs >= rhs) ? (lhs - rhs) : (rhs - lhs);
}

uint64_t absDiffInt64(int64_t lhs, int64_t rhs) {
    return (lhs >= rhs)
        ? static_cast<uint64_t>(lhs - rhs)
        : static_cast<uint64_t>(rhs - lhs);
}

} // namespace

void Regression::run(std::atomic<bool>& snap_ready, const FpgaSyncSnapshot& snapshot) {
    if (!snap_ready.exchange(false, std::memory_order_acq_rel)) {
        return;
    }
    _updateRegression(snapshot);
}

RegressionPara Regression::readParaSnapshot() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    return m_regression_para;
}

bool Regression::isFrozen() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    return m_convergence.is_frozen;
}

bool Regression::convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    const RegressionPara model = m_regression_para;
    if (!model.has_para) {
        return false;
    }

    const __int128 scaled =
        (static_cast<__int128>(fpga_tick) * static_cast<__int128>(model.a_q32)) >> 32;
    const int64_t signed_host_time = static_cast<int64_t>(scaled) + model.b_ns;
    if (signed_host_time < 0) {
        return false;
    }

    host_time_ns = static_cast<uint64_t>(signed_host_time);
    return true;
}

void Regression::_updateRegression(const FpgaSyncSnapshot& snapshot) {
    {
        std::lock_guard<std::mutex> lock(m_regression_mutex);
        if (m_convergence.is_frozen) {
            return;
        }
    }

    if (snapshot.interval_ns == 0) {
        return;
    }

    const uint64_t x = snapshot.fpga_tick;
    const uint64_t y = snapshot.host_time_ns;
    ++m_sync_acc.sample_count;
    m_sync_acc.sum_fpga_tick += static_cast<__int128>(x);
    m_sync_acc.sum_host_time_ns += static_cast<__int128>(y);

    if (!m_sync_acc.has_prev_sample) {
        m_sync_acc.prev_fpga_tick = x;
        m_sync_acc.prev_host_time_ns = y;
        m_sync_acc.has_prev_sample = true;
        return;
    }

    if (x <= m_sync_acc.prev_fpga_tick || y <= m_sync_acc.prev_host_time_ns) {
        m_sync_acc.prev_fpga_tick = x;
        m_sync_acc.prev_host_time_ns = y;
        return;
    }

    const uint64_t fpga_delta = x - m_sync_acc.prev_fpga_tick;
    const uint64_t host_delta_ns = y - m_sync_acc.prev_host_time_ns;
    m_sync_acc.sum_fpga_delta += static_cast<__int128>(fpga_delta);
    m_sync_acc.sum_host_delta_ns += static_cast<__int128>(host_delta_ns);
    ++m_sync_acc.delta_sample_count;
    m_sync_acc.prev_fpga_tick = x;
    m_sync_acc.prev_host_time_ns = y;

    if (m_sync_acc.sum_fpga_delta <= 0 ||
        m_sync_acc.delta_sample_count < kMinDeltaSamples) {
        return;
    }

    const uint64_t a_q32 = static_cast<uint64_t>(
        (m_sync_acc.sum_host_delta_ns << 32) / m_sync_acc.sum_fpga_delta);
    const __int128 n = static_cast<__int128>(m_sync_acc.sample_count);
    const __int128 mean_x_q32 = (m_sync_acc.sum_fpga_tick << 32) / n;
    const __int128 mean_y_q32 = (m_sync_acc.sum_host_time_ns << 32) / n;
    const __int128 b_q32 =
        mean_y_q32 - (static_cast<__int128>(a_q32) * mean_x_q32 >> 32);

    const RegressionPara candidate {
        .has_para = true,
        .a_q32 = a_q32,
        .b_ns = static_cast<int64_t>(b_q32 >> 32)
    };

    std::lock_guard<std::mutex> lock(m_regression_mutex);
    if (m_convergence.is_frozen) {
        return;
    }

    if (m_convergence.has_prev_candidate) {
        const bool a_stable =
            absDiffUint64(candidate.a_q32, m_convergence.prev_candidate.a_q32) <= kStableAQ32Delta;
        const bool b_stable =
            absDiffInt64(candidate.b_ns, m_convergence.prev_candidate.b_ns) <=
            static_cast<uint64_t>(kStableBDeltaNs);

        if (a_stable && b_stable) {
            ++m_convergence.stable_update_count;
        } else {
            m_convergence.stable_update_count = 0;
        }
    }

    m_convergence.prev_candidate = candidate;
    m_convergence.has_prev_candidate = true;
    m_regression_para = candidate;

    if (m_convergence.stable_update_count >= kStableUpdatesRequired) {
        m_convergence.is_frozen = true;
    }
}
