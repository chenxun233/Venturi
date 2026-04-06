#include "../exchange/exchange_protocol.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

namespace {

ProtocolConfig makeProtocolConfig() {
    ProtocolConfig config {};
    config.username = "client";
    config.password = "secret";
    config.session_id = "SESSION01";
    config.price_min = 1;
    config.price_max = 5'000'000;
    config.max_shares = 1'000'000;
    config.fill_delay = std::chrono::milliseconds(5);
    config.replay_capacity = 256;
    return config;
}

ExchangeEnterOrder makeOrder(uint32_t user_ref_num,
                             uint16_t stock_locate,
                             uint32_t shares,
                             uint32_t price,
                             char side = 'B') {
    ExchangeEnterOrder order {};
    order.user_ref_num = user_ref_num;
    order.stock_locate = stock_locate;
    order.shares = shares;
    order.price = price;
    order.side = side;
    return order;
}

} // namespace

TEST(DummyExchangeProtocolSessionTest, validateOrderRejectsOutOfBandPrice) {
    auto config = makeProtocolConfig();
    config.price_min = 100000;
    config.price_max = 200000;

    ExchangeProtocol session(config);
    const auto now = std::chrono::steady_clock::now();
    session.reset(1, now);

    const auto result = session.validateEnterOrder(makeOrder(42, 0x000d, 100, 99999));
    EXPECT_EQ(result.kind, ExchangeValidationKind::Rejected);
}

TEST(DummyExchangeProtocolSessionTest, duplicateUserRefNumReusesOriginalOutcome) {
    ExchangeProtocol session(makeProtocolConfig());
    const auto now = std::chrono::steady_clock::now();
    session.reset(1, now);

    const auto first = session.handleEnterOrderForTest(makeOrder(42, 0x000d, 100, 123450), now);
    const auto replay = session.handleEnterOrderForTest(makeOrder(42, 0x000d, 100, 123450), now);

    EXPECT_FALSE(first.is_duplicate);
    EXPECT_TRUE(replay.is_duplicate);
}

TEST(DummyExchangeProtocolSessionTest, acceptedOrderUsesOfficialOuchAcceptedSize) {
    ExchangeProtocol session(makeProtocolConfig());
    const auto now = std::chrono::steady_clock::now();
    session.reset(1, now);

    const auto result = session.handleEnterOrderForTest(makeOrder(42, 0x000d, 100, 123450), now);

    ASSERT_EQ(result.outbound_message_count, 1U);
    EXPECT_EQ(result.outbound_messages[0].size, 64U);
}

TEST(DummyExchangeProtocolSessionTest, rejectedOrderUsesOfficialOuchRejectedSize) {
    auto config = makeProtocolConfig();
    config.price_min = 100000;
    config.price_max = 200000;
    ExchangeProtocol session(config);
    const auto now = std::chrono::steady_clock::now();
    session.reset(1, now);

    const auto result = session.handleEnterOrderForTest(makeOrder(42, 0x000d, 100, 99999), now);

    ASSERT_EQ(result.outbound_message_count, 1U);
    EXPECT_EQ(result.outbound_messages[0].size, 31U);
}

TEST(DummyExchangeProtocolSessionTest, duplicateUserRefNumIsScopedPerSession) {
    ExchangeProtocol first_session(makeProtocolConfig());
    ExchangeProtocol second_session(makeProtocolConfig());
    const auto now = std::chrono::steady_clock::now();
    first_session.reset(1, now);
    second_session.reset(2, now);

    const auto first = first_session.handleEnterOrderForTest(makeOrder(42, 0x000d, 100, 123450), now);
    const auto replay = first_session.handleEnterOrderForTest(makeOrder(42, 0x000d, 100, 123450), now);
    const auto other_session = second_session.handleEnterOrderForTest(makeOrder(42, 0x000d, 100, 123450), now);

    EXPECT_FALSE(first.is_duplicate);
    EXPECT_TRUE(replay.is_duplicate);
    EXPECT_FALSE(other_session.is_duplicate);
}

TEST(DummyExchangeProtocolSessionTest, freshSessionStartsSequencingAtOne) {
    ExchangeProtocol first_session(makeProtocolConfig());
    ExchangeProtocol second_session(makeProtocolConfig());
    const auto now = std::chrono::steady_clock::now();
    first_session.reset(1, now);
    second_session.reset(2, now);

    const auto first = first_session.handleEnterOrderForTest(makeOrder(7, 0x000d, 100, 123450), now);
    const auto second = second_session.handleEnterOrderForTest(makeOrder(7, 0x000d, 100, 123450), now);

    ASSERT_GT(first.outbound_message_count, 0U);
    ASSERT_GT(second.outbound_message_count, 0U);
    EXPECT_EQ(first_session.readNextSequenceForTest(), 2U);
    EXPECT_EQ(second_session.readNextSequenceForTest(), 2U);
}

