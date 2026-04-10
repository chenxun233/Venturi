#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"
#include "../sync/regression.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

namespace {

TimeRecord makeRecord(uint16_t que_idx, uint64_t event_ts, stage event_stage, uint64_t time_captured) {
    return TimeRecord {
        .que_idx = que_idx,
        .event_ts = event_ts,
        .event_stage = event_stage,
        .time_captured = time_captured,
    };
}

} // namespace

TEST(LatencyTrackerTest, emitsCompleteChainOnlyAfterTxSend) {
    Regression regression;
    regression.updateSnapshot(FpgaSyncSnapshot {
        .fpga_tick = 100ULL,
        .host_time_ns = 1000ULL,
        .interval_ns = 1ULL,
    });
    regression.updateSnapshot(FpgaSyncSnapshot {
        .fpga_tick = 200ULL,
        .host_time_ns = 2000ULL,
        .interval_ns = 1ULL,
    });

    LatencyTracker tracker(1, 8);
    tracker.attachRegression(&regression);

    LogPrinter printer(8);
    printer.start();
    tracker.attachLogPrinter(&printer);

    const uint16_t que_idx = 0;
    const uint64_t event_ts = 77ULL;

    EXPECT_TRUE(tracker.pushRecord(makeRecord(que_idx, event_ts, stage::FRAME_START, 150ULL)));
    EXPECT_EQ(tracker.run(), 1U);

    EXPECT_TRUE(tracker.pushRecord(makeRecord(que_idx, event_ts, stage::DMA_EMIT, 151ULL)));
    EXPECT_EQ(tracker.run(), 1U);

    EXPECT_TRUE(tracker.pushRecord(makeRecord(que_idx, event_ts, stage::DECODE, 1520ULL)));
    EXPECT_EQ(tracker.run(), 1U);

    EXPECT_TRUE(tracker.pushRecord(makeRecord(que_idx, event_ts, stage::STRATEGY, 1530ULL)));
    EXPECT_EQ(tracker.run(), 1U);

    EXPECT_TRUE(tracker.pushRecord(makeRecord(que_idx, event_ts, stage::EXECUTOR, 1540ULL)));
    EXPECT_EQ(tracker.run(), 1U);

    EXPECT_TRUE(tracker.pushRecord(makeRecord(que_idx, event_ts, stage::TX_ENQUEUE, 1550ULL)));
    EXPECT_EQ(tracker.run(), 1U);

    testing::internal::CaptureStdout();
    printer.stop();
    const std::string pre_tx_send_output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(pre_tx_send_output.empty());

    printer.start();
    testing::internal::CaptureStdout();
    EXPECT_TRUE(tracker.pushRecord(makeRecord(que_idx, event_ts, stage::TX_SEND, 1560ULL)));
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
