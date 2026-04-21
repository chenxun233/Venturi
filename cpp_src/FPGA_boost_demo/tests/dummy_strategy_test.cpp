#include "../strategy/dummy_strategy.h"

#define private public
#include "../latency/latency_tracker.h"
#undef private

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

TEST(DummyStrategyTest, tracedEventPushesStrategyStartRecordAndPropagatesTraceId) {
    DummyStrategy strategy;
    LatencyTracker tracker(1, 8);
    strategy.attachLatenyTracker(&tracker);

    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.event_tk = 12345U;
    event.trace_id = 88U;
    event.bid_price = 100U;
    event.ask_price = 120U;
    event.bid_shares = 2000U;
    event.ask_shares = 100U;

    OrderIntent intent {};
    ASSERT_TRUE(strategy.evaluateEvent(0, event, intent));
    EXPECT_EQ(intent.trace_id, 88U);

    TimeRecord record {};
    ASSERT_TRUE(tracker.m_latency_queues[0]->pop(record));
    EXPECT_EQ(record.que_idx, 0U);
    EXPECT_EQ(record.event_stage, stage::STRATEGY_START);
    EXPECT_EQ(record.event_tag, 12345U);
    EXPECT_EQ(record.trace_id, 88U);
    EXPECT_GT(record.time_captured, 0U);
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(DummyStrategyTest, untracedEventDoesNotPushStrategyRecord) {
    DummyStrategy strategy;
    LatencyTracker tracker(1, 8);
    strategy.attachLatenyTracker(&tracker);

    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.event_tk = 12345U;
    event.trace_id = 0U;
    event.bid_price = 100U;
    event.ask_price = 120U;
    event.bid_shares = 2000U;
    event.ask_shares = 100U;

    OrderIntent intent {};
    ASSERT_TRUE(strategy.evaluateEvent(0, event, intent));
    EXPECT_EQ(intent.trace_id, 0U);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(DummyStrategyTest, rejectedTracedEventQueuesDropRequestInsteadOfDroppingInline) {
    DummyStrategy strategy;
    LatencyTracker tracker(1, 8);
    strategy.attachLatenyTracker(&tracker);

    const uint32_t trace_id = tracker.tryAllocateTraceId(0, true);
    ASSERT_NE(trace_id, 0U);
    tracker.pushRecord(TimeRecord {
        .que_idx = 0,
        .event_tag = 24680U,
        .trace_id = trace_id,
        .event_stage = stage::FRAME_START,
        .time_captured = 100U,
    });

    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.event_tk = 24680U;
    event.trace_id = trace_id;
    event.bid_price = 100U;
    event.ask_price = 100U;
    event.bid_shares = 100U;
    event.ask_shares = 100U;

    OrderIntent intent {};
    EXPECT_FALSE(strategy.evaluateEvent(0, event, intent));
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
