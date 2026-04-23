#pragma once

#include "../common/shared_types.h"
#include "../common/spsc_ring_queue.h"

#include <atomic>
#include <cstddef>
#include <cstdint>

class LatencyTracker;

class Executor {
public:
    explicit Executor(std::size_t buffer_capacity = 1024);
    ~Executor() = default;
    void attachQueueIdx(uint16_t que_idx);
    void attachLatenyTracker(LatencyTracker* latency_tracker);
    bool acceptIntent(const OrderIntent& intent);
    bool popExecution(OrderExecution& execution);

private:
    SpscRingBuffer<OrderExecution> m_execution_buffer;
    LatencyTracker* p_latency_tracker {nullptr};
    uint16_t m_queue_idx {0};
    bool m_has_queue_idx {false};
};
