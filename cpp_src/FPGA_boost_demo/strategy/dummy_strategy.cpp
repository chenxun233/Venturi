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

bool DummyStrategy::evaluateEvent(const FPGAEventDesc& event, OrderIntent& out_intent) {
    const OrderIntentAction action = _readAction(event);
    if (action == OrderIntentAction::None) {
        return false;
    }

    out_intent = OrderIntent {
        .stock_locate = event.stock_locate,
        .que_idx = 0,
        .event_ts = event.event_tk,
        .intent = {
            .action = action,
            .price = (action == OrderIntentAction::Buy)
                ? ((event.bid_price != 0) ? event.bid_price : event.ask_price)
                : ((event.ask_price != 0) ? event.ask_price : event.bid_price),
            .shares = kIntentShares
        }
    };
    return true;
}
