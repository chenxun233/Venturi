#include "../driver/fpga_dev.h"
#include "../engine/fpga_rx_engine.h"
#include "../latency/latency_log_printer.h"
#include "../latency/latency_tracker.h"
#include "../strategy/dummy_strategy.h"
#include "../sync/regression.h"
#include "../sync/sync_handler.h"
#include "../tx/executor.h"
#include "../tx/tx_engine.h"
#include "../../common/log.h"

#include <atomic>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <limits>
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
constexpr auto kSnapshotSampleInterval = std::chrono::milliseconds(10);
constexpr auto kSnapshotPrintInterval = std::chrono::seconds(1);
constexpr auto kControlLoopSleep = std::chrono::microseconds(100);
constexpr uint64_t kSnapshotSamplePeriod =
    static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(kSnapshotSampleInterval).count() /
                          kControlLoopSleep.count());
constexpr uint64_t kSnapshotPrintPeriod =
    static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(kSnapshotPrintInterval).count() /
                          kControlLoopSleep.count());
constexpr std::size_t kInitSyncWarmupAttempts = 32;
constexpr std::size_t kInitSyncMaxAttempts = 100000;
constexpr std::size_t kLatencyQueueCapacity = 1024;
constexpr std::size_t kLatencyLogCapacity = 4096;
constexpr std::size_t kExecutorQueueCapacity = 1024;
constexpr uint64_t      accepted_interval_ns = 2000;

bool initSync(FPGARxEngine& sync_engine,
              Regression& regression,
              FpgaSyncSnapshot& snapshot) {

    for (std::size_t attempt = 0; attempt < kInitSyncMaxAttempts; ++attempt) {
        snapshot = sync_engine.initSync();
        if (snapshot.interval_ns != 0 && snapshot.interval_ns <= accepted_interval_ns) {
            regression.updateSnapshot(snapshot);
        }
        if (regression.isFrozen()) {
            printf("initSync converged at attempt %zu \n", attempt + 1);
            return true;
        }
    }
    return false;
}


} // namespace

int main() {
    FPGADev device("0000:05:00.0");
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

    FPGARxEngine engine0(device, 0);
    FPGARxEngine engine1(device, 1);
    DummyStrategy strategy0;
    DummyStrategy strategy1;
    Executor executor(kQueueCount, kExecutorQueueCapacity);
    TxEngine tx_engine;
    LatencyTracker latency_tracker(kQueueCount, kLatencyQueueCapacity);
    LatencyLogPrinter latency_log_printer(kLatencyLogCapacity);

    SyncHandler sync_handler(kSnapshotSamplePeriod);
    Regression regression;


    latency_tracker.attachLogPrinter(&latency_log_printer);
    latency_log_printer.start();
    executor.attachLogPrinter(&latency_log_printer);
    executor.attachTx(&tx_engine);
    strategy0.attachExecutor(executor, 0);
    strategy1.attachExecutor(executor, 1);
    latency_tracker.attachRegression(&regression);
    engine0.attachLatencyTracker(latency_tracker);
    engine1.attachLatencyTracker(latency_tracker);
    engine0.attachRegression(regression);

    std::atomic<bool> running {true};
    CapSignal capture_signal {};
    std::mutex snapshot_mutex;
    FpgaSyncSnapshot sync_snapshot {};
    bool has_latest_snapshot {false};
    if (!initSync(engine0, regression, sync_snapshot)) {
        error("initSync failed to converge within %zu attempts", kInitSyncMaxAttempts);
        latency_log_printer.stop();
        return 1;
    }
    {
        const std::lock_guard<std::mutex> snapshot_lock(snapshot_mutex);
        sync_snapshot = regression.readSnapshot();
        has_latest_snapshot = true;
    }
    const RegressionPara regression_para = regression.returnParaSnapshot();
    if (regression_para.has_para) {
        const double a_ns_per_tick =
            static_cast<double>(regression_para.a_q32) / static_cast<double>(1ULL << 32);
        std::printf("a=%.9f\n", a_ns_per_tick);
        std::fflush(stdout);
    }

    std::thread control_thread([&]() {
        uint64_t print_countdown = kSnapshotPrintPeriod;
        while (running.load(std::memory_order_acquire)) {
            sync_handler.run(capture_signal);
            if (print_countdown > 0) {
                --print_countdown;
            }
            if (print_countdown == 0) {
                FpgaSyncSnapshot snapshot_to_print {};
                bool should_print = false;
                {
                    const std::lock_guard<std::mutex> snapshot_lock(snapshot_mutex);
                    if (has_latest_snapshot) {
                        snapshot_to_print = sync_snapshot;
                        should_print = true;
                    }
                }
                if (should_print) {
                    latency_log_printer.pushSnapshot(snapshot_to_print);
                }
                print_countdown = kSnapshotPrintPeriod;
            }

            std::this_thread::sleep_for(kControlLoopSleep);
        }
    });
    std::thread executor_thread([&]() {
        executor.run(running);
    });

    std::vector<std::thread> rx_threads;
    rx_threads.emplace_back([&]() {
        while (running.load(std::memory_order_acquire)) {
            const bool get_time =
                capture_signal.request.exchange(false, std::memory_order_acq_rel);
            FpgaSyncSnapshot snapshot {};
            const std::size_t count =
                engine0.pollBatchSync(MAX_POLL_RECORDS, get_time, snapshot);
            if (get_time && snapshot.interval_ns != 0) {
                const std::lock_guard<std::mutex> snapshot_lock(snapshot_mutex);
                sync_snapshot = regression.readSnapshot();
                has_latest_snapshot = true;
            }
            if (count > 0) {
                strategy0.onEvents(engine0.readEventBuffer().data(), count);
                latency_tracker.run();
            }
            if (count == 0) {
                std::this_thread::yield();
            }
        }
    });
    rx_threads.emplace_back([&]() {
        while (running.load(std::memory_order_acquire)) {
            const std::size_t count = engine1.pollBatch(MAX_POLL_RECORDS, false);
            if (count > 0) {
                strategy1.onEvents(engine1.readEventBuffer().data(), count);
                latency_tracker.run();
            }
            if (count == 0) {
                std::this_thread::yield();
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
    latency_log_printer.stop();

    return 0;
}
