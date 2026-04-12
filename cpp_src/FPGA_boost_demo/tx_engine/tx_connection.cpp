#include "tx_connection.h"

#include "../latency/log_printer.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace {

int connectBlocking(const GatewayClientConfig& config) {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::printf("Gateway connect failed: socket errno=%d (%s)\n", errno, std::strerror(errno));
        std::fflush(stdout);
        return -1;
    }

    sockaddr_in local_addr {};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(0);
    if (::inet_pton(AF_INET, config.bind_ip.c_str(), &local_addr.sin_addr) != 1) {
        std::printf("Gateway connect failed: invalid bind ip %s\n", config.bind_ip.c_str());
        std::fflush(stdout);
        ::close(fd);
        return -1;
    }
    if (::bind(fd, reinterpret_cast<sockaddr*>(&local_addr), sizeof(local_addr)) != 0) {
        std::printf("Gateway connect failed: bind errno=%d (%s)\n", errno, std::strerror(errno));
        std::fflush(stdout);
        ::close(fd);
        return -1;
    }

    sockaddr_in remote_addr {};
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(config.port);
    if (::inet_pton(AF_INET, config.server_ip.c_str(), &remote_addr.sin_addr) != 1) {
        std::printf("Gateway connect failed: invalid server ip %s\n", config.server_ip.c_str());
        std::fflush(stdout);
        ::close(fd);
        return -1;
    }
    if (::connect(fd, reinterpret_cast<sockaddr*>(&remote_addr), sizeof(remote_addr)) != 0) {
        std::printf("Gateway connect failed: connect errno=%d (%s)\n", errno, std::strerror(errno));
        std::fflush(stdout);
        ::close(fd);
        return -1;
    }

    return fd;
}

} // namespace

TxConnection::TxConnection()
    : TxConnection(GatewayClientConfig {}) {}

TxConnection::TxConnection(GatewayClientConfig config)
    : m_config(std::move(config)),
      m_next_connect_attempt_at(std::chrono::steady_clock::now()) {}

TxConnection::~TxConnection() {
    _closeConnection();
}

void TxConnection::attachLogPrinter(LogPrinter* log_printer) {
    m_log_printer = log_printer;
}

bool TxConnection::pollConnectStep() {
    const auto now = std::chrono::steady_clock::now();
    if (m_socket_fd >= 0 || now < m_next_connect_attempt_at) {
        return false;
    }

    const int fd = connectBlocking(m_config);
    const auto connected_at = std::chrono::steady_clock::now();
    if (fd < 0) {
        if (m_socket_fd < 0) {
            m_next_connect_attempt_at = connected_at + m_config.reconnect_delay;
        }
        return false;
    }

    if (m_socket_fd >= 0) {
        ::close(fd);
        return false;
    }

    m_socket_fd = fd;
    if (!_enableLowLatencySocketOptions()) {
        std::printf("Gateway connect failed: setsockopt TCP_NODELAY errno=%d (%s)\n",
                    errno,
                    std::strerror(errno));
        std::fflush(stdout);
        _closeConnection();
        m_next_connect_attempt_at = connected_at + m_config.reconnect_delay;
        return false;
    }

    if (!_publishConnectedControlForCurrentSocket()) {
        std::printf("Gateway connect failed: dup errno=%d (%s)\n", errno, std::strerror(errno));
        std::fflush(stdout);
        _closeConnection();
        m_next_connect_attempt_at = connected_at + m_config.reconnect_delay;
        return false;
    }

    _logConnectionEstablished();
    return true;
}

