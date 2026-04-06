#pragma once

#include "../common/fixed_ring_buffer.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

constexpr std::size_t kOuchExecutedSize = 36;
constexpr std::size_t kMaxSoupPayloadSize = 64;
constexpr std::size_t kMaxSoupFrameSize = 3 + kMaxSoupPayloadSize; // 3 is the header of soupbinTCP
constexpr std::size_t kMaxOutboundMessagesPerOrder = 1;
constexpr std::size_t kReadBufferSize = 4096;
constexpr std::size_t kMaxPendingFills = 1024;
constexpr std::size_t kOutboundQueueSize = 1024;

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

struct EncodedPayload {
    std::array<uint8_t, kMaxSoupPayloadSize> bytes {};
    std::size_t size {0};
};

struct HandledOrderResult {
    bool is_duplicate {false};
    ExchangeValidationResult validation {};
    std::array<EncodedPayload, kMaxOutboundMessagesPerOrder> outbound_messages {};
    std::size_t outbound_message_count {0};
};

struct SOUPBinFrame {
    std::array<uint8_t, kMaxSoupFrameSize> payload {};
    std::size_t size {0};
    std::size_t offset {0};
};

class ExchangeProtocol {
public:
    explicit ExchangeProtocol(ProtocolConfig config);

    void reset(uint64_t session_id, std::chrono::steady_clock::time_point now);
    bool appendReceivedBytes(const uint8_t* bytes, std::size_t size, std::chrono::steady_clock::time_point now);
    void onTimerTick(std::chrono::steady_clock::time_point now);
    bool shouldClose() const;
    const SOUPBinFrame& readFrontFrame() const;
    SOUPBinFrame& readFrontFrame();
    void eraseFrontFrame();
    bool hasOutboundFrame() const;
    void writeLastTime(std::chrono::steady_clock::time_point now);
    ExchangeValidationResult validateEnterOrder(const ExchangeEnterOrder& order) const;
    HandledOrderResult handleEnterOrderForTest(const ExchangeEnterOrder& order,
                                               std::chrono::steady_clock::time_point now);
    void queuePacketForTest(const std::vector<uint8_t>& bytes);
    void markLoggedInForTest();
    void setLastSendAgoForTest(std::chrono::seconds age);
    std::optional<uint8_t> tryReadPacketTypeForTest();
    std::optional<uint8_t> peekFrontPacketTypeForTest() const;
    std::vector<uint8_t> readFrontPacketBytesForTest() const;
    std::size_t readQueuedPacketCountForTest() const;
    uint64_t readNextSequenceForTest() const;

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

    struct SoupPacket {
        uint8_t type {0};
        EncodedPayload payload {};
    };

    std::optional<SoupPacket> _tryReadPayload();
    bool _handleClientPacket(uint8_t type,
                             const EncodedPayload& payload,
                             std::chrono::steady_clock::time_point now);
    void _queueOutboundMaintenance(std::chrono::steady_clock::time_point now);
    bool _insertReplayResult(uint32_t user_ref_num, const HandledOrderResult& result);
    std::optional<HandledOrderResult> _findReplayResult(uint32_t user_ref_num) const;
    HandledOrderResult _buildReplayCapacityReject(uint32_t user_ref_num);
    HandledOrderResult _handleEnterOrder(const ExchangeEnterOrder& order,
                                         std::chrono::steady_clock::time_point now);
    void _queueSoupFrame(uint8_t type, const uint8_t* payload, std::size_t payload_size);
    bool _pushOutQueue(const uint8_t* bytes, std::size_t size);
    bool _isOutFrameEmpty() const;
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
    FixedRingBuffer<uint8_t, kReadBufferSize> m_read_buffer {};
    FixedRingBuffer<SOUPBinFrame, kOutboundQueueSize> m_outbound_queue {};
    FixedRingBuffer<PendingFill, kMaxPendingFills> m_pending_fills {};
    std::vector<ReplayEntry> m_replay_entries {};
    std::size_t m_replay_count {0};
};
