#include <gtest/gtest.h>

#include "../common/shared_types.h"
#define private public
#include "../tx_engine/tx_connection.h"
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

static_assert(std::is_member_function_pointer_v<decltype(&TxConnection::attachSender)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxConnection::pollConnect)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxConnection::pushSenderDisconNotice)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxConnection::isConnected)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxConnection::takeSenderConnectionInfo)>);

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
    TxConnection connection {};
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
    ASSERT_TRUE(connection.m_has_sender_connection_info);
    TxConnectionInfo info {};
    ASSERT_TRUE(connection.takeSenderConnectionInfo(info));
    ASSERT_FALSE(connection.m_has_sender_connection_info);
    sender.updateConnectionInfo(info);

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

    TxConnection connection {};
    connection.m_socket_fd = sockets[0];
    connection.m_socket_generation = 5;

    ASSERT_TRUE(connection.pushSenderDisconNotice(TxDisconnectNotice {
        .generation = 5,
    }));
    (void)connection.pollConnect();
    EXPECT_FALSE(connection.isConnected());

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

TEST(TxLogRecordTest, txConnectionDestructorClosesUnreadPublishedSenderFd) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    int published_fd = -1;
    {
        TxConnection connection {};
        connection.m_socket_fd = sockets[0];
        connection.m_socket_generation = 0;
        ASSERT_TRUE(connection._updateConnectedInfo());
        ASSERT_TRUE(connection.m_has_sender_connection_info);
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

    TxConnection connection {};
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
    TxConnection connection {};

    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    connection.m_socket_fd = sockets[0];
    ASSERT_TRUE(connection._updateConnectedInfo());
    ASSERT_TRUE(connection.m_has_sender_connection_info);

    TxConnectionInfo info {};
    ASSERT_TRUE(connection.takeSenderConnectionInfo(info));
    EXPECT_EQ(info.kind, TxConnectionKind::Connected);
    EXPECT_GE(info.fd, 0);

    ::close(info.fd);
    ::close(sockets[1]);
}
