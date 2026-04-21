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
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.FRAME_START_to_DMA_EMIT), uint64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.BATCH_DURATION), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.BATCH_END_to_STRATEGY_START), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.STRATEGY_START_to_TX_SEND_ACCEPTED), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.TX_SEND_ENQUEUE_to_TX_SEND_ENTER), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER), int64_t>);
static_assert(std::is_same_v<decltype(LatencyLogRecord{}.TX_SEND_SYSCALL_ENTER_to_TX_SEND), int64_t>);
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
        .FRAME_START_to_DMA_EMIT = 10,
        .BATCH_DURATION = 100,
        .BATCH_END_to_STRATEGY_START = 200,
        .STRATEGY_START_to_TX_SEND_ACCEPTED = 300,
        .TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE = 400,
        .TX_SEND_ENQUEUE_to_TX_SEND_ENTER = 500,
        .TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER = 600,
        .TX_SEND_SYSCALL_ENTER_to_TX_SEND = 700,
    };
    LatencyLogRecord q0_b = q0_a;
    q0_b.event_tag = 12;
    q0_b.FRAME_START_to_DMA_EMIT = 30;
    q0_b.BATCH_DURATION = 300;
    q0_b.BATCH_END_to_STRATEGY_START = 400;
    q0_b.STRATEGY_START_to_TX_SEND_ACCEPTED = 500;
    q0_b.TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE = 600;
    q0_b.TX_SEND_ENQUEUE_to_TX_SEND_ENTER = 700;
    q0_b.TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER = 800;
    q0_b.TX_SEND_SYSCALL_ENTER_to_TX_SEND = 900;

    LatencyLogRecord q1_a = q0_a;
    q1_a.que_idx = 1;
    q1_a.event_tag = 21;
    q1_a.FRAME_START_to_DMA_EMIT = 20;
    q1_a.BATCH_DURATION = 120;
    q1_a.BATCH_END_to_STRATEGY_START = 220;
    q1_a.STRATEGY_START_to_TX_SEND_ACCEPTED = 320;
    q1_a.TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE = 420;
    q1_a.TX_SEND_ENQUEUE_to_TX_SEND_ENTER = 520;
    q1_a.TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER = 620;
    q1_a.TX_SEND_SYSCALL_ENTER_to_TX_SEND = 720;

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
    EXPECT_NE(output.find("FRAME_START_to_DMA_EMIT"), std::string::npos);
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
        .FRAME_START_to_DMA_EMIT = 10,
        .BATCH_DURATION = 100,
        .BATCH_END_to_STRATEGY_START = 200,
        .STRATEGY_START_to_TX_SEND_ACCEPTED = 300,
        .TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE = 400,
        .TX_SEND_ENQUEUE_to_TX_SEND_ENTER = 500,
        .TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER = 600,
        .TX_SEND_SYSCALL_ENTER_to_TX_SEND = 700,
    };

    LatencyLogRecord measured = warmup;
    measured.event_tag = 2;
    measured.FRAME_START_to_DMA_EMIT = 50;
    measured.BATCH_DURATION = 150;
    measured.BATCH_END_to_STRATEGY_START = 250;
    measured.STRATEGY_START_to_TX_SEND_ACCEPTED = 350;
    measured.TX_SEND_ACCEPTED_to_TX_SEND_ENQUEUE = 450;
    measured.TX_SEND_ENQUEUE_to_TX_SEND_ENTER = 550;
    measured.TX_SEND_ENTER_to_TX_SEND_SYSCALL_ENTER = 650;
    measured.TX_SEND_SYSCALL_ENTER_to_TX_SEND = 750;

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
