#pragma once

#include "../common/fixed_circular_buffer.h"
#include "../common/shared_types.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class LatencyTracker;
class TxConnector;
class TxReceiver;

struct TxSenderConfig {
    std::string username {"client"};
    std::string password {"secret"};
    std::string requested_session {};
    std::chrono::steady_clock::duration heartbeat_interval {std::chrono::seconds(1)};
    std::size_t pending_capacity {1024};
};

class TxSender {
public:
    explicit TxSender(std::size_t pending_capacity = 1024);
    explicit TxSender(TxSenderConfig config);
    ~TxSender();

    void attachLatencyTracker(LatencyTracker* latency_tracker);
    void attachConnection(TxConnector* connection);
    bool acceptExecution(const OrderExecution& execution) noexcept;

    bool runOnce();
    int readSendFd() const noexcept;

    void login();



private:
    friend class TxConnector;
    void updateConnectionInfo(const TxConnectionInfo& info);
    bool _queueHeartbeat();
    bool _queueOutFrames();
    bool _popReadyOutbound(TxOutboundRecord& record);
    void _onTransportDisconnected();

    struct PendingSlot {
        bool occupied {false};
        TxOutboundRecord record {};
    };

    struct PendingOrderState {
        std::size_t capacity {0};
        std::size_t live_count {0};
        std::vector<PendingSlot> slots {};
    };



    static constexpr std::size_t kExecutionBufferCapacity = 1024;

    void _updateConnectionInfo(const TxConnectionInfo& info);
    void _retireGeneration(uint64_t generation);
    bool _sendPayload(const TxOutboundRecord& record);
    void _closeSendFd(bool clear_generation = true);
    bool _buildOrderFrame(const OrderExecution& execution, TxOutboundRecord& record);
    void _queueReadyRecord(const TxOutboundRecord& record);
    std::size_t _computePendingSlotIndex(uint32_t user_ref_num) const noexcept;
    PendingSlot* _lookupPendingSlot(uint32_t user_ref_num) noexcept;
    const PendingSlot* _lookupPendingSlot(uint32_t user_ref_num) const noexcept;
    void _clearPendingSlot(PendingSlot& slot) noexcept;
    bool _recordPendingOrder(const TxOutboundRecord& record);
    void _erasePendingOrder(uint32_t user_ref_num);





    TxSenderConfig m_config {};
    PendingOrderState m_pending_orders {};
    RingBuffer<OrderExecution, kExecutionBufferCapacity> m_execution_buffer;
    RingBuffer<TxOutboundRecord, 8192> m_ready_outbound {};
    uint32_t m_next_tag {1};
    uint64_t m_next_expected_sequence {1};
    uint64_t m_transport_generation {0};
    int m_send_fd {-1};
    std::string m_active_session {};
    bool m_login_pending {false};
    bool m_logged_in {false};
    std::chrono::steady_clock::time_point m_last_successful_send {};
    std::size_t m_heartbeat_ready_count {0};
    TxConnector* p_connection {nullptr};
    TxReceiver* p_receiver {nullptr};
    LatencyTracker* p_latency_tracker {nullptr};
};
