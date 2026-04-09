#include "regression.h"

#include <cmath>

namespace {

constexpr std::size_t kMinFitSamples = 16;
constexpr std::size_t kRequiredStableUpdates = 4;
constexpr long double kConvergenceThresholdNsPerTick = 1e-10L;
constexpr long double kQ32Scale = static_cast<long double>(1ULL << 32);

} // namespace

void Regression::updateSnapshot(const FpgaSyncSnapshot& snapshot) {
    _updateModel(snapshot);
}

FpgaSyncSnapshot Regression::readSnapshot() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    return m_latest_snapshot;
}

RegressionPara Regression::returnParaSnapshot() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    return m_regression_para;
}

RegressionStatusLogRecord Regression::readStatusLogRecord() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    RegressionStatusLogRecord record {};
    record.has_para = m_regression_para.has_para;
    record.a_ns_per_tick = static_cast<double>(m_regression_para.a_q32) /
                           static_cast<double>(1ULL << 32);
    return record;
}

bool Regression::isFrozen() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    return m_is_frozen;
}

bool Regression::convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    if (!m_regression_para.has_para || m_latest_snapshot.interval_ns == 0) {
        return false;
    }

    const uint64_t delta_ns = (fpga_tick >= m_latest_snapshot.fpga_tick)
        ? _scaleTicksToNs(fpga_tick - m_latest_snapshot.fpga_tick, m_regression_para.a_q32)
        : _scaleTicksToNs(m_latest_snapshot.fpga_tick - fpga_tick, m_regression_para.a_q32);

    host_time_ns = (fpga_tick >= m_latest_snapshot.fpga_tick)
        ? m_latest_snapshot.host_time_ns + delta_ns
        : m_latest_snapshot.host_time_ns - delta_ns;

    return true;
}

void Regression::_updateModel(const FpgaSyncSnapshot& snapshot) {
    if (snapshot.interval_ns == 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    m_latest_snapshot = snapshot;
    if (m_is_frozen) {
        return;
    }

    ++m_snapshot_count;
    const long double fpga_tick = static_cast<long double>(snapshot.fpga_tick);
    const long double host_time_ns = static_cast<long double>(snapshot.host_time_ns);
    m_sum_fpga_ticks += fpga_tick;
    m_sum_host_time_ns += host_time_ns;
    m_sum_fpga_ticks_sq += fpga_tick * fpga_tick;
    m_sum_fpga_host_product += fpga_tick * host_time_ns;

    if (m_snapshot_count < 2) {
        return;
    }

    const long double sample_count = static_cast<long double>(m_snapshot_count);
    const long double denominator =
        sample_count * m_sum_fpga_ticks_sq - m_sum_fpga_ticks * m_sum_fpga_ticks;
    if (denominator <= 0.0L) {
        return;
    }

    const long double a_ns_per_tick =
        (sample_count * m_sum_fpga_host_product - m_sum_fpga_ticks * m_sum_host_time_ns) /
        denominator;
    const long double b_ns =
        (m_sum_host_time_ns - a_ns_per_tick * m_sum_fpga_ticks) / sample_count;
    if (a_ns_per_tick <= 0.0L) {
        return;
    }

    m_regression_para.has_para = true;
    m_regression_para.a_q32 = static_cast<uint64_t>(std::llround(a_ns_per_tick * kQ32Scale));
    m_regression_para.b_ns = static_cast<int64_t>(std::llround(b_ns));

    if (m_snapshot_count >= kMinFitSamples) {
        if (m_last_a_ns_per_tick > 0.0L &&
            std::fabs(a_ns_per_tick - m_last_a_ns_per_tick) <= kConvergenceThresholdNsPerTick) {
            ++m_stable_count;
        } else {
            m_stable_count = 0;
        }
        m_is_frozen = (m_stable_count >= kRequiredStableUpdates);
    }

    m_last_a_ns_per_tick = a_ns_per_tick;
}

uint64_t Regression::_scaleTicksToNs(uint64_t tick_delta, uint64_t a_q32) {
    return static_cast<uint64_t>(
        std::llround((static_cast<long double>(tick_delta) *
                      static_cast<long double>(a_q32)) /
                     kQ32Scale));
}
