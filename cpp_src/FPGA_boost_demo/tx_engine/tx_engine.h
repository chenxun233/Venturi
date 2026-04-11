#pragma once

#include "../common/shared_types.h"

#include <chrono>
#include <cstdint>
#include <vector>

struct GatewayClientConfig {
    std::string bind_ip                         {"192.168.51.1"};
    std::string server_ip                       {"192.168.51.2"};
    uint16_t port                               {9000};
    std::chrono::milliseconds reconnect_delay   {250};
    std::chrono::microseconds idle_sleep        {100};
};

class LogPrinter;
class LatencyTracker;

class TxEngine {
public:
    TxEngine();
    explicit TxEngine(GatewayClientConfig config);

    void attachLogPrinter(LogPrinter* log_printer);
    void attachLatenyTracker(LatencyTracker* latency_tracker);
    bool pollConnectStep();
    bool sendOutboundRecord(const TxOutboundRecord& record);
    bool pollInboundFrame(std::vector<uint8_t>& frame);
    bool takeConnectEvent();
    bool takeDisconnectEvent();
    bool isConnected() const;

private:
    bool _connect();
    void _closeConnection();
    void _handleDisconnect(const char* reason);
    bool _sendPayload(const TxOutboundRecord& record);
    void _logConnectionEstablished();
    void _logConnectionLost();
    void _logOrderSent(const TxOutboundRecord& record);
    void _pushTxEvent(const TxLogRecord& record);

    GatewayClientConfig m_config {};
    LogPrinter* m_log_printer {nullptr};
    LatencyTracker* m_latency_tracker {nullptr};
    int m_socket_fd {-1};
    bool m_connect_event_pending {false};
    bool m_disconnect_event_pending {false};
    std::chrono::steady_clock::time_point m_next_connect_attempt_at {};
};
