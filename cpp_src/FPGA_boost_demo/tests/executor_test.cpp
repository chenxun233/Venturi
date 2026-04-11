#include "../tx_engine/executor.h"

#define private public
#include "../latency/latency_tracker.h"
#undef private

#include <gtest/gtest.h>
#include <stdexcept>

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

TEST(ExecutorTest, successfulTrackedAcceptPushesExecutorRecord) {
    Executor executor(1, 8);
    LatencyTracker tracker(1, 8);
    executor.attachLatenyTracker(&tracker);

    OrderIntent intent {};
    intent.que_idx = 0;
    intent.event_ts = 77U;

    ASSERT_TRUE(executor.acceptIntent(0, intent));

    TimeRecord record {};
    ASSERT_TRUE(tracker.m_trace_buffer[0]->pop(record));
    EXPECT_EQ(record.que_idx, 0U);
    EXPECT_EQ(record.event_stage, stage::EXECUTOR);
    EXPECT_EQ(record.event_ts, 77U);
    EXPECT_GT(record.time_captured, 0U);
    EXPECT_FALSE(tracker.m_trace_buffer[0]->pop(record));
}

TEST(ExecutorTest, trackedIntentMismatchProducerIndexThrows) {
    Executor executor(2, 8);
    LatencyTracker tracker(2, 8);
    executor.attachLatenyTracker(&tracker);

    OrderIntent intent {};
    intent.que_idx = 1;
    intent.event_ts = 55U;

    EXPECT_THROW(executor.acceptIntent(0, intent), std::invalid_argument);
}
