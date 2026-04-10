#include "../strategy/dummy_strategy.h"

#include <gtest/gtest.h>

TEST(DummyStrategyTest, evaluatesOneEventIntoAnIntentWithoutOwningExecutor) {
    DummyStrategy strategy;
    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.event_tk = 12345U;
    event.bid_price = 100;
    event.ask_price = 120;
    event.bid_shares = 2000;
    event.ask_shares = 100;

    OrderIntent intent {};
    ASSERT_TRUE(strategy.evaluateEvent(event, intent));
    EXPECT_EQ(intent.stock_locate, 0x000d);
    EXPECT_EQ(intent.event_ts, 12345U);
    EXPECT_EQ(intent.intent.action, OrderIntentAction::Buy);
    EXPECT_EQ(intent.intent.price, 100U);
    EXPECT_EQ(intent.intent.shares, 100U);
}

TEST(DummyStrategyTest, returnsFalseWhenEventDoesNotProduceIntent) {
    DummyStrategy strategy;
    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.bid_price = 100;
    event.ask_price = 500;
    event.bid_shares = 10;
    event.ask_shares = 10;

    OrderIntent intent {};
    EXPECT_FALSE(strategy.evaluateEvent(event, intent));
}
