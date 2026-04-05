#include "dummy_exchange_server.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string_view>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace {

// SoupBinTCP session-control packet types.
constexpr uint8_t kSoupLoginAcceptedType = static_cast<uint8_t>('A');   // Login Accepted
constexpr uint8_t kSoupLoginRejectedType = static_cast<uint8_t>('J');   // Login Rejected
// Outbound (server to clients) OUCH messages are sequenced, so outbound OUCH payloads ride inside SoupBinTCP 'S'.
constexpr uint8_t kSoupSequencedDataType = static_cast<uint8_t>('S');   // Sequenced Data
constexpr uint8_t kSoupServerHeartbeatType = static_cast<uint8_t>('H'); // Server Heartbeat
constexpr uint8_t kSoupLoginRequestType = static_cast<uint8_t>('L');    // Login Request
// Inbound (client to server) OUCH messages are not sequenced, so client OUCH payloads arrive inside SoupBinTCP 'U'.
constexpr uint8_t kSoupUnsequencedDataType = static_cast<uint8_t>('U'); // Unsequenced Data
constexpr uint8_t kSoupClientHeartbeatType = static_cast<uint8_t>('R'); // Client Heartbeat
constexpr uint8_t kSoupLogoutRequestType = static_cast<uint8_t>('O');   // Logout Request

// OUCH application-layer payload types carried inside SoupBinTCP data packets.
constexpr uint8_t kOuchEnterOrderType = static_cast<uint8_t>('O'); // Enter Order
constexpr uint8_t kOuchAcceptedType = static_cast<uint8_t>('A');   // Accepted
constexpr uint8_t kOuchExecutedType = static_cast<uint8_t>('E');   // Executed
constexpr uint8_t kOuchRejectedType = static_cast<uint8_t>('J');   // Rejected

constexpr std::size_t kSoupHeaderSize = 3; // 2 bytes for length prefix, 1 byte for packet type.
constexpr std::size_t kSessionSize = 10;
constexpr std::size_t kSequenceSize = 20;
constexpr std::size_t kUsernameSize = 6;
constexpr std::size_t kPasswordSize = 10;

constexpr std::size_t kOuchEnterOrderSize = 16;
constexpr std::size_t kOuchAcceptedSize = 64;
constexpr std::size_t kOuchRejectedSize = 31;

static_assert(kMaxSoupPayloadSize >= kSessionSize + kSequenceSize,
              "max Soup payload must fit login-accepted payload");
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

const char* readSoupTypeName(uint8_t type) {
    switch (type) {
        case kSoupLoginAcceptedType:
            return "LOGIN_ACCEPTED";
        case kSoupLoginRejectedType:
            return "LOGIN_REJECTED";
        case kSoupSequencedDataType:
            return "SEQUENCED_DATA";
        case kSoupServerHeartbeatType:
            return "SERVER_HEARTBEAT";
        case kSoupLoginRequestType:
            return "LOGIN_REQUEST";
        case kSoupUnsequencedDataType:
            return "UNSEQUENCED_DATA";
        case kSoupClientHeartbeatType:
            return "CLIENT_HEARTBEAT";
        case kSoupLogoutRequestType:
            return "LOGOUT_REQUEST";
        default:
            return "UNKNOWN";
    }
}

const char* readOuchTypeName(uint8_t type) {
    switch (type) {
        case kOuchEnterOrderType:
            return "ENTER_ORDER";
        case kOuchAcceptedType:
            return "ACCEPTED";
        case kOuchExecutedType:
            return "EXECUTED";
        case kOuchRejectedType:
            return "REJECTED";
        default:
            return "UNKNOWN";
    }
}

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
    uint64_t timestamp_ns       {0};
    uint32_t user_ref_num                {0};
    uint32_t executed_shares    {0};
    uint32_t price              {0};
    uint64_t match_number       {0};
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

