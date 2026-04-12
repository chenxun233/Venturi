#include <gtest/gtest.h>

#include "../common/shared_types.h"
#define private public
#include "../tx_engine/tx_connection.h"
#undef private

#define private public
#include "../tx_engine/tx_sender.h"
#undef private

#include "../tx_engine/tx_receiver.h"

#include "../tx_engine/tx_send_socket.h"

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
static_assert(std::is_member_function_pointer_v<decltype(&TxConnection::takeTransportControl)>);
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

TEST(TxLogRecordTest, successfulConnectPublishesConnectedControlWithDuplicatedSenderFd) {
    TxConnection connection {};

    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    // Mirror the original test intent (use a stable fd number) while ensuring it is valid.
    constexpr int kStableFd = 11;
    ASSERT_EQ(::dup2(sockets[0], kStableFd), kStableFd);
    ::close(sockets[0]);
    sockets[0] = -1;

    connection.m_socket_fd = kStableFd;
    connection.m_generation = 0;
    ASSERT_TRUE(connection._publishConnectedControlForCurrentSocket());

    TxTransportControl control {};
    ASSERT_TRUE(connection.takeTransportControl(control));
    EXPECT_EQ(control.kind, TxTransportControlKind::Connected);
    EXPECT_EQ(control.generation, 1U);
    EXPECT_GE(control.tx_fd, 0);

    if (control.tx_fd >= 0) {
        ::close(control.tx_fd);
        control.tx_fd = -1;
    }
    ::close(sockets[1]);
}

TEST(TxLogRecordTest, handleDisconnectPublishesDisconnectedControlForCurrentGeneration) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];
    connection.m_generation = 5;

    connection._handleDisconnect("test disconnect");

    TxTransportControl control {};
    ASSERT_TRUE(connection.takeTransportControl(control));
    EXPECT_EQ(control.kind, TxTransportControlKind::Disconnected);
    EXPECT_EQ(control.generation, 5U);
    EXPECT_LT(control.tx_fd, 0);

    ::close(sockets[1]);
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

    TxSenderInboundRecord inbound {};
    ASSERT_TRUE(sender.m_inbound_records.pop(inbound));
    EXPECT_EQ(inbound.kind, TxSenderInboundKind::Frame);
    EXPECT_EQ(inbound.frame.payload_length, frame.size());

    ::close(sockets[1]);
}

TEST(TxLogRecordTest, txReceiverForwardsConnectEventIntoSenderOwnedQueue) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];
    ASSERT_TRUE(connection._publishConnectedControlForCurrentSocket());

    TxSender sender(8);
    TxReceiver receiver(connection, sender);

    ASSERT_TRUE(receiver.pollOnce());

    TxSenderInboundRecord inbound {};
    ASSERT_TRUE(sender.m_inbound_records.pop(inbound));
    EXPECT_EQ(inbound.kind, TxSenderInboundKind::TransportEvent);
    EXPECT_EQ(inbound.transport_event, TxTransportEvent::Connected);

    ::close(sockets[1]);
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

    TxSenderInboundRecord inbound {};
    ASSERT_TRUE(sender.m_inbound_records.pop(inbound));
    EXPECT_EQ(inbound.kind, TxSenderInboundKind::TransportEvent);
    EXPECT_EQ(inbound.transport_event, TxTransportEvent::Disconnected);

    EXPECT_LT(connection.m_socket_fd, 0);
}

TEST(TxLogRecordTest, txReceiverRetriesRetainedInboundFrameWhenSenderQueueHasSpace) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];

    TxSender sender(TxSenderConfig {
        .intent_capacity = 1,
        .pending_capacity = 1,
        .inbound_capacity = 1,
        .transport_capacity = 1,
    });
    TxReceiver receiver(connection, sender);

    std::size_t queued_transport_events = 0;
    while (sender.acceptTransportEvent(TxTransportEvent::Connected)) {
        ++queued_transport_events;
    }
    ASSERT_GT(queued_transport_events, 0U);

    const std::array<uint8_t, 3> frame {0x00, 0x01, static_cast<uint8_t>('H')};
    ASSERT_EQ(::send(sockets[1], frame.data(), frame.size(), 0), 3);
    ASSERT_TRUE(receiver.pollOnce());

    TxSenderInboundRecord ingress {};
    ASSERT_TRUE(sender.m_inbound_records.pop(ingress));
    EXPECT_EQ(ingress.kind, TxSenderInboundKind::TransportEvent);
    EXPECT_EQ(ingress.transport_event, TxTransportEvent::Connected);

    ASSERT_TRUE(receiver.pollOnce());
    bool found_frame = false;
    while (sender.m_inbound_records.pop(ingress)) {
        if (ingress.kind != TxSenderInboundKind::Frame) {
            continue;
        }

        found_frame = true;
        EXPECT_EQ(ingress.frame.payload_length, frame.size());
        EXPECT_EQ(ingress.frame.payload[2], static_cast<uint8_t>('H'));
        break;
    }
    EXPECT_TRUE(found_frame);

    ::close(sockets[1]);
}

