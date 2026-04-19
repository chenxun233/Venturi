#include "../latency/latency_analyzer.h"

#include <gtest/gtest.h>

#include <string>

TEST(LatencyAnalyzerTest, warmupDropsLeadingRecordsPerQueue) {
    LatencyAnalyzer analyzer(1);
    analyzer.setWarmupRecords(1);

    analyzer.pushCompletedRecord(LatencyLogRecord {
        .que_idx = 0,
        .event_tag = 1,
        .frame_start_to_dma_emit_ns = 10,
        .batch_duration_ns = 100,
        .batch_end_to_strategy_start_ns = 200,
        .strategy_start_to_tx_execution_accepted_ns = 300,
        .tx_execution_accepted_to_tx_enqueue_ns = 400,
        .tx_enqueue_to_tx_send_enter_ns = 500,
        .tx_send_enter_to_tx_send_syscall_enter_ns = 600,
        .tx_send_syscall_enter_to_tx_send_ns = 700,
    });
    analyzer.pushCompletedRecord(LatencyLogRecord {
        .que_idx = 0,
        .event_tag = 2,
        .frame_start_to_dma_emit_ns = 20,
        .batch_duration_ns = 120,
        .batch_end_to_strategy_start_ns = 220,
        .strategy_start_to_tx_execution_accepted_ns = 320,
        .tx_execution_accepted_to_tx_enqueue_ns = 420,
        .tx_enqueue_to_tx_send_enter_ns = 520,
        .tx_send_enter_to_tx_send_syscall_enter_ns = 620,
        .tx_send_syscall_enter_to_tx_send_ns = 720,
    });

    testing::internal::CaptureStdout();
    analyzer.printSummary();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("completed_records=2"), std::string::npos);
    EXPECT_NE(output.find("warmup=1"), std::string::npos);
    EXPECT_NE(output.find("analyzed=1"), std::string::npos);
    EXPECT_NE(output.find("frame_start_to_dma_emit_ns"), std::string::npos);
}
