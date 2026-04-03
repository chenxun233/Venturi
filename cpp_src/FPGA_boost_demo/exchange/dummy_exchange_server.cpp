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

DummyExchangeServer::SessionState& DummyExchangeServer::_findOrCreateTestSession(uint64_t session_id) {
    auto& session = m_test_sessions[session_id];
    session.session_id = session_id;
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
    for (auto it = session.pending_fills.begin(); it != session.pending_fills.end();) {
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
        _storeSequenced(session, payload);
        _queueSoupFrame(session, kSoupSequencedDataType, payload.data(), payload.size());
        it = session.pending_fills.erase(it);
    }

    if (session.is_logged_in && now - session.last_send >= std::chrono::seconds(1)) {
        _queueSoupFrame(session, kSoupServerHeartbeatType, nullptr, 0);
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

        SessionState session {};
        session.fd = client_fd;
        session.session_id = m_next_live_session_id++;
        session.last_send = std::chrono::steady_clock::now();
        session.last_receive = session.last_send;
        m_live_sessions.emplace(client_fd, std::move(session));

        epoll_event event {};
        event.events = EPOLLIN | EPOLLRDHUP;
        event.data.fd = client_fd;
        if (::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) != 0) {
            m_live_sessions.erase(client_fd);
            ::close(client_fd);
        }
    }
}

std::optional<std::vector<uint8_t>> DummyExchangeServer::_tryReadPayload(SessionState& session) {
    if (session.read_buffer.size() < 2U) {
        return std::nullopt;
    }

    const uint16_t encoded_length = readBigEndian16(session.read_buffer.data());
    const std::size_t packet_size = static_cast<std::size_t>(encoded_length) + 2U;
    if (encoded_length == 0 || session.read_buffer.size() < packet_size) {
        return std::nullopt;
    }

    std::vector<uint8_t> payload(session.read_buffer.begin() + 2, session.read_buffer.begin() + packet_size);
    session.read_buffer.erase(session.read_buffer.begin(), session.read_buffer.begin() + packet_size);
    return payload;
}

bool DummyExchangeServer::_receiveClientData(SessionState& session) {
    std::array<uint8_t, 1024> buffer {};
    while (true) {
        const ssize_t count = ::recv(session.fd, buffer.data(), buffer.size(), 0);
        if (count > 0) {
            session.read_buffer.insert(session.read_buffer.end(), buffer.begin(), buffer.begin() + count);
            session.last_receive = std::chrono::steady_clock::now();
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

bool DummyExchangeServer::_flushQueuedPackets(SessionState& session) {
    while (!session.write_queue.empty()) {
        auto& front = session.write_queue.front();
        const ssize_t written = ::send(session.fd,
                                       front.bytes.data() + static_cast<std::ptrdiff_t>(front.offset),
                                       front.bytes.size() - front.offset,
                                       MSG_NOSIGNAL);
        if (written <= 0) {
            return false;
        }

        front.offset += static_cast<std::size_t>(written);
        session.last_send = std::chrono::steady_clock::now();
        if (front.offset == front.bytes.size()) {
            session.write_queue.erase(session.write_queue.begin());
        }
    }
    return true;
}

void DummyExchangeServer::_queueSoupFrame(SessionState& session,
                                          uint8_t type,
                                          const uint8_t* payload,
                                          std::size_t payload_size) {
    session.write_queue.push_back(OutboundPacket {
        .bytes = writeSoupPacket(type, payload, payload_size),
        .offset = 0,
    });
}

bool DummyExchangeServer::_handleClientPacket(SessionState& session,
                                              uint8_t type,
                                              const std::vector<uint8_t>& payload) {
    if (!session.is_logged_in) {
        SoupPacket packet {};
        packet.type = type;
        packet.payload = payload;

        LoginRequest login {};
        if (!parseLoginRequest(packet, login)) {
            _queueSoupFrame(session, kSoupLoginRejectedType, reinterpret_cast<const uint8_t*>("A"), 1U);
            return _flushQueuedPackets(session) && false;
        }

        const std::string username = trimSpaces(login.username);
        const std::string password = trimSpaces(login.password);
        const std::string requested_session = trimSpaces(login.requested_session);
        if (username != m_config.username || password != m_config.password ||
            (!requested_session.empty() && requested_session != m_config.session_id)) {
            _queueSoupFrame(session, kSoupLoginRejectedType, reinterpret_cast<const uint8_t*>("A"), 1U);
            return _flushQueuedPackets(session) && false;
        }

        session.is_logged_in = true;
        _queueSoupFrame(session, kSoupLoginAcceptedType, writeLoginAcceptedFrame(m_config.session_id, 1).data() + 3, 30);
        return _flushQueuedPackets(session);
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
    const auto handled = _handleEnterOrder(session, order);
    if (!handled.is_duplicate) {
        for (const auto& message : handled.outbound_messages) {
            _queueSoupFrame(session, kSoupSequencedDataType, message.data(), message.size());
        }
    }
    return _flushQueuedPackets(session);
}

void DummyExchangeServer::_closeSession(int epoll_fd, int fd) {
    (void)::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    ::close(fd);
    m_live_sessions.erase(fd);
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
    m_stop_requested.store(false);
    m_live_sessions.clear();

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
    listen_event.data.fd = listen_fd;
    epoll_event timer_event {};
    timer_event.events = EPOLLIN;
    timer_event.data.fd = timer_fd;
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
            if (event.data.fd == listen_fd) {
                _acceptClients(epoll_fd, listen_fd);
                continue;
            }
            if (event.data.fd == timer_fd) {
                uint64_t expirations = 0;
                const ssize_t timer_read = ::read(timer_fd, &expirations, sizeof(expirations));
                if (timer_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    ::close(timer_fd);
                    ::close(epoll_fd);
                    ::close(listen_fd);
                    return 1;
                }
                const auto now = std::chrono::steady_clock::now();
                std::vector<int> expired_fds {};
                for (auto& [fd, session] : m_live_sessions) {
                    _handleTimerTick(session, now);
                    if (!session.write_queue.empty() && !_flushQueuedPackets(session)) {
                        expired_fds.push_back(fd);
                        continue;
                    }
                    if (now - session.last_receive >= std::chrono::seconds(15)) {
                        expired_fds.push_back(fd);
                    }
                }
                for (const int fd : expired_fds) {
                    _closeSession(epoll_fd, fd);
                }
                continue;
            }

            auto found = m_live_sessions.find(event.data.fd);
            if (found == m_live_sessions.end()) {
                continue;
            }
            SessionState& session = found->second;
            if ((event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) != 0U) {
                _closeSession(epoll_fd, event.data.fd);
                continue;
            }
            if (!_receiveClientData(session)) {
                _closeSession(epoll_fd, event.data.fd);
                continue;
            }
            while (true) {
                auto packet = _tryReadPayload(session);
                if (!packet.has_value()) {
                    break;
                }
                if (packet->empty()) {
                    _closeSession(epoll_fd, event.data.fd);
                    break;
                }
                const uint8_t type = packet->front();
                const std::vector<uint8_t> payload(packet->begin() + 1, packet->end());
                if (!_handleClientPacket(session, type, payload)) {
                    _closeSession(epoll_fd, event.data.fd);
                    break;
                }
            }
        }
    }

    for (auto& [fd, session] : m_live_sessions) {
        (void)session;
        ::close(fd);
    }
    m_live_sessions.clear();
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