TEST(TxLogRecordTest, txReceiverPollOnceReportsWorkPendingUnderRetainedBackpressure) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];

    TxSender sender(TxSenderConfig {
        .intent_capacity = 1,
        .pending_capacity = 1,
        .inbound_capacity = 1,
        .transport_capacity = 1,
    });
    TxReceiver receiver(connection, sender);

    std::size_t queued_transport_events = 0;
    while (sender.acceptTransportEvent(TxTransportEvent::Connected)) {
        ++queued_transport_events;
    }
    ASSERT_GT(queued_transport_events, 0U);

    const std::array<uint8_t, 3> frame {0x00, 0x01, static_cast<uint8_t>('H')};
    ASSERT_EQ(::send(sockets[1], frame.data(), frame.size(), 0), 3);
    ASSERT_TRUE(receiver.pollOnce());
    EXPECT_TRUE(receiver.pollOnce());

    TxSenderInboundRecord ingress {};
    ASSERT_TRUE(sender.m_inbound_records.pop(ingress));
    EXPECT_EQ(ingress.kind, TxSenderInboundKind::TransportEvent);
    EXPECT_EQ(ingress.transport_event, TxTransportEvent::Connected);

    ASSERT_TRUE(receiver.pollOnce());
    bool found_frame = false;
    while (sender.m_inbound_records.pop(ingress)) {
        if (ingress.kind != TxSenderInboundKind::Frame) {
            continue;
        }

        found_frame = true;
        EXPECT_EQ(ingress.frame.payload_length, frame.size());
        EXPECT_EQ(ingress.frame.payload[2], static_cast<uint8_t>('H'));
        break;
    }
    EXPECT_TRUE(found_frame);

    ::close(sockets[1]);
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

TEST(TxLogRecordTest, txSendSocketWritesPayloadToInstalledFd) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSendSocket send_socket {};
    send_socket.install(TxTransportControl {
        .kind = TxTransportControlKind::Connected,
        .generation = 3,
        .tx_fd = sockets[0],
    });

    TxOutboundRecord record {};
    record.payload[0] = 0x00;
    record.payload[1] = 0x02;
    record.payload[2] = static_cast<uint8_t>('U');
    record.payload[3] = static_cast<uint8_t>('R');
    record.payload_length = 4;

    ASSERT_TRUE(send_socket.sendPayload(record));

    std::array<uint8_t, 4> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0), 4);
    EXPECT_EQ(received[2], static_cast<uint8_t>('U'));
    EXPECT_EQ(received[3], static_cast<uint8_t>('R'));

    ::close(sockets[1]);
}

TEST(TxLogRecordTest, txSendSocketClosesLocalFdWhenSendFails) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSendSocket send_socket {};
    send_socket.install(TxTransportControl {
        .kind = TxTransportControlKind::Connected,
        .generation = 4,
        .tx_fd = sockets[0],
    });

    ::close(sockets[1]);
    sockets[1] = -1;

    TxOutboundRecord record {};
    record.payload[0] = 0x00;
    record.payload[1] = 0x02;
    record.payload[2] = static_cast<uint8_t>('U');
    record.payload[3] = static_cast<uint8_t>('R');
    record.payload_length = 4;

    EXPECT_FALSE(send_socket.sendPayload(record));
    EXPECT_FALSE(send_socket.hasActiveFd());
}

TEST(TxLogRecordTest, txSendSocketStaleRetireDoesNotCloseNewGeneration) {
    int sockets1[2] {-1, -1};
    int sockets2[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets1), 0);
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets2), 0);

    TxSendSocket send_socket {};
    send_socket.install(TxTransportControl {
        .kind = TxTransportControlKind::Connected,
        .generation = 1,
        .tx_fd = sockets1[0],
    });

    send_socket.install(TxTransportControl {
        .kind = TxTransportControlKind::Connected,
        .generation = 2,
        .tx_fd = sockets2[0],
    });

    send_socket.retireGeneration(1);
    EXPECT_TRUE(send_socket.hasActiveFd());
    EXPECT_EQ(send_socket.activeGeneration(), 2U);

    TxOutboundRecord record {};
    record.payload[0] = static_cast<uint8_t>('A');
    record.payload[1] = static_cast<uint8_t>('B');
    record.payload_length = 2;
    ASSERT_TRUE(send_socket.sendPayload(record));

    std::array<uint8_t, 2> received {};
    ASSERT_EQ(::recv(sockets2[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));
    EXPECT_EQ(received[0], record.payload[0]);
    EXPECT_EQ(received[1], record.payload[1]);

    send_socket.retireGeneration(2);
    ::close(sockets1[1]);
    ::close(sockets2[1]);
}

