#include "dummy_exchange_server.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <string_view>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

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
constexpr std::size_t kSessionWidth = 10;
constexpr std::size_t kSequenceWidth = 20;
constexpr std::size_t kUsernameWidth = 6;
constexpr std::size_t kPasswordWidth = 10;

constexpr std::size_t kOuchEnterOrderSize = 16;
constexpr std::size_t kOuchAcceptedSize = 32;
constexpr std::size_t kOuchExecutedSize = 29;
constexpr std::size_t kOuchRejectedSize = 15;

constexpr uint16_t kRejectUnsupportedSymbol = 0x0011;
constexpr uint16_t kRejectInvalidShares = 0x0002;
constexpr uint16_t kRejectInvalidPrice = 0x0015;

struct SoupPacket {
    uint8_t type {0};
    std::vector<uint8_t> payload {};
};

struct LoginRequest {
    std::string username {};
    std::string password {};
    std::string requested_session {};
    uint64_t requested_sequence {1};
};

struct AcceptedMessage {
    uint64_t timestamp_ns {0};
    uint32_t tag {0};
    uint16_t stock_locate {0};
    uint32_t shares {0};
    uint32_t price {0};
    char side {'B'};
    uint64_t order_ref_num {0};
};

struct ExecutedMessage {
    uint64_t timestamp_ns {0};
    uint32_t tag {0};
    uint32_t executed_shares {0};
    uint32_t price {0};
    uint64_t match_number {0};
};

