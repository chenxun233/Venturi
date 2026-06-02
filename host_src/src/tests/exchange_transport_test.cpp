#include "../exchange/exchange_transport.h"

#include <gtest/gtest.h>

#include <sys/socket.h>
#include <unistd.h>

TEST(ExchangeTransportTest, attachAndDetachSocketBySlot) {
    ExchangeTransport transport(2);
    int socket_pair[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair), 0);

    transport.attachClientFd(1, socket_pair[0]);
    EXPECT_TRUE(transport.hasSocket(1));
    EXPECT_EQ(transport.readFd(1), socket_pair[0]);

    transport.closeConnection(1);
    EXPECT_FALSE(transport.hasSocket(1));

    ::close(socket_pair[1]);
}
