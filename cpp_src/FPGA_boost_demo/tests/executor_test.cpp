#include "../tx_engine/executor.h"

#include <gtest/gtest.h>

TEST(ExecutorTest, acceptsIntentAndEmitsExecutionStageOutput) {
    Executor executor(1, 8);
    OrderIntent intent {};
    intent.stock_locate = 0x000d;
    intent.intent.action = OrderIntentAction::Buy;

    ASSERT_TRUE(executor.acceptIntent(0, intent));

    OrderIntent ready {};
    ASSERT_TRUE(executor.popReadyIntent(ready));
    EXPECT_EQ(ready.stock_locate, 0x000d);
}

TEST(ExecutorTest, popsReadyIntentsAcrossProducersInRoundRobinOrder) {
    Executor executor(2, 8);

    OrderIntent first {};
    first.stock_locate = 0x000d;
    first.event_ts = 11U;
    OrderIntent second {};
    second.stock_locate = 0x0ee8;
    second.event_ts = 22U;

    ASSERT_TRUE(executor.acceptIntent(0, first));
    ASSERT_TRUE(executor.acceptIntent(1, second));

    OrderIntent ready {};
    ASSERT_TRUE(executor.popReadyIntent(ready));
    EXPECT_EQ(ready.stock_locate, 0x000d);
    ASSERT_TRUE(executor.popReadyIntent(ready));
    EXPECT_EQ(ready.stock_locate, 0x0ee8);
    EXPECT_FALSE(executor.popReadyIntent(ready));
}
