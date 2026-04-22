#pragma once

#include "../common/shared_types.h"
#include "../common/spsc_ring_queue.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>

struct GatewayClientConfig {
    std::string bind_ip                         {"192.168.51.1"};
    std::string server_ip                       {"192.168.51.2"};
    uint16_t port                               {9000};
    std::chrono::milliseconds reconnect_delay   {250};
    std::chrono::microseconds idle_sleep        {100};
};

class TxSender;
class LogPrinter;

class TxConnection {
public:
    TxConnection();
    explicit TxConnection(GatewayClientConfig config);
    ~TxConnection();

    TxConnection(const TxConnection&) = delete;
    TxConnection& operator=(const TxConnection&) = delete;
    TxConnection(TxConnection&&) = delete;
    TxConnection& operator=(TxConnection&&) = delete;

    void attachLogPrinter(LogPrinter* log_printer);
    void attachQueueIdx(uint16_t queue_idx);
    void attachSender(TxSender* sender);
    bool pollConnect();
    bool pushSenderDisconNotice(const TxDisconnectNotice& notice);
    bool isConnected() const;


private:
    bool _enableNonBlocking();
    bool _enableTCP_NODELAY();
    bool _updateConnectedInfo();
    bool _drainDisconNotices();
    void _updateDisconInfo(uint64_t generation);
    void _closeConnection();
    void _handleDisconnect();
    void _logConnectionEstablished();
    void _logConnectionLost();
    void _pushTxEvent(const TxLogRecord& record);

    GatewayClientConfig m_config {};
    LogPrinter* m_log_printer {nullptr};
    uint16_t m_queue_idx {0};
    bool m_has_queue_idx {false};
    TxSender* m_sender {nullptr};
    SpscRingQueue<TxDisconnectNotice> m_sender_disconnect_notices{8};
    int m_socket_fd {-1};
    uint64_t m_socket_generation {0};
    TxConnectionInfo m_sender_connection_info {};
    std::chrono::steady_clock::time_point m_next_connect_attempt_time {};
};
