#pragma once

#include "../common/shared_types.h"
#include "../latency/trace_buffer.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class LogPrinter;

class Executor {
public:
    explicit Executor(uint16_t producer_num, std::size_t buffer_capacity = 1024);
    ~Executor() = default;
    void attachLogPrinter(LogPrinter* log_printer);
    bool acceptIntent(uint16_t producer_idx, const OrderIntent& intent);
    bool popReadyIntent(OrderIntent& intent);
    void logExecution(const OrderIntent& intent);
    void run(const std::atomic<bool>& running);
    void drain();

private:
    std::vector<std::unique_ptr<TraceBuffer<OrderIntent>>> m_intent_buffers;
    uint16_t m_producer_num {0};
    uint16_t m_next_buffer_idx {0};
    LogPrinter* m_log_printer {nullptr};
};
