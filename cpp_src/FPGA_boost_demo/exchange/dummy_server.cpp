#include "dummy_server.h"

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

DummyServer::DummyServer(DummyExchangeConfig config)
    : m_config(std::move(config))
    , m_transport(m_config.session_capacity)
    , m_protocol(makeProtocolConfig(m_config), m_config.session_capacity) {
    m_server_slots.reserve(m_config.session_capacity);
    for (std::size_t index = 0; index < m_config.session_capacity; ++index) {
        ServerSlot server_slot {};
        server_slot.slot_idx = static_cast<uint32_t>(index);
        server_slot.event_tag.slot_idx = static_cast<uint32_t>(index);
        server_slot.event_tag.generation = server_slot.generation;
        m_server_slots.push_back(server_slot);
    }

    m_free_server_slot_idx.resize(m_config.session_capacity);
    m_free_slot_count = m_config.session_capacity;
    for (std::size_t index = 0; index < m_config.session_capacity; ++index) {
        m_free_server_slot_idx[index] = static_cast<uint32_t>(m_config.session_capacity - 1U - index);
    }
}

void DummyServer::requestStop() {
    m_stop_requested.store(true);
}

DummyServer::ServerSlot& DummyServer::_allocateServerSlot(SlotMode mode) {
    if (m_free_slot_count == 0) {
        throw std::runtime_error("session pool full");
    }
    const uint32_t slot_idx = m_free_server_slot_idx[--m_free_slot_count];
    ServerSlot& server_slot = m_server_slots[slot_idx];
    server_slot.mode = mode;
    server_slot.slot_idx = slot_idx;
    server_slot.event_tag.slot_idx = slot_idx;
    server_slot.event_tag.generation = server_slot.generation;
    server_slot.event = epoll_event {};
    return server_slot;
}

void DummyServer::_releaseSlot(ServerSlot& server_slot) {
    if (server_slot.mode == SlotMode::Free) {
        return;
    }

    server_slot.mode = SlotMode::Free;
    ++server_slot.generation;
    server_slot.event_tag.generation = server_slot.generation;
    server_slot.event = epoll_event {};
    m_free_server_slot_idx[m_free_slot_count++] = server_slot.slot_idx;
}

DummyServer::ServerSlot* DummyServer::_getLiveSlot(const EpollEventTag& event_tag) {
    if (event_tag.kind != EpollEventTag::Kind::Connected ||
        event_tag.slot_idx >= m_server_slots.size()) {
        return nullptr;
    }

    ServerSlot& server_slot = m_server_slots[event_tag.slot_idx];
    if (server_slot.mode != SlotMode::Live || server_slot.generation != event_tag.generation) {
        return nullptr;
    }
    return &server_slot;
}

bool DummyServer::_setNonBlocking(int fd) const {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

int DummyServer::_openTimerFd() const {
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

void DummyServer::_resetLiveSlotsBeforeRun() {
    for (ServerSlot& server_slot : m_server_slots) {
        if (server_slot.mode != SlotMode::Live) {
            continue;
        }
        if (m_transport.hasSocket(static_cast<int>(server_slot.slot_idx))) {
            m_transport.closeConnection(static_cast<int>(server_slot.slot_idx));
        }
        m_protocol.releaseSlot(static_cast<int>(server_slot.slot_idx));
        _releaseSlot(server_slot);
    }
}

int DummyServer::_openListenFd() const {
    const int listen_fd = openListenSocket(m_config);
    if (!_setNonBlocking(listen_fd)) {
        ::close(listen_fd);
        throw std::runtime_error("failed to make listen socket nonblocking");
    }

    std::printf("dummy_server listening on %s:%u\n", m_config.listen_ip.c_str(), m_config.port);
    std::fflush(stdout);
    return listen_fd;
}

int DummyServer::_openEpollFd() const {
    const int epoll_fd = ::epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd < 0) {
        throw std::runtime_error("failed to create epoll");
    }
    return epoll_fd;
}

void DummyServer::_registerEpoll() {
    m_listen_event.events = EPOLLIN;
    m_listen_event.data.ptr = &m_listen_event_tag;

    m_timer_event.events = EPOLLIN;
    m_timer_event.data.ptr = &m_timer_event_tag;

    if (::epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_listen_fd, &m_listen_event) != 0 ||
        ::epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_timer_fd, &m_timer_event) != 0) {
        throw std::runtime_error("failed to register epoll fd");
    }
}

