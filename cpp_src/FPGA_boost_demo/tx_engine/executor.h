#pragma once

#include "../common/shared_types.h"
#include "../common/spsc_ring_queue.h"

#include <atomic>
#include <cstddef>
#include <cstdint>

class LatencyTracker;
class LogPrinter;

class Executor {
public:
    explicit Executor(std::size_t buffer_capacity = 1024);
    ~Executor() = default;
    void attachLogPrinter(LogPrinter* log_printer);
    void attachQueueIdx(uint16_t queue_idx);
    void attachLatenyTracker(LatencyTracker* latency_tracker);
    bool acceptIntent(const OrderIntent& intent);
    bool takeReadyExecution(OrderExecution& execution);



private:
    SpscRingQueue<OrderExecution> m_execution_buffer;
    LogPrinter* m_log_printer {nullptr};
    LatencyTracker* m_latency_tracker {nullptr};
    uint16_t m_queue_idx {0};
    bool m_has_queue_idx {false};
};
