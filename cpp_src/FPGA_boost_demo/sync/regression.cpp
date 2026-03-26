#include "regression.h"

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

bool Regression::isFrozen() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    return m_is_frozen;
}

bool Regression::convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    if (m_latest_snapshot.interval_ns == 0) {
        return false;
    }
    host_time_ns = (fpga_tick >= m_latest_snapshot.fpga_tick)
        ? static_cast<int64_t>(fpga_tick - m_latest_snapshot.fpga_tick) * 6.4 + m_latest_snapshot.host_time_ns
        : m_latest_snapshot.host_time_ns - static_cast<int64_t>(m_latest_snapshot.fpga_tick - fpga_tick) * 6.4 ;

    return true;
}

void Regression::_updateModel(const FpgaSyncSnapshot& snapshot) {
    if (snapshot.interval_ns == 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    if (m_is_frozen) {
        return;
    }
    ++m_snapshot_count;
    m_latest_snapshot = snapshot;
    m_regression_para.has_para = true;
    m_regression_para.a_q32 = kFixedAQ32;
    m_regression_para.b_ns =
        static_cast<int64_t>(snapshot.host_time_ns) -
        static_cast<int64_t>((snapshot.fpga_tick * kTickNumer) / kTickDenom);
    m_is_frozen = (m_snapshot_count >= 4);
    return;

}
