#include "../latency/latency_analyzer.h"

#include <gtest/gtest.h>

#include <string>

TEST(LatencyAnalyzerTest, warmupDropsLeadingRecordsPerQueue) {
    LatencyAnalyzer analyzer(1);
    analyzer.setWarmupRecords(1);
    analyzer.setTotalReceived(0, 5);

    analyzer.pushCompletedRecord(LatencyLogRecord {
        .que_idx = 0,
        .event_tag = 1,
        .FRAME_START_to_DMA_EMIT = 10,
        .BATCH_DURATION = 100,
        .BATCH_END_to_STRATEGY_START = 200,
        .STRATEGY_START_to_TX_SEND_ACCEPTED = 300,
        .TX_SEND_ACCEPTED_to_TX_SEND_SYSCALL_ENTER = 1000,
        .TX_SEND_SYSCALL_ENTER_to_TX_SEND = 700,
    });
    analyzer.pushCompletedRecord(LatencyLogRecord {
        .que_idx = 0,
        .event_tag = 2,
        .FRAME_START_to_DMA_EMIT = 20,
        .BATCH_DURATION = 120,
        .BATCH_END_to_STRATEGY_START = 220,
        .STRATEGY_START_to_TX_SEND_ACCEPTED = 320,
        .TX_SEND_ACCEPTED_to_TX_SEND_SYSCALL_ENTER = 1040,
        .TX_SEND_SYSCALL_ENTER_to_TX_SEND = 720,
    });

    testing::internal::CaptureStdout();
    analyzer.printSummary();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("total_received=5"), std::string::npos);
    EXPECT_NE(output.find("traced=2"), std::string::npos);
    EXPECT_NE(output.find("warmup=1"), std::string::npos);
    EXPECT_NE(output.find("analyzed=1"), std::string::npos);
    EXPECT_NE(output.find("FRAME_START_to_DMA_EMIT"), std::string::npos);
    EXPECT_NE(output.find("TX_SEND_ACCEPTED_to_TX_SEND_SYSCALL_ENTER"), std::string::npos);
    EXPECT_EQ(output.find("TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE"), std::string::npos);
    EXPECT_EQ(output.find("TX_SEND_ENQUEUE_to_TX_SEND_SYSCALL_ENTER"), std::string::npos);
    EXPECT_EQ(output.find("record_count="), std::string::npos);
}
