#include "exchange_protocol.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

constexpr uint8_t kSoupLoginAcceptedType = static_cast<uint8_t>('A');
constexpr uint8_t kSoupLoginRejectedType = static_cast<uint8_t>('J');
constexpr uint8_t kSoupSequencedDataType = static_cast<uint8_t>('S');
constexpr uint8_t kSoupHeartbeatType = static_cast<uint8_t>('H');
constexpr uint8_t kSoupLoginRequestType = static_cast<uint8_t>('L');
constexpr uint8_t kSoupUnsequencedDataType = static_cast<uint8_t>('U');
constexpr uint8_t kSoupClientHeartbeatType = static_cast<uint8_t>('R');
constexpr uint8_t kSoupLogoutRequestType = static_cast<uint8_t>('O');

constexpr uint8_t kOuchEnterOrderType = static_cast<uint8_t>('O');
constexpr uint8_t kOuchAcceptedType = static_cast<uint8_t>('A');
constexpr uint8_t kOuchExecutedType = static_cast<uint8_t>('E');
constexpr uint8_t kOuchRejectedType = static_cast<uint8_t>('J');

constexpr std::size_t kSoupHeaderSize = 3;
constexpr std::size_t kSessionSize = 10;
constexpr std::size_t kSequenceSize = 20;
constexpr std::size_t kUsernameSize = 6;
constexpr std::size_t kPasswordSize = 10;
constexpr std::size_t kOuchEnterOrderSize = 16;
constexpr std::size_t kOuchAcceptedSize = 64;
constexpr std::size_t kOuchRejectedSize = 31;
constexpr std::size_t kMaxInboundPayloadSize = 64;

static_assert(kMaxSoupPayloadSize >= kSessionSize + kSequenceSize,
              "max Soup payload must fit login accepted payload");
static_assert(kMaxSoupPayloadSize >= kOuchAcceptedSize,
              "max Soup payload must fit accepted OUCH payload");
static_assert(kMaxSoupPayloadSize >= kOuchExecutedSize,
              "max Soup payload must fit executed OUCH payload");
static_assert(kMaxSoupPayloadSize >= kOuchRejectedSize,
              "max Soup payload must fit rejected OUCH payload");

constexpr uint16_t kRejectUnsupportedSymbol = 0x0011;
constexpr uint16_t kRejectInvalidShares = 0x0002;
constexpr uint16_t kRejectInvalidPrice = 0x0015;
constexpr uint16_t kRejectReplayCapacity = 0x00ff;

struct AcceptedMessage {
    uint64_t timestamp_ns {0};
    uint32_t user_ref_num {0};
    uint16_t stock_locate {0};
    uint32_t shares {0};
    uint32_t price {0};
    char side {'B'};
    uint64_t order_ref_num {0};
};

struct ExecutedMessage {
    uint64_t timestamp_ns {0};
    uint32_t user_ref_num {0};
    uint32_t executed_shares {0};
    uint32_t price {0};
    uint64_t match_number {0};
};

struct RejectedMessage {
    uint64_t timestamp_ns {0};
    uint32_t user_ref_num {0};
    uint16_t reason {0};
};

uint16_t readBigEndian16(const uint8_t* bytes) {
    return static_cast<uint16_t>((static_cast<uint16_t>(bytes[0]) << 8) |
                                 static_cast<uint16_t>(bytes[1]));
}

uint32_t readBigEndian32(const uint8_t* bytes) {
    return (static_cast<uint32_t>(bytes[0]) << 24) |
           (static_cast<uint32_t>(bytes[1]) << 16) |
           (static_cast<uint32_t>(bytes[2]) << 8) |
           static_cast<uint32_t>(bytes[3]);
}

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

