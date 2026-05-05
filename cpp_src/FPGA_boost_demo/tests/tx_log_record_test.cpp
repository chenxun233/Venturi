#include <gtest/gtest.h>

#include "../common/shared_types.h"
#define private public
#include "../tx_engine/tx_connector.h"
#include "../tx_engine/tx_receiver.h"
#include "../tx_engine/tx_sender.h"
#undef private

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <type_traits>

static_assert(std::is_member_function_pointer_v<decltype(&TxConnector::attachSender)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxConnector::attachReceiver)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxConnector::pollConnect)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxConnector::recvSenderDisconNotice)>);


TEST(TxLogRecordTest, connectionInfoCanCarryConnectedGenerationAndSenderFd) {
    TxConnectionInfo info {
        .kind = TxConnectionKind::Connected,
        .generation = 7,
        .fd = 19,
    };

    EXPECT_EQ(info.kind, TxConnectionKind::Connected);
    EXPECT_EQ(info.generation, 7U);
    EXPECT_EQ(info.fd, 19);
}

TEST(TxLogRecordTest, disconnectedConnectionInfoDoesNotRequireSenderFd) {
    TxConnectionInfo info {
        .kind = TxConnectionKind::Disconnected,
        .generation = 8,
        .fd = -1,
    };

    EXPECT_EQ(info.kind, TxConnectionKind::Disconnected);
    EXPECT_EQ(info.generation, 8U);
    EXPECT_LT(info.fd, 0);
}

TEST(TxLogRecordTest, successfulConnectPublishesConnectedInfoIntoAttachedSenderQueue) {
    TxConnector connection {};
    TxSender sender(4);
    connection.attachSender(&sender);

    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    constexpr int kStableFd = 11;
    ASSERT_EQ(::dup2(sockets[0], kStableFd), kStableFd);
    ::close(sockets[0]);
    sockets[0] = -1;

    connection.m_socket_fd = kStableFd;
    connection.m_socket_generation = 0;
    ASSERT_TRUE(connection._updateConnectedInfo());
    EXPECT_EQ(connection.m_sender_connection_info.kind, TxConnectionKind::Connected);
    EXPECT_EQ(connection.m_sender_connection_info.generation, 1U);
    EXPECT_GE(connection.m_sender_connection_info.fd, 0);

    (void)sender.runOnce();

    std::array<uint8_t, 39> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));
    EXPECT_EQ(received[2], static_cast<uint8_t>('L'));

    ::close(sockets[1]);
}

TEST(TxLogRecordTest, matchingDisconnectNoticeClosesCurrentConnection) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxConnector connection {};
    connection.m_socket_fd = sockets[0];
    connection.m_socket_generation = 5;

    ASSERT_TRUE(connection.recvSenderDisconNotice(TxDisconnectNotice {
        .generation = 5,
    }));
    (void)connection.pollConnect();


    ::close(sockets[1]);
}

TEST(TxLogRecordTest, txConnectionDestructorClosesSocketFd) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    const int fd = sockets[0];
    {
        TxConnector connection {};
        connection.m_socket_fd = fd;
    }

    errno = 0;
    EXPECT_EQ(::fcntl(fd, F_GETFD), -1);
    EXPECT_EQ(errno, EBADF);

    ::close(sockets[1]);
}

TEST(TxLogRecordTest, txConnectionDestructorClosesUnreadPublishedSenderFd) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    int published_fd = -1;
    {
        TxConnector connection {};
        TxSender sender(4);
        connection.attachSender(&sender);
        connection.m_socket_fd = sockets[0];
        connection.m_socket_generation = 0;
        ASSERT_TRUE(connection._updateConnectedInfo());
        published_fd = connection.m_sender_connection_info.fd;
        ASSERT_GE(published_fd, 0);
    }

    errno = 0;
    EXPECT_EQ(::fcntl(published_fd, F_GETFD), -1);
    EXPECT_EQ(errno, EBADF);

    ::close(sockets[1]);
}

TEST(TxLogRecordTest, txConnectionEnablesTcpNoDelayOnConnectedSocket) {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(fd, 0);

    TxConnector connection {};
    connection.m_socket_fd = fd;

    ASSERT_TRUE(connection._enableTCP_NODELAY());

    int flag = 0;
    socklen_t flag_size = sizeof(flag);
    ASSERT_EQ(::getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, &flag_size), 0);
    EXPECT_EQ(flag, 1);

    ::close(fd);
    connection.m_socket_fd = -1;
}

TEST(TxLogRecordTest, updateConnectedInfoMakesDirectSenderConnectionInfoAvailable) {
    TxConnector connection {};
    TxSender sender(4);
    connection.attachSender(&sender);

    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    connection.m_socket_fd = sockets[0];
    ASSERT_TRUE(connection._updateConnectedInfo());
    EXPECT_EQ(connection.m_sender_connection_info.kind, TxConnectionKind::Connected);
    EXPECT_EQ(connection.m_sender_connection_info.generation, 1U);
    EXPECT_GE(connection.m_sender_connection_info.fd, 0);
    EXPECT_EQ(sender.m_transport_generation, 1U);
    EXPECT_EQ(sender.m_send_fd, connection.m_sender_connection_info.fd);

    ::close(sockets[1]);
}

TEST(TxLogRecordTest, updateConnectedInfoDoesNotPublishDedicatedReceiverFd) {
    TxConnector connection {};
    TxSender sender(4);
    TxReceiver receiver(8);
    connection.attachSender(&sender);
    connection.attachReceiver(&receiver);

    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    connection.m_socket_fd = sockets[0];
    ASSERT_TRUE(connection._updateConnectedInfo());
    EXPECT_EQ(connection.m_sender_connection_info.kind, TxConnectionKind::Connected);
    EXPECT_GE(connection.m_sender_connection_info.fd, 0);
    EXPECT_EQ(connection.m_receiver_connection_info.kind, TxConnectionKind::Connected);
    EXPECT_LT(connection.m_receiver_connection_info.fd, 0);

    ::close(sockets[1]);
}
