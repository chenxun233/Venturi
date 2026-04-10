#pragma once
#include "../common/shared_types.h"

class BasicStrategy {
public:
    virtual ~BasicStrategy() = default;

    virtual bool evaluateEvent(const FPGAEventDesc& event, OrderIntent& out_intent) = 0;
};
