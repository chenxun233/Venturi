#pragma once

#include "../common/shared_types.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class TxReceiver {
public:
    explicit TxReceiver(std::size_t pending_capacity = 1024);
    ~TxReceiver();
    TxReceiver(const TxReceiver&) = delete;
    TxReceiver& operator=(const TxReceiver&) = delete;
    TxReceiver(TxReceiver&&) = delete;
    TxReceiver& operator=(TxReceiver&&) = delete;

    void attachQueueIdx(uint16_t queue_idx);
    bool acceptSentOrder(const TxSentOrderRecord& record);
    void updateConnectionInfo(const TxConnectionInfo& info);
    bool pollOnce(int recv_fd = -1);
    TxReceiverStats readStats() const;
    void printSummary() const;

private:
    struct PendingSlot {
        bool occupied {false};
        bool accepted {false};
        bool rejected {false};
        uint32_t filled_shares {0};
        TxSentOrderRecord record {};
    };

    bool _readSocketFrames(int recv_fd);
    bool _appendSocketBytes(int recv_fd);
    bool _parseBufferedFrames();
    bool _parseSoupFrame(const uint8_t* frame, std::size_t frame_size);
    bool _handleSequencedData(const uint8_t* frame, std::size_t frame_size);
    void _handleAccepted(uint32_t user_ref_num);
    void _handleExecuted(uint32_t user_ref_num, uint32_t executed_shares);
    void _handleRejected(uint32_t user_ref_num);
    void _clearPendingSlot(PendingSlot& slot);
    std::size_t _computePendingSlotIndex(uint32_t user_ref_num) const;
    PendingSlot* _lookupPendingSlot(uint32_t user_ref_num);
    const PendingSlot* _lookupPendingSlot(uint32_t user_ref_num) const;
    uint64_t _countPending() const;
    void _markDisconnected();

    uint16_t m_queue_idx {0};
    std::vector<PendingSlot> m_pending_slots;
    std::size_t m_pending_mask {0};
    std::vector<uint8_t> m_frame_buffer;
    TxReceiverStats m_stats {};
    uint64_t m_generation {0};
};
