#pragma once
#include "../driver/basic_rx_source.h"
#include "../decoder/fpga_rx_decoder.h"
#include "../common/shared_types.h"
#include <array>
#include <cstddef>
#include <cstdint>

class FPGARxEngine {

public:
    FPGARxEngine(BasicRxSource& source, const FPGARxDecoder& decoder, uint16_t que_idx);

    std::size_t pollDecodedBatch(FirstEventMask& mask,
                                 std::size_t max_count);
    std::size_t pollDecodedBatchSync(FirstEventMask& mask,
                                     std::size_t max_count,
                                     bool get_time,
                                     FpgaSyncSnapshot& snapshot);
    const std::array<FPGAEventDesc, MAX_POLL_RECORDS>& readEventBuffer() const;


private:
    std::size_t pollDecodedBatchImpl(FirstEventMask& mask,
                                     std::size_t max_count,
                                     bool get_time,
                                     FpgaSyncSnapshot* snapshot);

    BasicRxSource& m_source;
    const FPGARxDecoder& m_decoder;
    uint16_t m_que_idx {0};
    uint64_t m_cons_ptr {0};
    std::array<FPGAEventDesc, MAX_POLL_RECORDS> m_event_buffer;


};