void writeBigEndian64(uint8_t* out, uint64_t value) {
    out[0] = static_cast<uint8_t>((value >> 56) & 0xffU);
    out[1] = static_cast<uint8_t>((value >> 48) & 0xffU);
    out[2] = static_cast<uint8_t>((value >> 40) & 0xffU);
    out[3] = static_cast<uint8_t>((value >> 32) & 0xffU);
    out[4] = static_cast<uint8_t>((value >> 24) & 0xffU);
    out[5] = static_cast<uint8_t>((value >> 16) & 0xffU);
    out[6] = static_cast<uint8_t>((value >> 8) & 0xffU);
    out[7] = static_cast<uint8_t>(value & 0xffU);
}

void writePaddedField(uint8_t* out, std::size_t size, std::string_view value, bool left_pad) {
    std::fill_n(out, static_cast<std::ptrdiff_t>(size), static_cast<uint8_t>(' '));
    const std::size_t copy_size = std::min(size, value.size());
    if (left_pad) {
        std::copy_n(value.end() - static_cast<std::ptrdiff_t>(copy_size),
                    static_cast<std::ptrdiff_t>(copy_size),
                    out + static_cast<std::ptrdiff_t>(size - copy_size));
        return;
    }

    std::copy_n(value.begin(), static_cast<std::ptrdiff_t>(copy_size), out);
}

void writeSequenceField(uint8_t* out, std::size_t size, uint64_t value) {
    std::fill_n(out, static_cast<std::ptrdiff_t>(size), static_cast<uint8_t>(' '));
    const std::string text = std::to_string(value);
    const std::size_t copy_size = std::min(size, text.size());
    std::copy_n(text.end() - static_cast<std::ptrdiff_t>(copy_size),
                static_cast<std::ptrdiff_t>(copy_size),
                out + static_cast<std::ptrdiff_t>(size - copy_size));
}

std::string trimSpaces(std::string_view text) {
    std::size_t begin = 0;
    while (begin < text.size() && text[begin] == ' ') {
        ++begin;
    }

    std::size_t end = text.size();
    while (end > begin && text[end - 1] == ' ') {
        --end;
    }

    return std::string(text.substr(begin, end - begin));
}

bool readSequenceField(const uint8_t* data, std::size_t size, uint64_t& value) {
    std::string_view text(reinterpret_cast<const char*>(data), size);
    const std::size_t first_non_space = text.find_first_not_of(' ');
    if (first_non_space == std::string_view::npos) {
        value = 0;
        return true;
    }

    const char* begin = text.data() + static_cast<std::ptrdiff_t>(first_non_space);
    const char* end = text.data() + static_cast<std::ptrdiff_t>(text.size());
    const auto [ptr, ec] = std::from_chars(begin, end, value);
    return ec == std::errc() && ptr == end;
}

std::string_view readSymbolFromLocate(uint16_t stock_locate) {
    switch (stock_locate) {
        case 0x000d:
            return "AAPL";
        case 0x0ee8:
            return "HSBC";
        default:
            return "UNKNOWN";
    }
}

OutboundFrameRaw buildSoupFrame(uint8_t soup_type, const uint8_t* payload, std::size_t payload_size) {
    OutboundFrameRaw frame {};
    writeBigEndian16(frame.payload.data(), static_cast<uint16_t>(payload_size + 1U));
    frame.payload[2] = soup_type;
    if (payload_size > 0) {
        std::copy_n(payload, static_cast<std::ptrdiff_t>(payload_size), frame.payload.data() + kSoupHeaderSize);
    }
    frame.size = kSoupHeaderSize + payload_size;
    frame.offset = 0;
    return frame;
}

OutboundFrameRaw buildSoupLoginAcceptedFrame(std::string_view session_id, uint64_t next_sequence) {
    std::array<uint8_t, kSessionSize + kSequenceSize> payload {};
    writePaddedField(payload.data(), kSessionSize, session_id, true);
    writeSequenceField(payload.data() + static_cast<std::ptrdiff_t>(kSessionSize), kSequenceSize, next_sequence);
    return buildSoupFrame(kSoupLoginAcceptedType, payload.data(), payload.size());
}

