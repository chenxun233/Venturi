#include "../driver/fpga_dev.h"
#include "../engine/fpga_rx_engine.h"
#include "../validation/fpga_live_buffer_validator.h"
#include "../../common/log.h"

#include <array>
#include <span>
#include <string_view>
#include <thread>

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
constexpr std::array<std::string_view, kQueueCount> kFixturePaths = {
    "market_data/AAPL_13_B_payload_frames_hex.txt",
    "market_data/HSBC_3816_S_payload_frames_hex.txt",
};

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

    device.setSync(false);
    
    if (device.validateRxAll()) {
        info("FPGA RX queue validation passed");
    } else {
        error("FPGA RX queue validation failed");
        return 1;
    }


    FPGARxEngine engine0(device, 0);
    FPGARxEngine engine1(device, 1);

    std::thread queue0_thread([&engine0]() {
        FpgaQueueLiveValidator validator(0,
                                         std::string(kSymbolNames[0]),
                                         kStockLocates[0],
                                         std::string(kFixturePaths[0]));
        if (!validator.loadExpectedEvents()) {
            error("Queue 0 live validation fixture load failed");
            return;
        }

        while (!validator.isComplete()) {
            engine0.poll();
            const std::size_t record_count = engine0.readLastRecordCount();
            if (record_count == 0) {
                std::this_thread::yield();
                continue;
            }

            const auto& event_buffer = engine0.readEventBuffer();
            if (!validator.validateBatch(std::span<const FPGAEventDesc>(event_buffer.data(), record_count))) {
                error("Queue 0 live validation failed");
                return;
            }
        }
    });

    std::thread queue1_thread([&engine1]() {
        FpgaQueueLiveValidator validator(1,
                                         std::string(kSymbolNames[1]),
                                         kStockLocates[1],
                                         std::string(kFixturePaths[1]));
        if (!validator.loadExpectedEvents()) {
            error("Queue 1 live validation fixture load failed");
            return;
        }

        while (!validator.isComplete()) {
            engine1.poll();
            const std::size_t record_count = engine1.readLastRecordCount();
            if (record_count == 0) {
                std::this_thread::yield();
                continue;
            }

            const auto& event_buffer = engine1.readEventBuffer();
            if (!validator.validateBatch(std::span<const FPGAEventDesc>(event_buffer.data(), record_count))) {
                error("Queue 1 live validation failed");
                return;
            }
        }
    });

    queue0_thread.join();
    queue1_thread.join();

    return 0;
}
