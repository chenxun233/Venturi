#include "../exchange/dummy_server.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace {

constexpr std::size_t kUsernameWidth = 6;
constexpr std::size_t kPasswordWidth = 10;
constexpr std::size_t kSessionWidth = 10;
constexpr std::size_t kSequenceWidth = 20;
constexpr std::size_t kLoginPayloadSize = kUsernameWidth + kPasswordWidth + kSessionWidth + kSequenceWidth;
constexpr std::size_t kEnterOrderPayloadSize = 16;

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

void writeSequenceField(uint8_t* out, std::size_t width, uint64_t value) {
    std::fill_n(out, static_cast<std::ptrdiff_t>(width), static_cast<uint8_t>(' '));
    const std::string text = std::to_string(value);
    const std::size_t copy_size = std::min(width, text.size());
    std::copy_n(text.end() - static_cast<std::ptrdiff_t>(copy_size),
                static_cast<std::ptrdiff_t>(copy_size),
                out + static_cast<std::ptrdiff_t>(width - copy_size));
}

bool sendAll(int fd, const std::vector<uint8_t>& bytes) {
    std::size_t offset = 0;
    while (offset < bytes.size()) {
        const ssize_t written = ::send(fd,
                                       bytes.data() + static_cast<std::ptrdiff_t>(offset),
                                       bytes.size() - offset,
                                       MSG_NOSIGNAL);
        if (written <= 0) {
            return false;
        }
        offset += static_cast<std::size_t>(written);
    }
    return true;
}

bool readExact(int fd, uint8_t* out, std::size_t size) {
    std::size_t offset = 0;
    while (offset < size) {
        const ssize_t count = ::recv(fd, out + static_cast<std::ptrdiff_t>(offset), size - offset, 0);
        if (count <= 0) {
            return false;
        }
        offset += static_cast<std::size_t>(count);
    }
    return true;
}

std::vector<uint8_t> buildSoupFrame(uint8_t type, const uint8_t* payload, std::size_t payload_size) {
    std::vector<uint8_t> bytes(3 + payload_size, 0);
    writeBigEndian16(bytes.data(), static_cast<uint16_t>(payload_size + 1U));
    bytes[2] = type;
    if (payload_size > 0) {
        std::copy_n(payload, static_cast<std::ptrdiff_t>(payload_size), bytes.data() + 3);
    }
    return bytes;
}

std::vector<uint8_t> buildLoginPacket(const DummyExchangeConfig& config) {
    std::array<uint8_t, kLoginPayloadSize> payload {};
    std::fill(payload.begin(), payload.end(), static_cast<uint8_t>(' '));
    std::copy_n(config.username.data(),
                static_cast<std::ptrdiff_t>(std::min(config.username.size(), kUsernameWidth)),
                payload.data());
    std::copy_n(config.password.data(),
                static_cast<std::ptrdiff_t>(std::min(config.password.size(), kPasswordWidth)),
                payload.data() + static_cast<std::ptrdiff_t>(kUsernameWidth));
    std::copy_n(config.session_id.data(),
                static_cast<std::ptrdiff_t>(std::min(config.session_id.size(), kSessionWidth)),
                payload.data() + static_cast<std::ptrdiff_t>(kUsernameWidth + kPasswordWidth));
    writeSequenceField(payload.data() + static_cast<std::ptrdiff_t>(kUsernameWidth + kPasswordWidth + kSessionWidth),
                       kSequenceWidth,
                       1);
    return buildSoupFrame(static_cast<uint8_t>('L'), payload.data(), payload.size());
}

std::vector<uint8_t> buildHeartbeatPacket() {
    return buildSoupFrame(static_cast<uint8_t>('R'), nullptr, 0);
}

std::vector<uint8_t> buildEnterOrderPacket(uint32_t user_ref_num) {
    std::array<uint8_t, kEnterOrderPayloadSize> payload {};
    payload[0] = static_cast<uint8_t>('O');
    writeBigEndian32(payload.data() + 1, user_ref_num);
    payload[5] = static_cast<uint8_t>('B');
    writeBigEndian16(payload.data() + 6, 0x000d);
    writeBigEndian32(payload.data() + 8, 100);
    writeBigEndian32(payload.data() + 12, 123450);
    return buildSoupFrame(static_cast<uint8_t>('U'), payload.data(), payload.size());
}