OutboundFrameRaw buildSoupLoginRejectedFrame() {
    const uint8_t reject_code = static_cast<uint8_t>('A');
    return buildSoupFrame(kSoupLoginRejectedType, &reject_code, 1U);
}

OutboundFrameRaw buildSoupHeartbeatFrame() {
    return buildSoupFrame(kSoupHeartbeatType, nullptr, 0);
}

OutboundFrameRaw buildSoupSequencedAcceptedFrame(const AcceptedMessage& accepted) {
    std::array<uint8_t, kOuchAcceptedSize> payload {};
    payload[0] = kOuchAcceptedType;
    writeBigEndian64(payload.data() + 1, accepted.timestamp_ns);
    writeBigEndian32(payload.data() + 9, accepted.user_ref_num);
    payload[13] = static_cast<uint8_t>(accepted.side);
    writeBigEndian32(payload.data() + 14, accepted.shares);
    writePaddedField(payload.data() + 18, 8, readSymbolFromLocate(accepted.stock_locate), false);
    writeBigEndian64(payload.data() + 26, accepted.price);
    payload[34] = static_cast<uint8_t>('0');
    payload[35] = static_cast<uint8_t>('Y');
    writeBigEndian64(payload.data() + 36, accepted.order_ref_num);
    payload[44] = static_cast<uint8_t>('A');
    payload[45] = static_cast<uint8_t>('N');
    payload[46] = static_cast<uint8_t>('N');
    payload[47] = static_cast<uint8_t>('L');
    writePaddedField(payload.data() + 48, 14, "", false);
    writeBigEndian16(payload.data() + 62, 0);
    return buildSoupFrame(kSoupSequencedDataType, payload.data(), payload.size());
}

OutboundFrameRaw buildSoupSequencedExecutedFrame(const ExecutedMessage& executed) {
    std::array<uint8_t, kOuchExecutedSize> payload {};
    payload[0] = kOuchExecutedType;
    writeBigEndian64(payload.data() + 1, executed.timestamp_ns);
    writeBigEndian32(payload.data() + 9, executed.user_ref_num);
    writeBigEndian32(payload.data() + 13, executed.executed_shares);
    writeBigEndian64(payload.data() + 17, executed.price);
    payload[25] = static_cast<uint8_t>('A');
    writeBigEndian64(payload.data() + 26, executed.match_number);
    writeBigEndian16(payload.data() + 34, 0);
    return buildSoupFrame(kSoupSequencedDataType, payload.data(), payload.size());
}

OutboundFrameRaw buildSoupSequencedRejectedFrame(const RejectedMessage& rejected) {
    std::array<uint8_t, kOuchRejectedSize> payload {};
    payload[0] = kOuchRejectedType;
    writeBigEndian64(payload.data() + 1, rejected.timestamp_ns);
    writeBigEndian32(payload.data() + 9, rejected.user_ref_num);
    writeBigEndian16(payload.data() + 13, rejected.reason);
    writePaddedField(payload.data() + 15, 14, "", false);
    writeBigEndian16(payload.data() + 29, 0);
    return buildSoupFrame(kSoupSequencedDataType, payload.data(), payload.size());
}

bool parseLoginRequestPayload(const uint8_t* payload, std::size_t payload_size, LoginRequest& request) {
    if (payload_size != kUsernameSize + kPasswordSize + kSessionSize + kSequenceSize) {
        return false;
    }

    request.username.assign(reinterpret_cast<const char*>(payload), kUsernameSize);
    request.password.assign(reinterpret_cast<const char*>(payload + kUsernameSize), kPasswordSize);
    request.requested_session.assign(reinterpret_cast<const char*>(payload + kUsernameSize + kPasswordSize), kSessionSize);
    return readSequenceField(payload + kUsernameSize + kPasswordSize + kSessionSize,
                             kSequenceSize,
                             request.requested_sequence);
}

