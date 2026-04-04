#include "tx_engine.h"

#include "../latency/latency_log_printer.h"

#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {

bool sendAll(int fd, const uint8_t* data, std::size_t size) {
    std::size_t offset = 0;
    while (offset < size) {
        const ssize_t written = ::send(fd,
                                       data + static_cast<std::ptrdiff_t>(offset),
                                       size - offset,
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

bool readFrameBlocking(int fd, std::vector<uint8_t>& frame) {
    std::array<uint8_t, 2> header {};
    if (!readExact(fd, header.data(), header.size())) {
        return false;
    }

    const uint16_t encoded_length = static_cast<uint16_t>((static_cast<uint16_t>(header[0]) << 8) |
                                                          static_cast<uint16_t>(header[1]));
    if (encoded_length == 0) {
        return false;
    }

    frame.assign(header.begin(), header.end());
    const std::size_t payload_size = encoded_length;
    frame.resize(header.size() + payload_size);
    return readExact(fd,
                     frame.data() + static_cast<std::ptrdiff_t>(header.size()),
                     payload_size);
}

enum class FrameReadStatus : uint8_t {
    NoData,
    Frame,
    Disconnected,
};

FrameReadStatus readFrameNonBlocking(int fd, std::vector<uint8_t>& frame) {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);

    timeval timeout {};
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    const int ready = ::select(fd + 1, &read_set, nullptr, nullptr, &timeout);
    if (ready < 0) {
        return FrameReadStatus::Disconnected;
    }
    if (ready == 0 || !FD_ISSET(fd, &read_set)) {
        return FrameReadStatus::NoData;
    }

    if (!readFrameBlocking(fd, frame)) {
        return FrameReadStatus::Disconnected;
    }
    return FrameReadStatus::Frame;
}

} // namespace

TxEngine::TxEngine()
    : TxEngine(GatewayClientConfig {}) {}

TxEngine::TxEngine(GatewayClientConfig config)
    : m_config(std::move(config)),
      m_outbound_buffer(std::make_unique<TraceBuffer<TxOutboundRecord>>(m_config.outbound_buffer_capacity)),
      m_next_connect_attempt_at(std::chrono::steady_clock::now()) {}

void TxEngine::attachLogPrinter(LatencyLogPrinter* log_printer) {
    m_log_printer = log_printer;
}

bool TxEngine::pushPayload(const TxOutboundRecord& record) {
    if (m_outbound_buffer == nullptr) {
        return false;
    }

    return m_outbound_buffer->push(record);
}

bool TxEngine::runTransportStep() {
    bool did_work = false;
    const auto now = std::chrono::steady_clock::now();

    if (m_socket_fd < 0 && now >= m_next_connect_attempt_at) {
        if (_connect()) {
            did_work = true;
        } else {
            m_next_connect_attempt_at = now + m_config.reconnect_delay;
        }
    }

    if (m_socket_fd >= 0) {
        did_work = _drainOutboundBuffer() || did_work;
        did_work = _pollInboundPayloads() || did_work;
    }

    return did_work;
}

std::vector<std::vector<uint8_t>> TxEngine::drainInboundPayloads() {
    std::vector<std::vector<uint8_t>> inbound = std::move(m_inbound_payloads);
    m_inbound_payloads.clear();
    return inbound;
}

bool TxEngine::takeConnectEvent() {
    const bool had_connect = m_connect_event_pending;
    m_connect_event_pending = false;
    return had_connect;
}

bool TxEngine::takeDisconnectEvent() {
    const bool had_disconnect = m_disconnect_event_pending;
    m_disconnect_event_pending = false;
    return had_disconnect;
}

bool TxEngine::isConnected() const {
    return m_socket_fd >= 0;
}

bool TxEngine::_connect() {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::printf("Gateway connect failed: socket errno=%d (%s)\n", errno, std::strerror(errno));
        std::fflush(stdout);
        return false;
    }

    sockaddr_in local_addr {};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(0);
    if (::inet_pton(AF_INET, m_config.bind_ip.c_str(), &local_addr.sin_addr) != 1) {
        std::printf("Gateway connect failed: invalid bind ip %s\n", m_config.bind_ip.c_str());
        std::fflush(stdout);
        ::close(fd);
        return false;
    }
    if (::bind(fd, reinterpret_cast<sockaddr*>(&local_addr), sizeof(local_addr)) != 0) {
        std::printf("Gateway connect failed: bind errno=%d (%s)\n", errno, std::strerror(errno));
        std::fflush(stdout);
        ::close(fd);
        return false;
    }

    sockaddr_in remote_addr {};
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(m_config.port);
    if (::inet_pton(AF_INET, m_config.server_ip.c_str(), &remote_addr.sin_addr) != 1) {
        std::printf("Gateway connect failed: invalid server ip %s\n", m_config.server_ip.c_str());
        std::fflush(stdout);
        ::close(fd);
        return false;
    }
    if (::connect(fd, reinterpret_cast<sockaddr*>(&remote_addr), sizeof(remote_addr)) != 0) {
        std::printf("Gateway connect failed: connect errno=%d (%s)\n", errno, std::strerror(errno));
        std::fflush(stdout);
        ::close(fd);
        return false;
    }

    m_socket_fd = fd;
    m_connect_event_pending = true;
    m_disconnect_event_pending = false;
    _logConnectionEstablished();
    return true;
}