void DummyServer::_acceptClient() {
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

        ServerSlot& server_slot = _allocateServerSlot(SlotMode::Live);
        const auto now = std::chrono::steady_clock::now();
        m_protocol.activateSlot(static_cast<int>(server_slot.slot_idx), m_next_live_session_id++, now);
        try {
            m_transport.attachClientFd(static_cast<int>(server_slot.slot_idx), client_fd);
            server_slot.event_tag.kind = EpollEventTag::Kind::Connected;
            server_slot.event.events = EPOLLIN | EPOLLRDHUP;
            server_slot.event.data.ptr = &server_slot.event_tag;
            if (::epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, client_fd, &server_slot.event) != 0) {
                throw std::runtime_error("failed to register client fd");
            }
        } catch (const std::runtime_error&) {
            if (m_transport.hasSocket(static_cast<int>(server_slot.slot_idx))) {
                m_transport.closeConnection(static_cast<int>(server_slot.slot_idx));
            } else {
                ::close(client_fd);
            }
            m_protocol.releaseSlot(static_cast<int>(server_slot.slot_idx));
            _releaseSlot(server_slot);
        }
    }
}

bool DummyServer::_flushRuntimeOutbound(int slot_idx) {
    ServerSlot& server_slot = m_server_slots[static_cast<std::size_t>(slot_idx)];
    if (server_slot.mode != SlotMode::Live) {
        return true;
    }

    while (m_protocol.hasOutboundFrame(slot_idx)) {
        OutboundFrameRaw& frame = m_protocol.readFrontOutboundFrame(slot_idx);
        std::size_t bytes_sent = 0;
        const bool send_ok = m_transport.sendBytes(slot_idx,
                                                   frame.payload.data() + static_cast<std::ptrdiff_t>(frame.offset),
                                                   frame.size - frame.offset,
                                                   bytes_sent);
        if (bytes_sent > 0) {
            frame.offset += bytes_sent;
            m_protocol.writeLastSendTime(slot_idx, std::chrono::steady_clock::now());
        }
        if (!send_ok) {
            return false;
        }
        if (frame.offset < frame.size) {
            server_slot.event.events |= EPOLLOUT;
            return ::epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, m_transport.readFd(slot_idx), &server_slot.event) == 0;
        }

        m_protocol.eraseFrontOutboundFrame(slot_idx);
    }

    server_slot.event.events &= ~EPOLLOUT;
    return ::epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, m_transport.readFd(slot_idx), &server_slot.event) == 0;
}

void DummyServer::_closeSlot(int slot_idx) {
    ServerSlot& server_slot = m_server_slots[static_cast<std::size_t>(slot_idx)];
    if (server_slot.mode == SlotMode::Free) {
        return;
    }

    if (server_slot.mode == SlotMode::Live && m_transport.hasSocket(slot_idx)) {
        const int fd = m_transport.readFd(slot_idx);
        if (m_epoll_fd >= 0) {
            (void)::epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
        }
        m_transport.closeConnection(slot_idx);
    }

    m_protocol.releaseSlot(slot_idx);
    _releaseSlot(server_slot);
}

bool DummyServer::_handleTimerEvent() {
    uint64_t expirations = 0;
    const ssize_t timer_read = ::read(m_timer_fd, &expirations, sizeof(expirations));
    if (timer_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        return false;
    }

    // handle timeout
    const auto now = std::chrono::steady_clock::now();
#ifndef VENTURI_STABLE_LINK
    m_protocol.markTimedOutSlots(now, std::chrono::seconds(15));
    while (m_protocol.hasTimedOutSlot()) {
        _closeSlot(m_protocol.readTimedOutSlot());
    }
#endif

    m_protocol.queueHeartbeatFrames(now);
    m_protocol.queueDueFillFrames(now);

    for (ServerSlot& server_slot : m_server_slots) {
        if (server_slot.mode != SlotMode::Live) {
            continue;
        }
        if (!_flushRuntimeOutbound(static_cast<int>(server_slot.slot_idx))) {
#ifdef VENTURI_STABLE_LINK
            continue;
#else
            _closeSlot(static_cast<int>(server_slot.slot_idx));
            continue;
#endif
        }
#ifndef VENTURI_STABLE_LINK
        if (m_protocol.shouldClose(static_cast<int>(server_slot.slot_idx))) {
            _closeSlot(static_cast<int>(server_slot.slot_idx));
        }
#endif
    }
    return true;
}

