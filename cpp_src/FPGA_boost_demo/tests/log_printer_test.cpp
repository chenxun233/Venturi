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
static_assert(std::is_same_v<decltype(ExecutionLogRecord{}.queue_idx), uint16_t>);

TEST(LogPrinterTest, executionLogsRouteToIndependentQueues) {
    LogPrinter printer(2, 1);

    ExecutionLogRecord q0 {
        .queue_idx = 0,
        .stock_locate = 0x0ee8,
        .intent = {.action = OrderIntentAction::Buy, .price = 100100, .shares = 10},
    };
    ExecutionLogRecord q1 {
        .queue_idx = 1,
        .stock_locate = 0x0ee9,
        .intent = {.action = OrderIntentAction::Sell, .price = 100200, .shares = 20},
    };

    EXPECT_TRUE(printer.pushExecutionLog(q0));
    EXPECT_TRUE(printer.pushExecutionLog(q1));
}

TEST(LogPrinterTest, txLogsRouteToIndependentQueues) {
    LogPrinter printer(2, 1);

    TxLogRecord q0 {
        .queue_idx = 0,
        .event = TxEventKind::ConnectionEstablished,
    };
    TxLogRecord q1 {
        .queue_idx = 1,
        .event = TxEventKind::ConnectionLost,
    };

    EXPECT_TRUE(printer.pushTxLog(q0));
    EXPECT_TRUE(printer.pushTxLog(q1));
}

TEST(LogPrinterTest, txConnectionIssuesPrintReadableLabel) {
    LogPrinter printer(1, 8);

    TxLogRecord issue {
        .queue_idx = 0,
        .event = TxEventKind::ConnectionIssue,
        .reason = 0x006f,
    };

    testing::internal::CaptureStdout();
    printer.start();
    EXPECT_TRUE(printer.pushTxLog(issue));
    printer.stop();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("TxEvent event=ConnectionIssue"), std::string::npos);
    EXPECT_NE(output.find("reason=0x006f"), std::string::npos);
}

TEST(LogPrinterTest, latencySummaryPrintsFinalPercentilesPerQueue) {
    LogPrinter printer(2, 8);

    LatencyLogRecord q0_a {
        .que_idx = 0,
        .event_tag = 11,
        .frame_start_to_dma_emit_ns = 10,
        .batch_duration_ns = 100,
        .batch_end_to_strategy_start_ns = 200,
        .strategy_start_to_tx_execution_accepted_ns = 300,
        .tx_execution_accepted_to_tx_enqueue_ns = 400,
        .tx_enqueue_to_tx_send_enter_ns = 500,
        .tx_send_enter_to_tx_send_syscall_enter_ns = 600,
        .tx_send_syscall_enter_to_tx_send_ns = 700,
    };
    LatencyLogRecord q0_b = q0_a;
    q0_b.event_tag = 12;
    q0_b.frame_start_to_dma_emit_ns = 30;
    q0_b.batch_duration_ns = 300;
    q0_b.batch_end_to_strategy_start_ns = 400;
    q0_b.strategy_start_to_tx_execution_accepted_ns = 500;
    q0_b.tx_execution_accepted_to_tx_enqueue_ns = 600;
    q0_b.tx_enqueue_to_tx_send_enter_ns = 700;
    q0_b.tx_send_enter_to_tx_send_syscall_enter_ns = 800;
    q0_b.tx_send_syscall_enter_to_tx_send_ns = 900;

    LatencyLogRecord q1_a = q0_a;
    q1_a.que_idx = 1;
    q1_a.event_tag = 21;
    q1_a.frame_start_to_dma_emit_ns = 20;
    q1_a.batch_duration_ns = 120;
    q1_a.batch_end_to_strategy_start_ns = 220;
    q1_a.strategy_start_to_tx_execution_accepted_ns = 320;
    q1_a.tx_execution_accepted_to_tx_enqueue_ns = 420;
    q1_a.tx_enqueue_to_tx_send_enter_ns = 520;
    q1_a.tx_send_enter_to_tx_send_syscall_enter_ns = 620;
    q1_a.tx_send_syscall_enter_to_tx_send_ns = 720;

    testing::internal::CaptureStdout();
    printer.start();
    EXPECT_TRUE(printer.pushLatencyLog(q0_a));
    EXPECT_TRUE(printer.pushLatencyLog(q0_b));
    EXPECT_TRUE(printer.pushLatencyLog(q1_a));
    printer.stop();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output.find("event_tag="), std::string::npos);
    EXPECT_NE(output.find("LatencySummary queue=0"), std::string::npos);
    EXPECT_NE(output.find("LatencySummary queue=1"), std::string::npos);
    EXPECT_NE(output.find("frame_start -> dma_emit_ns"), std::string::npos);
    EXPECT_NE(output.find("count="), std::string::npos);
    EXPECT_NE(output.find("min="), std::string::npos);
    EXPECT_NE(output.find("p50="), std::string::npos);
    EXPECT_NE(output.find("p99="), std::string::npos);
    EXPECT_NE(output.find("max="), std::string::npos);
}

TEST(LogPrinterTest, warmupRecordsAreIgnoredBeforeFinalSummary) {
    LogPrinter printer(1, 8);
    printer.setLatencyWarmupRecords(1);

    LatencyLogRecord warmup {
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
    };

    LatencyLogRecord measured = warmup;
    measured.event_tag = 2;
    measured.frame_start_to_dma_emit_ns = 50;
    measured.batch_duration_ns = 150;
    measured.batch_end_to_strategy_start_ns = 250;
    measured.strategy_start_to_tx_execution_accepted_ns = 350;
    measured.tx_execution_accepted_to_tx_enqueue_ns = 450;
    measured.tx_enqueue_to_tx_send_enter_ns = 550;
    measured.tx_send_enter_to_tx_send_syscall_enter_ns = 650;
    measured.tx_send_syscall_enter_to_tx_send_ns = 750;

    testing::internal::CaptureStdout();
    printer.start();
    EXPECT_TRUE(printer.pushLatencyLog(warmup));
    EXPECT_TRUE(printer.pushLatencyLog(measured));
    printer.stop();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("LatencySummary queue=0"), std::string::npos);
    EXPECT_NE(output.find("count=1"), std::string::npos);
    EXPECT_NE(output.find("min=50"), std::string::npos);
    EXPECT_NE(output.find("p50=50"), std::string::npos);
    EXPECT_NE(output.find("p99=50"), std::string::npos);
    EXPECT_NE(output.find("max=50"), std::string::npos);
}
