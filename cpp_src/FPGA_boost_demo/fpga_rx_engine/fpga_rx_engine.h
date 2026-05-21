#pragma once
#include "../fpga_dev/fpga_dev.h"
#include "../common/shared_types.h"
#include <atomic>
#include <cstddef>
#include <cstdint>

class LatencyTracker;

class alignas(64) FPGARxEngine {

public:
    FPGARxEngine(FPGADev& source, uint16_t que_idx);

    std::size_t pollDecodedBatch(std::size_t max_count,
                                 FPGAEventDesc* out);
    // Non-owning; attach before polling; tracker must outlive engine; do not swap concurrently.
    void attachLatencyTracker(LatencyTracker* latency_tracker);
    uint64_t readDecodedCount() const noexcept;
    uint64_t readFirstEventCount() const noexcept;
    uint16_t readQueueIdx() const noexcept;


private:
    static void _decodeRawRecord(const uint8_t* record, FPGAEventDesc& event);
    std::size_t _pollDecodedBatchImpl(std::size_t max_count,
                                     bool emit_batch_start,
                                     FPGAEventDesc* out);

    FPGADev& m_device;
    uint16_t m_que_idx {0};
    uint64_t m_cons_ptr {0};
    std::atomic<uint64_t> m_decoded_count {0};
    std::atomic<uint64_t> m_first_event_count {0};
    // Attach before polling; tracker must outlive engine and not be swapped concurrently.
    LatencyTracker* p_latency_tracker {nullptr};
};