TEST(DummyExchangeProtocolSessionTest, replayTableFullRejectsLaterDistinctOrder) {
    auto config = makeProtocolConfig();
    config.replay_capacity = 1;
    ExchangeProtocol session(config);
    const auto now = std::chrono::steady_clock::now();
    session.reset(1, now);

    const auto first = session.handleEnterOrderForTest(makeOrder(41, 0x000d, 100, 123450), now);
    const auto second = session.handleEnterOrderForTest(makeOrder(42, 0x000d, 100, 123451), now);

    EXPECT_EQ(first.validation.kind, ExchangeValidationKind::Accepted);
    EXPECT_EQ(second.validation.kind, ExchangeValidationKind::Rejected);
}

TEST(DummyExchangeProtocolSessionTest, bufferedReadWaitsUntilWholeSoupPacketArrives) {
    ExchangeProtocol session(makeProtocolConfig());
    const std::vector<uint8_t> partial {0x00, 0x11, 'L', 'c', 'l'};

    ASSERT_TRUE(session.appendReceivedBytes(partial.data(), partial.size(), std::chrono::steady_clock::now()));
    EXPECT_FALSE(session.tryReadPacketTypeForTest().has_value());
}

TEST(DummyExchangeProtocolSessionTest, flushQueueConsumesWholeFrontPacket) {
    ExchangeProtocol session(makeProtocolConfig());
    const auto now = std::chrono::steady_clock::now();
    session.reset(1, now);

    session.queuePacketForTest({0x00, 0x01, 'H'});
    EXPECT_EQ(session.readQueuedPacketCountForTest(), 0U);
}

TEST(DummyExchangeProtocolSessionTest, timerTickQueuesHeartbeatForIdleLoggedInSession) {
    ExchangeProtocol session(makeProtocolConfig());
    const auto now = std::chrono::steady_clock::now();
    session.reset(1, now);
    session.markLoggedInForTest();
    session.setLastSendAgoForTest(std::chrono::seconds(2));

    session.onTimerTick(now + std::chrono::seconds(2));

    ASSERT_TRUE(session.peekFrontPacketTypeForTest().has_value());
    EXPECT_EQ(*session.peekFrontPacketTypeForTest(), static_cast<uint8_t>('H'));
}

TEST(DummyExchangeProtocolSessionTest, acceptedOrderLaterQueuesExecutedMessageOnTimerTick) {
    auto config = makeProtocolConfig();
    config.fill_delay = std::chrono::milliseconds(1);
    ExchangeProtocol session(config);
    const auto now = std::chrono::steady_clock::now();
    session.reset(1, now);

    const auto accepted = session.handleEnterOrderForTest(makeOrder(42, 0x000d, 100, 123450), now);
    ASSERT_EQ(accepted.outbound_message_count, 1U);

    session.markLoggedInForTest();
    session.onTimerTick(now + std::chrono::milliseconds(5));

    ASSERT_TRUE(session.peekFrontPacketTypeForTest().has_value());
    EXPECT_EQ(*session.peekFrontPacketTypeForTest(), static_cast<uint8_t>('S'));

    const auto packet = session.readFrontPacketBytesForTest();
    ASSERT_EQ(packet.size(), 39U);
    EXPECT_EQ(packet[2], static_cast<uint8_t>('S'));
    EXPECT_EQ(packet[3], static_cast<uint8_t>('E'));
}

TEST(DummyExchangeProtocolSessionTest, consumeOutboundBytesAdvancesAcrossPacketBoundary) {
    ExchangeProtocol session(makeProtocolConfig());
    const auto now = std::chrono::steady_clock::now();
    session.reset(1, now);

    session.queuePacketForTest({0x00, 0x01, 'H'});
    session.queuePacketForTest({0x00, 0x01, 'A'});

    ASSERT_TRUE(session.peekFrontPacketTypeForTest().has_value());
    EXPECT_EQ(*session.peekFrontPacketTypeForTest(), static_cast<uint8_t>('A'));
}

TEST(DummyExchangeProtocolSessionTest, queuePacketForTestRejectsOversizedFrame) {
    ExchangeProtocol session(makeProtocolConfig());
    const auto now = std::chrono::steady_clock::now();
    session.reset(1, now);
    std::vector<uint8_t> oversized_packet(68, 0);

    EXPECT_THROW(session.queuePacketForTest(oversized_packet), std::runtime_error);
}