uint64_t readBigEndian64(const uint8_t* bytes) {
    return (static_cast<uint64_t>(bytes[0]) << 56) |
           (static_cast<uint64_t>(bytes[1]) << 48) |
           (static_cast<uint64_t>(bytes[2]) << 40) |
           (static_cast<uint64_t>(bytes[3]) << 32) |
           (static_cast<uint64_t>(bytes[4]) << 24) |
           (static_cast<uint64_t>(bytes[5]) << 16) |
           (static_cast<uint64_t>(bytes[6]) << 8) |
           static_cast<uint64_t>(bytes[7]);
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

void writeSOUPHeader(uint8_t* out, uint8_t type, std::size_t payload_size) {
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

bool readSequenceField(const uint8_t* data, std::size_t Size, uint64_t& value) {
    std::string_view text(reinterpret_cast<const char*>(data), Size);
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

void writePaddedField(uint8_t* out,
                      std::size_t Size,
                      std::string_view value,
                      bool left_pad) {
    std::fill_n(out, static_cast<std::ptrdiff_t>(Size), static_cast<uint8_t>(' '));
    const std::size_t copy_size = std::min(Size, value.size());
    if (left_pad) {
        std::copy_n(value.end() - static_cast<std::ptrdiff_t>(copy_size),
                    static_cast<std::ptrdiff_t>(copy_size),
                    out + static_cast<std::ptrdiff_t>(Size - copy_size));
        return;
    }

    std::copy_n(value.begin(), static_cast<std::ptrdiff_t>(copy_size), out);
}

void writeSequenceField(uint8_t* out, std::size_t Size, uint64_t value) {
    std::fill_n(out, static_cast<std::ptrdiff_t>(Size), static_cast<uint8_t>(' '));
    const std::string text = std::to_string(value);
    const std::size_t copy_size = std::min(Size, text.size());
    std::copy_n(text.end() - static_cast<std::ptrdiff_t>(copy_size),
                static_cast<std::ptrdiff_t>(copy_size),
                out + static_cast<std::ptrdiff_t>(Size - copy_size));
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
    writeSOUPHeader(out, type, payload_size);
    if (payload_size > 0) {
        std::copy_n(payload, static_cast<std::ptrdiff_t>(payload_size), out + 3);
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

void writeOuchExecuted(EncodedPayload &payload, const ExecutedMessage& executed) {
    payload.size = kOuchExecutedSize;
    payload.bytes[0] = kOuchExecutedType;
    writeBigEndian64(payload.bytes.data() + 1, executed.timestamp_ns);
    writeBigEndian32(payload.bytes.data() + 9, executed.user_ref_num);
    writeBigEndian32(payload.bytes.data() + 13, executed.executed_shares);
    writeBigEndian64(payload.bytes.data() + 17, executed.price);
    payload.bytes[25] = static_cast<uint8_t>('A'); //liquidity flag, just set to 'A' for added liquidity for demo
    writeBigEndian64(payload.bytes.data() + 26, executed.match_number);
    writeBigEndian16(payload.bytes.data() + 34, 0);
    return;
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

bool parseLoginRequest(const SoupPacket& packet, LoginRequest& request) {
    if (packet.type != kSoupLoginRequestType ||
        packet.payload.size != kUsernameSize + kPasswordSize + kSessionSize + kSequenceSize) {
        return false;
    }

    request.username.assign(reinterpret_cast<const char*>(packet.payload.bytes.data()), kUsernameSize);
    request.password.assign(reinterpret_cast<const char*>(packet.payload.bytes.data() + kUsernameSize), kPasswordSize);
    request.requested_session.assign(reinterpret_cast<const char*>(packet.payload.bytes.data() + kUsernameSize + kPasswordSize),
                                     kSessionSize);
    return readSequenceField(packet.payload.bytes.data() + kUsernameSize + kPasswordSize + kSessionSize,
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

void logOutboundPacket(uint64_t session_id, const uint8_t* bytes, std::size_t size) {
    if (size < kSoupHeaderSize) {
        std::printf("ExchangeTx session=%llu soup=SHORT_PACKET bytes=%zu\n",
                    static_cast<unsigned long long>(session_id),
                    size);
        std::fflush(stdout);
        return;
    }

    const uint8_t soup_type = bytes[2];
    std::printf("ExchangeTx session=%llu soup=%s(%c) bytes=%zu",
                static_cast<unsigned long long>(session_id),
                readSoupTypeName(soup_type),
                static_cast<char>(soup_type),
                size);

    if (soup_type == kSoupSequencedDataType && size >= kSoupHeaderSize + 1U) {
        const uint8_t ouch_type = bytes[3];
        std::printf(" ouch=%s(%c)",
                    readOuchTypeName(ouch_type),
                    static_cast<char>(ouch_type));

        if (ouch_type == kOuchAcceptedType && size >= kSoupHeaderSize + kOuchAcceptedSize) {
            std::printf(" user_ref=%u shares=%u price=%llu order_ref=%llu",
                        readBigEndian32(bytes + 12),
                        readBigEndian32(bytes + 17),
                        static_cast<unsigned long long>(readBigEndian64(bytes + 29)),
                        static_cast<unsigned long long>(readBigEndian64(bytes + 39)));
        } else if (ouch_type == kOuchExecutedType && size >= kSoupHeaderSize + kOuchExecutedSize) {
            std::printf(" user_ref=%u shares=%u price=%llu match=%llu",
                        readBigEndian32(bytes + 12),
                        readBigEndian32(bytes + 16),
                        static_cast<unsigned long long>(readBigEndian64(bytes + 20)),
                        static_cast<unsigned long long>(readBigEndian64(bytes + 29)));
        } else if (ouch_type == kOuchRejectedType && size >= kSoupHeaderSize + kOuchRejectedSize) {
            std::printf(" user_ref=%u reason=0x%04x",
                        readBigEndian32(bytes + 12),
                        readBigEndian16(bytes + 16));
        } else {
            std::printf(" payload_len=%zu", size - kSoupHeaderSize);
        }
    }

    std::printf("\n");
    std::fflush(stdout);
}

int openListenSocket(const DummyExchangeConfig& config) {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        throw std::runtime_error("failed to create listen socket");
    }

    int one = 1;
    (void)::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(config.port);
    if (::inet_pton(AF_INET, config.listen_ip.c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        throw std::runtime_error("invalid listen ip");
    }

    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        ::close(fd);
        throw std::runtime_error("failed to bind listen socket");
    }
    if (::listen(fd, 4) != 0) {
        ::close(fd);
        throw std::runtime_error("failed to listen");
    }
    return fd;
}

} // namespace

DummyExchangeServer::DummyExchangeServer(DummyExchangeConfig config)
    : m_config(std::move(config)) {
    m_session_slots.resize(m_config.session_capacity);
    m_free_slot_indexes.resize(m_config.session_capacity);
    m_free_slot_count = m_config.session_capacity;
    for (std::size_t index = 0; index < m_session_slots.size(); ++index) {
        auto& slot = m_session_slots[index];
        slot.mode = SessionSlotMode::Free;
        slot.generation = 1;
        slot.replay_entries.resize(m_config.replay_capacity);
        slot.replay_count = 0;
        slot.event_token.slot_index = static_cast<uint32_t>(index);
        slot.event_token.generation = slot.generation;
        m_free_slot_indexes[index] = static_cast<uint32_t>(m_session_slots.size() - 1U - index);
    }
}

void DummyExchangeServer::requestStopForTest() {
    m_stop_requested.store(true);
}

ExchangeValidationResult DummyExchangeServer::validateEnterOrder(const ExchangeEnterOrder& order) const {
    if (order.stock_locate != 0x000d && order.stock_locate != 0x0ee8) {
        return ExchangeValidationResult {
            .kind = ExchangeValidationKind::Rejected,
            .reject_reason = kRejectUnsupportedSymbol
        };
    }
    if (order.shares == 0 || order.shares > m_config.max_shares) {
        return ExchangeValidationResult {
            .kind = ExchangeValidationKind::Rejected,
            .reject_reason = kRejectInvalidShares
        };
    }
    if (order.price < m_config.price_min || order.price > m_config.price_max) {
        return ExchangeValidationResult {
            .kind = ExchangeValidationKind::Rejected,
            .reject_reason = kRejectInvalidPrice
        };
    }
    return ExchangeValidationResult {
        .kind = ExchangeValidationKind::Accepted,
        .reject_reason = 0
    };
}

DummyExchangeServer::SessionSlot& DummyExchangeServer::_acquireSessionSlot(SessionSlotMode mode) {
    if (m_free_slot_count == 0) {
        throw std::runtime_error("session pool full");
    }

    const uint32_t slot_index = m_free_slot_indexes[--m_free_slot_count];
    SessionSlot& slot = m_session_slots[slot_index];
    slot.mode = mode;
    slot.client = ClientState {};
    slot.replay_count = 0;
    for (auto& replay_entry : slot.replay_entries) {
        replay_entry = ReplayEntry {};
    }
    slot.event_token.slot_index = slot_index;
    slot.event_token.generation = slot.generation;
    return slot;
}

void DummyExchangeServer::_releaseSessionSlot(SessionSlot& slot) {
    if (slot.mode == SessionSlotMode::Free) {
        return;
    }

    slot.client = ClientState {};
    slot.replay_count = 0;
    for (auto& replay_entry : slot.replay_entries) {
        replay_entry = ReplayEntry {};
    }
    slot.mode = SessionSlotMode::Free;
    ++slot.generation;
    slot.event_token.generation = slot.generation;
    m_free_slot_indexes[m_free_slot_count++] = slot.event_token.slot_index;
}

DummyExchangeServer::SessionSlot& DummyExchangeServer::_resolveTestSlot(TestSessionHandle session) {
    if (session.slot_index >= m_session_slots.size()) {
        throw std::runtime_error("invalid test session handle");
    }

    SessionSlot& slot = m_session_slots[session.slot_index];
    if (slot.mode != SessionSlotMode::Test || slot.generation != session.generation) {
        throw std::runtime_error("stale test session handle");
    }
    return slot;
}

const DummyExchangeServer::SessionSlot& DummyExchangeServer::_resolveTestSlot(TestSessionHandle session) const {
    if (session.slot_index >= m_session_slots.size()) {
        throw std::runtime_error("invalid test session handle");
    }

    const SessionSlot& slot = m_session_slots[session.slot_index];
    if (slot.mode != SessionSlotMode::Test || slot.generation != session.generation) {
        throw std::runtime_error("stale test session handle");
    }
    return slot;
}

DummyExchangeServer::TestSessionHandle DummyExchangeServer::createSessionForTest() {
    SessionSlot& slot = _acquireSessionSlot(SessionSlotMode::Test);
    slot.client.session_id = m_next_test_session_id++;
    const auto now = std::chrono::steady_clock::now();
    slot.client.last_send_time = now;
    slot.client.last_receive_time = now;
    return TestSessionHandle {
        .slot_index = slot.event_token.slot_index,
        .generation = slot.generation,
    };
}

void DummyExchangeServer::releaseSessionForTest(TestSessionHandle session) {
    SessionSlot& slot = _resolveTestSlot(session);
    _releaseSessionSlot(slot);
}

void DummyExchangeServer::appendReadBytesForTest(TestSessionHandle session, const std::vector<uint8_t>& bytes) {
    auto& client = _resolveTestSlot(session).client;
    if (!client.read_buffer.write(bytes.data(), bytes.size())) {
        throw std::runtime_error("test read buffer full");
    }
}

std::optional<uint8_t> DummyExchangeServer::tryReadPacketTypeForTest(TestSessionHandle session) {
    return _tryReadPacketType(_resolveTestSlot(session).client);
}

void DummyExchangeServer::queuePacketForTest(TestSessionHandle session, const std::vector<uint8_t>& bytes) {
    auto& client = _resolveTestSlot(session).client;
    if (!_pushOutQueue(client, bytes.data(), bytes.size())) {
        throw std::runtime_error("test out queue full");
    }
}

bool DummyExchangeServer::consumeQueuedBytesForTest(TestSessionHandle session, std::size_t count) {
    auto& client = _resolveTestSlot(session).client;
    while (count > 0 && !_isOutQueueEmpty(client)) {
        auto& front = _readOutQueueFront(client);
        const std::size_t remaining = front.size - front.offset;
        const std::size_t chunk = std::min(remaining, count);
        front.offset += chunk;
        count -= chunk;
        if (front.offset == front.size) {
            _popOutQueue(client);
        }
    }
    return count == 0;
}

std::size_t DummyExchangeServer::readQueuedPacketCountForTest(TestSessionHandle session) const {
    return _readOutQueueSize(_resolveTestSlot(session).client);
}

std::vector<uint8_t> DummyExchangeServer::readFrontPacketBytesForTest(TestSessionHandle session) const {
    const auto& client = _resolveTestSlot(session).client;
    if (_isOutQueueEmpty(client)) {
        return {};
    }

    const auto& front = _readOutQueueFront(client);
    return std::vector<uint8_t>(front.payload.begin(),
                                front.payload.begin() + static_cast<std::ptrdiff_t>(front.size));
}

void DummyExchangeServer::markLoggedInForTest(TestSessionHandle session) {
    _resolveTestSlot(session).client.is_logged_in = true;
}

void DummyExchangeServer::setLastSendAgoForTest(TestSessionHandle session, std::chrono::seconds age) {
    _resolveTestSlot(session).client.last_send_time = std::chrono::steady_clock::now() - age;
}

void DummyExchangeServer::handleTimerTickForTest() {
    const auto now = std::chrono::steady_clock::now();
    for (auto& slot : m_session_slots) {
        if (slot.mode != SessionSlotMode::Test) {
            continue;
        }
        _queOutBound(slot.client, now);
    }
}

std::optional<uint8_t> DummyExchangeServer::peekFrontPacketTypeForTest(TestSessionHandle session) const {
    const auto& client = _resolveTestSlot(session).client;
    if (_isOutQueueEmpty(client) || _readOutQueueFront(client).size < 3) {
        return std::nullopt;
    }
    return _readOutQueueFront(client).payload[2];
}

std::optional<uint8_t> DummyExchangeServer::_tryReadPacketType(ClientState& client) {
    if (client.read_buffer.readSize() < kSoupHeaderSize) {
        return std::nullopt;
    }

    const uint16_t encoded_length = static_cast<uint16_t>(
        (static_cast<uint16_t>(client.read_buffer.readAt(0)) << 8) |
        static_cast<uint16_t>(client.read_buffer.readAt(1)));
    const std::size_t packet_size = static_cast<std::size_t>(encoded_length) + 2U;
    if (encoded_length == 0 || client.read_buffer.readSize() < packet_size) {
        return std::nullopt;
    }

    const uint8_t type = client.read_buffer.readAt(2);
    (void)client.read_buffer.eraseFrontN(packet_size);
    return type;
}

// take out pending fills/heartbeat to queue
void DummyExchangeServer::_queOutBound(ClientState& client, std::chrono::steady_clock::time_point now) {
    while (!client.pending_fills.isEmpty()) {
        const PendingFill& fill = client.pending_fills.readFront();
        if (fill.due_time > now) {
            break;
        }
        EncodedPayload payload;
        writeOuchExecuted(payload, ExecutedMessage {
            .timestamp_ns = _readTimestampNs(),
            .user_ref_num = fill.user_ref_num,
            .executed_shares = fill.executed_shares,
            .price = fill.price,
            .match_number = fill.match_number,
        });
        ++client.next_sequence;
        _queueSoupFrame(client, kSoupSequencedDataType, payload.bytes.data(), payload.size);
        (void)client.pending_fills.eraseFront();
    }
    // heartbeat renewal
    if (client.is_logged_in && (now - client.last_send_time) >= std::chrono::seconds(1)) {
        _queueSoupFrame(client, kSoupServerHeartbeatType, nullptr, 0);
    }
}

bool DummyExchangeServer::_setNonBlocking(int fd) const {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

int DummyExchangeServer::_openTimerFd() const {
    const int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timer_fd < 0) {
        throw std::runtime_error("failed to create timer fd");
    }

    itimerspec spec {};
    spec.it_interval.tv_sec = 0;
    spec.it_interval.tv_nsec = 100000000;
    spec.it_value = spec.it_interval;
    if (::timerfd_settime(timer_fd, 0, &spec, nullptr) != 0) {
        ::close(timer_fd);
        throw std::runtime_error("failed to arm timer fd");
    }
    return timer_fd;
}

void DummyExchangeServer::_acceptClients(int epoll_fd, int listen_fd) {
    while (true) {
        const int client_fd = ::accept(listen_fd, nullptr, nullptr);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }
            return;
        }

        if (!_setNonBlocking(client_fd)) {
            ::close(client_fd);
            continue;
        }

        if (m_free_slot_count == 0) {
            ::close(client_fd);
            continue;
        }

        SessionSlot& slot = _acquireSessionSlot(SessionSlotMode::Live);
        slot.client.fd = client_fd;
        slot.client.session_id = m_next_live_session_id++;
        slot.client.last_send_time = std::chrono::steady_clock::now();
        slot.client.last_receive_time = slot.client.last_send_time;
        slot.event_token.generation = slot.generation;

        epoll_event event {};
        event.events = EPOLLIN | EPOLLRDHUP;
        event.data.ptr = &slot.event_token;
        if (::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) != 0) {
            ::close(client_fd);
            _releaseSessionSlot(slot);
        }
    }
}

std::optional<SoupPacket> DummyExchangeServer::_tryReadPayload(ClientState& client) {
    if (client.read_buffer.readSize() < 2U) {
        return std::nullopt;
    }

    const uint16_t encoded_length = static_cast<uint16_t>(
        (static_cast<uint16_t>(client.read_buffer.readAt(0)) << 8) |
        static_cast<uint16_t>(client.read_buffer.readAt(1)));
    const std::size_t packet_size = static_cast<std::size_t>(encoded_length) + 2U;
    if (encoded_length == 0 || client.read_buffer.readSize() < packet_size) {
        return std::nullopt;
    }

    SoupPacket packet {};
    packet.type = client.read_buffer.readAt(2);
    packet.payload.size = static_cast<std::size_t>(encoded_length - 1U);
    if (packet.payload.size > packet.payload.bytes.size()) {
        packet.type = 0;
        packet.payload.size = 0;
        (void)client.read_buffer.eraseFrontN(packet_size);
        return packet;
    }
    if (!client.read_buffer.copyFrom(kSoupHeaderSize, packet.payload.bytes.data(), packet.payload.size)) {
        return std::nullopt;
    }
    (void)client.read_buffer.eraseFrontN(packet_size);
    return packet;
}

bool DummyExchangeServer::_receiveClientData(ClientState& client) {
    std::array<uint8_t, 1024> buffer;
    while (true) {
        const ssize_t count = ::recv(client.fd, buffer.data(), buffer.size(), 0);
        if (count > 0) {
            if (!client.read_buffer.write(buffer.data(), static_cast<std::size_t>(count))) {
                return false;
            }
            client.last_receive_time = std::chrono::steady_clock::now();
            continue;
        }
        if (count == 0) {
            return false;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true;
        }
        return false;
    }
}

bool DummyExchangeServer::_sendQueFront(ClientState& client) {
    while (!_isOutQueueEmpty(client)) {
        auto& front = _readOutQueueFront(client);
        const ssize_t written = ::send(client.fd,
                                       front.payload.data() + static_cast<std::ptrdiff_t>(front.offset),
                                       front.size - front.offset,
                                       MSG_NOSIGNAL);
        if (written <= 0) {
            return false;
        }

        front.offset += static_cast<std::size_t>(written);
        client.last_send_time = std::chrono::steady_clock::now();
        if (front.offset == front.size) {
            logOutboundPacket(client.session_id, front.payload.data(), front.size);
            _popOutQueue(client);
        }
    }
    return true;
}

void DummyExchangeServer::_queueSoupFrame(ClientState& client,
                                          uint8_t type,
                                          const uint8_t* payload,
                                          std::size_t payload_size) {
    if (payload_size > kMaxSoupPayloadSize) {
        throw std::runtime_error("client out queue full");
    }

    OutboundPacket packet {};
    writeSoupPacket(packet.payload.data(), type, payload, payload_size);
    packet.size = kSoupHeaderSize + payload_size;
    if (!client.out_bound_que.pushBack(packet)) {
        throw std::runtime_error("client out queue full");
    }
}

bool DummyExchangeServer::_handleClientPacket(SessionSlot& slot,
                                              uint8_t type,
                                              const EncodedPayload& payload) {
    ClientState& client = slot.client;
    if (!client.is_logged_in) {
        SoupPacket packet {};
        packet.type = type;
        packet.payload = payload;

        LoginRequest login {};
        if (!parseLoginRequest(packet, login)) {
            _queueSoupFrame(client, kSoupLoginRejectedType, reinterpret_cast<const uint8_t*>("A"), 1U);
            return _sendQueFront(client) && false;
        }

        const std::string username = trimSpaces(login.username);
        const std::string password = trimSpaces(login.password);
        const std::string requested_session = trimSpaces(login.requested_session);
        if (username != m_config.username || password != m_config.password ||
            (!requested_session.empty() && requested_session != m_config.session_id)) {
            _queueSoupFrame(client, kSoupLoginRejectedType, reinterpret_cast<const uint8_t*>("A"), 1U);
            return _sendQueFront(client) && false;
        }

        client.is_logged_in = true;
        const auto login_accepted_payload = writeLoginAcceptedPayload(m_config.session_id, 1);
        _queueSoupFrame(client,
                        kSoupLoginAcceptedType,
                        login_accepted_payload.data(),
                        login_accepted_payload.size());
        return _sendQueFront(client);
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
    const auto handled = _handleEnterOrder(slot, order);
    if (!handled.is_duplicate) {
        for (std::size_t index = 0; index < handled.outbound_message_count; ++index) {
            const auto& message = handled.outbound_messages[index];
            _queueSoupFrame(client, kSoupSequencedDataType, message.bytes.data(), message.size);
        }
    }
    return _sendQueFront(client);
}

DummyExchangeServer::SessionSlot* DummyExchangeServer::_resolveLiveSlot(const EventToken& token) {
    if (token.kind != EventToken::Kind::LiveSession || token.slot_index >= m_session_slots.size()) {
        return nullptr;
    }

    SessionSlot& slot = m_session_slots[token.slot_index];
    if (slot.mode != SessionSlotMode::Live || slot.generation != token.generation) {
        return nullptr;
    }
    return &slot;
}

void DummyExchangeServer::_closeLiveSession(int epoll_fd, SessionSlot& slot) {
    if (slot.mode != SessionSlotMode::Live) {
        return;
    }

    const int fd = slot.client.fd;
    if (fd >= 0) {
        (void)::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
        ::close(fd);
    }
    _releaseSessionSlot(slot);
}

HandledOrderResult DummyExchangeServer::handleEnterOrderForTest(TestSessionHandle session,
                                                                const ExchangeEnterOrder& order) {
    return _handleEnterOrder(_resolveTestSlot(session), order);
}

uint64_t DummyExchangeServer::readSessionNextSequenceForTest(TestSessionHandle session) const {
    return _readNextSequence(_resolveTestSlot(session).client);
}
std::optional<HandledOrderResult> DummyExchangeServer::_findReplayResult(const SessionSlot& slot,
                                                                         uint32_t user_ref_num) const {
    if (slot.replay_entries.empty()) {
        return std::nullopt;
    }

    const std::size_t capacity = slot.replay_entries.size();
    const std::size_t start = static_cast<std::size_t>(user_ref_num) % capacity;
    for (std::size_t offset = 0; offset < capacity; ++offset) {
        const ReplayEntry& entry = slot.replay_entries[(start + offset) % capacity];
        if (!entry.is_occupied) {
            return std::nullopt;
        }
        if (entry.user_ref_num == user_ref_num) {
            return entry.result;
        }
    }
    return std::nullopt;
}

bool DummyExchangeServer::_insertReplayResult(SessionSlot& slot,
                                              uint32_t user_ref_num,
                                              const HandledOrderResult& result) {
    if (slot.replay_entries.empty() || slot.replay_count >= slot.replay_entries.size()) {
        return false;
    }

    const std::size_t capacity = slot.replay_entries.size();
    const std::size_t start = static_cast<std::size_t>(user_ref_num) % capacity;
    for (std::size_t offset = 0; offset < capacity; ++offset) {
        ReplayEntry& entry = slot.replay_entries[(start + offset) % capacity];
        if (!entry.is_occupied) {
            entry.is_occupied = true;
            entry.user_ref_num = user_ref_num;
            entry.result = result;
            ++slot.replay_count;
            return true;
        }
        if (entry.user_ref_num == user_ref_num) {
            entry.result = result;
            return true;
        }
    }
    return false;
}

HandledOrderResult DummyExchangeServer::_buildReplayCapacityReject(SessionSlot& slot, uint32_t user_ref_num) {
    ClientState& client = slot.client;
    HandledOrderResult reject {};
    reject.validation = ExchangeValidationResult {
        .kind = ExchangeValidationKind::Rejected,
        .reject_reason = kRejectReplayCapacity,
    };
    reject.outbound_messages[0] = writeOuchRejected(RejectedMessage {
        .timestamp_ns = _readTimestampNs(),
        .user_ref_num = user_ref_num,
        .reason = kRejectReplayCapacity,
    });
    reject.outbound_message_count = 1;
    ++client.next_sequence;
    return reject;
}

HandledOrderResult DummyExchangeServer::_handleEnterOrder(SessionSlot& slot,
                                                          const ExchangeEnterOrder& order) {
    ClientState& client = slot.client;
    if (const auto replay = _findReplayResult(slot, order.user_ref_num); replay.has_value()) {
        HandledOrderResult result = *replay;
        result.is_duplicate = true;
        return result;
    }

    HandledOrderResult result {};
    result.validation = validateEnterOrder(order);

    if (result.validation.kind == ExchangeValidationKind::Rejected) {
        const EncodedPayload payload = writeOuchRejected(RejectedMessage {
            .timestamp_ns = _readTimestampNs(),
            .user_ref_num = order.user_ref_num,
            .reason = result.validation.reject_reason,
        });
        ++client.next_sequence;
        result.outbound_messages[result.outbound_message_count++] = payload;
    } else {
        const EncodedPayload accepted_payload = writeOuchAccepted(AcceptedMessage {
            .timestamp_ns = _readTimestampNs(),
            .user_ref_num = order.user_ref_num,
            .stock_locate = order.stock_locate,
            .shares = order.shares,
            .price = order.price,
            .side = order.side,
            .order_ref_num = _readNextOrderRef(client),
        });
        ++client.next_sequence;
        ++client.next_order_ref_num;
        result.outbound_messages[result.outbound_message_count++] = accepted_payload;

        if (!client.pending_fills.pushBack(PendingFill {
            .user_ref_num = order.user_ref_num,
            .executed_shares = order.shares,
            .price = order.price,
            .match_number = _readNextMatchNumber(client),
            .due_time = std::chrono::steady_clock::now() + m_config.fill_delay,
        })) {
            throw std::runtime_error("pending fill queue full");
        }
        ++client.next_match_number;
    }

    if (!_insertReplayResult(slot, order.user_ref_num, result)) {
        return _buildReplayCapacityReject(slot, order.user_ref_num);
    }
    return result;
}

int DummyExchangeServer::run() {
    m_stop_requested.store(false);
    for (auto& slot : m_session_slots) {
        if (slot.mode == SessionSlotMode::Live) {
            _releaseSessionSlot(slot);
        }
    }

    const int listen_fd = openListenSocket(m_config);
    if (!_setNonBlocking(listen_fd)) {
        ::close(listen_fd);
        throw std::runtime_error("failed to make listen socket nonblocking");
    }
    std::printf("dummy_exchange_server listening on %s:%u\n", m_config.listen_ip.c_str(), m_config.port);
    std::fflush(stdout);

    const int epoll_fd = ::epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd < 0) {
        ::close(listen_fd);
        throw std::runtime_error("failed to create epoll");
    }

    const int timer_fd = _openTimerFd();

    epoll_event listen_event {};
    listen_event.events = EPOLLIN;
    listen_event.data.ptr = &m_listen_event_token;
    epoll_event timer_event {};
    timer_event.events = EPOLLIN;
    timer_event.data.ptr = &m_timer_event_token;
    if (::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &listen_event) != 0 ||
        ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &timer_event) != 0) {
        ::close(timer_fd);
        ::close(epoll_fd);
        ::close(listen_fd);
        throw std::runtime_error("failed to register epoll fd");
    }

    std::array<epoll_event, 32> events {};
    while (!m_stop_requested.load()) {
        const int ready = ::epoll_wait(epoll_fd, events.data(), static_cast<int>(events.size()), -1);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            ::close(timer_fd);
            ::close(epoll_fd);
            ::close(listen_fd);
            return 1;
        }

        for (int index = 0; index < ready; ++index) {
            const epoll_event& event = events[static_cast<std::size_t>(index)];
            auto* token = static_cast<EventToken*>(event.data.ptr);
            if (token == nullptr) {
                continue;
            }
            if (token->kind == EventToken::Kind::Listen) {
                _acceptClients(epoll_fd, listen_fd);
                continue;
            }
            if (token->kind == EventToken::Kind::Timer) {
                uint64_t expirations = 0;
                const ssize_t timer_read = ::read(timer_fd, &expirations, sizeof(expirations));
                if (timer_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    ::close(timer_fd);
                    ::close(epoll_fd);
                    ::close(listen_fd);
                    return 1;
                }
                const auto now = std::chrono::steady_clock::now();
                for (auto& slot : m_session_slots) {
                    if (slot.mode != SessionSlotMode::Live) {
                        continue;
                    }
                    _queOutBound(slot.client, now);
                    if (!_isOutQueueEmpty(slot.client) && !_sendQueFront(slot.client)) {
                        _closeLiveSession(epoll_fd, slot);
                        continue;
                    }
                    if (now - slot.client.last_receive_time >= std::chrono::seconds(15)) {
                        _closeLiveSession(epoll_fd, slot);
                    }
                }
                continue;
            }

            SessionSlot* slot = _resolveLiveSlot(*token);
            if (slot == nullptr) {
                continue;
            }
            ClientState& client = slot->client;
            if ((event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) != 0U) {
                _closeLiveSession(epoll_fd, *slot);
                continue;
            }
            if (!_receiveClientData(client)) {
                _closeLiveSession(epoll_fd, *slot);
                continue;
            }
            while (true) {
                auto packet = _tryReadPayload(client);
                if (!packet.has_value()) {
                    break;
                }
                if (!_handleClientPacket(*slot, packet->type, packet->payload)) {
                    _closeLiveSession(epoll_fd, *slot);
                    break;
                }
            }
        }
    }

    for (auto& slot : m_session_slots) {
        if (slot.mode != SessionSlotMode::Live) {
            continue;
        }
        if (slot.client.fd >= 0) {
            ::close(slot.client.fd);
        }
        _releaseSessionSlot(slot);
    }
    ::close(timer_fd);
    ::close(epoll_fd);
    ::close(listen_fd);
    return 0;
}

