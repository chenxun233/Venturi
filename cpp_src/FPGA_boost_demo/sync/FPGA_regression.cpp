#include "FPGA_regression.h"

#include <cmath>

namespace {

constexpr long double kQ32Scale = static_cast<long double>(1ULL << 32);

} // namespace

FPGARegression::FPGARegression(uint64_t trigger_period,
                               std::size_t required_stable_updates,
                               long double convergence_threshold_ns_per_tick)
    : m_required_stable_updates(required_stable_updates),
      m_convergence_threshold_ns_per_tick(convergence_threshold_ns_per_tick),
      m_convergence_threshold_q32(static_cast<uint64_t>(
          std::llround(convergence_threshold_ns_per_tick * kQ32Scale))),
      m_trigger_period(trigger_period),
      m_trigger_countdown(0) {
    m_frozen_regression_para.has_para = false;
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

    std::lock_guard<std::mutex> lock(m_regression_mutex);
    if (m_is_frozen) {
        m_latest_snapshot = snapshot;
        return true;
    }

    m_pre_snapshot = m_cur_snapshot;
    m_cur_snapshot = snapshot;
    return true;
}

void FPGARegression::updateRegression() {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    if (m_is_frozen) {
        return;
    }

    const uint64_t tick_delta = m_cur_snapshot.fpga_tick - m_pre_snapshot.fpga_tick;
    const uint64_t host_time_delta_ns = m_cur_snapshot.host_time_ns - m_pre_snapshot.host_time_ns;
    if (tick_delta == 0 || m_pre_snapshot.interval_ns == 0) {
        return;
    }

    const uint64_t raw_a_q32 = _computeRawAQ32(tick_delta, host_time_delta_ns);
    if (!m_has_prev_raw_a) {
        m_prev_raw_a_q32 = raw_a_q32;
        m_has_prev_raw_a = true;
        m_stable_update_count = 1;
    } else {
        const uint64_t delta_q32 =
            (raw_a_q32 >= m_prev_raw_a_q32) ? (raw_a_q32 - m_prev_raw_a_q32)
                                            : (m_prev_raw_a_q32 - raw_a_q32);
        m_stable_update_count =
            (delta_q32 <= m_convergence_threshold_q32) ? (m_stable_update_count + 1U) : 1U;
        m_prev_raw_a_q32 = raw_a_q32;
    }

    if (m_stable_update_count >= m_required_stable_updates) {
        m_frozen_regression_para.has_para = true;
        m_frozen_regression_para.a_q32 = raw_a_q32;
        m_is_frozen = true;
        m_latest_snapshot = m_cur_snapshot;
    }
}

RegressionStatusLogRecord FPGARegression::readStatusLogRecord() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    RegressionStatusLogRecord record {};
    record.has_para = m_frozen_regression_para.has_para;
    record.a_ns_per_tick = static_cast<double>(m_frozen_regression_para.a_q32) / kQ32Scale;
    return record;
}

bool FPGARegression::isFrozen() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    return m_is_frozen;
}

bool FPGARegression::convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    if (!m_frozen_regression_para.has_para || m_latest_snapshot.interval_ns == 0) {
        return false;
    }

    if (fpga_tick >= m_latest_snapshot.fpga_tick) {
        const uint64_t delta_ticks = fpga_tick - m_latest_snapshot.fpga_tick;
        host_time_ns =
            m_latest_snapshot.host_time_ns +
            _scaleTicksToNs(delta_ticks, m_frozen_regression_para.a_q32);
        return true;
    }

    const uint64_t delta_ticks = m_latest_snapshot.fpga_tick - fpga_tick;
    const uint64_t delta_ns = _scaleTicksToNs(delta_ticks, m_frozen_regression_para.a_q32);
    host_time_ns = m_latest_snapshot.host_time_ns - delta_ns;
    return true;
}

uint64_t FPGARegression::_computeRawAQ32(uint64_t tick_delta, uint64_t host_time_delta_ns) {
    return static_cast<uint64_t>(
        std::llround((static_cast<long double>(host_time_delta_ns) * kQ32Scale) /
                     static_cast<long double>(tick_delta)));
}

uint64_t FPGARegression::_scaleTicksToNs(uint64_t tick_delta, uint64_t a_q32) {
    return static_cast<uint64_t>(
        std::llround((static_cast<long double>(tick_delta) * static_cast<long double>(a_q32)) /
                     kQ32Scale));
}
