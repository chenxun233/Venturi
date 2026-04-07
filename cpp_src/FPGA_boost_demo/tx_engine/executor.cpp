#include "executor.h"

#include "../latency/latency_log_printer.h"
#include "tx_translator.h"

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

void Executor::attachLogPrinter(LatencyLogPrinter* log_printer) {
    m_log_printer = log_printer;
}

void Executor::attachTranslator(TxTranslator* translator) {
    m_translator = translator;
}

bool Executor::pushIntent(uint16_t producer_idx, const OrderIntent& intent) {
    if (producer_idx >= m_intent_buffers.size()) {
        throw std::out_of_range("Executor producer index out of range");
    }
    return m_intent_buffers[producer_idx]->push(intent);
}

void Executor::drain() {
    OrderIntent intent {};
    for (uint16_t producer_idx = 0; producer_idx < m_producer_num; ++producer_idx) {
        while (m_intent_buffers[producer_idx]->pop(intent)) {
            _executeIntent(intent);
        }
    }
}

void Executor::run(const std::atomic<bool>& running) {
    while (running.load(std::memory_order_acquire)) {
        drain();
    }
    drain();
}

void Executor::_executeIntent(const OrderIntent& intent) {
    if (m_log_printer != nullptr) {
        m_log_printer->pushExecution(ExecutionLogRecord {
            .stock_locate = intent.stock_locate,
            .intent = intent.intent
        });
    }

    if (m_translator == nullptr) {
        return;
    }

    (void)m_translator->pushIntent(intent);
}
