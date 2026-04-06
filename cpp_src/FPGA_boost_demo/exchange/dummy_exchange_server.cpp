#include "dummy_exchange_server.h"

#include <array>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace {

ProtocolConfig makeProtocolConfig(const DummyExchangeConfig& config) {
    ProtocolConfig protocol {};
    protocol.username = config.username;
    protocol.password = config.password;
    protocol.session_id = config.session_id;
    protocol.price_min = config.price_min;
    protocol.price_max = config.price_max;
    protocol.max_shares = config.max_shares;
    protocol.fill_delay = config.fill_delay;
    protocol.replay_capacity = config.replay_capacity;
    return protocol;
}

const char* readSoupTypeName(uint8_t type) {
    switch (type) {
        case static_cast<uint8_t>('A'):
            return "LOGIN_ACCEPTED";
        case static_cast<uint8_t>('J'):
            return "LOGIN_REJECTED";
        case static_cast<uint8_t>('S'):
            return "SEQUENCED_DATA";
        case static_cast<uint8_t>('H'):
            return "SERVER_HEARTBEAT";
        case static_cast<uint8_t>('L'):
            return "LOGIN_REQUEST";
        case static_cast<uint8_t>('U'):
            return "UNSEQUENCED_DATA";
        case static_cast<uint8_t>('R'):
            return "CLIENT_HEARTBEAT";
        case static_cast<uint8_t>('O'):
            return "LOGOUT_REQUEST";
        default:
            return "UNKNOWN";
    }
}

const char* readOuchTypeName(uint8_t type) {
    switch (type) {
        case static_cast<uint8_t>('O'):
            return "ENTER_ORDER";
        case static_cast<uint8_t>('A'):
            return "ACCEPTED";
        case static_cast<uint8_t>('E'):
            return "EXECUTED";
        case static_cast<uint8_t>('J'):
            return "REJECTED";
        default:
            return "UNKNOWN";
    }
}

