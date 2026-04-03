#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

enum class ExchangeValidationKind : uint8_t {
    Accepted,
    Rejected,
};

struct ExchangeValidationResult {
    ExchangeValidationKind kind {ExchangeValidationKind::Accepted};
    uint16_t reject_reason {0};
};

struct ExchangeEnterOrder {
    uint32_t tag {0};
    uint16_t stock_locate {0};
    uint32_t shares {0};
    uint32_t price {0};
    char side {'B'};
};

struct DummyExchangeConfig {
    std::string listen_ip {"192.168.50.2"};
    uint16_t port {9000};
    std::string username {"client"};
    std::string password {"secret"};
    std::string session_id {"SESSION01"};
    uint32_t price_min {1};
    uint32_t price_max {5'000'000};
    uint32_t max_shares {1'000'000};
    std::chrono::milliseconds fill_delay {5};
};

struct HandledOrderResult {
    bool is_duplicate {false};
    ExchangeValidationResult validation {};
    std::vector<std::vector<uint8_t>> outbound_messages {};
};

class DummyExchangeServer {
public:
    explicit DummyExchangeServer(DummyExchangeConfig config);

    uint64_t createSessionForTest();
    void appendReadBytesForTest(uint64_t session_id, const std::vector<uint8_t>& bytes);
    std::optional<uint8_t> tryReadPacketTypeForTest(uint64_t session_id);
    void queuePacketForTest(uint64_t session_id, const std::vector<uint8_t>& bytes);
    bool consumeQueuedBytesForTest(uint64_t session_id, std::size_t count);
    std::size_t readQueuedPacketCountForTest(uint64_t session_id) const;
    void markLoggedInForTest(uint64_t session_id);
    void setLastSendAgoForTest(uint64_t session_id, std::chrono::seconds age);
    void handleTimerTickForTest();
    std::optional<uint8_t> peekFrontPacketTypeForTest(uint64_t session_id) const;
    ExchangeValidationResult validateEnterOrder(const ExchangeEnterOrder& order) const;
    HandledOrderResult handleEnterOrderForTest(uint64_t session_id, const ExchangeEnterOrder& order);
    uint64_t readSessionNextSequenceForTest(uint64_t session_id) const;
    HandledOrderResult handleEnterOrder(const ExchangeEnterOrder& order);
    int run();

private:
    struct PendingFill {
        uint32_t tag {0};
        uint32_t executed_shares {0};
        uint32_t price {0};
        uint64_t match_number {0};
        std::chrono::steady_clock::time_point due_time {};
    };

    struct OutboundPacket {
        std::vector<uint8_t> bytes {};
        std::size_t offset {0};
    };

    struct SessionState {
        bool is_logged_in {false};
        std::vector<uint8_t> read_buffer {};
        std::vector<OutboundPacket> write_queue {};
        std::chrono::steady_clock::time_point last_send {};
        std::chrono::steady_clock::time_point last_receive {};
        uint64_t next_sequence {1};
        uint64_t next_order_ref_num {1};
        uint64_t next_match_number {1};
        std::unordered_map<uint32_t, HandledOrderResult> order_results {};
        std::vector<std::vector<uint8_t>> sequenced_history {};
        std::vector<PendingFill> pending_fills {};
    };

    SessionState& _findOrCreateTestSession(uint64_t session_id);
    void _handleTimerTick(SessionState& session, std::chrono::steady_clock::time_point now);
    std::optional<uint8_t> _tryReadPacketType(SessionState& session);
    HandledOrderResult _handleEnterOrder(SessionState& session, const ExchangeEnterOrder& order);
    uint64_t _readTimestampNs() const;
    uint64_t _readNextSequence(const SessionState& session) const;
    uint64_t _readNextOrderRef(const SessionState& session) const;
    uint64_t _readNextMatchNumber(const SessionState& session) const;
    void _storeSequenced(SessionState& session, const std::vector<uint8_t>& payload);

    DummyExchangeConfig m_config {};
    SessionState m_single_session {};
    std::unordered_map<uint64_t, SessionState> m_test_sessions {};
};
