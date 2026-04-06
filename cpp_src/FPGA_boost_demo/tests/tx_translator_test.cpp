#include "../common/shared_types.h"
#include "../tx/tx_translator.h"

#include <gtest/gtest.h>

#include <array>
#include <vector>

namespace {

void writeBigEndian16(uint8_t* out, uint16_t value) {
    out[0] = static_cast<uint8_t>((value >> 8) & 0xffU);
    out[1] = static_cast<uint8_t>(value & 0xffU);
}

void writeBigEndian32(uint8_t* out, uint32_t value) {
    out[0] = static_cast<uint8_t>((value >> 24) & 0xffU);
    out[1] = static_cast<uint8_t>((value >> 16) & 0xffU);
    out[2] = static_cast<uint8_t>((value >> 8) & 0xffU);
    out[3] = static_cast<uint8_t>(value & 0xffU);
}

void writeBigEndian64(uint8_t* out, uint64_t value) {
    out[0] = static_cast<uint8_t>((value >> 56) & 0xffU);
    out[1] = static_cast<uint8_t>((value >> 48) & 0xffU);
    out[2] = static_cast<uint8_t>((value >> 40) & 0xffU);
    out[3] = static_cast<uint8_t>((value >> 32) & 0xffU);
    out[4] = static_cast<uint8_t>((value >> 24) & 0xffU);
    out[5] = static_cast<uint8_t>((value >> 16) & 0xffU);
    out[6] = static_cast<uint8_t>((value >> 8) & 0xffU);
    out[7] = static_cast<uint8_t>(value & 0xffU);
}

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

std::vector<uint8_t> makeSequencedAcceptedFrame(uint32_t user_ref_num,
                                                uint16_t stock_locate,
                                                uint32_t shares,
                                                uint32_t price) {
    (void)stock_locate;
    std::vector<uint8_t> bytes(67, 0);
    bytes[0] = 0x00;
    bytes[1] = 0x41;
    bytes[2] = static_cast<uint8_t>('S');
    bytes[3] = static_cast<uint8_t>('A');

    writeBigEndian32(bytes.data() + 12, user_ref_num);
    bytes[16] = static_cast<uint8_t>('B');
    writeBigEndian32(bytes.data() + 17, shares);
    const std::array<uint8_t, 8> symbol {'A', 'A', 'P', 'L', ' ', ' ', ' ', ' '};
    for (std::size_t idx = 0; idx < symbol.size(); ++idx) {
        bytes[21 + idx] = symbol[idx];
    }
    writeBigEndian64(bytes.data() + 29, static_cast<uint64_t>(price));
    bytes[37] = static_cast<uint8_t>('0');
    bytes[38] = static_cast<uint8_t>('Y');
    writeBigEndian64(bytes.data() + 39, 1U);
    bytes[47] = static_cast<uint8_t>('A');
    bytes[48] = static_cast<uint8_t>('N');
    bytes[49] = static_cast<uint8_t>('N');
    bytes[50] = static_cast<uint8_t>('L');
    bytes[65] = 0x00;
    bytes[66] = 0x00;
    return bytes;
}

std::vector<uint8_t> makeSequencedExecutedFrame(uint32_t user_ref_num,
                                                uint32_t executed_shares,
                                                uint32_t price,
                                                uint64_t match_number) {
    std::vector<uint8_t> bytes(39, 0);
    bytes[0] = 0x00;
    bytes[1] = 0x25;
    bytes[2] = static_cast<uint8_t>('S');
    bytes[3] = static_cast<uint8_t>('E');
    writeBigEndian32(bytes.data() + 12, user_ref_num);
    writeBigEndian32(bytes.data() + 16, executed_shares);
    writeBigEndian64(bytes.data() + 20, static_cast<uint64_t>(price));
    bytes[28] = static_cast<uint8_t>('A');
    writeBigEndian64(bytes.data() + 29, match_number);
    writeBigEndian16(bytes.data() + 37, 0);
    return bytes;
}

std::vector<uint8_t> makeSequencedRejectedFrame(uint32_t user_ref_num, uint16_t reason) {
    std::vector<uint8_t> bytes(34, 0);
    bytes[0] = 0x00;
    bytes[1] = 0x20;
    bytes[2] = static_cast<uint8_t>('S');
    bytes[3] = static_cast<uint8_t>('J');
    writeBigEndian32(bytes.data() + 12, user_ref_num);
    writeBigEndian16(bytes.data() + 16, reason);
    writeBigEndian16(bytes.data() + 32, 0);
    return bytes;
}

} // namespace

TEST(TxTranslatorTest, transportConnectQueuesLoginRequestFrame) {
    TxTranslator translator(4);
    translator.handleTransportConnected();

    TxOutboundRecord record {};
    ASSERT_TRUE(translator.popOutbound(record));
    EXPECT_EQ(record.user_ref_num, 0U);
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

    EXPECT_LT(first.user_ref_num, second.user_ref_num);
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
    translator.handleInboundPayload(makeSequencedAcceptedFrame(order.user_ref_num, order.stock_locate, order.shares, order.price));
    translator.handleTransportDisconnect();
    translator.handleTransportConnected();

    ASSERT_TRUE(translator.popOutbound(login));
    translator.handleInboundPayload(makeLoginAcceptedFrame());
    EXPECT_FALSE(translator.popOutbound(order));
}

TEST(TxTranslatorTest, executedOrderStopsBeingReplayCandidateAfterDisconnect) {
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
    translator.handleInboundPayload(makeSequencedExecutedFrame(order.user_ref_num, order.shares, order.price, 77U));
    translator.handleTransportDisconnect();
    translator.handleTransportConnected();

    ASSERT_TRUE(translator.popOutbound(login));
    translator.handleInboundPayload(makeLoginAcceptedFrame());
    EXPECT_FALSE(translator.popOutbound(order));
}

TEST(TxTranslatorTest, rejectedOrderStopsBeingReplayCandidateAfterDisconnect) {
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
    translator.handleInboundPayload(makeSequencedRejectedFrame(order.user_ref_num, 0x0015));
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
    EXPECT_EQ(resend_first.user_ref_num, second.user_ref_num);
    EXPECT_EQ(resend_second.user_ref_num, third.user_ref_num);
}
