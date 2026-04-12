#include "../driver/fpga_dev.h"
#include "../rx_engine/fpga_rx_engine.h"
#include "../latency/log_printer.h"
#include "../latency/latency_tracker.h"
#include "../strategy/dummy_strategy.h"
#include "../sync/FPGA_regression.h"
#include "../tx_engine/executor.h"
#include "../tx_engine/tx_connection.h"
#include "../tx_engine/tx_receiver.h"
#include "../tx_engine/tx_send_socket.h"
#include "../tx_engine/tx_sender.h"
#include "../../common/log.h"

#include <atomic>
#include <array>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <cstdio>
#include <mutex>
#include <string_view>
#include <thread>
#include <vector>


namespace {

constexpr uint16_t kQueueCount = 2;
constexpr uint32_t kRxSlotCount = 1024;
constexpr std::array<std::string_view, kQueueCount> kSymbolNames = {
    "AAPL",
    "HSBC",
};
constexpr std::array<uint16_t, kQueueCount> kStockLocates = {
    0x000d,
    0x0ee8,

};
constexpr std::array<uint32_t, kQueueCount> kPriceBases = {
    0U,
    0U,
};

constexpr bool kSyncEnabled = true;
constexpr auto kSnapshotPrintInterval = std::chrono::seconds(1);
constexpr auto kThreadSleepTime = std::chrono::microseconds(100);
constexpr uint64_t kSnapshotSamplePeriod =10000;
constexpr std::size_t kInitSyncMaxAttempts = 100000;
constexpr std::size_t kLatencyQueueCapacity = 1024;
constexpr std::size_t kLatencyLogCapacity = 4096;
constexpr std::size_t kExecutorQueueCapacity = 1024;
constexpr uint64_t    kMaxAcceptInterval = 2000;
constexpr std::string_view kTxBindIp = "192.168.51.1";
constexpr std::string_view kTxServerIp = "192.168.51.2";
constexpr uint16_t kTxServerPort = 9000;
constexpr std::string_view kTxUsername = "client";
constexpr std::string_view kTxPassword = "secret";
constexpr std::string_view kTxSession = "SESSION01";

} // namespace

