#pragma once

#include "../common/shared_types.h"
#include "../latency/trace_buffer.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class LogPrinter;
class LatencyTracker;

struct TxSenderConfig {
    std::string username {"client"};
    std::string password {"secret"};
    std::string requested_session {};
    std::chrono::steady_clock::duration heartbeat_interval {std::chrono::seconds(1)};
    std::size_t intent_capacity {1024};
    std::size_t pending_capacity {1024};
    std::size_t inbound_capacity {1024};
    std::size_t transport_capacity {1024};
};

class TxSender {
public:
    explicit TxSender(std::size_t pending_capacity = 1024);
    explicit TxSender(TxSenderConfig config);

    void attachLogPrinter(LogPrinter* log_printer);
    void attachLatenyTracker(LatencyTracker* latency_tracker);

    bool acceptIntent(const OrderIntent& intent);
    // acceptInboundFrame(), acceptTransportEvent(), and acceptTransportControl() all feed one
    // shared SPSC ingress queue. Callers must serialize all three APIs on the same producer
    // thread (TxReceiver thread in split mode).
    bool acceptInboundFrame(const TxInboundFrame& frame);
    bool acceptTransportEvent(TxTransportEvent event);
    bool acceptTransportControl(const TxTransportControl& control);

    // Drains sender-owned inbound queue in strict FIFO arrival order.
    bool processInboundQueues();

    bool popReadyOutbound(TxOutboundRecord& record);
    void restoreReadyOutbound(const TxOutboundRecord& record);
    void noteOutboundSent(const TxOutboundRecord& record);

    void login();
    void onTransportDisconnected();
    bool queueHeartbeatIfDue();
    bool buildOutboundFrame();

private:
    struct PendingOrderState {
        std::size_t capacity {0};
        std::unordered_map<uint32_t, TxOutboundRecord> order_records {};
        std::vector<uint32_t> ordered_tags {};
    };

    void _assertIngressProducerThread();
    void _acceptInboundFramePayload(const TxInboundFrame& frame);
    bool _buildOrderFrame(const OrderIntent& intent, TxOutboundRecord& record);
    void _queueReadyRecord(const TxOutboundRecord& record);
    void _queueBlockedRecord(const TxOutboundRecord& record);
    void _flushBlockedRecords();
    void _rebuildBlockedRecords();
    void _dropQueuedRecordByTag(uint32_t user_ref_num);
    void _recordPendingOrder(const TxOutboundRecord& record);
    void _erasePendingOrder(uint32_t user_ref_num);
    void _handleAccepted(uint32_t user_ref_num,
                         uint32_t shares,
                         uint32_t price);
    void _handleExecuted(uint32_t user_ref_num,
                         uint32_t executed_shares,
                         uint32_t price,
                         uint64_t match_number);
    void _handleRejected(uint32_t user_ref_num, uint16_t reason);
    void _logOrderAccepted(uint32_t user_ref_num,
                           uint16_t stock_locate,
                           uint32_t shares,
                           uint32_t price);
    void _logOrderRejected(uint32_t user_ref_num, uint16_t reason);
    void _logOrderFilled(uint32_t user_ref_num,
                         uint32_t shares,
                         uint32_t price,
                         uint64_t match_number);
    void _logOrderDropped(uint32_t user_ref_num);
    void _pushTxEvent(const TxLogRecord& record);
    void _clearReadyRecords();
    void _normalizeReadyRecords();

    TxSenderConfig m_config {};
    std::atomic<std::uintptr_t> m_ingress_producer_thread_token {0};
    PendingOrderState m_pending_orders {};
    TraceBuffer<OrderIntent> m_intent_buffer;
    TraceBuffer<TxSenderInboundRecord> m_inbound_records;
    std::vector<TxOutboundRecord> m_ready_outbound {};
    std::size_t m_ready_head {0};
    std::vector<TxOutboundRecord> m_blocked_outbound {};
    uint32_t m_next_tag {1};
    uint64_t m_next_expected_sequence {1};
    std::string m_active_session {};
    bool m_login_pending {false};
    bool m_logged_in {false};
    std::chrono::steady_clock::time_point m_last_successful_send {};
    std::size_t m_heartbeat_ready_count {0};
    LogPrinter* m_log_printer {nullptr};
    LatencyTracker* m_latency_tracker {nullptr};
};