uint64_t DummyExchangeServer::_readTimestampNs() const {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                     std::chrono::steady_clock::now().time_since_epoch())
                                     .count());
}

bool DummyExchangeServer::_isOutQueueEmpty(const ClientState& client) const {
    return client.out_bound_que.isEmpty();
}

std::size_t DummyExchangeServer::_readOutQueueSize(const ClientState& client) const {
    return client.out_bound_que.readSize();
}

DummyExchangeServer::OutboundPacket& DummyExchangeServer::_readOutQueueFront(ClientState& client) {
    return client.out_bound_que.readFront();
}

const DummyExchangeServer::OutboundPacket& DummyExchangeServer::_readOutQueueFront(const ClientState& client) const {
    return client.out_bound_que.readFront();
}

bool DummyExchangeServer::_pushOutQueue(ClientState& client, const uint8_t* bytes, std::size_t size) {
    if (size > kMaxSoupFrameSize) {
        return false;
    }

    OutboundPacket packet {};
    if (size > 0) {
        std::copy_n(bytes, static_cast<std::ptrdiff_t>(size), packet.payload.data());
    }
    packet.size = size;
    packet.offset = 0;
    return client.out_bound_que.pushBack(packet);
}

void DummyExchangeServer::_popOutQueue(ClientState& client) {
    (void)client.out_bound_que.eraseFront();
}

uint64_t DummyExchangeServer::_readNextSequence(const ClientState& client) const {
    return client.next_sequence;
}

uint64_t DummyExchangeServer::_readNextOrderRef(const ClientState& client) const {
    return client.next_order_ref_num;
}

uint64_t DummyExchangeServer::_readNextMatchNumber(const ClientState& client) const {
    return client.next_match_number;
}