void DummyServer::_handleConnectedEvent(const epoll_event& event,
                                                const EpollEventTag& event_tag) {
    ServerSlot* p_server_slot = _getLiveSlot(event_tag);
    if (p_server_slot == nullptr) {
        return;
    }
#ifndef VENTURI_STABLE_LINK
    if ((event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) != 0U) {
        _closeSlot(static_cast<int>(p_server_slot->slot_idx));
        return;
    }
#endif

    std::vector<uint8_t> buffer(1024, 0);
    std::size_t bytes_received = 0;
    const bool receive_ok = m_transport.receiveBytes(static_cast<int>(p_server_slot->slot_idx), buffer, bytes_received);
    const auto now = std::chrono::steady_clock::now();
    if (bytes_received > 0) {
        if (!m_protocol.appendBytes(static_cast<int>(p_server_slot->slot_idx),
                                    buffer.data(),
                                    bytes_received,
                                    now)) {
#ifdef VENTURI_STABLE_LINK
            return;
#else
            _closeSlot(static_cast<int>(p_server_slot->slot_idx));
            return;
#endif
        }

        while (true) {
            const std::optional<InboundMessage> message =
                m_protocol.tryReadInboundMessage(static_cast<int>(p_server_slot->slot_idx));
            if (!message.has_value()) {
                break;
            }
            if (!m_protocol.handleInboundMessage(static_cast<int>(p_server_slot->slot_idx), *message, now)) {
#ifdef VENTURI_STABLE_LINK
                return;
#else
                _closeSlot(static_cast<int>(p_server_slot->slot_idx));
                return;
#endif
            }
        }
    }
    if (!receive_ok) {
#ifdef VENTURI_STABLE_LINK
        return;
#else
        _closeSlot(static_cast<int>(p_server_slot->slot_idx));
        return;
#endif
    }
    if (!_flushRuntimeOutbound(static_cast<int>(p_server_slot->slot_idx))) {
#ifdef VENTURI_STABLE_LINK
        return;
#else
        _closeSlot(static_cast<int>(p_server_slot->slot_idx));
        return;
#endif
    }
#ifndef VENTURI_STABLE_LINK
    if (m_protocol.shouldClose(static_cast<int>(p_server_slot->slot_idx))) {
        _closeSlot(static_cast<int>(p_server_slot->slot_idx));
    }
#endif
}

void DummyServer::_shutdownLiveSlots() {
    for (ServerSlot& server_slot : m_server_slots) {
        if (server_slot.mode != SlotMode::Live) {
            continue;
        }
        if (m_transport.hasSocket(static_cast<int>(server_slot.slot_idx))) {
            m_transport.closeConnection(static_cast<int>(server_slot.slot_idx));
        }
        m_protocol.releaseSlot(static_cast<int>(server_slot.slot_idx));
        _releaseSlot(server_slot);
    }
}

void DummyServer::_closeRunFds() {
    if (m_timer_fd >= 0) {
        ::close(m_timer_fd);
        m_timer_fd = -1;
    }
    if (m_epoll_fd >= 0) {
        ::close(m_epoll_fd);
        m_epoll_fd = -1;
    }
    if (m_listen_fd >= 0) {
        ::close(m_listen_fd);
        m_listen_fd = -1;
    }
}

int DummyServer::_runHandle() {
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

bool DummyServer::_handleReadyEvent(const epoll_event& event) {
    auto* event_tag = static_cast<EpollEventTag*>(event.data.ptr);
    if (event_tag == nullptr) {
        return true;
    }
    if (event_tag->kind == EpollEventTag::Kind::Listen) {
        _acceptClient();
        return true;
    }
    if (event_tag->kind == EpollEventTag::Kind::Timer) {
        return _handleTimerEvent();
    }
    // EpollEventTag::Kind::Connected
    _handleConnectedEvent(event, *event_tag);
    return true;
}

int DummyServer::run() {
    m_stop_requested.store(false);
    _resetLiveSlotsBeforeRun();
    try {
        m_listen_fd = _openListenFd();
        m_epoll_fd = _openEpollFd();
        m_timer_fd = _openTimerFd();
        _registerEpoll();
        const int status = _runHandle();
        if (status == 0) {
            _shutdownLiveSlots();
        }
        _closeRunFds();
        return status;
    } catch (...) {
        _closeRunFds();
        throw;
    }
}
