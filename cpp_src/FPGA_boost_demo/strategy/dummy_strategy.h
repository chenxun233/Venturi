#pragma once

#include "basic_strategy.h"

class DummyStrategy : public BasicStrategy {
public:
    DummyStrategy() = default;
    ~DummyStrategy() override = default;

    bool evaluateEvent(const FPGAEventDesc& event, OrderIntent& out_intent, uint16_t que_idx) override;

private:
    static constexpr uint32_t kMaxSpread = 100;
    static constexpr uint32_t kIntentShares = 100;
    static constexpr uint32_t kImbalanceRatio = 10;

    bool _shouldBuy(const FPGAEventDesc& event) const;
    bool _shouldSell(const FPGAEventDesc& event) const;
    OrderIntentAction _readAction(const FPGAEventDesc& event) const;
};