void TxEngine::_closeSocket() {
    if (m_socket_fd >= 0) {
        ::close(m_socket_fd);
        m_socket_fd = -1;
    }
}

void TxEngine::_handleDisconnect(const char* reason) {
    const bool had_connection = (m_socket_fd >= 0);
    _closeSocket();
    m_next_connect_attempt_at = std::chrono::steady_clock::now() + m_config.reconnect_delay;
    m_disconnect_event_pending = had_connection;
    if (reason != nullptr && reason[0] != '\0') {
        std::printf("Gateway issue: %s\n", reason);
        std::fflush(stdout);
    }
    if (had_connection) {
        _logConnectionLost();
    }
}

bool TxEngine::_sendPayload(const TxOutboundRecord& record) {
    if (m_socket_fd < 0) {
        return false;
    }
    if (record.payload_length == 0 || record.payload_length > record.payload.size()) {
        return false;
    }

    return sendAll(m_socket_fd,
                   record.payload.data(),
                   static_cast<std::size_t>(record.payload_length));
}

bool TxEngine::_drainOutboundBuffer() {
    if (m_outbound_buffer == nullptr || m_socket_fd < 0) {
        return false;
    }

    bool did_work = false;
    TxOutboundRecord record {};
    while (m_outbound_buffer->pop(record)) {
        did_work = true;
        if (_sendPayload(record)) {
            if (record.tag != 0) {
                _logOrderSent(record);
            }
            continue;
        }

        _handleDisconnect("transport send failed");
        break;
    }
    return did_work;
}

bool TxEngine::_pollInboundPayloads() {
    if (m_socket_fd < 0) {
        return false;
    }

    bool did_work = false;
    std::vector<uint8_t> frame {};
    while (true) {
        const FrameReadStatus status = readFrameNonBlocking(m_socket_fd, frame);
        if (status == FrameReadStatus::NoData) {
            return did_work;
        }
        if (status == FrameReadStatus::Disconnected) {
            _handleDisconnect("server closed the session");
            return true;
        }

        did_work = true;
        m_inbound_payloads.push_back(frame);
    }
}

void TxEngine::_logConnectionEstablished() {
    _pushTxEvent(TxLogRecord {
        .event = TxEventKind::ConnectionEstablished
    });
}

void TxEngine::_logConnectionLost() {
    _pushTxEvent(TxLogRecord {
        .event = TxEventKind::ConnectionLost
    });
}

void TxEngine::_logOrderSent(const TxOutboundRecord& record) {
    _pushTxEvent(TxLogRecord {
        .event = TxEventKind::OrderSent,
        .tag = record.tag,
        .stock_locate = record.stock_locate,
        .price = record.price,
        .shares = record.shares
    });
}

void TxEngine::_pushTxEvent(const TxLogRecord& record) {
    if (m_log_printer != nullptr) {
        (void)m_log_printer->pushTxEvent(record);
    }
}
