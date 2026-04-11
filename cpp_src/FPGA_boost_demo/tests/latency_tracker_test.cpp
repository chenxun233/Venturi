#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"
#include "../sync/FPGA_regression.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <cmath>
#include <string>
#include <vector>

namespace {

TimeRecord makeRecord(uint16_t que_idx, uint64_t event_ts, stage event_stage, uint64_t time_captured) {
    return TimeRecord {
        .que_idx = que_idx,
        .event_ts = event_ts,
        .event_stage = event_stage,
        .time_captured = time_captured,
    };
}

constexpr uint64_t kOffsetNs = 0ULL;
constexpr long double kSlopeNsPerTick = 10.0L;
constexpr uint64_t kAcceptedIntervalNs = 2000ULL;

uint64_t readHostNs(uint64_t fpga_tick) {
    return static_cast<uint64_t>(
        std::llround(static_cast<long double>(kOffsetNs) +
                     static_cast<long double>(fpga_tick) * kSlopeNsPerTick));
}

struct SequencedSyncDevice {
    std::vector<FpgaSyncSnapshot> snapshots {};
    mutable std::size_t read_count {0};

    bool readSyncTimestamp(FpgaSyncSnapshot& snapshot) const {
        if (read_count >= snapshots.size()) {
            return false;
        }
        snapshot = snapshots[read_count];
        ++read_count;
        return true;
    }
};

void appendStableSnapshots(SequencedSyncDevice& device,
                           std::size_t count,
                           uint64_t first_tick = 1000ULL,
                           uint64_t tick_step = 200ULL) {
    device.snapshots.reserve(count);
    for (std::size_t idx = 0; idx < count; ++idx) {
        const uint64_t fpga_tick = first_tick + static_cast<uint64_t>(idx) * tick_step;
        device.snapshots.push_back(FpgaSyncSnapshot {
            .fpga_tick = fpga_tick,
            .host_time_ns = readHostNs(fpga_tick),
            .interval_ns = static_cast<uint64_t>(tick_step * kSlopeNsPerTick)
        });
    }
}

} // namespace

TEST(LatencyTrackerTest, emitsCompleteChainOnlyAfterTxSend) {
    FPGARegression regression;
    // Validate stage-assembled latency output using direct TimeRecord injection.
    SequencedSyncDevice device {};
    appendStableSnapshots(device, 256);
    ASSERT_TRUE(regression.initSync(device, device.snapshots.size(), kAcceptedIntervalNs));

    LatencyTracker tracker(1, 8);
    tracker.attachRegression(&regression);

    LogPrinter printer(8);
    testing::internal::CaptureStdout();
    printer.start();
    tracker.attachLogPrinter(&printer);

    const uint16_t que_idx = 0;
    const uint64_t base_tick = device.snapshots.back().fpga_tick;
    const uint64_t base_host = device.snapshots.back().host_time_ns;
    const uint64_t event_ts = base_tick + 150ULL;
    const uint64_t frame_start_tick = event_ts;
    const uint64_t dma_emit_tick = base_tick + 151ULL;
    const uint64_t decode_host_ns = base_host + 1520ULL;
    const uint64_t strategy_host_ns = base_host + 1530ULL;
    const uint64_t executor_host_ns = base_host + 1540ULL;
    const uint64_t tx_enqueue_host_ns = base_host + 1550ULL;
    const uint64_t tx_send_host_ns = base_host + 1560ULL;

    tracker.pushRecord(makeRecord(que_idx, event_ts, stage::FRAME_START, frame_start_tick));
    EXPECT_EQ(tracker.run(), 1U);

    tracker.pushRecord(makeRecord(que_idx, event_ts, stage::DMA_EMIT, dma_emit_tick));
    EXPECT_EQ(tracker.run(), 1U);

    tracker.pushRecord(makeRecord(que_idx, event_ts, stage::DECODE, decode_host_ns));
    EXPECT_EQ(tracker.run(), 1U);

    tracker.pushRecord(makeRecord(que_idx, event_ts, stage::STRATEGY, strategy_host_ns));
    EXPECT_EQ(tracker.run(), 1U);

    tracker.pushRecord(makeRecord(que_idx, event_ts, stage::EXECUTOR, executor_host_ns));
    EXPECT_EQ(tracker.run(), 1U);

    tracker.pushRecord(makeRecord(que_idx, event_ts, stage::TX_ENQUEUE, tx_enqueue_host_ns));
    EXPECT_EQ(tracker.run(), 1U);

    printer.stop();
    const std::string pre_tx_send_output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(pre_tx_send_output.empty());

    printer.start();
    testing::internal::CaptureStdout();
    tracker.pushRecord(makeRecord(que_idx, event_ts, stage::TX_SEND, tx_send_host_ns));
    EXPECT_EQ(tracker.run(), 1U);
    printer.stop();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("LatencyNs"), std::string::npos);
    EXPECT_NE(output.find("frame_start_to_dma_emit_ns=10"), std::string::npos);
    EXPECT_NE(output.find("dma_emit_to_decode_ns=10"), std::string::npos);
    EXPECT_NE(output.find("decode_to_strategy_ns=10"), std::string::npos);
    EXPECT_NE(output.find("strategy_to_executor_ns=10"), std::string::npos);
    EXPECT_NE(output.find("executor_to_tx_enqueue_ns=10"), std::string::npos);
    EXPECT_NE(output.find("tx_enqueue_to_tx_send_ns=10"), std::string::npos);
}
