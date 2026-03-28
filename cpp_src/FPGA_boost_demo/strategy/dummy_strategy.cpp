#include "dummy_strategy.h"

bool DummyStrategy::_shouldBuy(const FPGAEventDesc& event) const {
    if (event.bid_price == 0 && event.bid_shares == 0 && event.ask_price != 0 && event.ask_shares != 0) {
        return true;
    }
    if (event.bid_price == 0 || event.ask_price == 0 || event.bid_shares == 0 || event.ask_shares == 0) {
        return false;
    }
    if (event.ask_price <= event.bid_price) {
        return false;
    }
    if ((event.ask_price - event.bid_price) > kMaxSpread) {
        return false;
    }
    return event.bid_shares >= (event.ask_shares * kImbalanceRatio);
}

bool DummyStrategy::_shouldSell(const FPGAEventDesc& event) const {
    if (event.ask_price == 0 && event.ask_shares == 0 && event.bid_price != 0 && event.bid_shares != 0) {
        return true;
    }
    if (event.bid_price == 0 || event.ask_price == 0 || event.bid_shares == 0 || event.ask_shares == 0) {
        return false;
    }
    if (event.ask_price <= event.bid_price) {
        return false;
    }
    if ((event.ask_price - event.bid_price) > kMaxSpread) {
        return false;
    }
    return event.ask_shares >= (event.bid_shares * kImbalanceRatio);
}

OrderIntentAction DummyStrategy::_readAction(const FPGAEventDesc& event) const {
    if (_shouldBuy(event)) {
        return OrderIntentAction::Buy;
    }
    if (_shouldSell(event)) {
        return OrderIntentAction::Sell;
    }
    return OrderIntentAction::None;
}

void DummyStrategy::onEvents(const FPGAEventDesc* event,
                             std::size_t count) {
    for (std::size_t idx = 0; idx < count; ++idx) {
        const FPGAEventDesc& current = event[idx];
        const OrderIntentAction action = _readAction(current);
        if (action == OrderIntentAction::None) {
            continue;
        }
        if (action == OrderIntentAction::Buy) {
            _pushIntent(OrderIntent {
                .stock_locate = current.stock_locate,
                .intent = {
                    .action = action,
                    .price = (current.bid_price != 0) ? current.bid_price : current.ask_price,
                    .shares = kIntentShares
                }
            });
            continue;
        }
        if (action == OrderIntentAction::Sell) {
            _pushIntent(OrderIntent {
                .stock_locate = current.stock_locate,
                .intent = {
                    .action = action,
                    .price = (current.ask_price != 0) ? current.ask_price : current.bid_price,
                    .shares = kIntentShares
                }
            });
        }
    }
}