ExchangeEnterOrder readEnterOrder(const uint8_t* payload, std::size_t payload_size) {
    if (payload_size != kOuchEnterOrderSize || payload[0] != kOuchEnterOrderType) {
        throw std::runtime_error("invalid enter order payload");
    }

    ExchangeEnterOrder order {};
    order.user_ref_num = readBigEndian32(payload + 1);
    order.side = static_cast<char>(payload[5]);
    order.stock_locate = readBigEndian16(payload + 6);
    order.shares = readBigEndian32(payload + 8);
    order.price = readBigEndian32(payload + 12);
    return order;
}

} // namespace

ProtocolSession::ProtocolSession(ProtocolConfig config)
    : m_config(std::move(config)) {
    m_replay_entries.resize(m_config.replay_capacity);
}

void ProtocolSession::reset(uint64_t session_id, std::chrono::steady_clock::time_point now) {
    m_session_id = session_id;
    m_is_logged_in = false;
    m_should_close = false;
    m_next_sequence = 1;
    m_next_order_ref_num = 1;
    m_next_match_number = 1;
    m_last_send_time = now;
    m_last_receive_time = now;
    m_inbound_bytes.clear();
    m_outbound_frame_que.clear();
    m_pending_fills.clear();
    m_replay_count = 0;
    for (ReplayEntry& entry : m_replay_entries) {
        entry = ReplayEntry {};
    }
}

bool ProtocolSession::appendBytes(const uint8_t* bytes,
                                   std::size_t size,
                                   std::chrono::steady_clock::time_point now) {
    if (!m_inbound_bytes.write(bytes, size)) {
        m_should_close = true;
        return false;
    }
    m_last_receive_time = now;
    return true;
}




bool ProtocolSession::parseBytes(std::chrono::steady_clock::time_point now) {
    while (true) {
        const std::optional<InboundMessage> message = tryReadInboundMessage();
        if (!message.has_value()) {
            return true;
        }
        if (!handleInboundMessage(*message, now)) {
            return true;
        }
    }
}

bool ProtocolSession::shouldClose() const {
    return m_should_close;
}

bool ProtocolSession::hasTimedOut(std::chrono::steady_clock::time_point now,
                                   std::chrono::seconds timeout) const {
    return now - m_last_receive_time >= timeout;
}

void ProtocolSession::queueHeartbeatFrames(std::chrono::steady_clock::time_point now) {
    if (m_is_logged_in && (now - m_last_send_time) >= std::chrono::seconds(1)) {
        if (!m_outbound_frame_que.pushBack(buildSoupHeartbeatFrame())) {
            throw std::runtime_error("client out queue full");
        }
    }
}

void ProtocolSession::queueDueFillFrames(std::chrono::steady_clock::time_point now) {
    while (!m_pending_fills.isEmpty()) {
        const PendingFill& fill = m_pending_fills.readFront();
        if (fill.due_time > now) {
            break;
        }
        ++m_next_sequence;
        if (!m_outbound_frame_que.pushBack(buildSoupSequencedExecutedFrame(ExecutedMessage {
                .timestamp_ns = _readTimestampNs(now),
                .user_ref_num = fill.user_ref_num,
                .executed_shares = fill.executed_shares,
                .price = fill.price,
                .match_number = fill.match_number,
            }))) {
            throw std::runtime_error("client out queue full");
        }
        (void)m_pending_fills.eraseFront();
    }
}

uint64_t ProtocolSession::readSessionId() const {
    return m_session_id;
}

bool ProtocolSession::hasOutboundFrame() const {
    return !m_outbound_frame_que.isEmpty();
}

const OutboundFrameRaw& ProtocolSession::readFrontOutboundFrame() const {
    return m_outbound_frame_que.readFront();
}

OutboundFrameRaw& ProtocolSession::readFrontOutboundFrame() {
    return m_outbound_frame_que.readFront();
}

