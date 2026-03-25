#pragma once

#include "basic_strategy.h"

class PassiveStrategy : public BasicStrategy {
public:
    PassiveStrategy() = default;
    ~PassiveStrategy() override = default;

    void onEvents(const FPGAEventDesc* event, std::size_t count) override;
};
