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
constexpr uint8_t kSoupServerHeartbeatType = static_cast<uint8_t>('H');
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

struct LoginRequest {
    std::string username {};
    std::string password {};
    std::string requested_session {};
    uint64_t requested_sequence {1};
};

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

void writeSoupHeader(uint8_t* out, uint8_t type, std::size_t payload_size) {
    writeBigEndian16(out, static_cast<uint16_t>(payload_size + 1U));
    out[2] = type;
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

void writeSoupPacket(uint8_t* out, uint8_t type, const uint8_t* payload, std::size_t payload_size) {
    writeSoupHeader(out, type, payload_size);
    if (payload_size > 0) {
        std::copy_n(payload, static_cast<std::ptrdiff_t>(payload_size), out + kSoupHeaderSize);
    }
}

std::array<uint8_t, kSessionSize + kSequenceSize> writeLoginAcceptedPayload(std::string_view client,
                                                                             uint64_t next_sequence) {
    std::array<uint8_t, kSessionSize + kSequenceSize> payload {};
    writePaddedField(payload.data(), kSessionSize, client, true);
    writeSequenceField(payload.data() + static_cast<std::ptrdiff_t>(kSessionSize),
                       kSequenceSize,
                       next_sequence);
    return payload;
}

EncodedPayload writeOuchAccepted(const AcceptedMessage& accepted) {
    EncodedPayload payload {};
    payload.size = kOuchAcceptedSize;
    payload.bytes[0] = kOuchAcceptedType;
    writeBigEndian64(payload.bytes.data() + 1, accepted.timestamp_ns);
    writeBigEndian32(payload.bytes.data() + 9, accepted.user_ref_num);
    payload.bytes[13] = static_cast<uint8_t>(accepted.side);
    writeBigEndian32(payload.bytes.data() + 14, accepted.shares);
    writePaddedField(payload.bytes.data() + 18, 8, readSymbolFromLocate(accepted.stock_locate), false);
    writeBigEndian64(payload.bytes.data() + 26, accepted.price);
    payload.bytes[34] = static_cast<uint8_t>('0');
    payload.bytes[35] = static_cast<uint8_t>('Y');
    writeBigEndian64(payload.bytes.data() + 36, accepted.order_ref_num);
    payload.bytes[44] = static_cast<uint8_t>('A');
    payload.bytes[45] = static_cast<uint8_t>('N');
    payload.bytes[46] = static_cast<uint8_t>('N');
    payload.bytes[47] = static_cast<uint8_t>('L');
    writePaddedField(payload.bytes.data() + 48, 14, "", false);
    writeBigEndian16(payload.bytes.data() + 62, 0);
    return payload;
}

void writeOuchExecuted(EncodedPayload& payload, const ExecutedMessage& executed) {
    payload.size = kOuchExecutedSize;
    payload.bytes[0] = kOuchExecutedType;
    writeBigEndian64(payload.bytes.data() + 1, executed.timestamp_ns);
    writeBigEndian32(payload.bytes.data() + 9, executed.user_ref_num);
    writeBigEndian32(payload.bytes.data() + 13, executed.executed_shares);
    writeBigEndian64(payload.bytes.data() + 17, executed.price);
    payload.bytes[25] = static_cast<uint8_t>('A');
    writeBigEndian64(payload.bytes.data() + 26, executed.match_number);
    writeBigEndian16(payload.bytes.data() + 34, 0);
}

EncodedPayload writeOuchRejected(const RejectedMessage& rejected) {
    EncodedPayload payload {};
    payload.size = kOuchRejectedSize;
    payload.bytes[0] = kOuchRejectedType;
    writeBigEndian64(payload.bytes.data() + 1, rejected.timestamp_ns);
    writeBigEndian32(payload.bytes.data() + 9, rejected.user_ref_num);
    writeBigEndian16(payload.bytes.data() + 13, rejected.reason);
    writePaddedField(payload.bytes.data() + 15, 14, "", false);
    writeBigEndian16(payload.bytes.data() + 29, 0);
    return payload;
}

bool parseLoginRequest(uint8_t type, const EncodedPayload& payload, LoginRequest& request) {
    if (type != kSoupLoginRequestType ||
        payload.size != kUsernameSize + kPasswordSize + kSessionSize + kSequenceSize) {
        return false;
    }

    request.username.assign(reinterpret_cast<const char*>(payload.bytes.data()), kUsernameSize);
    request.password.assign(reinterpret_cast<const char*>(payload.bytes.data() + kUsernameSize), kPasswordSize);
    request.requested_session.assign(
        reinterpret_cast<const char*>(payload.bytes.data() + kUsernameSize + kPasswordSize),
        kSessionSize);
    return readSequenceField(payload.bytes.data() + kUsernameSize + kPasswordSize + kSessionSize,
                             kSequenceSize,
                             request.requested_sequence);
}

ExchangeEnterOrder readEnterOrder(const EncodedPayload& payload) {
    if (payload.size != kOuchEnterOrderSize || payload.bytes[0] != kOuchEnterOrderType) {
        throw std::runtime_error("invalid enter order payload");
    }

    ExchangeEnterOrder order {};
    order.user_ref_num = readBigEndian32(payload.bytes.data() + 1);
    order.side = static_cast<char>(payload.bytes[5]);
    order.stock_locate = readBigEndian16(payload.bytes.data() + 6);
    order.shares = readBigEndian32(payload.bytes.data() + 8);
    order.price = readBigEndian32(payload.bytes.data() + 12);
    return order;
}

} // namespace