void ProtocolSession::eraseFrontOutboundFrame() {
    (void)m_outbound_frame_que.eraseFront();
}

void ProtocolSession::writeLastSendTime(std::chrono::steady_clock::time_point now) {
    m_last_send_time = now;
}

std::optional<InboundMessage> ProtocolSession::tryReadInboundMessage() {
    if (m_inbound_bytes.readSize() < 2U) {
        return std::nullopt;
    }

    const uint16_t encoded_length = static_cast<uint16_t>(
        (static_cast<uint16_t>(m_inbound_bytes.readAt(0)) << 8) |
        static_cast<uint16_t>(m_inbound_bytes.readAt(1)));
    const std::size_t frame_size = static_cast<std::size_t>(encoded_length) + 2U;
    if (encoded_length == 0 || m_inbound_bytes.readSize() < frame_size) {
        return std::nullopt;
    }

    const uint8_t soup_type = m_inbound_bytes.readAt(2);
    const std::size_t payload_size = static_cast<std::size_t>(encoded_length - 1U);
    std::array<uint8_t, kMaxInboundPayloadSize> payload {};
    if (payload_size > payload.size()) {
        (void)m_inbound_bytes.eraseFrontN(frame_size);
        return InboundMessage {.kind = InboundMessageKind::Invalid};
    }
    if (!m_inbound_bytes.copyFrom(kSoupHeaderSize, payload.data(), payload_size)) {
        return std::nullopt;
    }
    (void)m_inbound_bytes.eraseFrontN(frame_size);

    switch (soup_type) {
        case kSoupLoginRequestType: {
            LoginRequest login {};
            if (!parseLoginRequestPayload(payload.data(), payload_size, login)) {
                return InboundMessage {.kind = InboundMessageKind::Invalid};
            }
            return InboundMessage {
                .kind = InboundMessageKind::LoginRequest,
                .login = std::move(login),
            };
        }
        case kSoupClientHeartbeatType:
            if (payload_size != 0) {
                return InboundMessage {.kind = InboundMessageKind::Invalid};
            }
            return InboundMessage {.kind = InboundMessageKind::ClientHeartbeat};
        case kSoupLogoutRequestType:
            if (payload_size != 0) {
                return InboundMessage {.kind = InboundMessageKind::Invalid};
            }
            return InboundMessage {.kind = InboundMessageKind::LogoutRequest};
        case kSoupUnsequencedDataType:
            try {
                return InboundMessage {
                    .kind = InboundMessageKind::EnterOrder,
                    .order = readEnterOrder(payload.data(), payload_size),
                };
            } catch (const std::runtime_error&) {
                return InboundMessage {.kind = InboundMessageKind::Invalid};
            }
        default:
            return InboundMessage {.kind = InboundMessageKind::Invalid};
    }
}

bool ProtocolSession::handleInboundMessage(const InboundMessage& message,
                                             std::chrono::steady_clock::time_point now) {
    // if not logged in
    if (!m_is_logged_in) {
        if (message.kind != InboundMessageKind::LoginRequest) {
            if (!m_outbound_frame_que.pushBack(buildSoupLoginRejectedFrame())) {
                throw std::runtime_error("client out queue full");
            }
            m_should_close = true;
            return false;
        }

        const std::string username = trimSpaces(message.login.username);
        const std::string password = trimSpaces(message.login.password);
        const std::string requested_session = trimSpaces(message.login.requested_session);
        if (username != m_config.username || password != m_config.password ||
            (!requested_session.empty() && requested_session != m_config.session_id)) {
            if (!m_outbound_frame_que.pushBack(buildSoupLoginRejectedFrame())) {
                throw std::runtime_error("client out queue full");
            }
            m_should_close = true;
            return false;
        }

        m_is_logged_in = true;
        if (!m_outbound_frame_que.pushBack(buildSoupLoginAcceptedFrame(m_config.session_id, 1))) {
            throw std::runtime_error("client out queue full");
        }
        return true;
    }
    // if logged in
    switch (message.kind) {
        case InboundMessageKind::ClientHeartbeat:
            return true;
        case InboundMessageKind::LogoutRequest:
            m_should_close = true;
            return false;
        case InboundMessageKind::EnterOrder: {
            const HandledOrderResult handled = _handleEnterOrder(message.order, now);
            if (!handled.is_duplicate) {
                for (std::size_t index = 0; index < handled.outbound_message_count; ++index) {
                    if (!m_outbound_frame_que.pushBack(handled.outbound_messages[index])) {
                        throw std::runtime_error("client out queue full");
                    }
                }
            }
            return true;
        }
        case InboundMessageKind::Invalid:
        case InboundMessageKind::LoginRequest:
        m_should_close = true;
        return false;
    }
    m_should_close = true;
    return false;
}

