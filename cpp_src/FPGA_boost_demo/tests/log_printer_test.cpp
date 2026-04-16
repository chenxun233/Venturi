#include <gtest/gtest.h>

#include "../common/shared_types.h"
#include "../latency/log_printer.h"

#include <cstdio>
#include <string>
#include <type_traits>

static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushLatencyLog)>);
static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushExecutionLog)>);
static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushTxLog)>);
static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::setWorkerCpu)>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.frame_start_to_dma_emit_ns), uint64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.batch_duration_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.batch_end_to_strategy_start_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.strategy_start_to_tx_execution_accepted_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_execution_accepted_to_tx_enqueue_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_enqueue_to_tx_send_enter_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_send_enter_to_tx_send_syscall_enter_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_send_syscall_enter_to_tx_send_ns), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_enqueue_backlog_depth), uint32_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_send_enter_backlog_depth), uint32_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_send_call_count), uint32_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_send_bytes_total), uint32_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_send_eintr_retry_count), uint32_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.tx_send_had_partial_write), uint32_t>);

TEST(LogPrinterTest, latencyRecordPrintsRenamedLatencyFields) {
    LogPrinter printer(4, 4);
    LatencyLogRecord record {
        .que_idx = 3,
        .event_tag = 42,
        .frame_start_to_dma_emit_ns = 17,
        .batch_duration_ns = -5,
        .batch_end_to_strategy_start_ns = -11,
        .strategy_start_to_tx_execution_accepted_ns = 13,
        .tx_execution_accepted_to_tx_enqueue_ns = -19,
        .tx_enqueue_to_tx_send_enter_ns = 23,
        .tx_send_enter_to_tx_send_syscall_enter_ns = -29,
        .tx_send_syscall_enter_to_tx_send_ns = 31,
        .tx_enqueue_backlog_depth = 7,
        .tx_send_enter_backlog_depth = 3,
        .tx_send_call_count = 2,
        .tx_send_bytes_total = 64,
        .tx_send_eintr_retry_count = 1,
        .tx_send_had_partial_write = 1,
    };

    testing::internal::CaptureStdout();
    printer.start();
    EXPECT_TRUE(printer.pushLatencyLog(record));
    printer.stop();
    const std::string output = testing::internal::GetCapturedStdout();

    auto formatSignedLine = [](const char* label, long long value) {
        char line[128];
        std::snprintf(line, sizeof(line), "%-48s = %lld\n", label, value);
        return std::string(line);
    };
    auto formatUnsignedLine = [](const char* label, unsigned long long value) {
        char line[128];
        std::snprintf(line, sizeof(line), "%-48s = %llu\n", label, value);
        return std::string(line);
    };

    std::string expected = "\nLatencyNs[NEG] queue=3 event_tag=42\n";
    expected += formatUnsignedLine("frame_start -> dma_emit_ns", 17ULL);
    expected += formatSignedLine("batch_duration_ns", -5LL);
    expected += formatSignedLine("batch_end -> strategy_start_ns", -11LL);
    expected += formatSignedLine("strategy_start -> tx_execution_accepted_ns", 13LL);
    expected += formatSignedLine("tx_execution_accepted -> tx_enqueue_ns", -19LL);
    expected += formatSignedLine("tx_enqueue -> tx_send_enter_ns", 23LL);
    expected += formatSignedLine("tx_send_enter -> tx_send_syscall_enter_ns", -29LL);
    expected += formatSignedLine("tx_send_syscall_enter -> tx_send_ns", 31LL);
    expected += formatUnsignedLine("tx_enqueue_backlog_depth", 7ULL);
    expected += formatUnsignedLine("tx_send_enter_backlog_depth", 3ULL);
    expected += formatUnsignedLine("tx_send_call_count", 2ULL);
    expected += formatUnsignedLine("tx_send_bytes_total", 64ULL);
    expected += formatUnsignedLine("tx_send_eintr_retry_count", 1ULL);
    expected += formatUnsignedLine("tx_send_had_partial_write", 1ULL);

    EXPECT_EQ(output, expected);
    EXPECT_EQ(output.find("tx_enqueue -> tx_send_ns"), std::string::npos);
    EXPECT_EQ(output.find("strategy_to_executor_ns="), std::string::npos);
    EXPECT_EQ(output.find("executor_to_execution_dequeue_ns="), std::string::npos);
    EXPECT_EQ(output.find("executor_to_tx_enqueue_ns"), std::string::npos);
}
