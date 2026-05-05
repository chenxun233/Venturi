#include "tx_connector.h"

#include "tx_receiver.h"
#include "tx_sender.h"
#include "../latency/log_printer.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace {

#ifndef ISO
int connectBlocking(const GatewayClientConfig& config) {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }

    sockaddr_in local_addr {};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(0);
    if (::inet_pton(AF_INET, config.bind_ip.c_str(), &local_addr.sin_addr) != 1) {
        errno = EINVAL;
        ::close(fd);
        return -1;
    }
    if (::bind(fd, reinterpret_cast<sockaddr*>(&local_addr), sizeof(local_addr)) != 0) {
        ::close(fd);
        return -1;
    }

    sockaddr_in remote_addr {};
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(config.port);
    if (::inet_pton(AF_INET, config.server_ip.c_str(), &remote_addr.sin_addr) != 1) {
        errno = EINVAL;
        ::close(fd);
        return -1;
    }
    if (::connect(fd, reinterpret_cast<sockaddr*>(&remote_addr), sizeof(remote_addr)) != 0) {
        ::close(fd);
        return -1;
    }

    return fd;
}
#endif

} // namespace

TxConnector::TxConnector()
    : TxConnector(GatewayClientConfig {}) {}

TxConnector::TxConnector(GatewayClientConfig config)
    : m_config(std::move(config)),
      m_next_connect_attempt_time(std::chrono::steady_clock::now()) {}

TxConnector::~TxConnector() {
    if (m_sender_connection_info.kind == TxConnectionKind::Connected) {
        const TxConnectionInfo sender_info {
            .kind = TxConnectionKind::Disconnected,
            .generation = m_sender_connection_info.generation,
            .fd = -1,
        };
        if (m_sender != nullptr) {
            m_sender->updateConnectionInfo(sender_info);
        }
        m_sender_connection_info = sender_info;
    }
    if (m_receiver_connection_info.kind == TxConnectionKind::Connected) {
        const TxConnectionInfo receiver_info {
            .kind = TxConnectionKind::Disconnected,
            .generation = m_receiver_connection_info.generation,
            .fd = -1,
        };
        if (m_receiver != nullptr) {
            m_receiver->updateConnectionInfo(receiver_info);
        }
        m_receiver_connection_info = receiver_info;
    }
    _closeConnection();
}

void TxConnector::attachLogPrinter(LogPrinter* log_printer) {
    m_log_printer = log_printer;
}

void TxConnector::attachQueueIdx(uint16_t que_idx) {
    m_queue_idx = que_idx;
    m_has_queue_idx = true;
}

void TxConnector::attachSender(TxSender* sender) {
    m_sender = sender;
    if (m_sender != nullptr) {
        m_sender->attachConnection(this);
    }
}

void TxConnector::attachReceiver(TxReceiver* receiver) {
    m_receiver = receiver;
}

bool TxConnector::recvSenderDisconNotice(const TxDisconnectNotice& notice) {
    return m_sender_disconnect_notices.push(notice);
}

bool TxConnector::pollConnect() {
#ifndef ISO
    _drainDisconNotices();
    if (m_socket_fd >= 0) {
        return false;
    }
    if (std::chrono::steady_clock::now() < m_next_connect_attempt_time) {
        return false;
    }
    const int new_fd = connectBlocking(m_config);
    const auto connected_at = std::chrono::steady_clock::now();
    if (new_fd < 0) {
        if (m_socket_fd < 0) {
            m_next_connect_attempt_time = connected_at + m_config.reconnect_delay;
        }
        return true;
    }
    if (m_socket_fd >= 0) {
        ::close(new_fd);
        return true;
    }
    m_socket_fd = new_fd;
    if (!_enableNonBlocking()) {
        _closeConnection();
        m_next_connect_attempt_time = connected_at + m_config.reconnect_delay;
        return true;
    }
    if (!_enableTCP_NODELAY()) {
        _closeConnection();
        m_next_connect_attempt_time = connected_at + m_config.reconnect_delay;
        return true;
    }

    if (!_updateConnectedInfo()) {
        _closeConnection();
        m_next_connect_attempt_time = connected_at + m_config.reconnect_delay;
        return true;
    }
    _logConnectionEstablished();
#endif 
    return true;
}



