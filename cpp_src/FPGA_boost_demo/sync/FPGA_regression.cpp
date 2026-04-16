#include "FPGA_regression.h"

#include <cmath>

namespace {

constexpr std::size_t kMinFitSamples = 16;
constexpr std::size_t kRequiredStableUpdates = 4;
constexpr long double kConvergenceThresholdNsPerTick = 1e-10L;
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
    if (m_is_frozen) {
        _publishState();
    }
    return true;
}

RegressionStatusLogRecord FPGARegression::readStatusLogRecord() const {
    const auto published = m_published_state.load();
    return RegressionStatusLogRecord {
        .has_para = published->regression_para.has_para,
        .a_ns_per_tick =
            static_cast<double>(published->regression_para.a_q32) /
            static_cast<double>(kQ32Scale)
    };
}

bool FPGARegression::isFrozen() const {
    return m_published_state.load()->is_frozen;
}

bool FPGARegression::convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const {
    const auto published = m_published_state.load();
    if (!published->regression_para.has_para || published->anchor_snapshot.interval_ns == 0) {
        return false;
    }
    if (fpga_tick < published->anchor_snapshot.fpga_tick) {
        return false;
    }

    const uint64_t delta_ns =
        _scaleTicksToNs(fpga_tick - published->anchor_snapshot.fpga_tick,
                        published->regression_para.a_q32);
    host_time_ns = published->anchor_snapshot.host_time_ns + delta_ns;
    return true;
}

void FPGARegression::_update_a() {
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

    m_regression_para.a_q32 = fitted_a_q32;
    m_regression_para.has_para = true;
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
    _publishState();
}

uint64_t FPGARegression::_scaleTicksToNs(uint64_t tick_delta, uint64_t a_q32) {
    return static_cast<uint64_t>(
        std::llround((static_cast<long double>(tick_delta) * static_cast<long double>(a_q32)) /
                     kQ32Scale));
}

void FPGARegression::_publishState() {
    m_published_state.publish(RegressionPublishedState {
        .regression_para = m_regression_para,
        .is_frozen = m_is_frozen,
        .anchor_snapshot = m_cur_snapshot
    });
}
