#pragma once

#include "tx_engine.h"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>

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
    bool pollConnectStep();
    bool readInboundFrame(TxInboundFrame& frame);
    bool takeTransportControl(TxTransportControl& control);
    bool isConnected() const;

private:
    bool _enableLowLatencySocketOptions();
    bool _publishConnectedControlForCurrentSocket();
    void _publishDisconnectedControl(uint64_t generation);
    void _closeConnection();
    void _handleDisconnect(const char* reason);
    void _logConnectionEstablished();
    void _logConnectionLost();
    void _pushTxEvent(const TxLogRecord& record);

    GatewayClientConfig m_config {};
    LogPrinter* m_log_printer {nullptr};
    int m_socket_fd {-1};
    uint64_t m_generation {0};
    bool m_has_pending_transport_control {false};
    TxTransportControl m_pending_transport_control {};
    std::chrono::steady_clock::time_point m_next_connect_attempt_at {};
    std::array<uint8_t, 67> m_inbound_buffer {};
    std::size_t m_inbound_size {0};
    std::size_t m_inbound_expected {0};
};
