#include <gtest/gtest.h>

#include "../common/shared_types.h"
#define private public
#include "../tx/tx_engine.h"
#undef private

#include <string>
#include <type_traits>

static_assert(std::is_member_function_pointer_v<decltype(&TxEngine::pushPayload)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxEngine::runTransportStep)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxEngine::drainInboundPayloads)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxEngine::takeConnectEvent)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxEngine::isConnected)>);

TEST(TxLogRecordTest, txEventRecordStoresConnectionAndOrderEvents) {
    TxLogRecord record {};
    record.event = TxEventKind::ConnectionEstablished;
    record.user_ref_num = 42;
    record.stock_locate = 0x000d;
    record.price = 123450;
    record.shares = 100;

    EXPECT_EQ(record.event, TxEventKind::ConnectionEstablished);
    EXPECT_EQ(record.user_ref_num, 42U);
    EXPECT_EQ(record.stock_locate, 0x000d);
    EXPECT_EQ(record.price, 123450U);
    EXPECT_EQ(record.shares, 100U);
}

TEST(TxLogRecordTest, outboundRecordCarriesPrebuiltPayloadAndMetadata) {
    TxOutboundRecord record {};
    record.user_ref_num = 42;
    record.payload_length = 16;

    EXPECT_EQ(record.user_ref_num, 42U);
    EXPECT_EQ(record.payload_length, 16U);
}

TEST(TxLogRecordTest, txEventRecordCanRepresentAcceptedFeedback) {
    TxLogRecord record {};
    record.event = TxEventKind::OrderAccepted;

    EXPECT_EQ(record.event, TxEventKind::OrderAccepted);
}

TEST(TxLogRecordTest, orderSentHelperDoesNotPrintDirectlyWithoutLogPrinter) {
    TxEngine engine {};
    TxOutboundRecord record {};
    record.user_ref_num = 42;
    record.stock_locate = 0x000d;
    record.price = 123450;
    record.shares = 100;

    testing::internal::CaptureStdout();
    engine._logOrderSent(record);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.empty());
}

TEST(TxLogRecordTest, connectEventDefaultsToFalse) {
    TxEngine engine {};

    EXPECT_FALSE(engine.takeConnectEvent());
}