ExchangeValidationResult ProtocolSession::_validateEnterOrder(const ExchangeEnterOrder& order) const {
    if (order.stock_locate != 0x000d && order.stock_locate != 0x0ee8) {
        return ExchangeValidationResult {
            .kind = ExchangeValidationKind::Rejected,
            .reject_reason = kRejectUnsupportedSymbol,
        };
    }
    if (order.shares == 0 || order.shares > m_config.max_shares) {
        return ExchangeValidationResult {
            .kind = ExchangeValidationKind::Rejected,
            .reject_reason = kRejectInvalidShares,
        };
    }
    if (order.price < m_config.price_min || order.price > m_config.price_max) {
        return ExchangeValidationResult {
            .kind = ExchangeValidationKind::Rejected,
            .reject_reason = kRejectInvalidPrice,
        };
    }
    return ExchangeValidationResult {};
}

bool ProtocolSession::_insertReplayResult(uint32_t user_ref_num, const HandledOrderResult& result) {
    if (m_replay_entries.empty() || m_replay_count >= m_replay_entries.size()) {
        return false;
    }

    const std::size_t capacity = m_replay_entries.size();
    const std::size_t start = static_cast<std::size_t>(user_ref_num) % capacity;
    for (std::size_t offset = 0; offset < capacity; ++offset) {
        ReplayEntry& entry = m_replay_entries[(start + offset) % capacity];
        if (!entry.is_occupied) {
            entry.is_occupied = true;
            entry.user_ref_num = user_ref_num;
            entry.result = result;
            ++m_replay_count;
            return true;
        }
        if (entry.user_ref_num == user_ref_num) {
            entry.result = result;
            return true;
        }
    }
    return false;
}

std::optional<HandledOrderResult> ProtocolSession::_findReplayResult(uint32_t user_ref_num) const {
    if (m_replay_entries.empty()) {
        return std::nullopt;
    }

    const std::size_t capacity = m_replay_entries.size();
    const std::size_t start = static_cast<std::size_t>(user_ref_num) % capacity;
    for (std::size_t offset = 0; offset < capacity; ++offset) {
        const ReplayEntry& entry = m_replay_entries[(start + offset) % capacity];
        if (!entry.is_occupied) {
            return std::nullopt;
        }
        if (entry.user_ref_num == user_ref_num) {
            return entry.result;
        }
    }
    return std::nullopt;
}

HandledOrderResult ProtocolSession::_buildReplayCapacityReject(uint32_t user_ref_num) {
    HandledOrderResult reject {};
    reject.validation = ExchangeValidationResult {
        .kind = ExchangeValidationKind::Rejected,
        .reject_reason = kRejectReplayCapacity,
    };
    reject.outbound_messages[0] = buildSoupSequencedRejectedFrame(RejectedMessage {
        .timestamp_ns = _readTimestampNs(std::chrono::steady_clock::now()),
        .user_ref_num = user_ref_num,
        .reason = kRejectReplayCapacity,
    });
    reject.outbound_message_count = 1;
    ++m_next_sequence;
    return reject;
}

