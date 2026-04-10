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
