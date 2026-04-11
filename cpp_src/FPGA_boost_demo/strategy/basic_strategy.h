#pragma once
#include "../common/shared_types.h"

class LatencyTracker;

class BasicStrategy {
public:
    virtual ~BasicStrategy() = default;
    void attachLatenyTracker(LatencyTracker* latency_tracker) {
        m_latency_tracker = latency_tracker;
    }

    virtual bool evaluateEvent(const FPGAEventDesc& event, OrderIntent& out_intent, uint16_t que_idx) = 0;

protected:
    LatencyTracker* m_latency_tracker {nullptr};
};
