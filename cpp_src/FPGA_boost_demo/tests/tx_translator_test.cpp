#include "../common/shared_types.h"
#include "../tx/tx_translator.h"

#include <gtest/gtest.h>

#include <array>
#include <vector>

namespace {

std::vector<uint8_t> makeLoginAcceptedFrame() {
    std::vector<uint8_t> bytes(33, 0);
    bytes[0] = 0x00;
    bytes[1] = 0x1f;
    bytes[2] = static_cast<uint8_t>('A');

    const std::array<uint8_t, 10> session {'S', 'E', 'S', 'S', 'I', 'O', 'N', '0', '1', ' '};
    for (std::size_t idx = 0; idx < session.size(); ++idx) {
        bytes[3 + idx] = session[idx];
    }

    const std::array<uint8_t, 20> sequence {
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '1'
    };
    for (std::size_t idx = 0; idx < sequence.size(); ++idx) {
        bytes[13 + idx] = sequence[idx];
    }

    return bytes;
}

std::vector<uint8_t> makeSequencedAcceptedFrame(uint32_t tag,
                                                uint16_t stock_locate,
                                                uint32_t shares,
                                                uint32_t price) {
    std::vector<uint8_t> bytes(35, 0);
    bytes[0] = 0x00;
    bytes[1] = 0x21;
    bytes[2] = static_cast<uint8_t>('S');
    bytes[3] = static_cast<uint8_t>('A');
    bytes[12] = static_cast<uint8_t>((tag >> 24) & 0xffU);
    bytes[13] = static_cast<uint8_t>((tag >> 16) & 0xffU);
    bytes[14] = static_cast<uint8_t>((tag >> 8) & 0xffU);
    bytes[15] = static_cast<uint8_t>(tag & 0xffU);
    bytes[16] = static_cast<uint8_t>('B');
    bytes[17] = static_cast<uint8_t>((stock_locate >> 8) & 0xffU);
    bytes[18] = static_cast<uint8_t>(stock_locate & 0xffU);
    bytes[19] = static_cast<uint8_t>((shares >> 24) & 0xffU);
    bytes[20] = static_cast<uint8_t>((shares >> 16) & 0xffU);
    bytes[21] = static_cast<uint8_t>((shares >> 8) & 0xffU);
    bytes[22] = static_cast<uint8_t>(shares & 0xffU);
    bytes[23] = static_cast<uint8_t>((price >> 24) & 0xffU);
    bytes[24] = static_cast<uint8_t>((price >> 16) & 0xffU);
    bytes[25] = static_cast<uint8_t>((price >> 8) & 0xffU);
    bytes[26] = static_cast<uint8_t>(price & 0xffU);
    bytes[34] = 0x01;
    return bytes;
}

} // namespace

TEST(TxTranslatorTest, transportConnectQueuesLoginRequestFrame) {
    TxTranslator translator(4);
    translator.handleTransportConnected();

    TxOutboundRecord record {};
    ASSERT_TRUE(translator.popOutbound(record));
    EXPECT_EQ(record.tag, 0U);
    ASSERT_GE(record.payload_length, 3U);
    EXPECT_EQ(record.payload[2], static_cast<uint8_t>('L'));
}

TEST(TxTranslatorTest, loginAcceptReleasesBufferedOrdersWithIncreasingTags) {
    TxTranslator translator(4);

    ASSERT_TRUE(translator.pushIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));
    ASSERT_TRUE(translator.pushIntent(OrderIntent {
        .stock_locate = 0x0ee8,
        .intent = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    translator.handleTransportConnected();

    TxOutboundRecord login {};
    ASSERT_TRUE(translator.popOutbound(login));
    EXPECT_EQ(login.payload[2], static_cast<uint8_t>('L'));

    translator.handleInboundPayload(makeLoginAcceptedFrame());

    TxOutboundRecord first {};
    TxOutboundRecord second {};
    ASSERT_TRUE(translator.popOutbound(first));
    ASSERT_TRUE(translator.popOutbound(second));

    EXPECT_LT(first.tag, second.tag);
    EXPECT_EQ(first.payload[2], static_cast<uint8_t>('U'));
    EXPECT_EQ(first.payload[3], static_cast<uint8_t>('O'));
    EXPECT_EQ(second.payload[2], static_cast<uint8_t>('U'));
    EXPECT_EQ(second.payload[3], static_cast<uint8_t>('O'));
}

TEST(TxTranslatorTest, invalidIntentActionDoesNotProduceOrderFrame) {
    TxTranslator translator(4);

    ASSERT_TRUE(translator.pushIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::None, .price = 123450, .shares = 100},
    }));

    translator.handleTransportConnected();

    TxOutboundRecord login {};
    ASSERT_TRUE(translator.popOutbound(login));
    translator.handleInboundPayload(makeLoginAcceptedFrame());

    TxOutboundRecord record {};
    EXPECT_FALSE(translator.popOutbound(record));
}

TEST(TxTranslatorTest, acceptedOrderStopsBeingReplayCandidateAfterDisconnect) {
    TxTranslator translator(4);

    ASSERT_TRUE(translator.pushIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));

    translator.handleTransportConnected();
    TxOutboundRecord login {};
    ASSERT_TRUE(translator.popOutbound(login));
    translator.handleInboundPayload(makeLoginAcceptedFrame());

    TxOutboundRecord order {};
    ASSERT_TRUE(translator.popOutbound(order));
    translator.handleInboundPayload(makeSequencedAcceptedFrame(order.tag, order.stock_locate, order.shares, order.price));
    translator.handleTransportDisconnect();
    translator.handleTransportConnected();

    ASSERT_TRUE(translator.popOutbound(login));
    translator.handleInboundPayload(makeLoginAcceptedFrame());
    EXPECT_FALSE(translator.popOutbound(order));
}

TEST(TxTranslatorTest, pendingCapacityDropsOldestRecordOnDisconnectRestore) {
    TxTranslator translator(TxTranslatorConfig {
        .intent_capacity = 4,
        .pending_capacity = 2,
    });

    ASSERT_TRUE(translator.pushIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 100000, .shares = 10},
    }));
    ASSERT_TRUE(translator.pushIntent(OrderIntent {
        .stock_locate = 0x0ee8,
        .intent = {.action = OrderIntentAction::Sell, .price = 100100, .shares = 11},
    }));
    ASSERT_TRUE(translator.pushIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 100200, .shares = 12},
    }));

    translator.handleTransportConnected();

    TxOutboundRecord login {};
    ASSERT_TRUE(translator.popOutbound(login));
    translator.handleInboundPayload(makeLoginAcceptedFrame());

    TxOutboundRecord first {};
    TxOutboundRecord second {};
    TxOutboundRecord third {};
    ASSERT_TRUE(translator.popOutbound(first));
    ASSERT_TRUE(translator.popOutbound(second));
    ASSERT_TRUE(translator.popOutbound(third));

    translator.handleTransportDisconnect();
    translator.handleTransportConnected();
    ASSERT_TRUE(translator.popOutbound(login));
    translator.handleInboundPayload(makeLoginAcceptedFrame());

    TxOutboundRecord resend_first {};
    TxOutboundRecord resend_second {};
    ASSERT_TRUE(translator.popOutbound(resend_first));
    ASSERT_TRUE(translator.popOutbound(resend_second));
    EXPECT_EQ(resend_first.tag, second.tag);
    EXPECT_EQ(resend_second.tag, third.tag);
}
