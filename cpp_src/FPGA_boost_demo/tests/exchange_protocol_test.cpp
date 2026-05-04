#include "../exchange/exchange_protocol.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kUsernameWidth = 6;
constexpr std::size_t kPasswordWidth = 10;
constexpr std::size_t kSessionWidth = 10;
constexpr std::size_t kSequenceWidth = 20;
constexpr std::size_t kLoginPayloadSize =
    kUsernameWidth + kPasswordWidth + kSessionWidth + kSequenceWidth;
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

std::vector<uint8_t> buildLoginPacketForProtocolTest(const ProtocolConfig& config) {
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

    std::vector<uint8_t> frame(3 + payload.size(), 0);
    writeBigEndian16(frame.data(), static_cast<uint16_t>(payload.size() + 1U));
    frame[2] = static_cast<uint8_t>('L');
    std::copy(payload.begin(), payload.end(), frame.begin() + 3);
    return frame;
}

std::vector<uint8_t> buildEnterOrderPacketForProtocolTest(uint32_t user_ref_num) {
    std::array<uint8_t, kEnterOrderPayloadSize> payload {};
    payload[0] = static_cast<uint8_t>('O');
    writeBigEndian32(payload.data() + 1, user_ref_num);
    payload[5] = static_cast<uint8_t>('B');
    writeBigEndian16(payload.data() + 6, 0x000d);
    writeBigEndian32(payload.data() + 8, 100);
    writeBigEndian32(payload.data() + 12, 123450);

    std::vector<uint8_t> frame(3 + payload.size(), 0);
    writeBigEndian16(frame.data(), static_cast<uint16_t>(payload.size() + 1U));
    frame[2] = static_cast<uint8_t>('U');
    std::copy(payload.begin(), payload.end(), frame.begin() + 3);
    return frame;
}

void logInProtocolSlot(ExchangeProtocol& protocol,
                       const ProtocolConfig& config,
                       std::chrono::steady_clock::time_point now) {
    const std::vector<uint8_t> login = buildLoginPacketForProtocolTest(config);
    ASSERT_TRUE(protocol.appendBytes(0, login.data(), login.size(), now));
    ASSERT_TRUE(protocol.parseBytes(0, now));
    ASSERT_TRUE(protocol.hasOutboundFrame(0));
    EXPECT_EQ(protocol.readFrontOutboundFrame(0).payload[2], static_cast<uint8_t>('A'));
    protocol.eraseFrontOutboundFrame(0);
}

} // namespace

TEST(ExchangeProtocolTest, activeSlotsKeepProtocolStateIndependent) {
    ProtocolConfig config {};
    ExchangeProtocol protocol(config, 2);
    const auto now = std::chrono::steady_clock::now();

    protocol.activateSlot(0, 101, now);
    protocol.activateSlot(1, 202, now);

    const std::vector<uint8_t> login = buildLoginPacketForProtocolTest(config);
    ASSERT_TRUE(protocol.appendBytes(0, login.data(), login.size(), now));
    ASSERT_TRUE(protocol.parseBytes(0, now));

    EXPECT_TRUE(protocol.hasOutboundFrame(0));
    EXPECT_FALSE(protocol.hasOutboundFrame(1));
}

TEST(ExchangeProtocolTest, explicitReadAndHandlePathMatchesParseBytesFlow) {
    ProtocolConfig config {};
    ExchangeProtocol protocol(config, 1);
    const auto now = std::chrono::steady_clock::now();

    protocol.activateSlot(0, 101, now);

    const std::vector<uint8_t> login = buildLoginPacketForProtocolTest(config);
    ASSERT_TRUE(protocol.appendBytes(0, login.data(), login.size(), now));

    const auto message = protocol.tryReadInboundMessage(0);
    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->kind, InboundMessageKind::LoginRequest);

    EXPECT_TRUE(protocol.handleInboundMessage(0, *message, now));
    EXPECT_TRUE(protocol.hasOutboundFrame(0));

    const OutboundFrameRaw& outbound = protocol.readFrontOutboundFrame(0);
    ASSERT_GE(outbound.size, 3U);
    EXPECT_EQ(outbound.payload[2], static_cast<uint8_t>('A'));

    EXPECT_FALSE(protocol.tryReadInboundMessage(0).has_value());
}

TEST(ExchangeProtocolTest, fullReplayTableEvictsOldEntryInsteadOfRejectingNewOrder) {
    ProtocolConfig config {};
    config.replay_capacity = 1;
    ExchangeProtocol protocol(config, 1);
    const auto now = std::chrono::steady_clock::now();

    protocol.activateSlot(0, 101, now);
    logInProtocolSlot(protocol, config, now);

    const std::vector<uint8_t> first_order = buildEnterOrderPacketForProtocolTest(1);
    ASSERT_TRUE(protocol.appendBytes(0, first_order.data(), first_order.size(), now));
    ASSERT_TRUE(protocol.parseBytes(0, now));
    ASSERT_TRUE(protocol.hasOutboundFrame(0));
    EXPECT_EQ(protocol.readFrontOutboundFrame(0).payload[2], static_cast<uint8_t>('S'));
    EXPECT_EQ(protocol.readFrontOutboundFrame(0).payload[3], static_cast<uint8_t>('A'));
    protocol.eraseFrontOutboundFrame(0);

    const std::vector<uint8_t> second_order = buildEnterOrderPacketForProtocolTest(2);
    ASSERT_TRUE(protocol.appendBytes(0, second_order.data(), second_order.size(), now));
    ASSERT_TRUE(protocol.parseBytes(0, now));
    ASSERT_TRUE(protocol.hasOutboundFrame(0));
    EXPECT_EQ(protocol.readFrontOutboundFrame(0).payload[2], static_cast<uint8_t>('S'));
    EXPECT_EQ(protocol.readFrontOutboundFrame(0).payload[3], static_cast<uint8_t>('A'));
}
