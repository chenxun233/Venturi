#pragma once
#include "../driver/basic_rx_source.h"
#include "../decoder/fpga_rx_decoder.h"
#include "../common/shared_types.h"
#include <atomic>
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
    // Non-owning; attach before polling; tracker must outlive engine; do not swap concurrently.
    void attachLatencyTracker(LatencyTracker* latency_tracker);
    uint64_t readDecodedCount() const noexcept;
    uint64_t readFirstEventCount() const noexcept;
    uint16_t readQueueIdx() const noexcept;


private:
    std::size_t pollDecodedBatchImpl(std::size_t max_count,
                                     bool get_snapshot,
                                     bool emit_batch_start,
                                     FpgaSyncSnapshot* snapshot,
                                     FPGAEventDesc* out);

    BasicRxDev& m_device;
    const FPGARxDecoder& m_decoder;
    uint16_t m_que_idx {0};
    uint64_t m_cons_ptr {0};
    std::atomic<uint64_t> m_decoded_count {0};
    std::atomic<uint64_t> m_first_event_count {0};
    // Attach before polling; tracker must outlive engine and not be swapped concurrently.
    LatencyTracker* p_latency_tracker {nullptr};
};
