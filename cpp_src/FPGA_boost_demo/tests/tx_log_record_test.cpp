#include <gtest/gtest.h>

#include "../common/shared_types.h"
#define private public
#include "../tx_engine/tx_engine.h"
#undef private

#define private public
#include "../latency/latency_tracker.h"
#undef private

#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <string>
#include <type_traits>

static_assert(std::is_member_function_pointer_v<decltype(&TxEngine::pollConnectStep)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxEngine::sendOutboundRecord)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxEngine::pollInboundFrame)>);
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
    record.que_idx = 1;
    record.event_ts = 123456789ULL;
    record.payload_length = 16;

    EXPECT_EQ(record.user_ref_num, 42U);
    EXPECT_EQ(record.que_idx, 1U);
    EXPECT_EQ(record.event_ts, 123456789ULL);
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

TEST(TxLogRecordTest, sendOutboundRecordWritesPayloadToSocket) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxEngine engine {};
    engine.m_socket_fd = sockets[0];

    TxOutboundRecord record {};
    record.user_ref_num = 42;
    record.payload[0] = 0x00;
    record.payload[1] = 0x02;
    record.payload[2] = static_cast<uint8_t>('U');
    record.payload[3] = static_cast<uint8_t>('R');
    record.payload_length = 4;

    ASSERT_TRUE(engine.sendOutboundRecord(record));

    std::array<uint8_t, 4> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0), 4);
    EXPECT_EQ(received[0], record.payload[0]);
    EXPECT_EQ(received[1], record.payload[1]);
    EXPECT_EQ(received[2], record.payload[2]);
    EXPECT_EQ(received[3], record.payload[3]);
    EXPECT_FALSE(engine.takeDisconnectEvent());

    ::close(sockets[0]);
    ::close(sockets[1]);
    engine.m_socket_fd = -1;
}

TEST(TxLogRecordTest, successfulTrackedSendPushesTxSendRecord) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxEngine engine {};
    LatencyTracker tracker(1, 8);
    engine.attachLatenyTracker(&tracker);
    engine.m_socket_fd = sockets[0];

    TxOutboundRecord record {};
    record.que_idx = 0;
    record.event_ts = 0x55ULL;
    record.payload[0] = 0x00;
    record.payload[1] = 0x02;
    record.payload[2] = static_cast<uint8_t>('U');
    record.payload[3] = static_cast<uint8_t>('R');
    record.payload_length = 4;

    ASSERT_TRUE(engine.sendOutboundRecord(record));

    TimeRecord pushed {};
    ASSERT_TRUE(tracker.m_trace_buffer[0]->pop(pushed));
    EXPECT_EQ(pushed.event_stage, stage::TX_SEND);
    EXPECT_EQ(pushed.event_ts, 0x55ULL);
    EXPECT_GT(pushed.time_captured, 0U);

    ::close(sockets[0]);
    ::close(sockets[1]);
    engine.m_socket_fd = -1;
}

TEST(TxLogRecordTest, pollInboundFrameReadsFrameAndDetectsDisconnect) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxEngine engine {};
    engine.m_socket_fd = sockets[0];

    const std::array<uint8_t, 3> frame {0x00, 0x01, static_cast<uint8_t>('H')};
    ASSERT_EQ(::send(sockets[1], frame.data(), frame.size(), 0), 3);

    std::vector<uint8_t> inbound {};
    ASSERT_TRUE(engine.pollInboundFrame(inbound));
    ASSERT_EQ(inbound.size(), frame.size());
    EXPECT_EQ(inbound[0], frame[0]);
    EXPECT_EQ(inbound[1], frame[1]);
    EXPECT_EQ(inbound[2], frame[2]);
    EXPECT_FALSE(engine.takeDisconnectEvent());

    ::close(sockets[1]);
    sockets[1] = -1;

    inbound.clear();
    EXPECT_FALSE(engine.pollInboundFrame(inbound));
    EXPECT_TRUE(engine.takeDisconnectEvent());

    ::close(sockets[0]);
    engine.m_socket_fd = -1;
}
