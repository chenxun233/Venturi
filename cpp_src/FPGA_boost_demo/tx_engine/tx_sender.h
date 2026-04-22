#pragma once

#include "../common/fixed_circular_buffer.h"
#include "../common/shared_types.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class LatencyTracker;
class TxConnection;
class LogPrinter;

struct TxSenderConfig {
    std::string username {"client"};
    std::string password {"secret"};
    std::string requested_session {};
    std::chrono::steady_clock::duration heartbeat_interval {std::chrono::seconds(1)};
    std::size_t intent_capacity {1024};
    std::size_t pending_capacity {1024};
    std::size_t pending_slot_count {65536};
    std::size_t transport_capacity {1024};
};

class TxSender {
public:
    explicit TxSender(std::size_t pending_capacity = 1024);
    explicit TxSender(TxSenderConfig config);
    ~TxSender();

    void attachLogPrinter(LogPrinter* log_printer);
    void attachLatenyTracker(LatencyTracker* latency_tracker);
    void attachConnection(TxConnection* connection);

    bool acceptExecution(const OrderExecution& execution) noexcept;
    void updateConnectionInfo(const TxConnectionInfo& info);
    bool runOnce();

    bool popReadyOutbound(TxOutboundRecord& record);
    void restoreReadyOutbound(const TxOutboundRecord& record);
    bool trySendOutbound(const TxOutboundRecord& record);
    void noteOutboundSent(const TxOutboundRecord& record);

    void login();
    void onTransportDisconnected();
    bool queueHeartbeat();
    bool buildOutboundFrames();

private:
    struct PendingSlot {
        bool occupied {false};
        TxOutboundRecord record {};
    };

    struct PendingOrderState {
        std::size_t capacity {0};
        std::size_t live_count {0};
        std::vector<PendingSlot> slots {};
    };

    struct PendingLatencyCommandState {
        bool occupied {false};
        TraceCommand command {};
    };

    static constexpr std::size_t kExecutionBufferCapacity = 1024;

    void _updateConnectionInfo(const TxConnectionInfo& info);
    void _retireGeneration(uint64_t generation);
    bool _sendPayload(const TxOutboundRecord& record);
    void _closeSendFd(bool clear_generation = true);
    bool _buildOrderFrame(const OrderExecution& execution, TxOutboundRecord& record);
    void _queueReadyRecord(const TxOutboundRecord& record);
    void _queueBlockedRecord(const TxOutboundRecord& record);
    void _flushBlockedRecords();
    void _rebuildBlockedRecords();
    std::size_t _computePendingSlotIndex(uint32_t user_ref_num) const noexcept;
    PendingSlot* _lookupPendingSlot(uint32_t user_ref_num) noexcept;
    const PendingSlot* _lookupPendingSlot(uint32_t user_ref_num) const noexcept;
    void _clearPendingSlot(PendingSlot& slot) noexcept;
    bool _recordPendingOrder(const TxOutboundRecord& record);
    void _erasePendingOrder(uint32_t user_ref_num);
    void _handleAccepted(uint32_t user_ref_num,
                         uint32_t shares,
                         uint32_t price);
    void _handleExecuted(uint32_t user_ref_num,
                         uint32_t executed_shares,
                         uint32_t price,
                         uint64_t match_number);
    void _handleRejected(uint32_t user_ref_num, uint16_t reason);

    void _pushTxEvent(uint16_t queue_idx, const TxLogRecord& record);
    void _clearReadyRecords();
    void _normalizeReadyRecords();
    bool _flushPendingLatencyCommand() noexcept;
    bool _requestLatencyCommand(uint16_t que_idx,
                                TraceCommandOp op,
                                uint32_t trace_id) noexcept;

    TxSenderConfig m_config {};
    PendingOrderState m_pending_orders {};
    PendingLatencyCommandState m_pending_latency_command {};
    FixedCircularBuffer<OrderExecution, kExecutionBufferCapacity> m_execution_buffer;
    std::vector<TxOutboundRecord> m_ready_outbound {};
    std::size_t m_ready_head {0};
    std::vector<TxOutboundRecord> m_blocked_outbound {};
    uint32_t m_next_tag {1};
    uint64_t m_next_expected_sequence {1};
    uint64_t m_transport_generation {0};
    int m_send_fd {-1};
    std::string m_active_session {};
    bool m_login_pending {false};
    bool m_logged_in {false};
    std::chrono::steady_clock::time_point m_last_successful_send {};
    std::size_t m_heartbeat_ready_count {0};
    TxConnection* m_connection {nullptr};
    LogPrinter* m_log_printer {nullptr};
    LatencyTracker* m_latency_tracker {nullptr};
};
