#include "../driver/fpga_dev.h"
#include "../engine/fpga_rx_engine.h"
#include "../latency/latency_log_printer.h"
#include "../latency/latency_tracker.h"
#include "../runtime/top_runner.h"
#include "../sync/regression.h"
#include "../sync/sync_handler.h"
#include "../../common/log.h"

#include <array>
#include <chrono>
#include <string_view>


namespace {

constexpr uint16_t kQueueCount = 2;
constexpr uint32_t kRxSlotCount = 1024;
constexpr std::array<std::string_view, kQueueCount> kSymbolNames = {
    "AAPL",
    "HSBC",
};
constexpr std::array<uint16_t, kQueueCount> kStockLocates = {
    0x0ee8,
    0x000d,
    
    
    

};
constexpr std::array<uint32_t, kQueueCount> kPriceBases = {
    0U,
    0U,
};

constexpr bool kSyncEnabled = true;
constexpr auto kPrintInterval = std::chrono::seconds(1);
constexpr auto kControlLoopSleep = std::chrono::microseconds(100);
constexpr uint64_t kSyncPeriod =
    static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(kPrintInterval).count() /
                          kControlLoopSleep.count());
constexpr std::size_t kLatencyQueueCapacity = 1024;
constexpr std::size_t kLatencyLogCapacity = 4096;

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
    LatencyTracker latency_tracker(kQueueCount, kLatencyQueueCapacity);
    LatencyLogPrinter latency_log_printer(kLatencyLogCapacity);

    SyncHandler sync_handler(kSyncPeriod);
    Regression regression;

    latency_tracker.setPrintInterval(kPrintInterval);
    latency_tracker.attachLogPrinter(&latency_log_printer);
    latency_log_printer.attachDebugDevice(&device);
    latency_log_printer.start();
    engine0.attachLogPrinter(latency_log_printer);
    engine1.attachLogPrinter(latency_log_printer);
    engine0.attachRegression(regression);

    TopRunner runner(device);
    runner.addRxEngine(engine0, true);
    runner.addRxEngine(engine1, false);
    runner.addSyncController(sync_handler);
    runner.attachLatencyTracker(&latency_tracker);
    runner.attachRegression(&regression);
    runner.setControlLoopSleep(kControlLoopSleep);
    runner.run();
    latency_log_printer.stop();

    return 0;
}