ExchangeProtocol::ExchangeProtocol(ProtocolConfig config)
    : m_config(std::move(config)) {
    m_replay_entries.resize(m_config.replay_capacity);
}

void ExchangeProtocol::reset(uint64_t session_id, std::chrono::steady_clock::time_point now) {
    m_session_id = session_id;
    m_is_logged_in = false;
    m_should_close = false;
    m_next_sequence = 1;
    m_next_order_ref_num = 1;
    m_next_match_number = 1;
    m_last_send_time = now;
    m_last_receive_time = now;
    m_read_buffer.clear();
    m_outbound_queue.clear();
    m_pending_fills.clear();
    m_replay_count = 0;
    for (auto& entry : m_replay_entries) {
        entry = ReplayEntry {};
    }
}

bool ExchangeProtocol::appendReceivedBytes(const uint8_t* bytes,
                                           std::size_t size,
                                           std::chrono::steady_clock::time_point now) {
    if (!m_read_buffer.write(bytes, size)) {
        m_should_close = true;
        return false;
    }

    m_last_receive_time = now;
    while (true) {
        const auto packet = _tryReadPayload();
        if (!packet.has_value()) {
            return true;
        }
        if (!_handleClientPacket(packet->type, packet->payload, now)) {
            m_should_close = true;
            return true;
        }
    }
}

void ExchangeProtocol::onTimerTick(std::chrono::steady_clock::time_point now) {
    _queueOutboundMaintenance(now);
    if (now - m_last_receive_time >= std::chrono::seconds(15)) {
        m_should_close = true;
    }
}

bool ExchangeProtocol::shouldClose() const {
    return m_should_close;
}

bool ExchangeProtocol::hasOutboundFrame() const {
    return !_isOutFrameEmpty();
}




void ExchangeProtocol::writeLastSendTime(std::chrono::steady_clock::time_point now) {
    m_last_send_time = now;
}

