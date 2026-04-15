#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"

#include <gtest/gtest.h>

#include <string>

namespace {

TimeRecord makeRecord(uint16_t que_idx, uint64_t event_tag, stage event_stage, uint64_t time_captured) {
    return TimeRecord {
        .que_idx = que_idx,
        .event_tag = event_tag,
        .event_stage = event_stage,
        .time_captured = time_captured,
    };
}

} // namespace

TEST(LatencyTrackerTest, emitsRenamedStageAndLatencyFields) {
    LatencyTracker tracker(1, 32);
    LogPrinter printer(1, 32);
    tracker.attachLogPrinter(&printer);

    const uint16_t que_idx = 0;
    const uint64_t event_tag = 1234ULL;
    const uint64_t frame_start_tick = 100ULL;
    const uint64_t dma_emit_tick = 110ULL;
    const uint64_t batch_start_ns = 1000ULL;
    const uint64_t batch_end_ns = 1300ULL;
    const uint64_t strategy_start_ns = 1400ULL;
    const uint64_t tx_execution_accepted_ns = 1500ULL;
    const uint64_t tx_execution_dequeue_ns = 1510ULL;
    const uint64_t tx_order_frame_built_ns = 1520ULL;
    const uint64_t tx_pending_recorded_ns = 1530ULL;
    const uint64_t tx_enqueue_ns = 1540ULL;
    const uint64_t tx_send_ns = 1550ULL;

    printer.start();
    testing::internal::CaptureStdout();
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::FRAME_START, frame_start_tick));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::DMA_EMIT, dma_emit_tick));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::BATCH_START, batch_start_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::BATCH_END, batch_end_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::STRATEGY_START, strategy_start_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_EXECUTION_ACCEPTED, tx_execution_accepted_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_EXECUTION_DEQUEUE, tx_execution_dequeue_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_ORDER_FRAME_BUILT, tx_order_frame_built_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_PENDING_RECORDED, tx_pending_recorded_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_ENQUEUE, tx_enqueue_ns));
    tracker.run();
    printer.stop();
    const std::string pre_tx_send_output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(pre_tx_send_output.empty());

    printer.start();
    testing::internal::CaptureStdout();
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_SEND, tx_send_ns));
    tracker.run();
    printer.stop();

    const std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("batch_end_to_strategy_start_ns=100"), std::string::npos);
    EXPECT_NE(output.find("strategy_start_to_tx_execution_accepted_ns=100"), std::string::npos);
    EXPECT_NE(output.find("tx_execution_accepted_to_tx_execution_dequeue_ns=10"), std::string::npos);
    EXPECT_NE(output.find("tx_execution_dequeue_to_tx_order_frame_built_ns=10"), std::string::npos);
    EXPECT_NE(output.find("tx_order_frame_built_to_tx_pending_recorded_ns=10"), std::string::npos);
    EXPECT_NE(output.find("tx_pending_recorded_to_tx_enqueue_ns=10"), std::string::npos);
    EXPECT_NE(output.find("tx_enqueue_to_tx_send_ns=10"), std::string::npos);
    EXPECT_EQ(output.find("executor_to_tx_enqueue_ns="), std::string::npos);
}
