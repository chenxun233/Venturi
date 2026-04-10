#pragma once

#include "basic_strategy.h"

class PassiveStrategy : public BasicStrategy {
public:
    PassiveStrategy() = default;
    ~PassiveStrategy() override = default;

    bool evaluateEvent(const FPGAEventDesc& event, OrderIntent& out_intent) override;
};