void TxConnector::_closeConnection() {
#ifndef ISO
    if (m_socket_fd >= 0) {
        (void)::shutdown(m_socket_fd, SHUT_RDWR);
        ::close(m_socket_fd);
        m_socket_fd = -1;
    }
#endif
    return;
}

void TxConnector::_handleDisconnect() {

    _updateDisconInfo(m_socket_generation);
    
    _closeConnection();
    m_next_connect_attempt_time = std::chrono::steady_clock::now() + m_config.reconnect_delay;
}

bool TxConnector::_enableNonBlocking() {
    if (m_socket_fd < 0) {
        return false;
    }

    const int flags = ::fcntl(m_socket_fd, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }

    return ::fcntl(m_socket_fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

bool TxConnector::_enableTCP_NODELAY() {
    if (m_socket_fd < 0) {
        return false;
    }
    int flag = 1;
    return ::setsockopt(m_socket_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == 0;
}

bool TxConnector::_updateConnectedInfo() {
    if (m_socket_fd < 0) {
        return false;
    }

    int sender_fd = -1;
    if (m_sender != nullptr) {
        sender_fd = ::dup(m_socket_fd);
        if (sender_fd < 0) {
            return false;
        }
    }

    if (m_sender_connection_info.kind == TxConnectionKind::Connected &&
        m_sender != nullptr) {
        m_sender->updateConnectionInfo(TxConnectionInfo {
            .kind = TxConnectionKind::Disconnected,
            .generation = m_sender_connection_info.generation,
            .fd = -1,
        });
    }
    if (m_receiver_connection_info.kind == TxConnectionKind::Connected &&
        m_receiver != nullptr) {
        m_receiver->updateConnectionInfo(TxConnectionInfo {
            .kind = TxConnectionKind::Disconnected,
            .generation = m_receiver_connection_info.generation,
            .fd = -1,
        });
    }

    m_socket_generation += 1U;
    if (m_sender != nullptr) {
        m_sender_connection_info = TxConnectionInfo {
            .kind = TxConnectionKind::Connected,
            .generation = m_socket_generation,
            .fd = sender_fd,
        };
    } else {
        m_sender_connection_info = TxConnectionInfo {};
    }
    if (m_receiver != nullptr) {
        m_receiver_connection_info = TxConnectionInfo {
            .kind = TxConnectionKind::Connected,
            .generation = m_socket_generation,
            .fd = -1,
        };
    } else {
        m_receiver_connection_info = TxConnectionInfo {};
    }
    if (m_sender != nullptr) {
        m_sender->updateConnectionInfo(m_sender_connection_info);
    }
    if (m_receiver != nullptr) {
        m_receiver->updateConnectionInfo(m_receiver_connection_info);
    }
    return true;
}


bool TxConnector::_drainDisconNotices() {
    bool did_work = false;
    TxDisconnectNotice notice {};
    while (m_sender_disconnect_notices.pop(notice)) {
        did_work = true;
        if (notice.generation != m_socket_generation) {
            continue;
        }
        _logConnectionLost();
        _handleDisconnect();
    }
    return did_work;
}

void TxConnector::_updateDisconInfo(uint64_t generation) {
    m_sender_connection_info = TxConnectionInfo {
        .kind = TxConnectionKind::Disconnected,
        .generation = generation,
        .fd = -1,
    };
    m_receiver_connection_info = TxConnectionInfo {
        .kind = TxConnectionKind::Disconnected,
        .generation = generation,
        .fd = -1,
    };
    if (m_sender != nullptr) {
        m_sender->updateConnectionInfo(m_sender_connection_info);
    }
    if (m_receiver != nullptr) {
        m_receiver->updateConnectionInfo(m_receiver_connection_info);
    }
}

void TxConnector::_logConnectionEstablished() {
    _pushTxEvent(TxLogRecord {
        .que_idx = m_queue_idx,
        .event = TxEventKind::ConnectionEstablished,
    });
}

void TxConnector::_logConnectionLost() {
    _pushTxEvent(TxLogRecord {
        .que_idx = m_queue_idx,
        .event = TxEventKind::ConnectionLost,
        
    });
}

void TxConnector::_pushTxEvent(const TxLogRecord& record) {
    if (m_log_printer == nullptr || !m_has_queue_idx) {
        return;
    }
    (void)m_log_printer->pushTxLog(record);
}
