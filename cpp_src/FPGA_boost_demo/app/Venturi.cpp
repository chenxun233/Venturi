#include "../driver/fpga_dev.h"
#include "../common/thread_affinity.h"
#include "../latency/latency_analyzer.h"
#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"
#include "../rx_engine/fpga_rx_engine.h"
#include "../strategy/dummy_strategy.h"
#include "../tx_engine/executor.h"
#include "../tx_engine/tx_connection.h"
#include "../tx_engine/tx_sender.h"
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

constexpr uint16_t kQueueNum = 2;
constexpr uint32_t kRxSlotNum = 1024;
constexpr std::array<std::string_view, kQueueNum> kSymbolNames = {
    "AAPL",
    "HSBC",
};
constexpr std::array<uint16_t, kQueueNum> kStockLocates = {
    0x000d,
    0x0ee8,

};
constexpr std::array<uint32_t, kQueueNum> kPriceBases = {
    0U,
    0U,
};

constexpr std::size_t kLatencyQueueCapacity = 1024;
constexpr std::size_t kLatencyLogCapacity = 4096;
constexpr std::size_t kExecutorQueueCapacity = 1024;
constexpr int kRxThread0Cpu = 2;
constexpr int kRxThread1Cpu = 4;
constexpr int kLatencyThreadCpu = 6;
constexpr int kMainAndLogPrinterCpu = 8;

std::atomic<bool> g_should_stop {false};

void handleStopSignal(int) {
    g_should_stop.store(true, std::memory_order_release);
}

} // namespace

