#include "../fpga_dev/fpga_dev.h"
#include "../common/thread_affinity.h"
#include "../latency/latency_analyzer.h"
#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"
#include "../fpga_rx_engine/fpga_rx_engine.h"
#include "../strategy/dummy_strategy.h"
#include "../tx_client/executor.h"
#include "../tx_client/tx_client.h"
#include "../../common/log.h"

#include <atomic>
#include <array>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <ctime>
#include <exception>
#include <cstdio>
#include <memory>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


namespace {



struct RuntimeQueueConfig {
    std::string_view symbol_name;
    uint16_t stock_locate;
    uint32_t price_base;
};

struct RuntimeConfig {
    static constexpr uint16_t queue_num = 2;
    static constexpr uint32_t rx_slot_num = 1024;
    static constexpr std::size_t latency_queue_capacity = 1024;
    static constexpr std::size_t latency_log_capacity = 4096;
    static constexpr std::size_t executor_queue_capacity = 1024;

    static constexpr std::array<RuntimeQueueConfig, queue_num> queues {{
        {.symbol_name = "AAPL", .stock_locate = 0x000d, .price_base = 0U},
        {.symbol_name = "HSBC", .stock_locate = 0x0ee8, .price_base = 0U},
    }};
};

struct CpuConfig {
    static constexpr int rx_thread0 = 2;
    static constexpr int rx_thread1 = 4;
    static constexpr int latency_thread = 6;
    static constexpr int main_and_log_printer = 8;
};

const GatewayClientConfig kTxConnectionConfig {
    .bind_ip = std::string("192.168.51.1"),
    .server_ip = std::string("192.168.51.2"),
    .port = 9000,
};

const TxSenderConfig kTxSenderConfig {
    .username = std::string("client"),
    .password = std::string("secret"),
    .requested_session = std::string("SESSION01"),
    .heartbeat_interval = std::chrono::seconds(1),
    .pending_capacity = 1024,
};

std::atomic<bool> g_should_stop {false};

void handleStopSignal(int) {
    g_should_stop.store(true, std::memory_order_release);
}

std::unique_ptr<LatencyTracker> makeLatencyTracker(uint16_t queue_num,
                                                   std::size_t queue_capacity) {
    try {
        return std::make_unique<LatencyTracker>(queue_num, queue_capacity);
    } catch (const std::exception& ex) {
        error("Failed to construct latency tracker: %s", ex.what());
        return nullptr;
    }
}

} // namespace