void printSOUPFrameInfo(uint64_t session_id, const uint8_t* bytes, std::size_t size) {
    if (size < 3) {
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

    if (soup_type == static_cast<uint8_t>('S') && size >= 4U) {
        const uint8_t ouch_type = bytes[3];
        std::printf(" ouch=%s(%c)",
                    readOuchTypeName(ouch_type),
                    static_cast<char>(ouch_type));
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
    : m_config(std::move(config))
{
    m_session_pool.reserve(m_config.session_capacity);
    for (std::size_t index = 0; index < m_config.session_capacity; ++index) {
        SessionSlot slot(makeProtocolConfig(m_config));
        slot.epoll_event_tag.slot_index = static_cast<uint32_t>(index);
        slot.epoll_event_tag.generation = slot.generation;
        m_session_pool.push_back(slot);
    }

    m_free_slot_indexes.resize(m_config.session_capacity);
    m_free_slot_count = m_config.session_capacity;
    for (std::size_t index = 0; index < m_config.session_capacity; ++index) {
        m_free_slot_indexes[index] = static_cast<uint32_t>(m_config.session_capacity - 1U - index);
    }
}

void DummyExchangeServer::requestStopForTest() {
    m_stop_requested.store(true);
}

DummyExchangeServer::SessionSlot& DummyExchangeServer::_acquireSessionSlot(SessionSlotMode mode) {
    if (m_free_slot_count == 0) {
        throw std::runtime_error("session pool full");
    }

    const uint32_t slot_index = m_free_slot_indexes[--m_free_slot_count];
    SessionSlot& slot = m_session_pool[slot_index];
    slot.mode = mode;
    slot.transport = TransportState {};
    slot.protocol.reset(0, std::chrono::steady_clock::now());
    slot.epoll_event_tag.slot_index = slot_index;
    slot.epoll_event_tag.generation = slot.generation;
    return slot;
}

void DummyExchangeServer::_releaseSessionSlot(SessionSlot& slot) {
    if (slot.mode == SessionSlotMode::Free) {
        return;
    }

    slot.transport = TransportState {};
    slot.protocol.reset(0, std::chrono::steady_clock::now());
    slot.mode = SessionSlotMode::Free;
    ++slot.generation;
    slot.epoll_event_tag.generation = slot.generation;
    m_free_slot_indexes[m_free_slot_count++] = slot.epoll_event_tag.slot_index;
}

DummyExchangeServer::SessionSlot* DummyExchangeServer::_getLiveSlot(const EpollEventTag& event_tag) {
    if (event_tag.kind != EpollEventTag::Kind::Connected || event_tag.slot_index >= m_session_pool.size()) {
        return nullptr;
    }

    SessionSlot& slot = m_session_pool[event_tag.slot_index];
    if (slot.mode != SessionSlotMode::Used || slot.generation != event_tag.generation) {
        return nullptr;
    }
    return &slot;
}

DummyExchangeServer::SessionSlot& DummyExchangeServer::_resolveTestSlot(TestSessionHandle session) {
    if (session.slot_index >= m_session_pool.size()) {
        throw std::runtime_error("invalid test session handle");
    }

    SessionSlot& slot = m_session_pool[session.slot_index];
    if (slot.mode != SessionSlotMode::Test || slot.generation != session.generation) {
        throw std::runtime_error("stale test session handle");
    }
    return slot;
}

const DummyExchangeServer::SessionSlot& DummyExchangeServer::_resolveTestSlot(TestSessionHandle session) const {
    if (session.slot_index >= m_session_pool.size()) {
        throw std::runtime_error("invalid test session handle");
    }

    const SessionSlot& slot = m_session_pool[session.slot_index];
    if (slot.mode != SessionSlotMode::Test || slot.generation != session.generation) {
        throw std::runtime_error("stale test session handle");
    }
    return slot;
}

DummyExchangeServer::TestSessionHandle DummyExchangeServer::createSessionForTest() {
    SessionSlot& slot = _acquireSessionSlot(SessionSlotMode::Test);
    slot.transport.session_id = m_next_test_session_id++;
    slot.protocol.reset(slot.transport.session_id, std::chrono::steady_clock::now());
    return TestSessionHandle {
        .slot_index = slot.epoll_event_tag.slot_index,
        .generation = slot.generation,
    };
}

void DummyExchangeServer::releaseSessionForTest(TestSessionHandle session) {
    SessionSlot& slot = _resolveTestSlot(session);
    _releaseSessionSlot(slot);
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

void DummyExchangeServer::_acceptClients() {
    while (true) {
        const int client_fd = ::accept(m_listen_fd, nullptr, nullptr);
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

        SessionSlot& slot = _acquireSessionSlot(SessionSlotMode::Used);
        slot.transport.fd = client_fd;
        slot.transport.session_id = m_next_live_session_id++;
        slot.protocol.reset(slot.transport.session_id, std::chrono::steady_clock::now());
        slot.epoll_event_tag.generation = slot.generation;
        slot.epoll_event_tag.kind = EpollEventTag::Kind::Connected;
        slot.event.events = EPOLLIN | EPOLLRDHUP;
        slot.event.data.ptr = &slot.epoll_event_tag;
        if (::epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, client_fd, &slot.event) != 0) {
            ::close(client_fd);
            _releaseSessionSlot(slot);
        }
    }
}

bool DummyExchangeServer::_receiveBytes(SessionSlot& slot) {
    std::array<uint8_t, 1024> buffer {};
    while (true) {
        const ssize_t count = ::recv(slot.transport.fd, buffer.data(), buffer.size(), 0);
        if (count > 0) {
            if (!slot.protocol.appendReceivedBytes(buffer.data(),
                                                  static_cast<std::size_t>(count),
                                                  std::chrono::steady_clock::now())) {
                return false;
            }
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



bool DummyExchangeServer::_sendFrame(SessionSlot& slot,
                                              std::chrono::steady_clock::time_point now) {
    while (slot.protocol.hasOutboundFrame()) {
        SOUPBinFrame& frame = slot.protocol.readFrontFrame();
        while (true) {
            const ssize_t written = ::send(slot.transport.fd,
                                           frame.payload.data() + static_cast<std::ptrdiff_t>(frame.offset),
                                           frame.size - frame.offset,
                                           MSG_NOSIGNAL);
            if (written < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    slot.event.events |= EPOLLOUT;
                    if (::epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, slot.transport.fd, &slot.event) != 0) {
                        return false;
                    }
                    return true;
                }
                return false;
            }
            if (written == 0) {
                return false;
            }

            frame.offset += static_cast<std::size_t>(written);
            slot.protocol.writeLastTime(now);
            if (frame.offset >= frame.size) {
                printSOUPFrameInfo(slot.transport.session_id, frame.payload.data(), frame.size);
                slot.protocol.eraseFrontFrame();
                break;
            }
        }
    }
    slot.event.events &= ~EPOLLOUT;
    if (::epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, slot.transport.fd, &slot.event) != 0) {
        return false;
    }
    return true;
}


void DummyExchangeServer::_closeLiveSession(SessionSlot& slot) {
    if (slot.mode != SessionSlotMode::Used) {
        return;
    }

    const int fd = slot.transport.fd;
    if (fd >= 0) {
        (void)::epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
        ::close(fd);
    }
    _releaseSessionSlot(slot);
}

void DummyExchangeServer::_resetLiveSessionsBeforeRun() {
    for (auto& slot : m_session_pool) {
        if (slot.mode == SessionSlotMode::Used) {
            _releaseSessionSlot(slot);
        }
    }
}

int DummyExchangeServer::_openListenFd() const {
    const int listen_fd = openListenSocket(m_config);
    if (!_setNonBlocking(listen_fd)) {
        ::close(listen_fd);
        throw std::runtime_error("failed to make listen socket nonblocking");
    }

    std::printf("dummy_exchange_server listening on %s:%u\n", m_config.listen_ip.c_str(), m_config.port);
    std::fflush(stdout);
    return listen_fd;
}

int DummyExchangeServer::_openEpollFd() const {
    const int epoll_fd = ::epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd < 0) {
        throw std::runtime_error("failed to create epoll");
    }
    return epoll_fd;
}

void DummyExchangeServer::_registerEpoll() {
    m_listen_event.events = EPOLLIN;
    m_listen_event.data.ptr = &m_listen_event_tag;

    m_timer_event.events = EPOLLIN;
    m_timer_event.data.ptr = &m_timer_event_tag;

    if (::epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_listen_fd, &m_listen_event) != 0 ||
        ::epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_timer_fd, &m_timer_event) != 0) {
        throw std::runtime_error("failed to register epoll fd");
    }
}

