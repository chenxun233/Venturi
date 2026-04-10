#include "../strategy/dummy_strategy.h"

#include <gtest/gtest.h>

TEST(DummyStrategyTest, evaluatesOneEventIntoAnIntentWithoutOwningExecutor) {
    DummyStrategy strategy;
    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.bid_price = 100;
    event.ask_price = 120;
    event.bid_shares = 2000;
    event.ask_shares = 100;

    OrderIntent intent {};
    ASSERT_TRUE(strategy.evaluateEvent(event, intent));
    EXPECT_EQ(intent.stock_locate, 0x000d);
}
