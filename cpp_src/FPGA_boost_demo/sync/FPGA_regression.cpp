#include "FPGA_regression.h"

#include <cmath>

namespace {

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

    const uint64_t tick_delta = m_cur_snapshot.fpga_tick - m_pre_snapshot.fpga_tick;
    const uint64_t host_time_delta_ns = m_cur_snapshot.host_time_ns - m_pre_snapshot.host_time_ns;
    if (tick_delta == 0) {
        return;
    }

    const int64_t delta_a_q32 =
        static_cast<int64_t>(m_cur_regression_para.a_q32) -
        static_cast<int64_t>(m_pre_regression_para.a_q32);
    if (std::abs(delta_a_q32) < kConvergenceThresholdNsPerTick * kQ32Scale &&
        delta_a_q32 != 0) {
        m_cur_regression_para.has_para = true;
        m_is_frozen = true;
    } else {
        const uint64_t new_a_q32 = static_cast<uint64_t>(
            std::llround((static_cast<long double>(host_time_delta_ns) * kQ32Scale) /
                         static_cast<long double>(tick_delta)));
        m_pre_regression_para.a_q32 = m_cur_regression_para.a_q32;
        m_cur_regression_para.a_q32 = (new_a_q32 + m_cur_regression_para.a_q32 * 7) >> 3;
        m_cur_regression_para.has_para = false;
    }

    ++m_snapshot_count;
    if (m_snapshot_count >= kRequiredStableUpdates && m_cur_regression_para.has_para) {
        m_is_frozen = true;
    }
}

uint64_t FPGARegression::_scaleTicksToNs(uint64_t tick_delta, uint64_t a_q32) {
    return static_cast<uint64_t>(
        std::llround((static_cast<long double>(tick_delta) * static_cast<long double>(a_q32)) /
                     kQ32Scale));
}
