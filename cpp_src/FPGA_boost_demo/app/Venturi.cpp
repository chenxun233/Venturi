#include "../driver/fpga_dev.h"
#include "../rx_engine/fpga_rx_engine.h"
#include "../latency/log_printer.h"
#include "../latency/latency_tracker.h"
#include "../strategy/dummy_strategy.h"
#include "../sync/FPGA_regression.h"
#include "../tx_engine/executor.h"
#include "../tx_engine/tx_translator.h"
#include "../tx_engine/tx_engine.h"
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

uint64_t readSystemTimeNs(bool is_first) {
    if (!is_first) {
        return 0;
    }
    timespec ts {};
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL) +
           static_cast<uint64_t>(ts.tv_nsec);
}




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
    TxTranslator tx_translator(TxTranslatorConfig {
        .username = std::string(kTxUsername),
        .password = std::string(kTxPassword),
        .requested_session = std::string(kTxSession),
        .heartbeat_interval = std::chrono::seconds(1),
        .intent_capacity = kExecutorQueueCapacity,
        .pending_capacity = 1024,
    });
    TxEngine tx_engine(GatewayClientConfig {
        .bind_ip = std::string(kTxBindIp),
        .server_ip = std::string(kTxServerIp),
        .port = kTxServerPort,
    });
    LatencyTracker latency_tracker(kQueueCount, kLatencyQueueCapacity);
    LogPrinter log_printer(kLatencyLogCapacity);
    
    latency_tracker.attachLogPrinter(&log_printer);
    latency_tracker.attachRegression(&FPGA_regression);
    log_printer.start();
    executor.attachLogPrinter(&log_printer);
    tx_translator.attachLogPrinter(&log_printer);
    tx_engine.attachLogPrinter(&log_printer);
    
    
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
        FirstEventMask mask {};
        FpgaSyncSnapshot snapshot {};
        DecodedEvent FPGA_events[MAX_POLL_RECORDS] {};
        OrderIntent intent {};
        while (true) {
            const bool get_snapshot =
                capture_signal.request.exchange(false, std::memory_order_acq_rel);
            const std::size_t count =
                rx_engine0.pollDecodedBatchSync(mask, MAX_POLL_RECORDS, get_snapshot, &snapshot, FPGA_events);
            if (get_snapshot) {
                (void)FPGA_regression.tryAcceptSnapshot(snapshot, kMaxAcceptInterval);
            }
            if (count > 0) {
                for (std::size_t idx = 0; idx < count; ++idx) {
                    DecodedEvent& decoded = FPGA_events[idx];
                    FPGAEventDesc& event = decoded.event;
                    const bool is_first_event =
                        ((mask.first_event_mask & (1U << idx)) != 0U);
                    const uint64_t decode_time_ns = decoded.captured_time_ns;

                    if (strategy0.evaluateEvent(event, intent, 0)) {
                       const uint64_t strategy_time_ns = readSystemTimeNs(is_first_event);
                        // if (!is_first_event) {
                        //     intent.event_ts = 0;
                        // }
                        while (!executor.acceptIntent(0, intent)) {
                            std::this_thread::yield();
                        }
                        if (is_first_event) {
                            latency_tracker.pushRecord(TimeRecord {
                                .que_idx = 0,
                                .event_ts = event.event_tk,
                                .event_stage = stage::FRAME_START,
                                .time_captured = event.frame_start_tk,
                            });
                            latency_tracker.pushRecord(TimeRecord {
                                .que_idx = 0,
                                .event_ts = event.event_tk,
                                .event_stage = stage::DMA_EMIT,
                                .time_captured = event.event_tk,
                            });
                            latency_tracker.pushRecord(TimeRecord {
                                .que_idx = 0,
                                .event_ts = event.event_tk,
                                .event_stage = stage::DECODE,
                                .time_captured = decode_time_ns,
                            });
                            latency_tracker.pushRecord(TimeRecord {
                                .que_idx = 0,
                                .event_ts = event.event_tk,
                                .event_stage = stage::STRATEGY,
                                .time_captured = strategy_time_ns,
                            });
                        }
                    }
                }
            }
            if (count == 0) {
                std::this_thread::yield();
            }
        }
    });
    rx_threads.emplace_back([&]() {//rx_engine1
        FirstEventMask mask {};
        DecodedEvent FPGA_events[MAX_POLL_RECORDS] {};
        OrderIntent intent {};
        while (true) {
            const std::size_t count = rx_engine1.pollDecodedBatch(mask, MAX_POLL_RECORDS, FPGA_events);
            if (count > 0) {
                for (std::size_t idx = 0; idx < count; ++idx) {
                    DecodedEvent& decoded = FPGA_events[idx];
                    FPGAEventDesc& event = decoded.event;
                    const bool is_first_event =
                        ((mask.first_event_mask & (1U << idx)) != 0U);
                    const uint64_t decode_time_ns = decoded.captured_time_ns;

                    if (strategy1.evaluateEvent(event, intent, 1)) {
                        // if (!is_first_event) {
                        //     intent.event_ts = 0;
                        // }
                        const uint64_t strategy_time_ns =readSystemTimeNs(is_first_event);
                        while (!executor.acceptIntent(1, intent)) {
                            std::this_thread::yield();
                        }
                        if (is_first_event) {
                            latency_tracker.pushRecord(TimeRecord {
                                .que_idx = 1,
                                .event_ts = event.event_tk,
                                .event_stage = stage::FRAME_START,
                                .time_captured = event.frame_start_tk,
                            });
                            latency_tracker.pushRecord(TimeRecord {
                                .que_idx = 1,
                                .event_ts = event.event_tk,
                                .event_stage = stage::DMA_EMIT,
                                .time_captured = event.event_tk,
                            });
                            latency_tracker.pushRecord(TimeRecord {
                                .que_idx = 1,
                                .event_ts = event.event_tk,
                                .event_stage = stage::DECODE,
                                .time_captured = decode_time_ns,
                            });
                            latency_tracker.pushRecord(TimeRecord {
                                .que_idx = 1,
                                .event_ts = event.event_tk,
                                .event_stage = stage::STRATEGY,
                                .time_captured = strategy_time_ns,
                            });
                        }
                    }
                }
            }
            if (count == 0) {
                std::this_thread::yield();
            }
        }
    });

    std::thread executor_thread([&]() 
    {
        while (true) {
            bool did_work = false;
            OrderIntent intent {};
            while (executor.popReadyIntent(intent)) {
                const bool tracked_latency = (intent.event_ts != 0);
                const uint64_t executor_time_ns =readSystemTimeNs(true);

                while (true &&
                       !tx_translator.acceptIntent(intent)) {
                    std::this_thread::yield();
                }
                if (!true) {
                    break;
                }
                if (tracked_latency) {
                latency_tracker.pushRecord(TimeRecord {
                    .que_idx = intent.que_idx,
                    .event_ts = intent.event_ts,
                    .event_stage = stage::EXECUTOR,
                    .time_captured = executor_time_ns,
                });
                }
                executor.logExecution(intent);
                did_work = true;
            }
            if (!did_work) {
                std::this_thread::yield();
            }
        }

        OrderIntent intent {};
        while (executor.popReadyIntent(intent)) {
            const bool tracked_latency = (intent.event_ts != 0);
            const uint64_t executor_time_ns =readSystemTimeNs(true);
            while (!tx_translator.acceptIntent(intent)) {
                std::this_thread::yield();
            }
            if (tracked_latency) {
                latency_tracker.pushRecord(TimeRecord {
                    .que_idx = intent.que_idx,
                    .event_ts = intent.event_ts,
                    .event_stage = stage::EXECUTOR,
                    .time_captured = executor_time_ns,
                });
            }
            executor.logExecution(intent);
        }
    });
    std::thread tx_thread([&]() {
        while (true) {
            bool did_work = false;
            did_work = tx_engine.pollConnectStep() || did_work;
            if (tx_engine.takeConnectEvent()) {
                tx_translator.onTransportConnected();
                did_work = true;
            }

            did_work = tx_translator.buildReadyOutboundFromAcceptedIntents() || did_work;
            did_work = tx_translator.queueHeartbeatIfDue() || did_work;

            TxOutboundRecord outbound {};
            while (tx_translator.popReadyOutbound(outbound)) {
                if (outbound.user_ref_num != 0 && outbound.event_ts != 0) {
                    latency_tracker.pushRecord(TimeRecord {
                        .que_idx = outbound.que_idx,
                        .event_ts = outbound.event_ts,
                        .event_stage = stage::TX_ENQUEUE,
                        .time_captured = readSystemTimeNs(true),
                    });
                }
                if (!tx_engine.sendOutboundRecord(outbound)) {
                    tx_translator.restoreReadyOutbound(outbound);
                    break;
                }
                if (outbound.user_ref_num != 0 && outbound.event_ts != 0) {
                    latency_tracker.pushRecord(TimeRecord {
                        .que_idx = outbound.que_idx,
                        .event_ts = outbound.event_ts,
                        .event_stage = stage::TX_SEND,
                        .time_captured = readSystemTimeNs(true),
                    });
                }
                did_work = true;
            }

            std::vector<uint8_t> payload {};
            while (tx_engine.pollInboundFrame(payload)) {
                tx_translator.acceptInboundPayload(payload);
                payload.clear();
                did_work = true;
            }
            if (tx_engine.takeDisconnectEvent()) {
                tx_translator.onTransportDisconnected();
                did_work = true;
            }

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
    if (tx_thread.joinable()) {
        tx_thread.join();
    }
    log_printer.stop();

    return 0;
}
