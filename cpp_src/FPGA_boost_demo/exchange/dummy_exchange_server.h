#pragma once

#include "../common/fixed_ring_buffer.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
constexpr std::size_t kOuchExecutedSize = 36; // outbound, order executed message size in bytes.
constexpr std::size_t kMaxSoupPayloadSize = 64;
constexpr std::size_t kMaxSoupFrameSize = 3 + kMaxSoupPayloadSize;
constexpr std::size_t kMaxOutboundMessagesPerOrder = 1;
constexpr std::size_t queue_size = 1024;
constexpr std::size_t kMaxPendingFills = 1024;
constexpr std::size_t kReadBufferSize = 4096;

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
    std::size_t session_capacity {64};
    std::size_t replay_capacity {256};
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

struct SoupPacket {
    uint8_t type {0};
    EncodedPayload payload {};
};

class DummyExchangeServer {
public:
    struct TestSessionHandle {
        uint32_t slot_index {0};
        uint32_t generation {0};
    };

    explicit DummyExchangeServer(DummyExchangeConfig config);

    void requestStopForTest();
    TestSessionHandle createSessionForTest();
    void releaseSessionForTest(TestSessionHandle session);
    void appendReadBytesForTest(TestSessionHandle session, const std::vector<uint8_t>& bytes);
    std::optional<uint8_t> tryReadPacketTypeForTest(TestSessionHandle session);
    void queuePacketForTest(TestSessionHandle session, const std::vector<uint8_t>& bytes);
    bool consumeQueuedBytesForTest(TestSessionHandle session, std::size_t count);
    std::size_t readQueuedPacketCountForTest(TestSessionHandle session) const;
    std::vector<uint8_t> readFrontPacketBytesForTest(TestSessionHandle session) const;
    void markLoggedInForTest(TestSessionHandle session);
    void setLastSendAgoForTest(TestSessionHandle session, std::chrono::seconds age);
    void handleTimerTickForTest();
    std::optional<uint8_t> peekFrontPacketTypeForTest(TestSessionHandle session) const;
    ExchangeValidationResult validateEnterOrder(const ExchangeEnterOrder& order) const;
    HandledOrderResult handleEnterOrderForTest(TestSessionHandle session, const ExchangeEnterOrder& order);
    uint64_t readSessionNextSequenceForTest(TestSessionHandle session) const;
    int run();

private:
    struct PendingFill {
        uint32_t user_ref_num {0};
        uint32_t executed_shares {0};
        uint32_t price {0};
        uint64_t match_number {0};
        std::chrono::steady_clock::time_point due_time {};
    };

    struct OutboundPacket {
        std::array<uint8_t, kMaxSoupFrameSize> payload {};
        std::size_t size {0};
        std::size_t offset {0};
    };

    struct ClientState {
        int fd {-1};
        uint64_t session_id {0};
        bool is_logged_in {false};
        FixedRingBuffer<uint8_t, kReadBufferSize> read_buffer {};
        FixedRingBuffer<OutboundPacket, queue_size> out_bound_que {};
        std::chrono::steady_clock::time_point last_send_time {};
        std::chrono::steady_clock::time_point last_receive_time {};
        uint64_t next_sequence {1};
        uint64_t next_order_ref_num {1};
        uint64_t next_match_number {1};
        std::unordered_map<uint32_t, HandledOrderResult> order_results {};
        FixedRingBuffer<PendingFill, kMaxPendingFills> pending_fills {};
    };

    enum class SessionSlotMode : uint8_t {
        Free,
        Live,
        Test,
    };

    struct ReplayEntry {
        bool is_occupied {false};
        uint32_t user_ref_num {0};
        HandledOrderResult result {};
    };

    struct EventToken {
        enum class Kind : uint8_t {
            Listen,
            Timer,
            LiveSession,
        };

        Kind kind {Kind::LiveSession};
        uint32_t slot_index {0};
        uint32_t generation {0};
    };

    struct SessionSlot {
        SessionSlotMode mode {SessionSlotMode::Free};
        uint32_t generation {1};
        ClientState client {};
        std::vector<ReplayEntry> replay_entries {};
        std::size_t replay_count {0};
        EventToken event_token {};
    };

    SessionSlot& _acquireSessionSlot(SessionSlotMode mode);
    void _releaseSessionSlot(SessionSlot& slot);
    SessionSlot& _resolveTestSlot(TestSessionHandle session);
    const SessionSlot& _resolveTestSlot(TestSessionHandle session) const;
    void _queOutBound(ClientState& client, std::chrono::steady_clock::time_point now);
    bool _setNonBlocking(int fd) const;
    int _openTimerFd() const;
    void _acceptClients(int epoll_fd, int listen_fd);
    std::optional<SoupPacket> _tryReadPayload(ClientState& client);
    bool _receiveClientData(ClientState& client);
    bool _sendQueFront(ClientState& client);
    void _queueSoupFrame(ClientState& client, uint8_t type, const uint8_t* payload, std::size_t payload_size);
    bool _handleClientPacket(ClientState& client, uint8_t type, const EncodedPayload& payload);
    void _closeSession(int epoll_fd, int fd);
    std::optional<uint8_t> _tryReadPacketType(ClientState& client);
    HandledOrderResult _handleEnterOrder(ClientState& client, const ExchangeEnterOrder& order);
    bool _isOutQueueEmpty(const ClientState& client) const;
    std::size_t _readOutQueueSize(const ClientState& client) const;
    OutboundPacket& _readOutQueueFront(ClientState& client);
    const OutboundPacket& _readOutQueueFront(const ClientState& client) const;
    bool _pushOutQueue(ClientState& client, const uint8_t* bytes, std::size_t size);
    void _popOutQueue(ClientState& client);
    uint64_t _readTimestampNs() const;
    uint64_t _readNextSequence(const ClientState& client) const;
    uint64_t _readNextOrderRef(const ClientState& client) const;
    uint64_t _readNextMatchNumber(const ClientState& client) const;

    DummyExchangeConfig m_config {};
    std::atomic_bool m_stop_requested {false};
    ClientState m_single_session {};
    std::vector<SessionSlot> m_session_slots {};
    std::vector<uint32_t> m_free_slot_indexes {};
    std::size_t m_free_slot_count {0};
    uint64_t m_next_test_session_id {1};
    uint64_t m_next_live_session_id {1};
    std::unordered_map<int, ClientState> m_live_clients {};

private:
};
