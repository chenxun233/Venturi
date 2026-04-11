#include "dummy_strategy.h"

#include "../common/time_utils.h"
#include "../latency/latency_tracker.h"

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

bool DummyStrategy::evaluateEvent(const FPGAEventDesc& event, OrderIntent& out_intent, uint16_t que_idx) {
    const OrderIntentAction action = _readAction(event);
    if (action == OrderIntentAction::None) {
        return false;
    }

    out_intent = OrderIntent {
        .stock_locate = event.stock_locate,
        .que_idx = que_idx,
        .event_ts = event.event_tk,
        .intent = {
            .action = action,
            .price = (action == OrderIntentAction::Buy)
                ? ((event.bid_price != 0) ? event.bid_price : event.ask_price)
                : ((event.ask_price != 0) ? event.ask_price : event.bid_price),
            .shares = kIntentShares
        }
    };
    if (event.is_first_event != 0 &&
        m_latency_tracker != nullptr &&
        out_intent.event_ts != 0) {
        try {
            m_latency_tracker->pushRecord(TimeRecord {
                .que_idx = out_intent.que_idx,
                .event_ts = out_intent.event_ts,
                .event_stage = stage::STRATEGY,
                .time_captured = readMonotonicRawNs(),
            });
        } catch (...) {
        }
    }
    return true;
}
