#pragma once
#include "../driver/basic_rx_source.h"
#include "../decoder/fpga_rx_decoder.h"
#include "../common/shared_types.h"
#include <cstddef>
#include <cstdint>
#include <vector>

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
    // Non-owning; attach before polling; tracker must outlive engine; do not swap concurrently.
    void attachLatenyTracker(LatencyTracker* latency_tracker);


private:
    uint32_t _allocateTraceId() noexcept;
    std::size_t pollDecodedBatchImpl(std::size_t max_count,
                                     bool get_snapshot,
                                     bool emit_batch_start,
                                     FpgaSyncSnapshot* snapshot,
                                     std::vector<uint32_t>* trace_ids,
                                     FPGAEventDesc* out);

    BasicRxDev& m_device;
    const FPGARxDecoder& m_decoder;
    uint16_t m_que_idx {0};
    uint64_t m_cons_ptr {0};
    uint32_t m_next_trace_id {1};
    // Attach before polling; tracker must outlive engine and not be swapped concurrently.
    LatencyTracker* m_latency_tracker {nullptr};
};
