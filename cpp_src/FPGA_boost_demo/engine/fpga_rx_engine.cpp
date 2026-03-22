#include "fpga_rx_engine.h"
#include <stdexcept>
#include <algorithm>

FPGARxEngine::FPGARxEngine(FPGADev& device, uint16_t que_idx, std::unique_ptr<BasicStrategy> strategy)
    : m_decoder(device, que_idx), m_strategy(std::move(strategy)) {
    if (!m_decoder.isValid()) {
        throw std::runtime_error("Failed to initialize FPGARxDecoder in FPGARxEngine");
    }
}

const std::array<FPGAEventDesc, MAX_POLL_RECORDS>& FPGARxEngine::readEventBuffer() const {
    return m_event_buffer;
}

std::size_t FPGARxEngine::readLastRecordCount() const {
    return m_last_record_count;
}

void FPGARxEngine::_adjustMaxCount(std::size_t decoded_count) {
    constexpr std::size_t kMinPollCount = 8;

    if (decoded_count >= m_max_poll_count && m_max_poll_count < m_event_buffer.size()) {
        m_max_poll_count = std::min(m_event_buffer.size(), m_max_poll_count +4);
        return;
    }

    if (decoded_count <= (m_max_poll_count / 4) && m_max_poll_count > kMinPollCount) {
        m_max_poll_count = std::max(kMinPollCount, m_max_poll_count -4);
    }
}

void FPGARxEngine::poll() {
    const std::size_t count = m_decoder.decodeRawBatch(m_event_buffer.data(), m_max_poll_count);
    m_last_record_count = count;
    _adjustMaxCount(count);
    if (m_strategy && count != 0) {
    m_strategy->onEvents(m_event_buffer.data(), count);
    }
}

void FPGARxEngine::pollSync() {
    if (m_counter == 0 && m_period != 0) {
        m_get_time = true;
        m_counter = m_period-1;
    } else if (m_period != 0) {
        --m_counter;
        m_get_time = false;
    }
    const std::size_t count = m_decoder.decodeRawBatchSync(m_event_buffer.data(), 
                                                            m_last_sync_snapshot, 
                                                            m_get_time, 
                                                            m_max_poll_count);
    m_last_record_count = count;
    _updateRegression();
    _adjustMaxCount(count);
    if (m_strategy && count != 0) {
        m_strategy->onEvents(m_event_buffer.data(), count);
    }
}

void FPGARxEngine::adjust_sync_period(uint64_t period) {
    m_period = period;
}

void FPGARxEngine::_updateRegression() {
    if (m_get_time == false) {
        return;
    }
    if (m_last_sync_snapshot.interval_ns == 0) {
            return;
    }
    const uint64_t x = m_last_sync_snapshot.fpga_tick;
    const uint64_t y = m_last_sync_snapshot.host_time_ns;

    ++m_sync_acc.sample_count;
    m_sync_acc.m_sum_fpga_tick      += static_cast<__int128>(x);
    m_sync_acc.m_sum_host_time_ns   += static_cast<__int128>(y);
    m_sync_acc.m_sum_fpga_tick_sq   += static_cast<__int128>(x) * static_cast<__int128>(x);
    m_sync_acc.m_sum_fpga_host      += static_cast<__int128>(x) * static_cast<__int128>(y);
    if (m_sync_acc.sample_count < 2) {
        a_b.m_b_ns = static_cast<int64_t>(y) - static_cast<int64_t>(x);
        return;
    }
    const __int128 n = static_cast<__int128>(m_sync_acc.sample_count);
    const __int128 denom = n * m_sync_acc.m_sum_fpga_tick_sq - m_sync_acc.m_sum_fpga_tick * m_sync_acc.m_sum_fpga_tick;
    if (denom == 0) {
        return;
    }
    const __int128 numer =
        n * m_sync_acc.m_sum_fpga_host - m_sync_acc.m_sum_fpga_tick * m_sync_acc.m_sum_host_time_ns;
    // Q32 fixed-point slope
    a_b.m_a_q32 = static_cast<uint64_t>((numer << 32) / denom);
    const __int128 mean_x_q32 = (m_sync_acc.m_sum_fpga_tick << 32) / n;
    const __int128 mean_y_q32 = (m_sync_acc.m_sum_host_time_ns << 32) / n;
    const __int128 b_q32 = mean_y_q32 - (static_cast<__int128>(a_b.m_a_q32) * mean_x_q32 >> 32);
    a_b.m_b_ns = static_cast<int64_t>(b_q32 >> 32);
}