TEST(TxLogRecordTest, txSendSocketMatchingRetireClosesActiveFd) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSendSocket send_socket {};
    send_socket.install(TxTransportControl {
        .kind = TxTransportControlKind::Connected,
        .generation = 3,
        .tx_fd = sockets[0],
    });

    EXPECT_TRUE(send_socket.hasActiveFd());
    send_socket.retireGeneration(3);
    EXPECT_FALSE(send_socket.hasActiveFd());

    ::close(sockets[1]);
}

TEST(TxLogRecordTest, txSendSocketLateRetireOfOldGenerationKeepsNewFd) {
    int sockets1[2] {-1, -1};
    int sockets2[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets1), 0);
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets2), 0);

    TxSendSocket send_socket {};
    send_socket.install(TxTransportControl {
        .kind = TxTransportControlKind::Connected,
        .generation = 4,
        .tx_fd = sockets1[0],
    });

    send_socket.retireGeneration(4);
    ::close(sockets1[1]);

    send_socket.install(TxTransportControl {
        .kind = TxTransportControlKind::Connected,
        .generation = 5,
        .tx_fd = sockets2[0],
    });

    send_socket.retireGeneration(4);
    EXPECT_TRUE(send_socket.hasActiveFd());
    EXPECT_EQ(send_socket.activeGeneration(), 5U);

    TxOutboundRecord record {};
    record.payload[0] = static_cast<uint8_t>('C');
    record.payload[1] = static_cast<uint8_t>('D');
    record.payload_length = 2;
    ASSERT_TRUE(send_socket.sendPayload(record));

    std::array<uint8_t, 2> received {};
    ASSERT_EQ(::recv(sockets2[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));
    EXPECT_EQ(received[0], record.payload[0]);
    EXPECT_EQ(received[1], record.payload[1]);

    send_socket.retireGeneration(5);
    ::close(sockets2[1]);
}

TEST(TxLogRecordTest, successfulTrackedSendPushesTxSendRecord) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    LatencyTracker tracker(2, 8);
    TxSendSocket send_socket {};
    send_socket.attachLatenyTracker(&tracker);
    send_socket.install(TxTransportControl {
        .kind = TxTransportControlKind::Connected,
        .generation = 0,
        .tx_fd = sockets[0],
    });

    TxOutboundRecord record {};
    record.que_idx = 1;
    record.event_tag = 0x55ULL;
    record.payload[0] = 0x00;
    record.payload[1] = 0x02;
    record.payload[2] = static_cast<uint8_t>('U');
    record.payload[3] = static_cast<uint8_t>('R');
    record.payload_length = 4;

    ASSERT_TRUE(send_socket.sendPayload(record));

    TimeRecord pushed {};
    ASSERT_TRUE(tracker.m_trace_buffer[1]->pop(pushed));
    EXPECT_EQ(pushed.event_stage, stage::TX_SEND);
    EXPECT_EQ(pushed.que_idx, 1U);
    EXPECT_EQ(pushed.event_tag, 0x55ULL);
    EXPECT_GT(pushed.time_captured, 0U);
    EXPECT_FALSE(tracker.m_trace_buffer[1]->pop(pushed));

    ::close(sockets[1]);
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

TEST(TxLogRecordTest, txConnectionReadInboundFrameReadsFrameAndDetectsDisconnect) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];
    connection.m_generation = 7;

    const std::array<uint8_t, 3> frame {0x00, 0x01, static_cast<uint8_t>('H')};
    ASSERT_EQ(::send(sockets[1], frame.data(), frame.size(), 0), 3);

    TxInboundFrame inbound {};
    ASSERT_TRUE(connection.readInboundFrame(inbound));
    EXPECT_EQ(inbound.payload_length, frame.size());
    EXPECT_EQ(inbound.payload[2], static_cast<uint8_t>('H'));

    ::close(sockets[1]);
    sockets[1] = -1;

    EXPECT_FALSE(connection.readInboundFrame(inbound));
    TxTransportControl control {};
    ASSERT_TRUE(connection.takeTransportControl(control));
    EXPECT_EQ(control.kind, TxTransportControlKind::Disconnected);
    EXPECT_EQ(control.generation, 7U);
    EXPECT_LT(control.tx_fd, 0);

    EXPECT_LT(connection.m_socket_fd, 0);
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
    TxTransportControl control {};
    EXPECT_FALSE(connection.takeTransportControl(control));

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
    TxTransportControl control {};
    EXPECT_FALSE(connection.takeTransportControl(control));

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
