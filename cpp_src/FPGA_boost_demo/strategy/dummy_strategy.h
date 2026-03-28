#pragma once

#include "basic_strategy.h"

class DummyStrategy : public BasicStrategy {
public:
    DummyStrategy() = default;
    ~DummyStrategy() override = default;

    void onEvents(const FPGAEventDesc* event,
                  std::size_t count) override;

private:
    static constexpr uint32_t kMaxSpread = 100;
    static constexpr uint32_t kIntentShares = 100;
    static constexpr uint32_t kImbalanceRatio = 10;

    bool _shouldBuy(const FPGAEventDesc& event) const;
    bool _shouldSell(const FPGAEventDesc& event) const;
    OrderIntentAction _readAction(const FPGAEventDesc& event) const;
};