void DummyExchangeServer::_shutdownLiveSessions() {
    for (auto& slot : m_session_pool) {
        if (slot.mode != SessionSlotMode::Used) {
            continue;
        }
        if (slot.transport.fd >= 0) {
            ::close(slot.transport.fd);
        }
        _releaseSessionSlot(slot);
    }
}

void DummyExchangeServer::_closeRunFds() const {
    if (m_timer_fd >= 0) {
        ::close(m_timer_fd);
    }
    if (m_epoll_fd >= 0) {
        ::close(m_epoll_fd);
    }
    if (m_listen_fd >= 0) {
        ::close(m_listen_fd);
    }
}

int DummyExchangeServer::_runEpollWait() {
    std::array<epoll_event, 32> events {};
    while (!m_stop_requested.load()) {
        const int ready = ::epoll_wait(m_epoll_fd, events.data(), static_cast<int>(events.size()), -1);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 1;
        }

        for (int index = 0; index < ready; ++index) {
            if (!_handleReadyEvent(events[static_cast<std::size_t>(index)])) {
                return 1;
            }
        }
    }
    return 0;
}

bool DummyExchangeServer::_handleReadyEvent(const epoll_event& event) {

                                                
    auto* event_tag = static_cast<EpollEventTag*>(event.data.ptr);
    if (event_tag == nullptr) {
        return true;
    }
    if (event_tag->kind == EpollEventTag::Kind::Listen) {
        _acceptClients();
        return true;
    }
    if (event_tag->kind == EpollEventTag::Kind::Timer) {
        return _handleTimerTick();
    }

    _handleLiveSessionEvent(event, *event_tag);

    return true;
}

bool DummyExchangeServer::_handleTimerTick() {
    uint64_t expirations = 0;
    const ssize_t timer_read = ::read(m_timer_fd, &expirations, sizeof(expirations));
    if (timer_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    for (SessionSlot& slot : m_session_pool) {
        if (slot.mode != SessionSlotMode::Used) {
            continue;
        }
        slot.protocol.onTimerTick(now);
        if (!_sendFrame(slot, now)) {
            _closeLiveSession(slot);
            continue;
        }
        if (slot.protocol.shouldClose()) {
            _closeLiveSession(slot);
        }
    }
    return true;
}

void DummyExchangeServer::_handleLiveSessionEvent(const epoll_event& event,
                                                  const EpollEventTag& event_tag) {
    SessionSlot* slot = _getLiveSlot(event_tag);
    if (slot == nullptr) {
        return;
    }
    if ((event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) != 0U) {
        _closeLiveSession(*slot);
        return;
    }
    if (!_receiveBytes(*slot)) {
        _closeLiveSession(*slot);
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (!_sendFrame(*slot, now)) {
        _closeLiveSession(*slot);
        return;
    }
    if (slot->protocol.shouldClose()) {
        _closeLiveSession(*slot);
    }
}

int DummyExchangeServer::run() {
    m_stop_requested.store(false);
    _resetLiveSessionsBeforeRun();

    try {
        m_listen_fd = _openListenFd();
        m_epoll_fd = _openEpollFd();
        m_timer_fd = _openTimerFd();
        _registerEpoll();

        const int status = _runEpollWait();
        if (status == 0) {
            _shutdownLiveSessions();
        }
        _closeRunFds();
        return status;
    } catch (...) {
        _closeRunFds();
        throw;
    }
}