HandledOrderResult ProtocolSession::_handleEnterOrder(const ExchangeEnterOrder& order,
                                                       std::chrono::steady_clock::time_point now) {
    if (const auto replay = _findReplayResult(order.user_ref_num); replay.has_value()) {
        HandledOrderResult result = *replay;
        result.is_duplicate = true;
        return result;
    }

    HandledOrderResult result {};
    result.validation = _validateEnterOrder(order);

    if (result.validation.kind == ExchangeValidationKind::Rejected) {
        result.outbound_messages[result.outbound_message_count++] = buildSoupSequencedRejectedFrame(RejectedMessage {
            .timestamp_ns = _readTimestampNs(now),
            .user_ref_num = order.user_ref_num,
            .reason = result.validation.reject_reason,
        });
        ++m_next_sequence;
    } else {
        result.outbound_messages[result.outbound_message_count++] = buildSoupSequencedAcceptedFrame(AcceptedMessage {
            .timestamp_ns = _readTimestampNs(now),
            .user_ref_num = order.user_ref_num,
            .stock_locate = order.stock_locate,
            .shares = order.shares,
            .price = order.price,
            .side = order.side,
            .order_ref_num = m_next_order_ref_num,
        });
        ++m_next_sequence;
        ++m_next_order_ref_num;

        if (!m_pending_fills.pushBack(PendingFill {
                .user_ref_num = order.user_ref_num,
                .executed_shares = order.shares,
                .price = order.price,
                .match_number = m_next_match_number,
                .due_time = now + m_config.fill_delay,
            })) {
            throw std::runtime_error("pending fill queue full");
        }
        ++m_next_match_number;
    }

    if (!_insertReplayResult(order.user_ref_num, result)) {
        return _buildReplayCapacityReject(order.user_ref_num);
    }
    return result;
}

uint64_t ProtocolSession::_readTimestampNs(std::chrono::steady_clock::time_point now) const {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
}

ExchangeProtocol::ExchangeProtocol(ProtocolConfig config, std::size_t slot_capacity) {
    m_slots.reserve(slot_capacity);
    for (std::size_t index = 0; index < slot_capacity; ++index) {
        m_slots.emplace_back(config);
    }
}

void ExchangeProtocol::activateSlot(int slot_idx,
                                         uint64_t session_id,
                                         std::chrono::steady_clock::time_point now) {
    ProtocolSlot& slot = _readSlot(slot_idx);
    slot.is_active = true;
    slot.protocol.reset(session_id, now);
}

void ExchangeProtocol::releaseSlot(int slot_idx) {
    ProtocolSlot& slot = _readSlot(slot_idx);
    slot.is_active = false;
    slot.protocol.reset(0, std::chrono::steady_clock::now());
}

bool ExchangeProtocol::appendBytes(int slot_idx,
                                        const uint8_t* bytes,
                                        std::size_t size,
                                        std::chrono::steady_clock::time_point now) {
    return _readActiveSlot(slot_idx).protocol.appendBytes(bytes, size, now);
}

std::optional<InboundMessage> ExchangeProtocol::tryReadInboundMessage(int slot_idx) {
    return _readActiveSlot(slot_idx).protocol.tryReadInboundMessage();
}

bool ExchangeProtocol::handleInboundMessage(int slot_idx,
                                            const InboundMessage& message,
                                            std::chrono::steady_clock::time_point now) {
    return _readActiveSlot(slot_idx).protocol.handleInboundMessage(message, now);
}

bool ExchangeProtocol::parseBytes(int slot_idx, std::chrono::steady_clock::time_point now) {
    return _readActiveSlot(slot_idx).protocol.parseBytes(now);
}

bool ExchangeProtocol::shouldClose(int slot_idx) const {
    return _readActiveSlot(slot_idx).protocol.shouldClose();
}

