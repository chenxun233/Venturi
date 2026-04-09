#include <gtest/gtest.h>

#include "../common/shared_types.h"
#include "../latency/log_printer.h"

#include <string>
#include <type_traits>

static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushLatency)>);
static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushSnapshot)>);
static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushRegressionStatus)>);
static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushExecution)>);
static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushTxEvent)>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.decode_to_strategy_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.strategy_to_executor_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.executor_to_tx_enqueue_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_enqueue_to_tx_send_ns), int64_t>);

TEST(LogPrinterTest, regressionStatusRecordCarriesConvertedSlope) {
    RegressionStatusLogRecord record {
        .has_para = true,
        .a_ns_per_tick = 8.125,
    };

    EXPECT_TRUE(record.has_para);
    EXPECT_DOUBLE_EQ(record.a_ns_per_tick, 8.125);
}

TEST(LogPrinterTest, latencyRecordPrintsSignedDownstreamDeltas) {
    LogPrinter printer(4);
    LatencyLogRecord record {
        .que_idx = 3,
        .event_ts = 42,
        .frame_start_to_dma_emit_ns = 17,
        .dma_emit_to_decode_ns = -5,
        .decode_to_strategy_ns = -11,
        .strategy_to_executor_ns = 13,
        .executor_to_tx_enqueue_ns = -19,
        .tx_enqueue_to_tx_send_ns = 23,
    };

    testing::internal::CaptureStdout();
    printer.start();
    EXPECT_TRUE(printer.pushLatency(record));
    printer.stop();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("dma_emit_to_decode_ns=-5"), std::string::npos);
    EXPECT_NE(output.find("decode_to_strategy_ns=-11"), std::string::npos);
    EXPECT_NE(output.find("strategy_to_executor_ns=13"), std::string::npos);
    EXPECT_NE(output.find("executor_to_tx_enqueue_ns=-19"), std::string::npos);
    EXPECT_NE(output.find("tx_enqueue_to_tx_send_ns=23"), std::string::npos);
}
