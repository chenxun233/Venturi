#pragma once
#include "../driver/basic_rx_source.h"
#include "../decoder/fpga_rx_decoder.h"
#include "../common/shared_types.h"
#include <cstddef>
#include <cstdint>

class LatencyTracker;
class FPGARegression;

class FPGARxEngine {

public:
    FPGARxEngine(BasicRxDev& source, const FPGARxDecoder& decoder, uint16_t que_idx);
    void attachLatencyTracker(LatencyTracker& latency_tracker) { m_latency_tracker = &latency_tracker; }
    void attachRegression(FPGARegression& regression) { m_regression = &regression; }

    std::size_t pollDecodedBatch(FirstEventMask& mask,
                                 std::size_t max_count,
                                DecodedEvent* out);
    std::size_t pollDecodedBatchSync(FirstEventMask& mask,
                                     std::size_t max_count,
                                     bool get_snapshot,
                                     FpgaSyncSnapshot* snapshot,
                                    DecodedEvent* out);


private:
    std::size_t pollDecodedBatchImpl(FirstEventMask& mask,
                                     std::size_t max_count,
                                     bool get_snapshot,
                                     FpgaSyncSnapshot* snapshot,
                                     DecodedEvent* out);

    BasicRxDev& m_source;
    const FPGARxDecoder& m_decoder;
    LatencyTracker* m_latency_tracker {nullptr};
    FPGARegression* m_regression {nullptr};
    uint16_t m_que_idx {0};
    uint64_t m_cons_ptr {0};


};