void ExchangeProtocol::markTimedOutSlots(std::chrono::steady_clock::time_point now,
                                              std::chrono::seconds timeout) {
    m_timed_out_slots.clear();
    for (std::size_t index = 0; index < m_slots.size(); ++index) {
        const ProtocolSlot& slot = m_slots[index];
        if (!slot.is_active) {
            continue;
        }
        if (slot.protocol.hasTimedOut(now, timeout)) {
            m_timed_out_slots.push_back(static_cast<int>(index));
        }
    }
}

bool ExchangeProtocol::hasTimedOutSlot() const {
    return !m_timed_out_slots.empty();
}

int ExchangeProtocol::readTimedOutSlot() {
    if (m_timed_out_slots.empty()) {
        throw std::runtime_error("no timed-out protocol slot available");
    }
    const int slot_idx = m_timed_out_slots.back();
    m_timed_out_slots.pop_back();
    return slot_idx;
}

void ExchangeProtocol::queueHeartbeatFrames(std::chrono::steady_clock::time_point now) {
    for (ProtocolSlot& slot : m_slots) {
        if (!slot.is_active) {
            continue;
        }
        slot.protocol.queueHeartbeatFrames(now);
    }
}

void ExchangeProtocol::queueDueFillFrames(std::chrono::steady_clock::time_point now) {
    for (ProtocolSlot& slot : m_slots) {
        if (!slot.is_active) {
            continue;
        }
        slot.protocol.queueDueFillFrames(now);
    }
}

uint64_t ExchangeProtocol::readSessionId(int slot_idx) const {
    return _readActiveSlot(slot_idx).protocol.readSessionId();
}

bool ExchangeProtocol::hasOutboundFrame(int slot_idx) const {
    return _readActiveSlot(slot_idx).protocol.hasOutboundFrame();
}

OutboundFrameRaw& ExchangeProtocol::readFrontOutboundFrame(int slot_idx) {
    return _readActiveSlot(slot_idx).protocol.readFrontOutboundFrame();
}

const OutboundFrameRaw& ExchangeProtocol::readFrontOutboundFrame(int slot_idx) const {
    return _readActiveSlot(slot_idx).protocol.readFrontOutboundFrame();
}

void ExchangeProtocol::eraseFrontOutboundFrame(int slot_idx) {
    _readActiveSlot(slot_idx).protocol.eraseFrontOutboundFrame();
}

void ExchangeProtocol::writeLastSendTime(int slot_idx,
                                              std::chrono::steady_clock::time_point now) {
    _readActiveSlot(slot_idx).protocol.writeLastSendTime(now);
}

ExchangeProtocol::ProtocolSlot& ExchangeProtocol::_readSlot(int slot_idx) {
    if (slot_idx < 0 || static_cast<std::size_t>(slot_idx) >= m_slots.size()) {
        throw std::runtime_error("invalid runtime slot index");
    }
    return m_slots[static_cast<std::size_t>(slot_idx)];
}

const ExchangeProtocol::ProtocolSlot& ExchangeProtocol::_readSlot(int slot_idx) const {
    if (slot_idx < 0 || static_cast<std::size_t>(slot_idx) >= m_slots.size()) {
        throw std::runtime_error("invalid runtime slot index");
    }
    return m_slots[static_cast<std::size_t>(slot_idx)];
}

ExchangeProtocol::ProtocolSlot& ExchangeProtocol::_readActiveSlot(int slot_idx) {
    ProtocolSlot& slot = _readSlot(slot_idx);
    if (!slot.is_active) {
        throw std::runtime_error("runtime slot is not active");
    }
    return slot;
}

const ExchangeProtocol::ProtocolSlot& ExchangeProtocol::_readActiveSlot(int slot_idx) const {
    const ProtocolSlot& slot = _readSlot(slot_idx);
    if (!slot.is_active) {
        throw std::runtime_error("runtime slot is not active");
    }
    return slot;
}
