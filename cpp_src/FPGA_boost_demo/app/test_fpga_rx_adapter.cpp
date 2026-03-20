#include "fpga_rx_adapter.h"
#include "../../common/log.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <memory>
#include <string>
#include <thread>

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::fprintf(stderr, "Usage: %s <pci_address> <queue_id> [max_events]\n", argv[0]);
        std::fprintf(stderr, "  e.g.: %s 0000:05:00.0 0 8\n", argv[0]);
        return 1;
    }

    const char* pci_addr = argv[1];
    const uint16_t queue_id = static_cast<uint16_t>(std::strtoul(argv[2], nullptr, 0));
    const uint64_t max_events = (argc == 4) ? std::strtoull(argv[3], nullptr, 0) : 8ULL;

    std::unique_ptr<FPGADev> dev;
    try {
        dev = std::make_unique<FPGADev>(std::string(pci_addr));
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Failed to create device: %s\n", e.what());
        return 1;
    }

    if (!dev || !dev->initHardware()) {
        warn("Hardware initialization failed");
        return 1;
    }

    if (queue_id >= dev->rxQueueCount()) {
        warn("Queue %u is out of range, valid queues are [0, %u)", queue_id, dev->rxQueueCount());
        return 1;
    }

    FPGARxDataAdaptor adapter(*dev);
    uint64_t event_count = 0;
    info("Polling queue %u for up to %llu decoded FPGA events", queue_id, static_cast<unsigned long long>(max_events));

    while (event_count < max_events) {
        FPGAEventDesc event;
        if (!adapter.pollOne(queue_id, event)) {
            std::this_thread::yield();
            continue;
        }

        std::printf("event=%llu queue=%u locate=%04x ask=(%u,%u) bid=(%u,%u) frame_start_ts=%llu frame_latency=%llu\n",
                    static_cast<unsigned long long>(event_count),
                    event.queue_id,
                    event.stock_locate,
                    event.ask_price,
                    event.ask_shares,
                    event.bid_price,
                    event.bid_shares,
                    static_cast<unsigned long long>(event.frame_start_ts),
                    static_cast<unsigned long long>(event.frame_latency));
        ++event_count;
    }

    return 0;
}