int main() {

    // === init the device ===
    FPGADev device("0000:01:00.0");
    if (!device.initHardware()) {
        error("Failed to initialize FPGA device");
        return 1;
    }

    if (!device.setRxRingBuffers(RuntimeConfig::rx_slot_num,SLOT_SIZE_BYTES)) {
        error("Failed to configure FPGA RX ring buffers");
        return 1;
    }

    for (uint16_t que_idx = 0; que_idx < RuntimeConfig::queue_num; ++que_idx) {
        if (!device.setSymbolLocate(que_idx, RuntimeConfig::queues[que_idx].stock_locate)) {
            error("Failed to configure stock_locate for queue %u", que_idx);
            return 1;
        }
        if (!device.setPriceBase(que_idx, RuntimeConfig::queues[que_idx].price_base)) {
            error("Failed to configure price_base for queue %u", que_idx);
            return 1;
        }
    }
    FPGARxEngine rx_engine0(device, 0);
    FPGARxEngine rx_engine1(device, 1);
    DummyStrategy strategy0;
    DummyStrategy strategy1;
    Executor executor0(RuntimeConfig::executor_queue_capacity);
    Executor executor1(RuntimeConfig::executor_queue_capacity);

    TxClient tx_client0(kTxConnectionConfig, kTxSenderConfig);
    TxClient tx_client1(kTxConnectionConfig, kTxSenderConfig);
    std::unique_ptr<LatencyTracker> latency_tracker =
        makeLatencyTracker(RuntimeConfig::queue_num, RuntimeConfig::latency_queue_capacity);
    if (latency_tracker == nullptr) {
        return 1;
    }
    LatencyAnalyzer latency_analyzer(RuntimeConfig::queue_num);
    LogPrinter log_printer(RuntimeConfig::queue_num, RuntimeConfig::latency_log_capacity);
    std::signal(SIGINT, handleStopSignal);
    latency_analyzer.setWarmupRecords(1000);

    rx_engine0.attachLatencyTracker(latency_tracker.get());
    rx_engine1.attachLatencyTracker(latency_tracker.get());
    strategy0.attachLatencyTracker(latency_tracker.get());
    strategy1.attachLatencyTracker(latency_tracker.get());
    executor0.attachLatencyTracker(latency_tracker.get());
    executor1.attachLatencyTracker(latency_tracker.get());
    tx_client0.attachLatencyTracker(latency_tracker.get());
    tx_client1.attachLatencyTracker(latency_tracker.get());
    executor0.attachQueueIdx(0);
    executor1.attachQueueIdx(1);
    
    latency_tracker->attachAnalyzer(&latency_analyzer);
    tx_client0.attachQueueIdx(0);
    tx_client1.attachQueueIdx(1);
    tx_client0.attachLogPrinter(&log_printer);
    tx_client1.attachLogPrinter(&log_printer);


    log_printer.setWorkerCpu(CpuConfig::main_and_log_printer);
    log_printer.start();
    std::thread latency_thread([&latency_tracker]() {
        pinCurrentThreadToCpu(CpuConfig::latency_thread);
        latency_tracker->run();
    });

    std::vector<std::thread> rx_threads;
    rx_threads.emplace_back([&]() {
        pinCurrentThreadToCpu(CpuConfig::rx_thread0);
        FPGAEventDesc events[MAX_POLL_RECORDS] {};
        OrderIntent intent {};
        OrderExecution execution {};

        while (!g_should_stop.load(std::memory_order_acquire)) {
            const std::size_t count =
                rx_engine0.pollDecodedBatch(MAX_POLL_RECORDS, events);
            if (count > 0) {
                for (std::size_t idx = 0; idx < count; ++idx) {
                    FPGAEventDesc& event = events[idx];
                    if (strategy0.evaluateEvent(0, event, intent)) {
                        executor0.acceptIntent(intent);
                    }
                }
            }

            while (executor0.popExecution(execution)) {
                tx_client0.acceptExecution(execution);
            }

            tx_client0.runOnce();
        }
    });

    rx_threads.emplace_back([&]() {
        pinCurrentThreadToCpu(CpuConfig::rx_thread1);
        FPGAEventDesc events[MAX_POLL_RECORDS] {};
        OrderIntent intent {};
        OrderExecution execution {};

        while (!g_should_stop.load(std::memory_order_acquire)) {
            const std::size_t count =
                rx_engine1.pollDecodedBatch(MAX_POLL_RECORDS,  events);

            if (count > 0) {
                for (std::size_t idx = 0; idx < count; ++idx) {
                    FPGAEventDesc& event = events[idx];
                    if (strategy1.evaluateEvent(1, event, intent)) {
                        executor1.acceptIntent(intent);
                    }
                }
            }

            while (executor1.popExecution(execution)) {
                tx_client1.acceptExecution(execution);
            }

            tx_client1.runOnce();
        }
    });

    pinCurrentThreadToCpu(CpuConfig::main_and_log_printer);

    for (std::thread& rx_thread : rx_threads) {
        if (rx_thread.joinable()) {
            rx_thread.join();
        }
    }
    latency_tracker->stop();
    if (latency_thread.joinable()) {
        latency_thread.join();
    }
    log_printer.stop();
    const uint16_t queue0 = rx_engine0.readQueueIdx();
    const uint16_t queue1 = rx_engine1.readQueueIdx();
    const uint64_t total_received0 = rx_engine0.readFirstEventCount();
    const uint64_t total_received1 = rx_engine1.readFirstEventCount();
    latency_analyzer.setTotalReceived(queue0, total_received0);
    latency_analyzer.setTotalReceived(queue1, total_received1);
    latency_analyzer.printSummary();
    tx_client0.printReceiverSummary();
    tx_client1.printReceiverSummary();

    return 0;
}
