#pragma once
#include "../common/shared_types.h"
#include "../tx/executor.h"

class BasicStrategy {
public:
    BasicStrategy() = default;
    virtual ~BasicStrategy() = default;

    void attachExecutor(Executor& executor, uint16_t producer_idx) {
        m_executor = &executor;
        m_producer_idx = producer_idx;
    }

    virtual void onEvents(const FPGAEventDesc* event,
                          std::size_t count) = 0;

protected:
    void _pushIntent(const OrderIntent& intent) const {
        if (m_executor != nullptr) {
            (void)m_executor->pushIntent(m_producer_idx, intent);
        }
    }

    Executor* m_executor {nullptr};
    uint16_t m_producer_idx {0};
};
