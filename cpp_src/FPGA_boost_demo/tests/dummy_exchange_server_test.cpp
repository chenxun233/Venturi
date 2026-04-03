#include "../exchange/dummy_exchange_server.h"

#include <gtest/gtest.h>

namespace {

ExchangeEnterOrder makeOrder(uint32_t tag,
                             uint16_t stock_locate,
                             uint32_t shares,
                             uint32_t price,
                             char side = 'B') {
    ExchangeEnterOrder order {};
    order.tag = tag;
    order.stock_locate = stock_locate;
    order.shares = shares;
    order.price = price;
    order.side = side;
    return order;
}

} // namespace

TEST(DummyExchangeServerTest, validateOrderRejectsOutOfBandPrice) {
    DummyExchangeConfig config {};
    config.price_min = 100000;
    config.price_max = 200000;

    DummyExchangeServer server(config);
    const auto result = server.validateEnterOrder(makeOrder(42, 0x000d, 100, 99999));

    EXPECT_EQ(result.kind, ExchangeValidationKind::Rejected);
}

TEST(DummyExchangeServerTest, duplicateUserRefNumReusesOriginalOutcome) {
    DummyExchangeServer server({});

    const auto first = server.handleEnterOrderForTest(1, makeOrder(42, 0x000d, 100, 123450));
    const auto replay = server.handleEnterOrderForTest(1, makeOrder(42, 0x000d, 100, 123450));

    EXPECT_FALSE(first.is_duplicate);
    EXPECT_TRUE(replay.is_duplicate);
}

TEST(DummyExchangeServerTest, duplicateUserRefNumIsScopedPerSession) {
    DummyExchangeServer server({});

    const auto first = server.handleEnterOrderForTest(1, makeOrder(42, 0x000d, 100, 123450));
    const auto replay = server.handleEnterOrderForTest(1, makeOrder(42, 0x000d, 100, 123450));
    const auto other_session = server.handleEnterOrderForTest(2, makeOrder(42, 0x000d, 100, 123450));

    EXPECT_FALSE(first.is_duplicate);
    EXPECT_TRUE(replay.is_duplicate);
    EXPECT_FALSE(other_session.is_duplicate);
}

TEST(DummyExchangeServerTest, freshSessionStartsSequencingAtOne) {
    DummyExchangeServer server({});

    const auto first = server.handleEnterOrderForTest(1, makeOrder(7, 0x000d, 100, 123450));
    const auto second = server.handleEnterOrderForTest(2, makeOrder(7, 0x000d, 100, 123450));

    ASSERT_FALSE(first.outbound_messages.empty());
    ASSERT_FALSE(second.outbound_messages.empty());
    EXPECT_EQ(server.readSessionNextSequenceForTest(1), 2U);
    EXPECT_EQ(server.readSessionNextSequenceForTest(2), 2U);
}
