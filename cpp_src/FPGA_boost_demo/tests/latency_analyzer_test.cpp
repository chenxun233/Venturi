#include "../latency/latency_analyzer.h"

#include <gtest/gtest.h>

#include <string>

TEST(LatencyAnalyzerTest, warmupDropsLeadingRecordsPerQueue) {
    LatencyAnalyzer analyzer(1);
    analyzer.setWarmupRecords(1);

    analyzer.pushCompletedRecord(LatencyLogRecord {
        .que_idx = 0,
        .event_tag = 1,
        .FRAME_START_to_DMA_EMIT = 10,
        .BATCH_DURATION = 100,
        .BATCH_END_to_STRATEGY_START = 200,
        .STRATEGY_START_to_TX_SEND_ACCEPTED = 300,
        .TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE = 400,
        .TX_SEND_ENQUEUE_to_TX_SEND_ENTER = 500,
        .TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER = 600,
        .TX_SEND_SYSCALL_ENTER_to_TX_SEND = 700,
    });
    analyzer.pushCompletedRecord(LatencyLogRecord {
        .que_idx = 0,
        .event_tag = 2,
        .FRAME_START_to_DMA_EMIT = 20,
        .BATCH_DURATION = 120,
        .BATCH_END_to_STRATEGY_START = 220,
        .STRATEGY_START_to_TX_SEND_ACCEPTED = 320,
        .TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE = 420,
        .TX_SEND_ENQUEUE_to_TX_SEND_ENTER = 520,
        .TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER = 620,
        .TX_SEND_SYSCALL_ENTER_to_TX_SEND = 720,
    });

    testing::internal::CaptureStdout();
    analyzer.printSummary();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("completed_records=2"), std::string::npos);
    EXPECT_NE(output.find("warmup=1"), std::string::npos);
    EXPECT_NE(output.find("analyzed=1"), std::string::npos);
    EXPECT_NE(output.find("FRAME_START_to_DMA_EMIT"), std::string::npos);
}
