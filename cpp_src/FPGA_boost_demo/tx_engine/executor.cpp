#include "executor.h"

#include "../common/time_utils.h"
#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"

#include <stdexcept>

Executor::Executor(uint16_t producer_num, std::size_t buffer_capacity)
    : m_producer_num(producer_num) {
    if (m_producer_num == 0) {
        throw std::invalid_argument("Executor producer number must be non-zero");
    }
    m_intent_buffers.reserve(producer_num);
    for (uint16_t producer_idx = 0; producer_idx < producer_num; ++producer_idx) {
        m_intent_buffers.push_back(std::make_unique<TraceBuffer<OrderIntent>>(buffer_capacity));
    }
}

void Executor::attachLogPrinter(LogPrinter* log_printer) {
    m_log_printer = log_printer;
}

void Executor::attachLatenyTracker(LatencyTracker* latency_tracker) {
    m_latency_tracker = latency_tracker;
}

bool Executor::acceptIntent(uint16_t producer_idx, const OrderIntent& intent) {
    if (producer_idx >= m_intent_buffers.size()) {
        throw std::out_of_range("Executor producer index out of range");
    }
    const bool pushed = m_intent_buffers[producer_idx]->push(intent);
    if (pushed && intent.event_ts != 0 && m_latency_tracker != nullptr) {
        m_latency_tracker->pushRecord(TimeRecord {
            .que_idx = intent.que_idx,
            .event_ts = intent.event_ts,
            .event_stage = stage::EXECUTOR,
            .time_captured = readMonotonicRawNs(),
        });
    }
    return pushed;
}

bool Executor::popReadyIntent(OrderIntent& intent) {
    for (uint16_t offset = 0; offset < m_producer_num; ++offset) {
        const uint16_t buffer_idx = static_cast<uint16_t>((m_next_buffer_idx + offset) % m_producer_num);
        if (!m_intent_buffers[buffer_idx]->pop(intent)) {
            continue;
        }
        m_next_buffer_idx = static_cast<uint16_t>((buffer_idx + 1) % m_producer_num);
        return true;
    }
    return false;
}

void Executor::logExecution(const OrderIntent& intent) {
    if (m_log_printer == nullptr) {
        return;
    }

    m_log_printer->pushExecution(ExecutionLogRecord {
        .stock_locate = intent.stock_locate,
        .intent = intent.intent
    });
}

void Executor::drain() {
    OrderIntent intent {};
    while (popReadyIntent(intent)) {
        logExecution(intent);
    }
}

void Executor::run(const std::atomic<bool>& running) {
    while (running.load(std::memory_order_acquire)) {
        drain();
    }
    drain();
}
