#include "../tx_engine/executor.h"

#define private public
#include "../latency/latency_tracker.h"
#undef private

#include <gtest/gtest.h>
#include <stdexcept>

TEST(ExecutorTest, takeReadyExecutionReturnsNextExecutionWithoutCrossThreadQueueContract) {
    Executor executor(8);

    ASSERT_TRUE(executor.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 11ULL,
        .intent = {.action = OrderIntentAction::Buy, .price = 100, .shares = 10},
    }));

    OrderExecution execution {};
    ASSERT_TRUE(executor.takeReadyExecution(execution));
    EXPECT_EQ(execution.que_idx, 0);
    EXPECT_EQ(execution.event_tag, 11ULL);
    EXPECT_FALSE(executor.takeReadyExecution(execution));
}

TEST(ExecutorTest, acceptsIntentAndMakesExecutionAvailable) {
    Executor executor(8);
    OrderIntent intent {};
    intent.stock_locate = 0x000d;
    intent.intent.action = OrderIntentAction::Buy;

    ASSERT_TRUE(executor.acceptIntent(intent));

    OrderExecution ready {};
    ASSERT_TRUE(executor.takeReadyExecution(ready));
    EXPECT_EQ(ready.stock_locate, 0x000d);
}

TEST(ExecutorTest, successfulTrackedAcceptDoesNotPushLatencyRecord) {
    Executor executor(8);
    LatencyTracker tracker(1, 8);
    executor.attachLatenyTracker(&tracker);

    OrderIntent intent {};
    intent.que_idx = 0;
    intent.event_tag = 77U;

    ASSERT_TRUE(executor.acceptIntent(intent));
    EXPECT_EQ(static_cast<int>(stage::TX_EXECUTION_ACCEPTED), 6);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(ExecutorTest, takeReadyExecutionDoesNotEmitExecutionTakenStage) {
    Executor executor(8);
    LatencyTracker tracker(1, 8);
    executor.attachLatenyTracker(&tracker);

    ASSERT_TRUE(executor.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 77ULL,
        .intent = {.action = OrderIntentAction::Buy, .price = 100, .shares = 10},
    }));

    OrderExecution execution {};
    ASSERT_TRUE(executor.takeReadyExecution(execution));

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(ExecutorTest, successfulTrackedAcceptToNonZeroQueueDoesNotPushLatencyRecord) {
    Executor executor(8);
    LatencyTracker tracker(2, 8);
    executor.attachLatenyTracker(&tracker);

    OrderIntent intent {};
    intent.que_idx = 1;
    intent.event_tag = 88U;

    ASSERT_TRUE(executor.acceptIntent(intent));

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));
}

TEST(ExecutorTest, trackedAcceptReturnsTrueAndExecutionRemainsAvailableWithoutLatencyEmission) {
    Executor executor(8);
    LatencyTracker tracker(1, 8);
    executor.attachLatenyTracker(&tracker);

    OrderIntent intent {};
    intent.que_idx = 1;
    intent.event_tag = 66U;
    intent.stock_locate = 0x000d;
    intent.intent.action = OrderIntentAction::Buy;

    ASSERT_TRUE(executor.acceptIntent(intent));

    OrderExecution ready {};
    ASSERT_TRUE(executor.takeReadyExecution(ready));
    EXPECT_EQ(ready.que_idx, 1U);
    EXPECT_EQ(ready.event_tag, 66U);
    EXPECT_EQ(ready.stock_locate, 0x000d);
    EXPECT_EQ(ready.order.action, OrderIntentAction::Buy);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}
