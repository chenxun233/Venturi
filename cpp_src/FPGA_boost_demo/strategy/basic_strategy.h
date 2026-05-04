#pragma once
#include "../common/shared_types.h"

class LatencyTracker;

class BasicStrategy {
public:
    virtual ~BasicStrategy() = default;
    void attachLatencyTracker(LatencyTracker* latency_tracker) {
        p_latency_tracker = latency_tracker;
    }

    virtual bool evaluateEvent(uint16_t que_idx,const FPGAEventDesc& event, OrderIntent& out_intent) = 0;

protected:
    LatencyTracker* p_latency_tracker {nullptr};
};
