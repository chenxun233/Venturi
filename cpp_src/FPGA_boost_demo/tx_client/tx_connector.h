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
};

class TxSender;
class TxReceiver;
class LogPrinter;

class TxConnector {
public:
    TxConnector();
    explicit TxConnector(GatewayClientConfig config);
    ~TxConnector();

    TxConnector(const TxConnector&) = delete;
    TxConnector& operator=(const TxConnector&) = delete;
    TxConnector(TxConnector&&) = delete;
    TxConnector& operator=(TxConnector&&) = delete;

    void attachLogPrinter(LogPrinter* log_printer);
    void attachQueueIdx(uint16_t que_idx);
    void attachSender(TxSender* sender);
    void attachReceiver(TxReceiver* receiver);
    bool pollConnect();
    bool recvSenderDisconNotice(const TxDisconnectNotice& notice);


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
    TxReceiver* m_receiver {nullptr};
    SpscRingBuffer<TxDisconnectNotice> m_sender_disconnect_notices{8};
    int m_socket_fd {-1};
    uint64_t m_socket_generation {0};
    TxConnectionInfo m_sender_connection_info {};
    TxConnectionInfo m_receiver_connection_info {};
    std::chrono::steady_clock::time_point m_next_connect_attempt_time {};
};