struct RejectedMessage {
    uint64_t timestamp_ns {0};
    uint32_t tag {0};
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

bool readSequenceField(const uint8_t* data, std::size_t width, uint64_t& value) {
    std::string_view text(reinterpret_cast<const char*>(data), width);
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
                      std::size_t width,
                      std::string_view value,
                      bool left_pad) {
    std::fill_n(out, static_cast<std::ptrdiff_t>(width), static_cast<uint8_t>(' '));
    const std::size_t copy_size = std::min(width, value.size());
    if (left_pad) {
        std::copy_n(value.end() - static_cast<std::ptrdiff_t>(copy_size),
                    static_cast<std::ptrdiff_t>(copy_size),
                    out + static_cast<std::ptrdiff_t>(width - copy_size));
        return;
    }

    std::copy_n(value.begin(), static_cast<std::ptrdiff_t>(copy_size), out);
}

void writeSequenceField(uint8_t* out, std::size_t width, uint64_t value) {
    std::fill_n(out, static_cast<std::ptrdiff_t>(width), static_cast<uint8_t>(' '));
    const std::string text = std::to_string(value);
    const std::size_t copy_size = std::min(width, text.size());
    std::copy_n(text.end() - static_cast<std::ptrdiff_t>(copy_size),
                static_cast<std::ptrdiff_t>(copy_size),
                out + static_cast<std::ptrdiff_t>(width - copy_size));
}

std::vector<uint8_t> writeSoupPacket(uint8_t type, const uint8_t* payload, std::size_t payload_size) {
    std::vector<uint8_t> bytes(kSoupHeaderSize + payload_size, 0);
    writeBigEndian16(bytes.data(), static_cast<uint16_t>(payload_size + 1U));
    bytes[2] = type;
    if (payload_size > 0) {
        std::copy_n(payload, static_cast<std::ptrdiff_t>(payload_size), bytes.data() + 3);
    }
    return bytes;
}

std::vector<uint8_t> writeLoginAcceptedFrame(std::string_view session, uint64_t next_sequence) {
    std::array<uint8_t, kSessionWidth + kSequenceWidth> payload {};
    writePaddedField(payload.data(), kSessionWidth, session, true);
    writeSequenceField(payload.data() + static_cast<std::ptrdiff_t>(kSessionWidth),
                       kSequenceWidth,
                       next_sequence);
    return writeSoupPacket(kSoupLoginAcceptedType, payload.data(), payload.size());
}

std::vector<uint8_t> writeLoginRejectedFrame(char reason) {
    const uint8_t payload = static_cast<uint8_t>(reason);
    return writeSoupPacket(kSoupLoginRejectedType, &payload, 1U);
}

std::vector<uint8_t> writeOuchAccepted(const AcceptedMessage& accepted) {
    std::vector<uint8_t> bytes(kOuchAcceptedSize, 0);
    bytes[0] = kOuchAcceptedType;
    writeBigEndian64(bytes.data() + 1, accepted.timestamp_ns);
    writeBigEndian32(bytes.data() + 9, accepted.tag);
    bytes[13] = static_cast<uint8_t>(accepted.side);
    writeBigEndian16(bytes.data() + 14, accepted.stock_locate);
    writeBigEndian32(bytes.data() + 16, accepted.shares);
    writeBigEndian32(bytes.data() + 20, accepted.price);
    writeBigEndian64(bytes.data() + 24, accepted.order_ref_num);
    return bytes;
}

std::vector<uint8_t> writeOuchExecuted(const ExecutedMessage& executed) {
    std::vector<uint8_t> bytes(kOuchExecutedSize, 0);
    bytes[0] = kOuchExecutedType;
    writeBigEndian64(bytes.data() + 1, executed.timestamp_ns);
    writeBigEndian32(bytes.data() + 9, executed.tag);
    writeBigEndian32(bytes.data() + 13, executed.executed_shares);
    writeBigEndian32(bytes.data() + 17, executed.price);
    writeBigEndian64(bytes.data() + 21, executed.match_number);
    return bytes;
}

std::vector<uint8_t> writeOuchRejected(const RejectedMessage& rejected) {
    std::vector<uint8_t> bytes(kOuchRejectedSize, 0);
    bytes[0] = kOuchRejectedType;
    writeBigEndian64(bytes.data() + 1, rejected.timestamp_ns);
    writeBigEndian32(bytes.data() + 9, rejected.tag);
    writeBigEndian16(bytes.data() + 13, rejected.reason);
    return bytes;
}

bool sendAll(int fd, const std::vector<uint8_t>& bytes) {
    std::size_t offset = 0;
    while (offset < bytes.size()) {
        const ssize_t written = ::send(fd,
                                       bytes.data() + static_cast<std::ptrdiff_t>(offset),
                                       bytes.size() - offset,
                                       0);
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

std::optional<SoupPacket> readSoupPacket(int fd) {
    std::array<uint8_t, 2> header {};
    if (!readExact(fd, header.data(), header.size())) {
        return std::nullopt;
    }

    const uint16_t encoded_length = readBigEndian16(header.data());
    if (encoded_length == 0) {
        return std::nullopt;
    }

    std::vector<uint8_t> bytes(header.begin(), header.end());
    bytes.resize(static_cast<std::size_t>(encoded_length) + 2U);
    if (!readExact(fd, bytes.data() + static_cast<std::ptrdiff_t>(header.size()), encoded_length)) {
        return std::nullopt;
    }

    SoupPacket packet {};
    packet.type = bytes[2];
    packet.payload.assign(bytes.begin() + 3, bytes.end());
    return packet;
}

bool parseLoginRequest(const SoupPacket& packet, LoginRequest& request) {
    if (packet.type != kSoupLoginRequestType ||
        packet.payload.size() != kUsernameWidth + kPasswordWidth + kSessionWidth + kSequenceWidth) {
        return false;
    }

    request.username.assign(reinterpret_cast<const char*>(packet.payload.data()), kUsernameWidth);
    request.password.assign(reinterpret_cast<const char*>(packet.payload.data() + kUsernameWidth), kPasswordWidth);
    request.requested_session.assign(reinterpret_cast<const char*>(packet.payload.data() + kUsernameWidth + kPasswordWidth),
                                     kSessionWidth);
    return readSequenceField(packet.payload.data() + kUsernameWidth + kPasswordWidth + kSessionWidth,
                             kSequenceWidth,
                             request.requested_sequence);
}

ExchangeEnterOrder readEnterOrder(const std::vector<uint8_t>& bytes) {
    if (bytes.size() != kOuchEnterOrderSize || bytes.front() != kOuchEnterOrderType) {
        throw std::runtime_error("invalid enter order payload");
    }

    ExchangeEnterOrder order {};
    order.tag = readBigEndian32(bytes.data() + 1);
    order.side = static_cast<char>(bytes[5]);
    order.stock_locate = readBigEndian16(bytes.data() + 6);
    order.shares = readBigEndian32(bytes.data() + 8);
    order.price = readBigEndian32(bytes.data() + 12);
    return order;
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
    : m_config(std::move(config)) {}

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

DummyExchangeServer::SessionState& DummyExchangeServer::_findOrCreateTestSession(uint64_t session_id) {
    auto& session = m_test_sessions[session_id];
    if (session.last_send == std::chrono::steady_clock::time_point {}) {
        const auto now = std::chrono::steady_clock::now();
        session.last_send = now;
        session.last_receive = now;
    }
    return session;
}

uint64_t DummyExchangeServer::createSessionForTest() {
    const uint64_t session_id = static_cast<uint64_t>(m_test_sessions.size() + 1);
    (void)_findOrCreateTestSession(session_id);
    return session_id;
}

void DummyExchangeServer::appendReadBytesForTest(uint64_t session_id, const std::vector<uint8_t>& bytes) {
    auto& session = _findOrCreateTestSession(session_id);
    session.read_buffer.insert(session.read_buffer.end(), bytes.begin(), bytes.end());
}

std::optional<uint8_t> DummyExchangeServer::tryReadPacketTypeForTest(uint64_t session_id) {
    return _tryReadPacketType(_findOrCreateTestSession(session_id));
}

void DummyExchangeServer::queuePacketForTest(uint64_t session_id, const std::vector<uint8_t>& bytes) {
    auto& session = _findOrCreateTestSession(session_id);
    session.write_queue.push_back(OutboundPacket {.bytes = bytes, .offset = 0});
}

bool DummyExchangeServer::consumeQueuedBytesForTest(uint64_t session_id, std::size_t count) {
    auto& session = _findOrCreateTestSession(session_id);
    while (count > 0 && !session.write_queue.empty()) {
        auto& front = session.write_queue.front();
        const std::size_t remaining = front.bytes.size() - front.offset;
        const std::size_t chunk = std::min(remaining, count);
        front.offset += chunk;
        count -= chunk;
        if (front.offset == front.bytes.size()) {
            session.write_queue.erase(session.write_queue.begin());
        }
    }
    return count == 0;
}

std::size_t DummyExchangeServer::readQueuedPacketCountForTest(uint64_t session_id) const {
    const auto found = m_test_sessions.find(session_id);
    if (found == m_test_sessions.end()) {
        return 0;
    }
    return found->second.write_queue.size();
}

void DummyExchangeServer::markLoggedInForTest(uint64_t session_id) {
    _findOrCreateTestSession(session_id).is_logged_in = true;
}

void DummyExchangeServer::setLastSendAgoForTest(uint64_t session_id, std::chrono::seconds age) {
    _findOrCreateTestSession(session_id).last_send = std::chrono::steady_clock::now() - age;
}

void DummyExchangeServer::handleTimerTickForTest() {
    const auto now = std::chrono::steady_clock::now();
    for (auto& [session_id, session] : m_test_sessions) {
        (void)session_id;
        _handleTimerTick(session, now);
    }
}

std::optional<uint8_t> DummyExchangeServer::peekFrontPacketTypeForTest(uint64_t session_id) const {
    const auto found = m_test_sessions.find(session_id);
    if (found == m_test_sessions.end() || found->second.write_queue.empty() ||
        found->second.write_queue.front().bytes.size() < 3) {
        return std::nullopt;
    }
    return found->second.write_queue.front().bytes[2];
}

std::optional<uint8_t> DummyExchangeServer::_tryReadPacketType(SessionState& session) {
    if (session.read_buffer.size() < kSoupHeaderSize) {
        return std::nullopt;
    }

    const uint16_t encoded_length = readBigEndian16(session.read_buffer.data());
    const std::size_t packet_size = static_cast<std::size_t>(encoded_length) + 2U;
    if (encoded_length == 0 || session.read_buffer.size() < packet_size) {
        return std::nullopt;
    }

    const uint8_t type = session.read_buffer[2];
    session.read_buffer.erase(session.read_buffer.begin(), session.read_buffer.begin() + packet_size);
    return type;
}

void DummyExchangeServer::_handleTimerTick(SessionState& session, std::chrono::steady_clock::time_point now) {
    if (!session.is_logged_in) {
        return;
    }
    if (now - session.last_send < std::chrono::seconds(1)) {
        return;
    }
    session.write_queue.push_back(OutboundPacket {
        .bytes = writeSoupPacket(kSoupServerHeartbeatType, nullptr, 0),
        .offset = 0,
    });
    session.last_send = now;
}

HandledOrderResult DummyExchangeServer::handleEnterOrderForTest(uint64_t session_id,
                                                                const ExchangeEnterOrder& order) {
    return _handleEnterOrder(_findOrCreateTestSession(session_id), order);
}

uint64_t DummyExchangeServer::readSessionNextSequenceForTest(uint64_t session_id) const {
    const auto found = m_test_sessions.find(session_id);
    if (found == m_test_sessions.end()) {
        return 0;
    }
    return _readNextSequence(found->second);
}

HandledOrderResult DummyExchangeServer::handleEnterOrder(const ExchangeEnterOrder& order) {
    return _handleEnterOrder(m_single_session, order);
}

HandledOrderResult DummyExchangeServer::_handleEnterOrder(SessionState& session,
                                                          const ExchangeEnterOrder& order) {
    const auto existing = session.order_results.find(order.tag);
    if (existing != session.order_results.end()) {
        HandledOrderResult replay = existing->second;
        replay.is_duplicate = true;
        return replay;
    }

    HandledOrderResult result {};
    result.validation = validateEnterOrder(order);

    if (result.validation.kind == ExchangeValidationKind::Rejected) {
        const std::vector<uint8_t> payload = writeOuchRejected(RejectedMessage {
            .timestamp_ns = _readTimestampNs(),
            .tag = order.tag,
            .reason = result.validation.reject_reason,
        });
        _storeSequenced(session, payload);
        result.outbound_messages.push_back(payload);
    } else {
        const std::vector<uint8_t> accepted_payload = writeOuchAccepted(AcceptedMessage {
            .timestamp_ns = _readTimestampNs(),
            .tag = order.tag,
            .stock_locate = order.stock_locate,
            .shares = order.shares,
            .price = order.price,
            .side = order.side,
            .order_ref_num = _readNextOrderRef(session),
        });
        _storeSequenced(session, accepted_payload);
        result.outbound_messages.push_back(accepted_payload);

        session.pending_fills.push_back(PendingFill {
            .tag = order.tag,
            .executed_shares = order.shares,
            .price = order.price,
            .match_number = _readNextMatchNumber(session),
            .due_time = std::chrono::steady_clock::now() + m_config.fill_delay,
        });
    }

    session.order_results.emplace(order.tag, result);
    return result;
}

int DummyExchangeServer::run() {
    const int listen_fd = openListenSocket(m_config);
    std::printf("dummy_exchange_server listening on %s:%u\n", m_config.listen_ip.c_str(), m_config.port);
    std::fflush(stdout);

    const int client_fd = ::accept(listen_fd, nullptr, nullptr);
    if (client_fd < 0) {
        ::close(listen_fd);
        std::perror("accept");
        return 1;
    }

    auto login_packet = readSoupPacket(client_fd);
    if (!login_packet.has_value()) {
        ::close(client_fd);
        ::close(listen_fd);
        return 1;
    }

    LoginRequest login {};
    if (!parseLoginRequest(*login_packet, login)) {
        (void)sendAll(client_fd, writeLoginRejectedFrame('A'));
        ::close(client_fd);
        ::close(listen_fd);
        return 1;
    }

    const std::string username = trimSpaces(login.username);
    const std::string password = trimSpaces(login.password);
    const std::string requested_session = trimSpaces(login.requested_session);
    if (username != m_config.username || password != m_config.password ||
        (!requested_session.empty() && requested_session != m_config.session_id)) {
        (void)sendAll(client_fd, writeLoginRejectedFrame('A'));
        ::close(client_fd);
        ::close(listen_fd);
        return 1;
    }

    uint64_t next_sequence = login.requested_sequence == 0 ? 1 : login.requested_sequence;
    if (next_sequence > _readNextSequence(m_single_session)) {
        next_sequence = _readNextSequence(m_single_session);
    }
    (void)sendAll(client_fd, writeLoginAcceptedFrame(m_config.session_id, next_sequence));

    for (uint64_t sequence = next_sequence; sequence < _readNextSequence(m_single_session); ++sequence) {
        const auto& payload = m_single_session.sequenced_history[static_cast<std::size_t>(sequence - 1)];
        if (!sendAll(client_fd, writeSoupPacket(kSoupSequencedDataType, payload.data(), payload.size()))) {
            ::close(client_fd);
            ::close(listen_fd);
            return 1;
        }
    }

    auto last_send = std::chrono::steady_clock::now();
    auto last_receive = std::chrono::steady_clock::now();
    while (true) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(client_fd, &read_set);

        timeval timeout {};
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        int ready = ::select(client_fd + 1, &read_set, nullptr, nullptr, &timeout);
        if (ready < 0) {
            break;
        }

        const auto now = std::chrono::steady_clock::now();
        if (ready > 0 && FD_ISSET(client_fd, &read_set)) {
            auto packet = readSoupPacket(client_fd);
            if (!packet.has_value()) {
                break;
            }
            last_receive = now;
            if (packet->type == kSoupUnsequencedDataType) {
                const ExchangeEnterOrder order = readEnterOrder(packet->payload);
                const auto handled = handleEnterOrder(order);
                if (!handled.is_duplicate) {
                    for (const auto& payload : handled.outbound_messages) {
                        if (!sendAll(client_fd, writeSoupPacket(kSoupSequencedDataType, payload.data(), payload.size()))) {
                            ready = -1;
                            break;
                        }
                        last_send = std::chrono::steady_clock::now();
                    }
                }
            } else if (packet->type == kSoupLogoutRequestType) {
                break;
            } else if (packet->type == kSoupClientHeartbeatType) {
                continue;
            }
        }

        if (ready < 0) {
            break;
        }

        for (auto it = m_single_session.pending_fills.begin(); it != m_single_session.pending_fills.end();) {
            if (it->due_time > now) {
                ++it;
                continue;
            }

            const std::vector<uint8_t> payload = writeOuchExecuted(ExecutedMessage {
                .timestamp_ns = _readTimestampNs(),
                .tag = it->tag,
                .executed_shares = it->executed_shares,
                .price = it->price,
                .match_number = it->match_number,
            });
            _storeSequenced(m_single_session, payload);
            if (!sendAll(client_fd, writeSoupPacket(kSoupSequencedDataType, payload.data(), payload.size()))) {
                ready = -1;
                break;
            }
            last_send = std::chrono::steady_clock::now();
            it = m_single_session.pending_fills.erase(it);
        }
        if (ready < 0) {
            break;
        }

        if (now - last_send >= std::chrono::seconds(1)) {
            if (!sendAll(client_fd, writeSoupPacket(kSoupServerHeartbeatType, nullptr, 0))) {
                break;
            }
            last_send = std::chrono::steady_clock::now();
        }

        if (now - last_receive >= std::chrono::seconds(15)) {
            break;
        }
    }

    ::close(client_fd);
    ::close(listen_fd);
    return 0;
}

uint64_t DummyExchangeServer::_readTimestampNs() const {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                     std::chrono::steady_clock::now().time_since_epoch())
                                     .count());
}

uint64_t DummyExchangeServer::_readNextSequence(const SessionState& session) const {
    return session.next_sequence;
}

uint64_t DummyExchangeServer::_readNextOrderRef(const SessionState& session) const {
    return session.next_order_ref_num;
}

uint64_t DummyExchangeServer::_readNextMatchNumber(const SessionState& session) const {
    return session.next_match_number;
}

void DummyExchangeServer::_storeSequenced(SessionState& session, const std::vector<uint8_t>& payload) {
    session.sequenced_history.push_back(payload);
    ++session.next_sequence;
    ++session.next_order_ref_num;
    ++session.next_match_number;
}
