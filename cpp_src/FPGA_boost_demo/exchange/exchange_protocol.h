#pragma once

#include "../common/fixed_circular_buffer.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

constexpr std::size_t kOuchExecutedSize       = 36;
constexpr std::size_t kMaxSoupPayloadSize     = 64;
constexpr std::size_t kMaxSoupFrameSize       = 3 + kMaxSoupPayloadSize;
constexpr std::size_t kMaxOutboundMsg         = 1;
constexpr std::size_t kReadBufferSize         = 4096;
constexpr std::size_t kMaxPendingFills        = 1024;
constexpr std::size_t kOutboundQueueSize      = 1024;

enum class ExchangeValidationKind : uint8_t {
    Accepted,
    Rejected,
};

struct ExchangeValidationResult {
    ExchangeValidationKind kind {ExchangeValidationKind::Accepted};
    uint16_t reject_reason {0};
};

struct ExchangeEnterOrder {
    uint32_t user_ref_num {0};
    uint16_t stock_locate {0};
    uint32_t shares {0};
    uint32_t price {0};
    char side {'B'};
};

struct ProtocolConfig {
    std::string username                    {"client"};
    std::string password                    {"secret"};
    std::string session_id                  {"SESSION01"};
    uint32_t price_min                      {1};
    uint32_t price_max                      {5'000'000};
    uint32_t max_shares                     {1'000'000};
    std::chrono::milliseconds fill_delay    {5};
    std::size_t replay_capacity             {256};
};

struct OutboundFrameRaw {
    std::array<uint8_t, kMaxSoupFrameSize> payload {};
    std::size_t size {0};
    std::size_t offset {0};
};

struct LoginRequest {
    std::string username {};
    std::string password {};
    std::string requested_session {};
    uint64_t requested_sequence {1};
};

enum class InboundMessageKind : uint8_t {
    Invalid,
    LoginRequest,
    ClientHeartbeat,
    LogoutRequest,
    EnterOrder,
};

struct InboundMessage {
    InboundMessageKind kind {InboundMessageKind::Invalid};
    LoginRequest login {};
    ExchangeEnterOrder order {};
};

struct HandledOrderResult {
    bool is_duplicate {false};
    ExchangeValidationResult validation {};
    std::array<OutboundFrameRaw, kMaxOutboundMsg> outbound_messages {};
    std::size_t outbound_message_count {0};
};

class ProtocolSession {
public:
    explicit ProtocolSession(ProtocolConfig config);
    void reset(uint64_t session_id, std::chrono::steady_clock::time_point now);
    bool appendBytes(const uint8_t* bytes, std::size_t size, std::chrono::steady_clock::time_point now);
    std::optional<InboundMessage> tryReadInboundMessage();
    bool handleInboundMessage(const InboundMessage& message, std::chrono::steady_clock::time_point now);
    bool parseBytes(std::chrono::steady_clock::time_point now);
    bool shouldClose() const;
    bool hasTimedOut(std::chrono::steady_clock::time_point now, std::chrono::seconds timeout) const;
    void queueHeartbeatFrames(std::chrono::steady_clock::time_point now);
    void queueDueFillFrames(std::chrono::steady_clock::time_point now);
    uint64_t readSessionId() const;
    bool hasOutboundFrame() const;
    const OutboundFrameRaw& readFrontOutboundFrame() const;
    OutboundFrameRaw& readFrontOutboundFrame();
    void eraseFrontOutboundFrame();
    void writeLastSendTime(std::chrono::steady_clock::time_point now);

private:
    struct PendingFill {
        uint32_t user_ref_num {0};
        uint32_t executed_shares {0};
        uint32_t price {0};
        uint64_t match_number {0};
        std::chrono::steady_clock::time_point due_time {};
    };

    struct ReplayEntry {
        bool is_occupied {false};
        uint32_t user_ref_num {0};
        HandledOrderResult result {};
    };

   
    bool _insertReplayResult(uint32_t user_ref_num, const HandledOrderResult& result);
    std::optional<HandledOrderResult> _findReplayResult(uint32_t user_ref_num) const;
    HandledOrderResult _buildReplayCapacityReject(uint32_t user_ref_num);
    HandledOrderResult _handleEnterOrder(const ExchangeEnterOrder& order, std::chrono::steady_clock::time_point now);
    ExchangeValidationResult _validateEnterOrder(const ExchangeEnterOrder& order) const;
    uint64_t _readTimestampNs(std::chrono::steady_clock::time_point now) const;

private:
    ProtocolConfig m_config {};
    uint64_t m_session_id {0};
    bool m_is_logged_in {false};
    bool m_should_close {false};
    uint64_t m_next_sequence {1};
    uint64_t m_next_order_ref_num {1};
    uint64_t m_next_match_number {1};
    std::chrono::steady_clock::time_point m_last_send_time {};
    std::chrono::steady_clock::time_point m_last_receive_time {};
    FixedCircularBuffer<uint8_t, kReadBufferSize> m_inbound_bytes {};
    FixedCircularBuffer<OutboundFrameRaw, kOutboundQueueSize> m_outbound_frame_que {};
    FixedCircularBuffer<PendingFill, kMaxPendingFills> m_pending_fills {};
    std::vector<ReplayEntry> m_replay_entries {};
    std::size_t m_replay_count {0};
};

class ExchangeProtocol {
public:
    ExchangeProtocol(ProtocolConfig config, std::size_t slot_capacity);

    void activateSlot(int slot_idx, uint64_t session_id, std::chrono::steady_clock::time_point now);
    void releaseSlot(int slot_idx);
    bool appendBytes(int slot_idx, const uint8_t* bytes, std::size_t size, std::chrono::steady_clock::time_point now);
    std::optional<InboundMessage> tryReadInboundMessage(int slot_idx);
    bool handleInboundMessage(int slot_idx, const InboundMessage& message, std::chrono::steady_clock::time_point now);
    bool parseBytes(int slot_idx, std::chrono::steady_clock::time_point now);
    bool shouldClose(int slot_idx) const;
    void markTimedOutSlots(std::chrono::steady_clock::time_point now, std::chrono::seconds timeout);
    bool hasTimedOutSlot() const;
    int readTimedOutSlot();
    void queueHeartbeatFrames(std::chrono::steady_clock::time_point now);
    void queueDueFillFrames(std::chrono::steady_clock::time_point now);
    uint64_t readSessionId(int slot_idx) const;
    bool hasOutboundFrame(int slot_idx) const;
    OutboundFrameRaw& readFrontOutboundFrame(int slot_idx);
    const OutboundFrameRaw& readFrontOutboundFrame(int slot_idx) const;
    void eraseFrontOutboundFrame(int slot_idx);
    void writeLastSendTime(int slot_idx, std::chrono::steady_clock::time_point now);

private:
    struct ProtocolSlot {
        bool is_active {false};
        ProtocolSession protocol;
        explicit ProtocolSlot(const ProtocolConfig& config)
            : protocol(config) {}
    };

    ProtocolSlot& _readSlot(int slot_idx);
    const ProtocolSlot& _readSlot(int slot_idx) const;
    ProtocolSlot& _readActiveSlot(int slot_idx);
    const ProtocolSlot& _readActiveSlot(int slot_idx) const;

private:
    std::vector<ProtocolSlot> m_slots {};
    std::vector<int> m_timed_out_slots {};
};
