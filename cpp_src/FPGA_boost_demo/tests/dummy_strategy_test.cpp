#include "../strategy/dummy_strategy.h"

#define private public
#include "../latency/latency_tracker.h"
#undef private

#include <gtest/gtest.h>

TEST(DummyStrategyTest, acceptedFirstEventPushesStrategyRecord) {
    DummyStrategy strategy;
    LatencyTracker tracker(1, 8);
    strategy.attachLatenyTracker(&tracker);

    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.event_tk = 12345U;
    event.is_first_event = 1U;
    event.bid_price = 100U;
    event.ask_price = 120U;
    event.bid_shares = 2000U;
    event.ask_shares = 100U;

    OrderIntent intent {};
    ASSERT_TRUE(strategy.evaluateEvent(event, intent, 0));

    TimeRecord record {};
    ASSERT_TRUE(tracker.m_trace_buffer[0]->pop(record));
    EXPECT_EQ(record.event_stage, stage::STRATEGY);
    EXPECT_EQ(record.event_ts, 12345U);
    EXPECT_GT(record.time_captured, 0U);
}
