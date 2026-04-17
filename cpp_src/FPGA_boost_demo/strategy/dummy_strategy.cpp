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

bool DummyStrategy::evaluateEvent(uint16_t que_idx,const FPGAEventDesc& event, OrderIntent& out_intent) {
    if (event.is_first_event != 0 &&
        m_latency_tracker != nullptr &&
        event.event_tk != 0) {
        try {
            m_latency_tracker->pushRecord(TimeRecord {
                .que_idx = que_idx,
                .event_tag = event.event_tk,
                .event_stage = stage::STRATEGY_START,
                .time_captured = readMonotonicRawNs(),
            });
        } catch (...) {
            // Latency tracking is best-effort and must not change strategy output.
        }
    }

    const OrderIntentAction action = _readAction(event);
    if (action == OrderIntentAction::None) {
        return false;
    }

    out_intent = OrderIntent {
        .stock_locate = event.stock_locate,
        .que_idx = que_idx,
        .event_tag = event.event_tk,
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
