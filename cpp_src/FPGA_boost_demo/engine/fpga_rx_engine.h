#pragma once
#include "../driver/fpga_dev.h"
#include "../decoder/fpga_rx_decoder.h"
#include "../strategy/basic_strategy.h"
#include <array>
#include <cstddef>
#include <memory>

#define MAX_POLL_RECORDS 32

class FPGARxEngine{

public:
    explicit    FPGARxEngine(FPGADev& device, uint16_t que_idx, std::unique_ptr<BasicStrategy> strategy = nullptr);
    void        poll();
    void        pollSync();
    void        adjust_sync_period(uint64_t period);
    const std::array<FPGAEventDesc, MAX_POLL_RECORDS>& readEventBuffer() const;
    std::size_t readLastRecordCount() const;

private:

struct SyncSampleAcc{
uint64_t sample_count {0};
__int128 m_sum_fpga_tick {0};
__int128 m_sum_host_time_ns {0};
__int128 m_sum_fpga_tick_sq {0};
__int128 m_sum_fpga_host {0};
};

struct RegressionResult{
uint64_t m_a_q32 {1ULL << 32};
int64_t  m_b_ns {0};
};

    void        _adjustMaxCount(std::size_t decoded_count);
    void        _updateRegression();

    FPGARxDecoder       m_decoder;
    std::unique_ptr<BasicStrategy> m_strategy;
    std::array<FPGAEventDesc, MAX_POLL_RECORDS> m_event_buffer;
    std::size_t         m_last_record_count {0};
    std::size_t         m_max_poll_count {8};
    bool                m_get_time {false};
    FpgaSyncSnapshot    m_last_sync_snapshot {};
    uint64_t            m_counter {0};
    uint64_t            m_period {0};
    SyncSampleAcc       m_sync_acc {};
    RegressionResult    a_b {};

};
