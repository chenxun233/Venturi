#include <gtest/gtest.h>

#include "../common/shared_types.h"
#define private public
#include "../tx_engine/tx_connection.h"
#undef private

#define private public
#include "../tx_engine/tx_sender.h"
#undef private

#include "../tx_engine/tx_receiver.h"

#define private public
#include "../latency/latency_tracker.h"
#undef private

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <csignal>
#include <sys/socket.h>
#include <sys/resource.h>
#include <unistd.h>

#include <array>
#include <atomic>
#include <cerrno>
#include <string>
#include <type_traits>

static_assert(std::is_member_function_pointer_v<decltype(&TxConnection::pollConnectStep)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxConnection::takeConnectEvent)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxConnection::takeDisconnectEvent)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxConnection::isConnected)>);

static_assert(std::is_member_function_pointer_v<decltype(&TxReceiver::pollOnce)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::acceptTransportControl)>);

TEST(TxLogRecordTest, inboundFrameCarriesFixedPayloadForReceiverToSenderHandoff) {
    TxInboundFrame frame {};
    frame.payload[0] = 0x00;
    frame.payload[1] = 0x01;
    frame.payload[2] = static_cast<uint8_t>('H');
    frame.payload_length = 3;

    EXPECT_EQ(frame.payload_length, 3U);
    EXPECT_EQ(frame.payload[2], static_cast<uint8_t>('H'));
}

TEST(TxLogRecordTest, transportEventCanRepresentConnectedAndDisconnectedStates) {
    const TxTransportEvent connected = TxTransportEvent::Connected;
    const TxTransportEvent disconnected = TxTransportEvent::Disconnected;

    EXPECT_NE(connected, disconnected);
}

TEST(TxLogRecordTest, transportControlCanCarryConnectedGenerationAndSenderFd) {
    TxTransportControl control {
        .kind = TxTransportControlKind::Connected,
        .generation = 7,
        .tx_fd = 19,
    };

    EXPECT_EQ(control.kind, TxTransportControlKind::Connected);
    EXPECT_EQ(control.generation, 7U);
    EXPECT_EQ(control.tx_fd, 19);
}

TEST(TxLogRecordTest, disconnectedTransportControlDoesNotRequireSenderFd) {
    TxTransportControl control {
        .kind = TxTransportControlKind::Disconnected,
        .generation = 8,
        .tx_fd = -1,
    };

    EXPECT_EQ(control.kind, TxTransportControlKind::Disconnected);
    EXPECT_EQ(control.generation, 8U);
    EXPECT_LT(control.tx_fd, 0);
}

TEST(TxLogRecordTest, txReceiverForwardsInboundFrameIntoSenderOwnedQueue) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];

    TxSender sender(8);
    TxReceiver receiver(connection, sender);

    const std::array<uint8_t, 3> frame {0x00, 0x01, static_cast<uint8_t>('H')};
    ASSERT_EQ(::send(sockets[1], frame.data(), frame.size(), 0), 3);

    ASSERT_TRUE(receiver.pollOnce());

    TxInboundFrame inbound {};
    ASSERT_TRUE(sender.m_inbound_frames.pop(inbound));
    EXPECT_EQ(inbound.payload_length, frame.size());

    ::close(sockets[0]);
    ::close(sockets[1]);
    connection.m_socket_fd = -1;
}

TEST(TxLogRecordTest, txReceiverForwardsConnectEventIntoSenderOwnedQueue) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];

    TxSender sender(8);
    TxReceiver receiver(connection, sender);

    connection.m_connect_event_pending = true;
    ASSERT_TRUE(receiver.pollOnce());

    TxTransportEvent event {};
    ASSERT_TRUE(sender.m_transport_events.pop(event));
    EXPECT_EQ(event, TxTransportEvent::Connected);

    ::close(sockets[0]);
    ::close(sockets[1]);
    connection.m_socket_fd = -1;
}

TEST(TxLogRecordTest, txReceiverForwardsDisconnectEventIntoSenderOwnedQueue) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];

    TxSender sender(8);
    TxReceiver receiver(connection, sender);

    ::close(sockets[1]);
    sockets[1] = -1;

    ASSERT_TRUE(receiver.pollOnce());

    TxTransportEvent event {};
    ASSERT_TRUE(sender.m_transport_events.pop(event));
    EXPECT_EQ(event, TxTransportEvent::Disconnected);

    ::close(sockets[0]);
    connection.m_socket_fd = -1;
}

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
    record.event_tag = 123456789ULL;
    record.payload_length = 16;

    EXPECT_EQ(record.user_ref_num, 42U);
    EXPECT_EQ(record.que_idx, 1U);
    EXPECT_EQ(record.event_tag, 123456789ULL);
    EXPECT_EQ(record.payload_length, 16U);
}

TEST(TxLogRecordTest, txEventRecordCanRepresentAcceptedFeedback) {
    TxLogRecord record {};
    record.event = TxEventKind::OrderAccepted;

    EXPECT_EQ(record.event, TxEventKind::OrderAccepted);
}

