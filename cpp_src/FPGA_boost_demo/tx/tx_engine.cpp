#include "tx_engine.h"

#include <cstdio>

namespace {

const char* readIntentActionName(OrderIntentAction action) {
    switch (action) {
        case OrderIntentAction::Buy:
            return "BUY";
        case OrderIntentAction::Sell:
            return "SELL";
        default:
            return "NONE";
    }
}

const char* readSymbolName(uint16_t stock_locate) {
    switch (stock_locate) {
        case 0x000d:
            return "AAPL";
        case 0x0ee8:
            return "HSBC";
        default:
            return "UNKNOWN";
    }
}

} // namespace

void TxEngine::sendIntent(const OrderIntent& intent) const {
    std::printf("TxStub sent action=%s symbol=%s stock_locate=0x%04x price=%u shares=%u\n",
                readIntentActionName(intent.intent.action),
                readSymbolName(intent.stock_locate),
                intent.stock_locate,
                intent.intent.price,
                intent.intent.shares);
    std::fflush(stdout);
}

