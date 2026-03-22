#pragma once

#include "../driver/basic_rx_source.h"

struct FPGAEventDesc {
    uint32_t ask_price              {0};
    uint32_t ask_shares             {0};
    uint32_t bid_price              {0};
    uint32_t bid_shares             {0};
    uint64_t frame_start_tk         {0};
    uint64_t event_logic_latency_tk {0};
    uint16_t stock_locate           {0};
};

struct FpgaSyncSnapshot {
    uint64_t fpga_tick      {0};
    uint64_t host_time_ns   {0};
    uint64_t interval_ns    {0};
};

// Translation-only decoder above FPGADev. Callers own any persistent buffering or analysis state.
class FPGARxDecoder {
public:
    std::size_t decodeRawBatch(FPGAEventDesc* out, std::size_t max_count);
    std::size_t decodeRawBatchSync(FPGAEventDesc* out, 
                                    FpgaSyncSnapshot& snapshot, bool get_time,
                                    std::size_t max_count);
    bool isValid() const { return m_source.isValid(); }
private:
    friend class FPGARxEngine;
    explicit    FPGARxDecoder(BasicRxSource& source, uint16_t que_idx);
    void        _decodeRawRecord(const uint8_t* raw_bytes, FPGAEventDesc& out);
    

    BasicRxSource& m_source;
    uint64_t m_cons_ptr;
    uint16_t m_que_idx;
};
