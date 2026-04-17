#include "../strategy/dummy_strategy.h"

#define private public
#include "../latency/latency_tracker.h"
#undef private

#include <gtest/gtest.h>

TEST(DummyStrategyTest, acceptedFirstEventPushesStrategyStartRecord) {
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
    ASSERT_TRUE(strategy.evaluateEvent(0 ,event, intent));
    EXPECT_EQ(static_cast<int>(stage::STRATEGY_START), 3);
    EXPECT_EQ(intent.stock_locate, 0x000d);
    EXPECT_EQ(intent.que_idx, 0U);
    EXPECT_EQ(intent.event_tag, 12345U);
    EXPECT_EQ(intent.intent.action, OrderIntentAction::Buy);
    EXPECT_EQ(intent.intent.price, 100U);
    EXPECT_EQ(intent.intent.shares, 100U);

    TimeRecord record {};
    ASSERT_TRUE(tracker.m_latency_queues[0]->pop(record));
    EXPECT_EQ(record.que_idx, 0U);
    EXPECT_EQ(record.event_stage, stage::STRATEGY_START);
    EXPECT_EQ(record.event_tag, 12345U);
    EXPECT_GT(record.time_captured, 0U);
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(DummyStrategyTest, acceptedFirstEventRoutesStrategyStartRecordToNonZeroQueue) {
    DummyStrategy strategy;
    LatencyTracker tracker(2, 8);
    strategy.attachLatenyTracker(&tracker);

    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.event_tk = 54321U;
    event.is_first_event = 1U;
    event.bid_price = 100U;
    event.ask_price = 120U;
    event.bid_shares = 2000U;
    event.ask_shares = 100U;

    OrderIntent intent {};
    ASSERT_TRUE(strategy.evaluateEvent(1, event, intent));
    EXPECT_EQ(intent.que_idx, 1U);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
    ASSERT_TRUE(tracker.m_latency_queues[1]->pop(record));
    EXPECT_EQ(record.que_idx, 1U);
    EXPECT_EQ(record.event_tag, 54321U);
    EXPECT_EQ(record.event_stage, stage::STRATEGY_START);
    EXPECT_GT(record.time_captured, 0U);
    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));
}

TEST(DummyStrategyTest, acceptedNonFirstEventDoesNotPushStrategyRecord) {
    DummyStrategy strategy;
    LatencyTracker tracker(1, 8);
    strategy.attachLatenyTracker(&tracker);

    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.event_tk = 12345U;
    event.is_first_event = 0U;
    event.bid_price = 100U;
    event.ask_price = 120U;
    event.bid_shares = 2000U;
    event.ask_shares = 100U;

    OrderIntent intent {};
    ASSERT_TRUE(strategy.evaluateEvent(0 ,event, intent));

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(DummyStrategyTest, acceptedNonFirstEventPreservesEventTagInIntent) {
    DummyStrategy strategy;
    FPGAEventDesc event {};
    event.event_tk = 12345U;
    event.is_first_event = 0U;
    event.bid_price = 100U;
    event.ask_price = 120U;
    event.bid_shares = 2000U;
    event.ask_shares = 100U;

    OrderIntent intent {};
    ASSERT_TRUE(strategy.evaluateEvent(0, event, intent));
    EXPECT_EQ(intent.event_tag, 12345U);
}

TEST(DummyStrategyTest, acceptedFirstEventReturnsIntentWhenLatencyEmissionThrows) {
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
    ASSERT_TRUE(strategy.evaluateEvent(1, event, intent));
    EXPECT_EQ(intent.stock_locate, 0x000d);
    EXPECT_EQ(intent.que_idx, 1U);
    EXPECT_EQ(intent.event_tag, 12345U);
    EXPECT_EQ(intent.intent.action, OrderIntentAction::Buy);
    EXPECT_EQ(intent.intent.price, 100U);
    EXPECT_EQ(intent.intent.shares, 100U);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(DummyStrategyTest, firstAcceptedEventWithZeroTimestampDoesNotPushStrategyRecord) {
    DummyStrategy strategy;
    LatencyTracker tracker(1, 8);
    strategy.attachLatenyTracker(&tracker);

    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.event_tk = 0U;
    event.is_first_event = 1U;
    event.bid_price = 100U;
    event.ask_price = 120U;
    event.bid_shares = 2000U;
    event.ask_shares = 100U;

    OrderIntent intent {};
    ASSERT_TRUE(strategy.evaluateEvent(0, event, intent));

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}

TEST(DummyStrategyTest, rejectsEventWhenNoActionIsSuggested) {
    DummyStrategy strategy;
    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.event_tk = 12345U;
    event.bid_price = 100U;
    event.ask_price = 100U;
    event.bid_shares = 100U;
    event.ask_shares = 100U;

    OrderIntent intent {};
    EXPECT_FALSE(strategy.evaluateEvent(0, event, intent));
}

TEST(DummyStrategyTest, firstEventWithoutActionStillPushesStrategyStartRecordBeforeRejection) {
    DummyStrategy strategy;
    LatencyTracker tracker(1, 8);
    strategy.attachLatenyTracker(&tracker);

    FPGAEventDesc event {};
    event.stock_locate = 0x000d;
    event.event_tk = 24680U;
    event.is_first_event = 1U;
    event.bid_price = 100U;
    event.ask_price = 100U;
    event.bid_shares = 100U;
    event.ask_shares = 100U;

    OrderIntent intent {};
    EXPECT_FALSE(strategy.evaluateEvent(0, event, intent));

    TimeRecord record {};
    ASSERT_TRUE(tracker.m_latency_queues[0]->pop(record));
    EXPECT_EQ(record.que_idx, 0U);
    EXPECT_EQ(record.event_tag, 24680U);
    EXPECT_EQ(record.event_stage, stage::STRATEGY_START);
    EXPECT_GT(record.time_captured, 0U);
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
}
