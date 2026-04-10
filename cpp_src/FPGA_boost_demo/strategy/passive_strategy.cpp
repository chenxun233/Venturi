#include "passive_strategy.h"

bool PassiveStrategy::evaluateEvent(const FPGAEventDesc& event, OrderIntent& out_intent) {
    (void)event;
    (void)out_intent;
    return false;
}
