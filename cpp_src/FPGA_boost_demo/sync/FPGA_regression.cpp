#include "FPGA_regression.h"

#include <cmath>

namespace {

constexpr std::size_t kMinFitSamples = 16;
constexpr std::size_t kRequiredStableUpdates = 4;
constexpr long double kConvergenceThresholdNsPerTick = 1e-8L;
constexpr long double kQ32Scale = static_cast<long double>(1ULL << 32);

} // namespace

FPGARegression::FPGARegression(uint64_t trigger_period)
    : m_trigger_period(trigger_period),
      m_trigger_countdown(0) {
}

void FPGARegression::run(CapSignal& cap_signal) {
    if (m_trigger_period == 0) {
        return;
    }
    if (m_trigger_countdown == 0) {
        cap_signal.request.store(true, std::memory_order_release);
        m_trigger_countdown = m_trigger_period - 1;
        return;
    }
    --m_trigger_countdown;
}

bool FPGARegression::tryAcceptSnapshot(const FpgaSyncSnapshot& snapshot,
                                       uint64_t accepted_interval_ns) {
    if (snapshot.interval_ns == 0 || snapshot.interval_ns > accepted_interval_ns) {
        return false;
    }
    m_pre_snapshot = m_cur_snapshot;
    m_cur_snapshot = snapshot;
    return true;
}

RegressionStatusLogRecord FPGARegression::readStatusLogRecord() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    RegressionStatusLogRecord record {};
    record.has_para = m_cur_regression_para.has_para;
    record.a_ns_per_tick = static_cast<double>(m_cur_regression_para.a_q32) / kQ32Scale;
    return record;
}

bool FPGARegression::isFrozen() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    return m_is_frozen;
}

bool FPGARegression::convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    if (!m_cur_regression_para.has_para || m_cur_snapshot.interval_ns == 0) {
        return false;
    }

    const uint64_t delta_ns =
        _scaleTicksToNs(fpga_tick - m_cur_snapshot.fpga_tick, m_cur_regression_para.a_q32);
    host_time_ns = m_cur_snapshot.host_time_ns + delta_ns;
    return true;
}

void FPGARegression::_update_a() {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    if (m_is_frozen) {
        return;
    }

    const long double fpga_tick = static_cast<long double>(m_cur_snapshot.fpga_tick);
    const long double host_time_ns = static_cast<long double>(m_cur_snapshot.host_time_ns);
    m_sum_fpga_tick += fpga_tick;
    m_sum_host_time_ns += host_time_ns;
    m_sum_fpga_tick_sq += fpga_tick * fpga_tick;
    m_sum_fpga_host_product += fpga_tick * host_time_ns;
    ++m_sample_count;

    if (m_sample_count < 2) {
        return;
    }

    const long double n = static_cast<long double>(m_sample_count);
    const long double denominator =
        n * m_sum_fpga_tick_sq - m_sum_fpga_tick * m_sum_fpga_tick;
    if (denominator <= 0.0L) {
        return;
    }

    const long double fitted_a_ns_per_tick =
        (n * m_sum_fpga_host_product - m_sum_fpga_tick * m_sum_host_time_ns) / denominator;
    const long double fitted_b_ns =
        (m_sum_host_time_ns - fitted_a_ns_per_tick * m_sum_fpga_tick) / n;
    if (fitted_a_ns_per_tick <= 0.0L) {
        return;
    }

    const uint64_t fitted_a_q32 = static_cast<uint64_t>(
        std::llround(fitted_a_ns_per_tick * kQ32Scale));

    m_cur_regression_para.a_q32 = fitted_a_q32;
    m_cur_regression_para.has_para = true;
    m_fitted_b_ns = fitted_b_ns;

    if (m_sample_count >= kMinFitSamples) {
        if (m_has_prev_fitted_a) {
            const uint64_t delta_a_q32 =
                (fitted_a_q32 >= m_prev_fitted_a_q32)
                    ? (fitted_a_q32 - m_prev_fitted_a_q32)
                    : (m_prev_fitted_a_q32 - fitted_a_q32);

            if (delta_a_q32 <= static_cast<uint64_t>(
                                   std::llround(kConvergenceThresholdNsPerTick * kQ32Scale))) {
                ++m_converge_count;
            } else {
                m_converge_count = 0;
            }
        }

        if (m_converge_count >= kRequiredStableUpdates) {
            m_is_frozen = true;
        }
    }

    m_prev_fitted_a_q32 = fitted_a_q32;
    m_has_prev_fitted_a = true;
}

uint64_t FPGARegression::_scaleTicksToNs(uint64_t tick_delta, uint64_t a_q32) {
    return static_cast<uint64_t>(
        std::llround((static_cast<long double>(tick_delta) * static_cast<long double>(a_q32)) /
                     kQ32Scale));
}
