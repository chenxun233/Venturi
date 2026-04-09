#pragma once

#include "exchange_protocol.h"
#include "exchange_transport.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <sys/epoll.h>
#include <vector>

struct DummyExchangeConfig {
    std::string listen_ip                   {"192.168.51.2"};
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

class DummyServer {
public:
    explicit DummyServer(DummyExchangeConfig config);
    void requestStop();
    int run();

private:
    enum class SlotMode : uint8_t {
        Free,
        Live,
    };

    struct EpollEventTag {
        enum class Kind : uint8_t {
            Listen,
            Timer,
            Connected,
        };
        Kind kind {Kind::Connected};
        uint32_t slot_idx {0};
        uint32_t generation {0};
    };

    struct ServerSlot {
        SlotMode mode {SlotMode::Free};
        uint32_t slot_idx {0};
        uint32_t generation {1};
        EpollEventTag event_tag {};
        struct epoll_event event {};
    };

    ServerSlot& _allocateServerSlot(SlotMode mode);
    void _releaseSlot(ServerSlot& slot);
    ServerSlot* _getLiveSlot(const EpollEventTag& token);
    bool _setNonBlocking(int fd) const;
    int _openTimerFd() const;
    void _resetLiveSlotsBeforeRun();
    int _openListenFd() const;
    int _openEpollFd() const;
    void _registerEpoll();
    int _runHandle();
    bool _handleReadyEvent(const epoll_event& event);
    bool _handleTimerEvent();
    void _handleConnectedEvent(const epoll_event& event, const EpollEventTag& token);
    void _acceptClient();
    bool _flushRuntimeOutbound(int slot_idx);
    void _closeSlot(int slot_idx);
    void _shutdownLiveSlots();
    void _closeRunFds();

private:
    DummyExchangeConfig m_config {};
    std::atomic_bool m_stop_requested {false};
    std::vector<ServerSlot> m_server_slots {};
    std::vector<uint32_t> m_free_server_slot_idx {};
    std::size_t m_free_slot_count {0};
    uint64_t m_next_live_session_id {1};
    EpollEventTag m_listen_event_tag {.kind = EpollEventTag::Kind::Listen};
    EpollEventTag m_timer_event_tag {.kind = EpollEventTag::Kind::Timer};
    struct epoll_event m_listen_event {};
    struct epoll_event m_timer_event {};
    int m_listen_fd {-1};
    int m_timer_fd {-1};
    int m_epoll_fd {-1};
    ExchangeTransport m_transport;
    ExchangeProtocol m_protocol;
};
