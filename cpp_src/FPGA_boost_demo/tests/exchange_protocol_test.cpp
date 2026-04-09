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

void writeBigEndian16(uint8_t* out, uint16_t value) {
    out[0] = static_cast<uint8_t>((value >> 8) & 0xffU);
    out[1] = static_cast<uint8_t>(value & 0xffU);
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