ExchangeValidationResult ExchangeProtocol::_validateEnterOrder(const ExchangeEnterOrder& order) const {
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






std::optional<ExchangeProtocol::SoupPacket> ExchangeProtocol::_tryReadPayload() {
    if (m_read_buffer.readSize() < 2U) {
        return std::nullopt;
    }

    const uint16_t encoded_length = static_cast<uint16_t>(
        (static_cast<uint16_t>(m_read_buffer.readAt(0)) << 8) |
        static_cast<uint16_t>(m_read_buffer.readAt(1)));
    const std::size_t packet_size = static_cast<std::size_t>(encoded_length) + 2U;
    if (encoded_length == 0 || m_read_buffer.readSize() < packet_size) {
        return std::nullopt;
    }

    SoupPacket packet {};
    packet.type = m_read_buffer.readAt(2);
    packet.payload.size = static_cast<std::size_t>(encoded_length - 1U);
    if (packet.payload.size > packet.payload.bytes.size()) {
        packet.type = 0;
        packet.payload.size = 0;
        (void)m_read_buffer.eraseFrontN(packet_size);
        return packet;
    }
    if (!m_read_buffer.copyFrom(kSoupHeaderSize, packet.payload.bytes.data(), packet.payload.size)) {
        return std::nullopt;
    }
    (void)m_read_buffer.eraseFrontN(packet_size);
    return packet;
}

bool ExchangeProtocol::_handleClientPacket(uint8_t type,
                                                       const EncodedPayload& payload,
                                                       std::chrono::steady_clock::time_point now) {
    if (!m_is_logged_in) {
        LoginRequest login {};
        if (!parseLoginRequest(type, payload, login)) {
            _queueSoupFrame(kSoupLoginRejectedType, reinterpret_cast<const uint8_t*>("A"), 1U);
            return false;
        }

        const std::string username = trimSpaces(login.username);
        const std::string password = trimSpaces(login.password);
        const std::string requested_session = trimSpaces(login.requested_session);
        if (username != m_config.username || password != m_config.password ||
            (!requested_session.empty() && requested_session != m_config.session_id)) {
            _queueSoupFrame(kSoupLoginRejectedType, reinterpret_cast<const uint8_t*>("A"), 1U);
            return false;
        }

        m_is_logged_in = true;
        const auto login_payload = writeLoginAcceptedPayload(m_config.session_id, 1);
        _queueSoupFrame(kSoupLoginAcceptedType, login_payload.data(), login_payload.size());
        return true;
    }

    if (type == kSoupClientHeartbeatType) {
        return true;
    }
    if (type == kSoupLogoutRequestType) {
        return false;
    }
    if (type != kSoupUnsequencedDataType) {
        return false;
    }

    const ExchangeEnterOrder order = readEnterOrder(payload);
    const auto handled = _handleEnterOrder(order, now);
    if (!handled.is_duplicate) {
        for (std::size_t index = 0; index < handled.outbound_message_count; ++index) {
            const auto& message = handled.outbound_messages[index];
            _queueSoupFrame(kSoupSequencedDataType, message.bytes.data(), message.size);
        }
    }
    return true;
}

void ExchangeProtocol::_queueOutboundMaintenance(std::chrono::steady_clock::time_point now) {
    while (!m_pending_fills.isEmpty()) {
        const PendingFill& fill = m_pending_fills.readFront();
        if (fill.due_time > now) {
            break;
        }
        EncodedPayload payload {};
        writeOuchExecuted(payload, ExecutedMessage {
            .timestamp_ns = _readTimestampNs(now),
            .user_ref_num = fill.user_ref_num,
            .executed_shares = fill.executed_shares,
            .price = fill.price,
            .match_number = fill.match_number,
        });
        ++m_next_sequence;
        _queueSoupFrame(kSoupSequencedDataType, payload.bytes.data(), payload.size);
        (void)m_pending_fills.eraseFront();
    }

    if (m_is_logged_in && (now - m_last_send_time) >= std::chrono::seconds(1)) {
        _queueSoupFrame(kSoupServerHeartbeatType, nullptr, 0);
    }
}

bool ExchangeProtocol::_insertReplayResult(uint32_t user_ref_num, const HandledOrderResult& result) {
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

std::optional<HandledOrderResult> ExchangeProtocol::_findReplayResult(uint32_t user_ref_num) const {
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

HandledOrderResult ExchangeProtocol::_buildReplayCapacityReject(uint32_t user_ref_num) {
    HandledOrderResult reject {};
    reject.validation = ExchangeValidationResult {
        .kind = ExchangeValidationKind::Rejected,
        .reject_reason = kRejectReplayCapacity,
    };
    reject.outbound_messages[0] = writeOuchRejected(RejectedMessage {
        .timestamp_ns = _readTimestampNs(std::chrono::steady_clock::now()),
        .user_ref_num = user_ref_num,
        .reason = kRejectReplayCapacity,
    });
    reject.outbound_message_count = 1;
    ++m_next_sequence;
    return reject;
}

HandledOrderResult ExchangeProtocol::_handleEnterOrder(const ExchangeEnterOrder& order,
                                                                   std::chrono::steady_clock::time_point now) {
    if (const auto replay = _findReplayResult(order.user_ref_num); replay.has_value()) {
        HandledOrderResult result = *replay;
        result.is_duplicate = true;
        return result;
    }

    HandledOrderResult result {};
    result.validation = _validateEnterOrder(order);

    if (result.validation.kind == ExchangeValidationKind::Rejected) {
        const EncodedPayload payload = writeOuchRejected(RejectedMessage {
            .timestamp_ns = _readTimestampNs(now),
            .user_ref_num = order.user_ref_num,
            .reason = result.validation.reject_reason,
        });
        ++m_next_sequence;
        result.outbound_messages[result.outbound_message_count++] = payload;
    } else {
        const EncodedPayload accepted_payload = writeOuchAccepted(AcceptedMessage {
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
        result.outbound_messages[result.outbound_message_count++] = accepted_payload;

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

void ExchangeProtocol::_queueSoupFrame(uint8_t type,
                                                   const uint8_t* payload,
                                                   std::size_t payload_size) {
    if (payload_size > kMaxSoupPayloadSize) {
        throw std::runtime_error("client out queue full");
    }

    SOUPBinFrame packet {};
    writeSoupPacket(packet.payload.data(), type, payload, payload_size);
    packet.size = kSoupHeaderSize + payload_size;
    if (!m_outbound_queue.pushBack(packet)) {
        throw std::runtime_error("client out queue full");
    }
}

bool ExchangeProtocol::_pushOutQueue(const uint8_t* bytes, std::size_t size) {
    if (size > kMaxSoupFrameSize) {
        return false;
    }

    SOUPBinFrame packet {};
    if (size > 0) {
        std::copy_n(bytes, static_cast<std::ptrdiff_t>(size), packet.payload.data());
    }
    packet.size = size;
    packet.offset = 0;
    return m_outbound_queue.pushBack(packet);
}

bool ExchangeProtocol::_isOutFrameEmpty() const {
    return m_outbound_queue.isEmpty();
}

const SOUPBinFrame& ExchangeProtocol::readFrontFrame() const {
    return m_outbound_queue.readFront();
}

SOUPBinFrame& ExchangeProtocol::readFrontFrame() {
    return m_outbound_queue.readFront();
}

void ExchangeProtocol::eraseFrontFrame() {
    (void)m_outbound_queue.eraseFront();
}

uint64_t ExchangeProtocol::_readTimestampNs(std::chrono::steady_clock::time_point now) const {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
}
