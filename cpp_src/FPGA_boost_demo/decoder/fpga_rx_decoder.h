#pragma once

#include "../driver/basic_rx_source.h"
#include "../common/shared_types.h"





// Translation-only decoder above FPGADev. Callers own any persistent buffering or analysis state.
class FPGARxDecoder {
public:
    explicit FPGARxDecoder(BasicRxSource& source, uint16_t que_idx);
    std::size_t decodeRawBatch(FirstEventMask& mask,
                               QueuePollLogRecord* queue_poll,
                               FPGAEventDesc* out,
                               std::size_t max_count);
    std::size_t decodeRawBatchSync(FirstEventMask& mask,
                                   QueuePollLogRecord* queue_poll,
                                   FPGAEventDesc* out,
                                   FpgaSyncSnapshot& snapshot,
                                   bool get_time,
                                   std::size_t max_count);
    bool isValid() const { return m_source.isValid(); }
private:
    friend class FPGARxEngine;
    void        _decodeRawRecord(const uint8_t* raw_bytes, FPGAEventDesc& out);
    
    BasicRxSource& m_source;
    uint64_t m_cons_ptr;
    uint16_t m_que_idx;
};
