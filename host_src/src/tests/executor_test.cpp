#define private public
#include "../latency/latency_tracker.h"
#undef private

#include "../tx_client/executor.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace {

template <typename TraceIdSlot>
uint32_t readActiveTraceId(const TraceIdSlot& trace_id) {
    using SlotType = std::remove_cv_t<std::remove_reference_t<TraceIdSlot>>;
    if constexpr (std::is_same_v<SlotType, std::atomic<uint32_t>>) {
        return trace_id.load();
    }
    return trace_id;
}

} // namespace

TEST(ExecutorTest, acceptsIntentAndMakesExecutionAvailable) {
    Executor executor(8);
    OrderIntent intent {};
    intent.stock_locate = 0x000d;
    intent.que_idx = 1U;
    intent.event_tag = 11ULL;
    intent.trace_id = 55U;
    intent.intent = {.action = OrderIntentAction::Buy, .price = 100U, .shares = 10U};

    ASSERT_TRUE(executor.acceptIntent(intent));

    OrderExecution ready {};
    ASSERT_TRUE(executor.popExecution(ready));
    EXPECT_EQ(ready.stock_locate, 0x000d);
    EXPECT_EQ(ready.que_idx, 1U);
    EXPECT_EQ(ready.event_tag, 11ULL);
    EXPECT_EQ(ready.trace_id, 55U);
    EXPECT_EQ(ready.order.action, OrderIntentAction::Buy);
}

TEST(ExecutorTest, queueMismatchQueuesDropRequestInsteadOfDroppingInline) {
    Executor executor(8);
    LatencyTracker tracker(2, 8);
    executor.attachLatencyTracker(&tracker);
    executor.attachQueueIdx(0);

    const uint32_t trace_id = tracker.tryAllocateTraceId(1, true);
    ASSERT_NE(trace_id, 0U);
    tracker.pushRecord(TimeRecord {
        .que_idx = 1,
        .event_tag = 99ULL,
        .trace_id = trace_id,
        .event_stage = stage::FRAME_START,
        .time_captured = 100U,
    });

    OrderIntent intent {};
    intent.que_idx = 1U;
    intent.event_tag = 99ULL;
    intent.trace_id = trace_id;
    intent.intent = {.action = OrderIntentAction::Buy, .price = 100U, .shares = 10U};

    EXPECT_FALSE(executor.acceptIntent(intent));
    EXPECT_EQ(readActiveTraceId(tracker.m_active_trace_ids[1]), trace_id);

    TraceCommand command {};
    ASSERT_TRUE(tracker.m_command_queues[0]->pop(command));
    EXPECT_EQ(command.que_idx, 0U);
    EXPECT_EQ(command.trace_id, trace_id);
    EXPECT_EQ(command.op, TraceCommandOp::Drop);
    EXPECT_FALSE(tracker.m_command_queues[1]->pop(command));
    ASSERT_TRUE(tracker.m_command_queues[0]->push(command));

    tracker.stop();
    tracker.run();

    EXPECT_EQ(readActiveTraceId(tracker.m_active_trace_ids[1]), trace_id);

    TimeRecord record {};
    EXPECT_TRUE(tracker.m_latency_queues[1]->pop(record));
    EXPECT_EQ(record.trace_id, trace_id);
}

TEST(ExecutorTest, failedPushQueuesDropRequestInsteadOfDroppingInline) {
    Executor executor(1);
    LatencyTracker tracker(1, 8);
    executor.attachLatencyTracker(&tracker);

    OrderIntent accepted_intent {};
    accepted_intent.stock_locate = 0x000d;
    accepted_intent.que_idx = 0U;
    accepted_intent.event_tag = 88ULL;
    accepted_intent.trace_id = 0U;
    accepted_intent.intent = {.action = OrderIntentAction::Buy, .price = 100U, .shares = 10U};
    ASSERT_TRUE(executor.acceptIntent(accepted_intent));

    const uint32_t trace_id = tracker.tryAllocateTraceId(0, true);
    ASSERT_NE(trace_id, 0U);
    tracker.pushRecord(TimeRecord {
        .que_idx = 0,
        .event_tag = 99ULL,
        .trace_id = trace_id,
        .event_stage = stage::FRAME_START,
        .time_captured = 100U,
    });

    OrderIntent rejected_intent {};
    rejected_intent.stock_locate = 0x000d;
    rejected_intent.que_idx = 0U;
    rejected_intent.event_tag = 99ULL;
    rejected_intent.trace_id = trace_id;
    rejected_intent.intent = {.action = OrderIntentAction::Buy, .price = 101U, .shares = 10U};

    EXPECT_FALSE(executor.acceptIntent(rejected_intent));
    EXPECT_EQ(readActiveTraceId(tracker.m_active_trace_ids[0]), trace_id);

    TraceCommand command {};
    ASSERT_TRUE(tracker.m_command_queues[0]->pop(command));
    EXPECT_EQ(command.que_idx, 0U);
    EXPECT_EQ(command.trace_id, trace_id);
    EXPECT_EQ(command.op, TraceCommandOp::Drop);
    ASSERT_TRUE(tracker.m_command_queues[0]->push(command));

    tracker.stop();
    tracker.run();

    EXPECT_EQ(readActiveTraceId(tracker.m_active_trace_ids[0]), 0U);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}