int main() {
    FPGADev device("0000:01:00.0");
    if (!device.initHardware()) {
        error("Failed to initialize FPGA device");
        return 1;
    }

    if (!device.setRxRingBuffers(kQueueNum, kRxSlotNum, SLOT_SIZE_BYTES)) {
        error("Failed to configure FPGA RX ring buffers");
        return 1;
    }

    for (uint16_t que_idx = 0; que_idx < kQueueNum; ++que_idx) {
        if (!device.setSymbolLocate(que_idx, kStockLocates[que_idx])) {
            error("Failed to configure stock_locate for queue %u", que_idx);
            return 1;
        }
        if (!device.setPriceBase(que_idx, kPriceBases[que_idx])) {
            error("Failed to configure price_base for queue %u", que_idx);
            return 1;
        }
    }

    // device.setSync(kSyncEnabled);
    const GatewayClientConfig tx_connection_config {
        .bind_ip = std::string("192.168.51.1"),
        .server_ip = std::string("192.168.51.2"),
        .port = 9000,
    };
    const TxSenderConfig tx_sender_config {
        .username = std::string("client"),
        .password = std::string("secret"),
        .requested_session = std::string("SESSION01"),
        .heartbeat_interval = std::chrono::seconds(1),
        .intent_capacity = kExecutorQueueCapacity,
        .pending_capacity = 1024,
        .pending_slot_count = 1024,
        .transport_capacity = 1024,
    };

    FPGARxDecoder decoder0;
    FPGARxDecoder decoder1;
    
    FPGARxEngine rx_engine0(device, decoder0, 0);
    FPGARxEngine rx_engine1(device, decoder1, 1);
    DummyStrategy strategy0;
    DummyStrategy strategy1;
    Executor executor0(kExecutorQueueCapacity);
    Executor executor1(kExecutorQueueCapacity);

    TxConnection tx_connection0(tx_connection_config);
    TxConnection tx_connection1(tx_connection_config);

    TxSender tx_sender0(tx_sender_config);
    TxSender tx_sender1(tx_sender_config);
    std::unique_ptr<LatencyTracker> latency_tracker_storage;
    try {
        latency_tracker_storage =
            std::make_unique<LatencyTracker>(kQueueNum, kLatencyQueueCapacity);
    } catch (const std::exception& ex) {
        error("Failed to construct latency tracker: %s", ex.what());
        return 1;
    }
    LatencyTracker& latency_tracker = *latency_tracker_storage;
    LatencyAnalyzer latency_analyzer(kQueueNum);
    LogPrinter log_printer(kQueueNum, kLatencyLogCapacity);
    std::signal(SIGINT, handleStopSignal);
    latency_analyzer.setWarmupRecords(1000);

    rx_engine0.attachLatenyTracker(&latency_tracker);
    rx_engine1.attachLatenyTracker(&latency_tracker);
    strategy0.attachLatenyTracker(&latency_tracker);
    strategy1.attachLatenyTracker(&latency_tracker);
    executor0.attachLatenyTracker(&latency_tracker);
    executor1.attachLatenyTracker(&latency_tracker);
    tx_sender0.attachLatenyTracker(&latency_tracker);
    tx_sender1.attachLatenyTracker(&latency_tracker);
    executor0.attachQueueIdx(0);
    executor1.attachQueueIdx(1);
    
    latency_tracker.attachAnalyzer(&latency_analyzer);
    executor0.attachLogPrinter(&log_printer);
    executor1.attachLogPrinter(&log_printer);
    tx_connection0.attachQueueIdx(0);
    tx_connection1.attachQueueIdx(1);
    tx_connection0.attachLogPrinter(&log_printer);
    tx_connection1.attachLogPrinter(&log_printer);
    tx_connection0.attachSender(&tx_sender0);
    tx_connection1.attachSender(&tx_sender1);
    tx_sender0.attachLogPrinter(&log_printer);
    tx_sender1.attachLogPrinter(&log_printer);

    log_printer.setWorkerCpu(kMainAndLogPrinterCpu);
    log_printer.start();
    std::thread latency_thread([&latency_tracker]() {
        pinCurrentThreadToCpu(kLatencyThreadCpu);
        latency_tracker.run();
    });

    std::vector<std::thread> rx_threads;
    rx_threads.emplace_back([&]() {
        pinCurrentThreadToCpu(kRxThread0Cpu);
        FPGAEventDesc events[MAX_POLL_RECORDS] {};
        OrderIntent intent {};
        OrderExecution execution {};
        TxConnectionInfo connection_info {};

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

            while (executor0.takeReadyExecution(execution)) {
                tx_sender0.acceptExecution(execution);
            }
            
            tx_connection0.pollConnect();
            while (tx_connection0.takeSenderConnectionInfo(connection_info)) {
                tx_sender0.updateConnectionInfo(connection_info);
            }
            tx_sender0.runOnce();
        }
    });

    rx_threads.emplace_back([&]() {
        pinCurrentThreadToCpu(kRxThread1Cpu);
        FPGAEventDesc events[MAX_POLL_RECORDS] {};
        OrderIntent intent {};
        OrderExecution execution {};
        TxConnectionInfo connection_info {};

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

            while (executor1.takeReadyExecution(execution)) {
                tx_sender1.acceptExecution(execution);
            }

            tx_connection1.pollConnect();
            while (tx_connection1.takeSenderConnectionInfo(connection_info)) {
                tx_sender1.updateConnectionInfo(connection_info);
            }
            tx_sender1.runOnce();
        }
    });

    pinCurrentThreadToCpu(kMainAndLogPrinterCpu);

    for (std::thread& rx_thread : rx_threads) {
        if (rx_thread.joinable()) {
            rx_thread.join();
        }
    }
    latency_tracker.stop();
    if (latency_thread.joinable()) {
        latency_thread.join();
    }
    log_printer.stop();
    std::printf("RX Engine Debug Summary\n");
    std::printf("queue=%u decoded=%llu first_event=%llu\n",
                static_cast<unsigned int>(rx_engine0.readQueueIdx()),
                static_cast<unsigned long long>(rx_engine0.readDecodedCount()),
                static_cast<unsigned long long>(rx_engine0.readFirstEventCount()));
    std::printf("queue=%u decoded=%llu first_event=%llu\n",
                static_cast<unsigned int>(rx_engine1.readQueueIdx()),
                static_cast<unsigned long long>(rx_engine1.readDecodedCount()),
                static_cast<unsigned long long>(rx_engine1.readFirstEventCount()));
    latency_tracker.printDebugSummary();
    latency_analyzer.printSummary();

    return 0;
}