bool readPacketType(int fd, uint8_t& type, std::vector<uint8_t>& payload) {
    std::array<uint8_t, 2> header {};
    if (!readExact(fd, header.data(), header.size())) {
        return false;
    }

    const uint16_t encoded_length =
        static_cast<uint16_t>((static_cast<uint16_t>(header[0]) << 8) | static_cast<uint16_t>(header[1]));
    if (encoded_length == 0) {
        return false;
    }

    std::vector<uint8_t> body(encoded_length, 0);
    if (!readExact(fd, body.data(), body.size())) {
        return false;
    }

    type = body[0];
    payload.assign(body.begin() + 1, body.end());
    return true;
}

int connectClient(uint16_t port) {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }

    timeval timeout {};
    timeout.tv_sec = 0;
    timeout.tv_usec = 300000;
    (void)::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    (void)::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        ::close(fd);
        return -1;
    }
    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        ::close(fd);
        return -1;
    }
    return fd;
}

struct ServerStopGuard {
    DummyServer* server {nullptr};

    ~ServerStopGuard() {
        if (server != nullptr) {
            server->requestStop();
        }
    }
};

} // namespace

TEST(DummyExchangeServerTest, extraClientIsRejectedWhenSessionPoolIsFull) {
    DummyExchangeConfig config {};
    config.listen_ip = "127.0.0.1";
    config.port = 9109;
    config.session_capacity = 1;

    DummyServer server(config);
    std::jthread server_thread([&server]() {
        EXPECT_EQ(server.run(), 0);
    });
    ServerStopGuard stop_guard {&server};

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    const int first_fd = connectClient(config.port);
    ASSERT_GE(first_fd, 0);
    const int second_fd = connectClient(config.port);
    ASSERT_GE(second_fd, 0);

    ASSERT_TRUE(sendAll(first_fd, buildLoginPacket(config)));

    uint8_t type = 0;
    std::vector<uint8_t> payload {};
    ASSERT_TRUE(readPacketType(first_fd, type, payload));
    EXPECT_EQ(type, static_cast<uint8_t>('A'));

    EXPECT_FALSE(sendAll(second_fd, buildLoginPacket(config)) && readPacketType(second_fd, type, payload));

    ::close(first_fd);
    ::close(second_fd);
}

TEST(DummyExchangeServerTest, closedLiveSlotCanBeReusedByNextClient) {
    DummyExchangeConfig config {};
    config.listen_ip = "127.0.0.1";
    config.port = 9110;
    config.session_capacity = 1;

    DummyServer server(config);
    std::jthread server_thread([&server]() {
        EXPECT_EQ(server.run(), 0);
    });
    ServerStopGuard stop_guard {&server};

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    const int first_fd = connectClient(config.port);
    ASSERT_GE(first_fd, 0);
    ASSERT_TRUE(sendAll(first_fd, buildLoginPacket(config)));

    uint8_t type = 0;
    std::vector<uint8_t> payload {};
    ASSERT_TRUE(readPacketType(first_fd, type, payload));
    EXPECT_EQ(type, static_cast<uint8_t>('A'));
    ::close(first_fd);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    const int second_fd = connectClient(config.port);
    ASSERT_GE(second_fd, 0);
    ASSERT_TRUE(sendAll(second_fd, buildLoginPacket(config)));
    ASSERT_TRUE(readPacketType(second_fd, type, payload));
    EXPECT_EQ(type, static_cast<uint8_t>('A'));

    ::close(second_fd);
}

