#pragma once

#include "../common/shared_types.h"
#include "../latency/trace_buffer.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class LatencyLogPrinter;

struct TxTranslatorConfig {
    std::string username {"client"};
    std::string password {"secret"};
    std::string requested_session {};
    std::chrono::seconds heartbeat_interval {1};
    std::size_t intent_capacity {1024};
    std::size_t pending_capacity {1024};
};

class TxTranslator {
public:
    explicit TxTranslator(std::size_t pending_capacity = 1024);
    explicit TxTranslator(TxTranslatorConfig config);

    void attachLogPrinter(LatencyLogPrinter* log_printer);
    bool pushIntent(const OrderIntent& intent);
    bool popOutbound(TxOutboundRecord& record);
    void restoreOutbound(const TxOutboundRecord& record);
    void handleTransportConnected();
    void handleInboundPayload(const std::vector<uint8_t>& payload);
    void handleTransportDisconnect();
    void runTransportMaintenance();

private:
    struct PendingOrderState {
        std::size_t capacity {0};
        std::unordered_map<uint32_t, TxOutboundRecord> records_by_tag {};
        std::vector<uint32_t> ordered_tags {};
    };

    void _drainIntentBuffer();
    bool _buildOrderFrame(const OrderIntent& intent, TxOutboundRecord& record);
    void _queueReadyRecord(const TxOutboundRecord& record);
    void _queueBlockedRecord(const TxOutboundRecord& record);
    void _flushBlockedRecords();
    void _rebuildBlockedRecords();
    void _recordPendingOrder(const TxOutboundRecord& record);
    void _erasePendingOrder(uint32_t tag);
    void _handleAccepted(uint32_t tag,
                         uint16_t stock_locate,
                         uint32_t shares,
                         uint32_t price);
    void _handleExecuted(uint32_t tag,
                         uint32_t executed_shares,
                         uint32_t price,
                         uint64_t match_number);
    void _handleRejected(uint32_t tag, uint16_t reason);
    void _logOrderAccepted(uint32_t tag,
                           uint16_t stock_locate,
                           uint32_t shares,
                           uint32_t price);
    void _logOrderRejected(uint32_t tag, uint16_t reason);
    void _logOrderFilled(uint32_t tag,
                         uint32_t shares,
                         uint32_t price,
                         uint64_t match_number);
    void _logOrderDropped(uint32_t tag);
    void _pushTxEvent(const TxLogRecord& record);
    void _clearReadyRecords();
    void _normalizeReadyRecords();

    TxTranslatorConfig m_config {};
    PendingOrderState m_pending_orders {};
    std::unique_ptr<TraceBuffer<OrderIntent>> m_intent_buffer;
    std::vector<TxOutboundRecord> m_ready_outbound {};
    std::size_t m_ready_head {0};
    std::vector<TxOutboundRecord> m_blocked_outbound {};
    uint32_t m_next_tag {1};
    uint64_t m_next_expected_sequence {1};
    std::string m_active_session {};
    bool m_login_pending {false};
    bool m_session_established {false};
    std::chrono::steady_clock::time_point m_last_send {};
    LatencyLogPrinter* m_log_printer {nullptr};
};
