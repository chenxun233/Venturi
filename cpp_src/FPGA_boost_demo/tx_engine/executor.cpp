#include "executor.h"

#include "../latency/latency_tracker.h"

#include <utility>

Executor::Executor(std::size_t buffer_capacity)
    : m_execution_buffer(buffer_capacity) {}

void Executor::attachQueueIdx(uint16_t queue_idx) {
    m_queue_idx = queue_idx;
    m_has_queue_idx = true;
}

void Executor::attachLatenyTracker(LatencyTracker* latency_tracker) {
    m_latency_tracker = latency_tracker;
}

bool Executor::acceptIntent(const OrderIntent& intent) {
    if (m_has_queue_idx && intent.que_idx != m_queue_idx) {
        if (intent.trace_id != 0U && m_latency_tracker != nullptr) {
            (void)m_latency_tracker->requestDrop(m_queue_idx, intent.trace_id);
        }
        return false;
    }
    const OrderExecution execution {
        .stock_locate = intent.stock_locate,
        .que_idx = intent.que_idx,
        .event_tag = intent.event_tag,
        .trace_id = intent.trace_id,
        .order = intent.intent,
    };

    const bool pushed = m_execution_buffer.push(execution);
    if (!pushed && intent.trace_id != 0U && m_latency_tracker != nullptr) {
        (void)m_latency_tracker->requestDrop(intent.que_idx, intent.trace_id);
    }
    return pushed;
}

bool Executor::popExecution(OrderExecution& execution) {
    return m_execution_buffer.pop(execution);
}
