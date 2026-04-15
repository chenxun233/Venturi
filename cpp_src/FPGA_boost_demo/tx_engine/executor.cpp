#include "executor.h"

#include "../common/time_utils.h"
#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"

#include <utility>

Executor::Executor(std::size_t buffer_capacity)
    : m_execution_buffer(buffer_capacity) {}

void Executor::attachLogPrinter(LogPrinter* log_printer) {
    m_log_printer = log_printer;
}

void Executor::attachQueueIdx(uint16_t queue_idx) {
    m_queue_idx = queue_idx;
    m_has_queue_idx = true;
}

void Executor::attachLatenyTracker(LatencyTracker* latency_tracker) {
    m_latency_tracker = latency_tracker;
}

bool Executor::acceptIntent(const OrderIntent& intent) {
    if (m_has_queue_idx && intent.que_idx != m_queue_idx) {
        return false;
    }
    const OrderExecution execution {
        .stock_locate = intent.stock_locate,
        .que_idx = intent.que_idx,
        .event_tag = intent.event_tag,
        .order = intent.intent,
    };

    const bool pushed = m_execution_buffer.push(execution);
    return pushed;
}

bool Executor::takeReadyExecution(OrderExecution& execution) {
    return m_execution_buffer.pop(execution);
}