bool TxConnection::readInboundFrame(TxInboundFrame& frame) {
    if (m_socket_fd < 0) {
        return false;
    }

    auto tryAssembleFrame = [&]() -> bool {
        if (m_inbound_expected == 0) {
            if (m_inbound_size < 2) {
                return false;
            }
            const uint16_t encoded_length = static_cast<uint16_t>(
                (static_cast<uint16_t>(m_inbound_buffer[0]) << 8) |
                static_cast<uint16_t>(m_inbound_buffer[1]));
            if (encoded_length == 0) {
                _handleDisconnect("invalid inbound frame length");
                return false;
            }
            m_inbound_expected = 2U + static_cast<std::size_t>(encoded_length);
            if (m_inbound_expected > frame.payload.size()) {
                _handleDisconnect("inbound frame exceeds receiver buffer");
                return false;
            }
        }

        if (m_inbound_size < m_inbound_expected) {
            return false;
        }

        std::memcpy(frame.payload.data(), m_inbound_buffer.data(), m_inbound_expected);
        frame.payload_length = static_cast<uint8_t>(m_inbound_expected);

        const std::size_t remaining = m_inbound_size - m_inbound_expected;
        if (remaining > 0) {
            std::memmove(m_inbound_buffer.data(),
                         m_inbound_buffer.data() + static_cast<std::ptrdiff_t>(m_inbound_expected),
                         remaining);
        }
        m_inbound_size = remaining;
        m_inbound_expected = 0;
        return true;
    };

    if (tryAssembleFrame()) {
        return true;
    }

    while (m_inbound_size < m_inbound_buffer.size()) {
        const ssize_t count = ::recv(m_socket_fd,
                                     m_inbound_buffer.data() +
                                         static_cast<std::ptrdiff_t>(m_inbound_size),
                                     m_inbound_buffer.size() - m_inbound_size,
                                     MSG_DONTWAIT);
        if (count > 0) {
            m_inbound_size += static_cast<std::size_t>(count);
            if (tryAssembleFrame()) {
                return true;
            }
            continue;
        }
        if (count == 0) {
            _handleDisconnect("server closed the client");
            return false;
        }
        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;
        }
        _handleDisconnect("server closed the client");
        return false;
    }

    return false;
}

bool TxConnection::takeTransportControl(TxTransportControl& control) {
    if (!m_has_pending_transport_control) {
        return false;
    }
    control = m_pending_transport_control;
    m_has_pending_transport_control = false;
    return true;
}

bool TxConnection::isConnected() const {
    return m_socket_fd >= 0;
}

void TxConnection::_closeConnection() {
    if (m_socket_fd >= 0) {
        (void)::shutdown(m_socket_fd, SHUT_RDWR);
        ::close(m_socket_fd);
        m_socket_fd = -1;
    }
}

void TxConnection::_handleDisconnect(const char* reason) {
    const bool had_connection = (m_socket_fd >= 0);
    if (had_connection) {
        _publishDisconnectedControl(m_generation);
    }
    _closeConnection();
    m_next_connect_attempt_at = std::chrono::steady_clock::now() + m_config.reconnect_delay;
    m_inbound_size = 0;
    m_inbound_expected = 0;
    if (reason != nullptr && reason[0] != '\0') {
        std::printf("Gateway issue: %s\n", reason);
        std::fflush(stdout);
    }
    if (had_connection) {
        _logConnectionLost();
    }
}

bool TxConnection::_enableLowLatencySocketOptions() {
    if (m_socket_fd < 0) {
        return false;
    }
    int flag = 1;
    return ::setsockopt(m_socket_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == 0;
}

bool TxConnection::_publishConnectedControlForCurrentSocket() {
    if (m_socket_fd < 0) {
        return false;
    }

    const int tx_fd = ::dup(m_socket_fd);
    if (tx_fd < 0) {
        return false;
    }

    if (m_has_pending_transport_control &&
        m_pending_transport_control.kind == TxTransportControlKind::Connected &&
        m_pending_transport_control.tx_fd >= 0) {
        ::close(m_pending_transport_control.tx_fd);
    }

    m_generation += 1U;
    m_pending_transport_control = TxTransportControl {
        .kind = TxTransportControlKind::Connected,
        .generation = m_generation,
        .tx_fd = tx_fd,
    };
    m_has_pending_transport_control = true;
    return true;
}

void TxConnection::_publishDisconnectedControl(uint64_t generation) {
    if (m_has_pending_transport_control &&
        m_pending_transport_control.kind == TxTransportControlKind::Connected &&
        m_pending_transport_control.tx_fd >= 0) {
        ::close(m_pending_transport_control.tx_fd);
    }

    m_pending_transport_control = TxTransportControl {
        .kind = TxTransportControlKind::Disconnected,
        .generation = generation,
        .tx_fd = -1,
    };
    m_has_pending_transport_control = true;
}

void TxConnection::_logConnectionEstablished() {
    _pushTxEvent(TxLogRecord {
        .event = TxEventKind::ConnectionEstablished
    });
}

void TxConnection::_logConnectionLost() {
    _pushTxEvent(TxLogRecord {
        .event = TxEventKind::ConnectionLost
    });
}

void TxConnection::_pushTxEvent(const TxLogRecord& record) {
    if (m_log_printer != nullptr) {
        (void)m_log_printer->pushTxEvent(record);
    }
}
