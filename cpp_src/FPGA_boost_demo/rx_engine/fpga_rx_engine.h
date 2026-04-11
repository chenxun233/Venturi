#pragma once
#include "../driver/basic_rx_source.h"
#include "../decoder/fpga_rx_decoder.h"
#include "../common/shared_types.h"
#include <cstddef>
#include <cstdint>

class LatencyTracker;

class FPGARxEngine {

public:
    FPGARxEngine(BasicRxDev& source, const FPGARxDecoder& decoder, uint16_t que_idx);

    std::size_t pollDecodedBatch(std::size_t max_count,
                                 FPGAEventDesc* out);
    std::size_t pollDecodedBatchSync(std::size_t max_count,
                                     bool get_snapshot,
                                     FpgaSyncSnapshot* snapshot,
                                     FPGAEventDesc* out);
    void attachLatenyTracker(LatencyTracker* latency_tracker);


private:
    std::size_t pollDecodedBatchImpl(std::size_t max_count,
                                     bool get_snapshot,
                                     FpgaSyncSnapshot* snapshot,
                                     FPGAEventDesc* out);

    BasicRxDev& m_device;
    const FPGARxDecoder& m_decoder;
    uint16_t m_que_idx {0};
    uint64_t m_cons_ptr {0};
    // Attach before polling; tracker must outlive engine and not be swapped concurrently.
    LatencyTracker* m_latency_tracker {nullptr};
};
