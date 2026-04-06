#pragma once

#include "exchange_protocol.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <sys/epoll.h>

struct DummyExchangeConfig {
    std::string listen_ip                   {"192.168.50.2"};
    uint16_t port                           {9000};
    std::string username                    {"client"};
    std::string password                    {"secret"};
    std::string session_id                  {"SESSION01"};
    uint32_t price_min                      {1};
    uint32_t price_max                      {5'000'000};
    uint32_t max_shares                     {1'000'000};
    std::chrono::milliseconds fill_delay    {5};
    std::size_t session_capacity            {64};
    std::size_t replay_capacity             {256};
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
    int run();

private:
    struct TransportState {
        int fd {-1};
        uint64_t session_id {0};
    };

    enum class SessionSlotMode : uint8_t {
        Free,
        Used,
        Test,
    };

    struct EpollEventTag {
        enum class Kind : uint8_t {
            Listen,
            Timer,
            Connected,
        };
        Kind kind {Kind::Connected};
        uint32_t slot_index {0};
        uint32_t generation {0};
    };

    struct SessionSlot {
        explicit SessionSlot(const ProtocolConfig& protocol_config)
            : protocol(protocol_config) {}
        SessionSlotMode mode {SessionSlotMode::Free};
        uint32_t generation {1};
        TransportState transport {};
        ExchangeProtocol protocol;
        EpollEventTag epoll_event_tag {};
        struct epoll_event event {};
    };

    SessionSlot& _acquireSessionSlot(SessionSlotMode mode);
    void _releaseSessionSlot(SessionSlot& slot);
    SessionSlot* _getLiveSlot(const EpollEventTag& token);
    SessionSlot& _resolveTestSlot(TestSessionHandle session);
    const SessionSlot& _resolveTestSlot(TestSessionHandle session) const;
    bool _setNonBlocking(int fd) const;
    int _openTimerFd() const;
    void _resetLiveSessionsBeforeRun();
    int _openListenFd() const;
    int _openEpollFd() const;
    void _registerEpoll();
    int _runEpollWait();
    bool _handleReadyEvent(const epoll_event& event);
    bool _handleTimerTick();
    void _handleLiveSessionEvent(const epoll_event& event, const EpollEventTag& token);
    void _shutdownLiveSessions();
    void _closeRunFds() const;
    void _acceptClients();
    bool _receiveBytes(SessionSlot& slot);
    bool _sendFrame(SessionSlot& slot, std::chrono::steady_clock::time_point now);
    void _closeLiveSession(SessionSlot& slot);

private:
    DummyExchangeConfig m_config {};
    std::atomic_bool m_stop_requested {false};
    std::vector<SessionSlot> m_session_pool {};
    std::vector<uint32_t> m_free_slot_indexes {};
    std::size_t m_free_slot_count {0};
    uint64_t m_next_test_session_id {1};
    uint64_t m_next_live_session_id {1};
    EpollEventTag m_listen_event_tag {.kind = EpollEventTag::Kind::Listen};
    EpollEventTag m_timer_event_tag {.kind = EpollEventTag::Kind::Timer};
    struct epoll_event m_listen_event {};
    struct epoll_event m_timer_event {};
    int m_listen_fd {-1};
    int m_timer_fd {-1};
    int m_epoll_fd {-1};
};
