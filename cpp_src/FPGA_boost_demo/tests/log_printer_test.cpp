#include <gtest/gtest.h>

#include "../common/shared_types.h"
#include "../latency/log_printer.h"

#include <string>
#include <type_traits>

static_assert(std::is_same_v<decltype(LatencyLogRecord{}.batch_end_to_strategy_start_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.strategy_start_to_tx_execution_accepted_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_execution_accepted_to_tx_execution_dequeue_ns), int64_t>);

TEST(LogPrinterTest, latencyRecordPrintsRenamedLatencyFields) {
    LogPrinter printer(4, 4);
    LatencyLogRecord record {
        .que_idx = 3,
        .event_tag = 42,
        .frame_start_to_dma_emit_ns = 17,
        .batch_duration_ns = -5,
        .batch_end_to_strategy_start_ns = -11,
        .strategy_start_to_tx_execution_accepted_ns = 13,
        .tx_execution_accepted_to_tx_execution_dequeue_ns = -19,
        .tx_execution_dequeue_to_tx_order_frame_built_ns = 41,
        .tx_order_frame_built_to_tx_pending_recorded_ns = 101,
        .tx_pending_recorded_to_tx_enqueue_ns = 47,
        .tx_enqueue_to_tx_send_ns = 23,
    };

    testing::internal::CaptureStdout();
    printer.start();
    EXPECT_TRUE(printer.pushLatencyLog(record));
    printer.stop();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("batch_end_to_strategy_start_ns=-11"), std::string::npos);
    EXPECT_NE(output.find("strategy_start_to_tx_execution_accepted_ns=13"), std::string::npos);
    EXPECT_NE(output.find("tx_execution_accepted_to_tx_execution_dequeue_ns=-19"), std::string::npos);
    EXPECT_EQ(output.find("strategy_to_executor_ns="), std::string::npos);
    EXPECT_EQ(output.find("executor_to_execution_dequeue_ns="), std::string::npos);
    EXPECT_EQ(output.find("executor_to_tx_enqueue_ns="), std::string::npos);
}