TEST(DummyExchangeServerTest, serverCanRestartAfterPreviousRunStops) {
    DummyExchangeConfig config {};
    config.listen_ip = "127.0.0.1";
    config.port = 9111;

    DummyServer server(config);

    {
        std::jthread server_thread([&server]() {
            EXPECT_EQ(server.run(), 0);
        });
        ServerStopGuard stop_guard {&server};

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        const int first_fd = connectClient(config.port);
        ASSERT_GE(first_fd, 0);
        ASSERT_TRUE(sendAll(first_fd, buildLoginPacket(config)));

        uint8_t type = 0;
        std::vector<uint8_t> payload {};
        ASSERT_TRUE(readPacketType(first_fd, type, payload));
        EXPECT_EQ(type, static_cast<uint8_t>('A'));

        ::close(first_fd);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    {
        std::jthread server_thread([&server]() {
            EXPECT_EQ(server.run(), 0);
        });
        ServerStopGuard stop_guard {&server};

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        const int second_fd = connectClient(config.port);
        ASSERT_GE(second_fd, 0);
        ASSERT_TRUE(sendAll(second_fd, buildLoginPacket(config)));

        uint8_t type = 0;
        std::vector<uint8_t> payload {};
        ASSERT_TRUE(readPacketType(second_fd, type, payload));
        EXPECT_EQ(type, static_cast<uint8_t>('A'));

        ::close(second_fd);
    }
}

TEST(DummyExchangeServerTest, oneClientDisconnectDoesNotStopOtherClient) {
    DummyExchangeConfig config {};
    config.listen_ip = "127.0.0.1";
    config.port = 9107;
    config.fill_delay = std::chrono::milliseconds(1);

    DummyServer server(config);
    std::jthread server_thread([&server]() {
        EXPECT_EQ(server.run(), 0);
    });
    ServerStopGuard stop_guard {&server};

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    const int first_fd = connectClient(config.port);
    ASSERT_GE(first_fd, 0);
    const int second_fd = connectClient(config.port);
    ASSERT_GE(second_fd, 0);

    ASSERT_TRUE(sendAll(first_fd, buildLoginPacket(config)));
    ASSERT_TRUE(sendAll(second_fd, buildLoginPacket(config)));

    uint8_t type = 0;
    std::vector<uint8_t> payload {};
    ASSERT_TRUE(readPacketType(first_fd, type, payload));
    EXPECT_EQ(type, static_cast<uint8_t>('A'));
    ASSERT_TRUE(readPacketType(second_fd, type, payload));
    EXPECT_EQ(type, static_cast<uint8_t>('A'));

    ::close(first_fd);
    ASSERT_TRUE(sendAll(second_fd, buildHeartbeatPacket()));
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    ASSERT_TRUE(readPacketType(second_fd, type, payload));
    EXPECT_EQ(type, static_cast<uint8_t>('H'));

    ::close(second_fd);
}

TEST(DummyExchangeServerTest, duplicateTagsAreIndependentAcrossConcurrentClients) {
    DummyExchangeConfig config {};
    config.listen_ip = "127.0.0.1";
    config.port = 9108;
    config.fill_delay = std::chrono::milliseconds(1);

    DummyServer server(config);
    std::jthread server_thread([&server]() {
        EXPECT_EQ(server.run(), 0);
    });
    ServerStopGuard stop_guard {&server};

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    const int first_fd = connectClient(config.port);
    ASSERT_GE(first_fd, 0);
    const int second_fd = connectClient(config.port);
    ASSERT_GE(second_fd, 0);

    ASSERT_TRUE(sendAll(first_fd, buildLoginPacket(config)));
    ASSERT_TRUE(sendAll(second_fd, buildLoginPacket(config)));

    uint8_t type = 0;
    std::vector<uint8_t> payload {};
    ASSERT_TRUE(readPacketType(first_fd, type, payload));
    ASSERT_TRUE(readPacketType(second_fd, type, payload));

    ASSERT_TRUE(sendAll(first_fd, buildEnterOrderPacket(77)));
    ASSERT_TRUE(sendAll(second_fd, buildEnterOrderPacket(77)));

    ASSERT_TRUE(readPacketType(first_fd, type, payload));
    EXPECT_EQ(type, static_cast<uint8_t>('S'));
    ASSERT_FALSE(payload.empty());
    EXPECT_EQ(payload.front(), static_cast<uint8_t>('A'));

    ASSERT_TRUE(readPacketType(second_fd, type, payload));
    EXPECT_EQ(type, static_cast<uint8_t>('S'));
    ASSERT_FALSE(payload.empty());
    EXPECT_EQ(payload.front(), static_cast<uint8_t>('A'));

    ::close(first_fd);
    ::close(second_fd);
}