TEST(TxLogRecordTest, orderSentHelperDoesNotPrintDirectlyWithoutLogPrinter) {
    TxConnection connection {};
    TxOutboundRecord record {};
    record.user_ref_num = 42;
    record.stock_locate = 0x000d;
    record.price = 123450;
    record.shares = 100;

    testing::internal::CaptureStdout();
    connection._logOrderSent(record);
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.empty());
}

TEST(TxLogRecordTest, connectEventDefaultsToFalse) {
    TxConnection connection {};

    EXPECT_FALSE(connection.takeConnectEvent());
}

TEST(TxLogRecordTest, sendPayloadWritesPayloadToSocket) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];

    TxOutboundRecord record {};
    record.user_ref_num = 42;
    record.payload[0] = 0x00;
    record.payload[1] = 0x02;
    record.payload[2] = static_cast<uint8_t>('U');
    record.payload[3] = static_cast<uint8_t>('R');
    record.payload_length = 4;

    ASSERT_TRUE(connection.sendPayload(record));

    std::array<uint8_t, 4> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0), 4);
    EXPECT_EQ(received[0], record.payload[0]);
    EXPECT_EQ(received[1], record.payload[1]);
    EXPECT_EQ(received[2], record.payload[2]);
    EXPECT_EQ(received[3], record.payload[3]);
    EXPECT_FALSE(connection.takeDisconnectEvent());

    ::close(sockets[0]);
    ::close(sockets[1]);
    connection.m_socket_fd = -1;
}

TEST(TxLogRecordTest, sendPayloadDoesNotRaiseSigpipeOnClosedSocket) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];

    ::close(sockets[1]);
    sockets[1] = -1;

    static std::atomic<int> sigpipe_hits {0};
    sigpipe_hits.store(0, std::memory_order_relaxed);
    const auto handler = +[](int) {
        sigpipe_hits.fetch_add(1, std::memory_order_relaxed);
    };

    struct sigaction old_action {};
    struct sigaction new_action {};
    new_action.sa_handler = handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    ASSERT_EQ(::sigaction(SIGPIPE, &new_action, &old_action), 0);

    TxOutboundRecord record {};
    record.payload[0] = 0x00;
    record.payload[1] = 0x02;
    record.payload[2] = static_cast<uint8_t>('U');
    record.payload[3] = static_cast<uint8_t>('R');
    record.payload_length = 4;

    EXPECT_FALSE(connection.sendPayload(record));
    EXPECT_EQ(sigpipe_hits.load(std::memory_order_relaxed), 0);

    ASSERT_EQ(::sigaction(SIGPIPE, &old_action, nullptr), 0);
    // sendPayload() may close the fd on disconnect.
    if (connection.m_socket_fd >= 0) {
        ::close(sockets[0]);
        connection.m_socket_fd = -1;
    }
}

TEST(TxLogRecordTest, successfulTrackedSendPushesTxSendRecord) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    LatencyTracker tracker(2, 8);
    connection.attachLatenyTracker(&tracker);
    connection.m_socket_fd = sockets[0];

    TxOutboundRecord record {};
    record.que_idx = 1;
    record.event_tag = 0x55ULL;
    record.payload[0] = 0x00;
    record.payload[1] = 0x02;
    record.payload[2] = static_cast<uint8_t>('U');
    record.payload[3] = static_cast<uint8_t>('R');
    record.payload_length = 4;

    ASSERT_TRUE(connection.sendPayload(record));

    TimeRecord pushed {};
    ASSERT_TRUE(tracker.m_trace_buffer[1]->pop(pushed));
    EXPECT_EQ(pushed.event_stage, stage::TX_SEND);
    EXPECT_EQ(pushed.que_idx, 1U);
    EXPECT_EQ(pushed.event_tag, 0x55ULL);
    EXPECT_GT(pushed.time_captured, 0U);
    EXPECT_FALSE(tracker.m_trace_buffer[1]->pop(pushed));

    ::close(sockets[0]);
    ::close(sockets[1]);
    connection.m_socket_fd = -1;
}

TEST(TxLogRecordTest, sendPayloadDupFailureDoesNotForceDisconnectEvent) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];

    struct rlimit old_limit {};
    ASSERT_EQ(::getrlimit(RLIMIT_NOFILE, &old_limit), 0);

    struct rlimit limited = old_limit;
    limited.rlim_cur = static_cast<rlim_t>(sockets[0] + 1);
    ASSERT_LT(limited.rlim_cur, old_limit.rlim_cur);
    ASSERT_EQ(::setrlimit(RLIMIT_NOFILE, &limited), 0);

    TxOutboundRecord record {};
    record.payload[0] = 0x00;
    record.payload[1] = 0x02;
    record.payload[2] = static_cast<uint8_t>('U');
    record.payload[3] = static_cast<uint8_t>('R');
    record.payload_length = 4;

    EXPECT_FALSE(connection.sendPayload(record));
    EXPECT_TRUE(connection.isConnected());
    EXPECT_FALSE(connection.takeDisconnectEvent());

    ASSERT_EQ(::setrlimit(RLIMIT_NOFILE, &old_limit), 0);

    ::close(sockets[0]);
    ::close(sockets[1]);
    connection.m_socket_fd = -1;
}