int main() {
    FPGADev device("0000:01:00.0");
    if (!device.initHardware()) {
        error("Failed to initialize FPGA device");
        return 1;
    }

    if (!device.setRxRingBuffers(kQueueCount, kRxSlotCount, SLOT_SIZE_BYTES)) {
        error("Failed to configure FPGA RX ring buffers");
        return 1;
    }

    for (uint16_t que_idx = 0; que_idx < kQueueCount; ++que_idx) {
        if (!device.setSymbolLocate(que_idx, kStockLocates[que_idx])) {
            error("Failed to configure stock_locate for queue %u", que_idx);
            return 1;
        }
        if (!device.setPriceBase(que_idx, kPriceBases[que_idx])) {
            error("Failed to configure price_base for queue %u", que_idx);
            return 1;
        }
    }

    device.setSync(kSyncEnabled);

    FPGARxDecoder decoder0;
    FPGARxDecoder decoder1;
    FPGARegression FPGA_regression;
    
    FPGARxEngine rx_engine0(device, decoder0, 0);
    FPGARxEngine rx_engine1(device, decoder1, 1);
    DummyStrategy strategy0;
    DummyStrategy strategy1;
    Executor executor(kQueueCount, kExecutorQueueCapacity);
    TxSender tx_sender(TxSenderConfig {
        .username = std::string(kTxUsername),
        .password = std::string(kTxPassword),
        .requested_session = std::string(kTxSession),
        .heartbeat_interval = std::chrono::seconds(1),
        .intent_capacity = kExecutorQueueCapacity,
        .pending_capacity = 1024,
    });
    TxConnection tx_connection(GatewayClientConfig {
        .bind_ip = std::string(kTxBindIp),
        .server_ip = std::string(kTxServerIp),
        .port = kTxServerPort,
    });
    TxReceiver tx_receiver(tx_connection, tx_sender);
    LatencyTracker latency_tracker(kQueueCount, kLatencyQueueCapacity);
    LogPrinter log_printer(kLatencyLogCapacity);

    rx_engine0.attachLatenyTracker(&latency_tracker);
    rx_engine1.attachLatenyTracker(&latency_tracker);
    strategy0.attachLatenyTracker(&latency_tracker);
    strategy1.attachLatenyTracker(&latency_tracker);
    executor.attachLatenyTracker(&latency_tracker);
    tx_sender.attachLatenyTracker(&latency_tracker);
    
    latency_tracker.attachLogPrinter(&log_printer);
    latency_tracker.attachRegression(&FPGA_regression);
    log_printer.start();
    executor.attachLogPrinter(&log_printer);
    tx_sender.attachLogPrinter(&log_printer);
    tx_connection.attachLogPrinter(&log_printer);
    tx_receiver.attachLogPrinter(&log_printer);
    
    
    CapSignal capture_signal {};
    // init sync
    if (!FPGA_regression.initSync(device, kInitSyncMaxAttempts, kMaxAcceptInterval)) {
        error("initSync failed to converge within %zu attempts", kInitSyncMaxAttempts);
        log_printer.stop();
        return 1;
    }
    (void)log_printer.pushRegressionStatus(FPGA_regression.readStatusLogRecord());

    std::thread control_thread([&]() 
    {
        while (true) {
            FPGA_regression.run(capture_signal);
            const std::size_t processed = latency_tracker.run();
            if (processed == 0){
                std::this_thread::sleep_for(kThreadSleepTime);
            }
        }
    });


    std::vector<std::thread> rx_threads; //rx_engine0
    rx_threads.emplace_back([&]() {
        FpgaSyncSnapshot snapshot {};
        FPGAEventDesc events[MAX_POLL_RECORDS] {};
        OrderIntent intent {};
        while (true) {
            const bool get_snapshot =
                capture_signal.request.exchange(false, std::memory_order_acq_rel);
            const std::size_t count =
                rx_engine0.pollDecodedBatchSync(MAX_POLL_RECORDS, get_snapshot, &snapshot, events);
            if (get_snapshot) {
                (void)FPGA_regression.tryAcceptSnapshot(snapshot, kMaxAcceptInterval);
            }
            if (count > 0) {
                for (std::size_t idx = 0; idx < count; ++idx) {
                    FPGAEventDesc& event = events[idx];
                    if (strategy0.evaluateEvent(0, event, intent)) {
                        executor.acceptIntent(0, intent);
                    }
                }
            }
            if (count == 0) {
               continue;
            }
        }
    });

    rx_threads.emplace_back([&]() {//rx_engine1
        FPGAEventDesc events[MAX_POLL_RECORDS] {};
        OrderIntent intent {};
        while (true) {
            const std::size_t count = rx_engine1.pollDecodedBatch(MAX_POLL_RECORDS, events);
            if (count > 0) {
                for (std::size_t idx = 0; idx < count; ++idx) {
                    FPGAEventDesc& event = events[idx];
                    if (strategy1.evaluateEvent(1, event, intent)) {
                        executor.acceptIntent(1, intent);
                    }
                }
            }
            if (count == 0) {
                continue;
            }
        }
    });

    std::thread executor_thread([&]() 
    {
        OrderIntent intent {};
        bool has_pending_intent = false;
        while (true) {
            if (!has_pending_intent && executor.popReadyIntent(intent)) {
                has_pending_intent = true;
            }
            if (has_pending_intent && tx_sender.acceptIntent(intent)) {
                executor.logExecution(intent);
                has_pending_intent = false;
            }
        }
    });
    std::thread tx_sender_thread([&]() {
        // Transitional bridge: TxConnection is receiver-owned and now publishes TxTransportControl
        // (including sender fd) on the receiver thread. The end-to-end transport-control plumbing
        // into the sender thread is not yet wired, so TxSendSocket will remain inactive until a
        // later task installs real Connected controls.
        TxSendSocket send_socket {};
        send_socket.attachLogPrinter(&log_printer);
        send_socket.attachLatenyTracker(&latency_tracker);
        while (true) {
            bool did_work = false;

            did_work = tx_sender.buildOutboundFrame() || did_work;
            did_work = tx_sender.queueHeartbeatIfDue() || did_work;
            did_work = tx_sender.processInboundQueues() || did_work;

            if (!send_socket.hasActiveFd()) {
                if (!did_work) {
                    std::this_thread::sleep_for(kThreadSleepTime);
                }
                continue;
            }

            TxOutboundRecord outbound {};
            while (tx_sender.popReadyOutbound(outbound)) {
                if (!send_socket.sendPayload(outbound)) {
                    tx_sender.restoreReadyOutbound(outbound);
                    break;
                }
                tx_sender.noteOutboundSent(outbound);
                did_work = true;
            }

            if (!did_work) {
                std::this_thread::sleep_for(kThreadSleepTime);
            }
        }
    });

    std::thread tx_receiver_thread([&]() {
        while (true) {
            const bool did_work = tx_receiver.pollOnce();
            if (!did_work) {
                std::this_thread::sleep_for(kThreadSleepTime);
            }
        }
    });
    

    for (std::thread& rx_thread : rx_threads) {
        if (rx_thread.joinable()) {
            rx_thread.join();
        }
    }
    if (control_thread.joinable()) {
        control_thread.join();
    }
    if (executor_thread.joinable()) {
        executor_thread.join();
    }
    if (tx_sender_thread.joinable()) {
        tx_sender_thread.join();
    }
    if (tx_receiver_thread.joinable()) {
        tx_receiver_thread.join();
    }
    log_printer.stop();

    return 0;
}