TEST(TxLogRecordTest, txConnectionDestructorClosesSocketFd) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    const int fd = sockets[0];
    {
        TxConnection connection {};
        connection.m_socket_fd = fd;
    }

    errno = 0;
    EXPECT_EQ(::fcntl(fd, F_GETFD), -1);
    EXPECT_EQ(errno, EBADF);

    ::close(sockets[1]);
}

TEST(TxLogRecordTest, sendPayloadSucceedsWhenLatencyTrackerThrows) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    LatencyTracker tracker(1, 8);
    connection.attachLatenyTracker(&tracker);
    connection.m_socket_fd = sockets[0];

    TxOutboundRecord record {};
    record.que_idx = 1;
    record.event_tag = 0x55ULL;
    record.payload[0] = 0x00;
    record.payload[1] = 0x02;
    record.payload[2] = static_cast<uint8_t>('U');
    record.payload[3] = static_cast<uint8_t>('R');
    record.payload_length = 4;

    ASSERT_TRUE(connection.sendPayload(record));

    ::close(sockets[0]);
    ::close(sockets[1]);
    connection.m_socket_fd = -1;
}

TEST(TxLogRecordTest, txConnectionReadInboundFrameReadsFrameAndDetectsDisconnect) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];

    const std::array<uint8_t, 3> frame {0x00, 0x01, static_cast<uint8_t>('H')};
    ASSERT_EQ(::send(sockets[1], frame.data(), frame.size(), 0), 3);

    TxInboundFrame inbound {};
    ASSERT_TRUE(connection.readInboundFrame(inbound));
    EXPECT_EQ(inbound.payload_length, frame.size());
    EXPECT_EQ(inbound.payload[2], static_cast<uint8_t>('H'));

    ::close(sockets[1]);
    sockets[1] = -1;

    EXPECT_FALSE(connection.readInboundFrame(inbound));
    EXPECT_TRUE(connection.takeDisconnectEvent());

    ::close(sockets[0]);
    connection.m_socket_fd = -1;
}

TEST(TxLogRecordTest, txConnectionReadInboundFrameDoesNotDisconnectOnPartialNonBlocking) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];
    const int flags = ::fcntl(sockets[0], F_GETFL, 0);
    ASSERT_GE(flags, 0);
    ASSERT_EQ(::fcntl(sockets[0], F_SETFL, flags | O_NONBLOCK), 0);

    const std::array<uint8_t, 3> frame {0x00, 0x01, static_cast<uint8_t>('H')};
    ASSERT_EQ(::send(sockets[1], frame.data(), 1, 0), 1);

    TxInboundFrame inbound {};
    EXPECT_FALSE(connection.readInboundFrame(inbound));
    EXPECT_FALSE(connection.takeDisconnectEvent());

    ASSERT_EQ(::send(sockets[1], frame.data() + 1, 2, 0), 2);
    EXPECT_TRUE(connection.readInboundFrame(inbound));
    EXPECT_EQ(inbound.payload_length, frame.size());
    EXPECT_EQ(inbound.payload[2], static_cast<uint8_t>('H'));

    ::close(sockets[0]);
    ::close(sockets[1]);
    connection.m_socket_fd = -1;
}

TEST(TxLogRecordTest, txConnectionReadInboundFrameHandlesMaxSoupFrame) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];

    std::array<uint8_t, 67> frame {};
    frame[0] = 0x00;
    frame[1] = 0x41;
    frame[2] = static_cast<uint8_t>('S');
    frame[66] = static_cast<uint8_t>('Z');
    ASSERT_EQ(::send(sockets[1], frame.data(), frame.size(), 0),
              static_cast<ssize_t>(frame.size()));

    TxInboundFrame inbound {};
    ASSERT_TRUE(connection.readInboundFrame(inbound));
    EXPECT_EQ(inbound.payload_length, frame.size());
    EXPECT_EQ(inbound.payload[0], frame[0]);
    EXPECT_EQ(inbound.payload[1], frame[1]);
    EXPECT_EQ(inbound.payload[2], frame[2]);
    EXPECT_EQ(inbound.payload[66], frame[66]);
    EXPECT_FALSE(connection.takeDisconnectEvent());

    ::close(sockets[0]);
    ::close(sockets[1]);
    connection.m_socket_fd = -1;
}

TEST(TxLogRecordTest, txConnectionEnablesTcpNoDelayOnConnectedSocket) {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(fd, 0);

    TxConnection connection {};
    connection.m_socket_fd = fd;

    ASSERT_TRUE(connection._enableLowLatencySocketOptions());

    int flag = 0;
    socklen_t flag_size = sizeof(flag);
    ASSERT_EQ(::getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, &flag_size), 0);
    EXPECT_EQ(flag, 1);

    ::close(fd);
    connection.m_socket_fd = -1;
}
