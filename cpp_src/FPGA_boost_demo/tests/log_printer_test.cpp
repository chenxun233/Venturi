#include <gtest/gtest.h>

#define private public
#include "../latency/log_printer.h"
#undef private

#include "../common/shared_types.h"

#include <algorithm>
#include <array>
#include <string>
#include <type_traits>

template <typename T, typename = void>
struct HasRecordCv : std::false_type {};

template <typename T>
struct HasRecordCv<T, std::void_t<decltype(&T::m_record_cv)>> : std::true_type {};

template <typename T, typename = void>
struct HasWaitMutex : std::false_type {};

template <typename T>
struct HasWaitMutex<T, std::void_t<decltype(&T::m_wait_mutex)>> : std::true_type {};

static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushLatencyLog)>);
static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushExecutionLog)>);
static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushTxLog)>);
static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::setWorkerCpu)>);
static_assert(!HasRecordCv<LogPrinter>::value);
static_assert(!HasWaitMutex<LogPrinter>::value);
static_assert(std::is_same_v<decltype(TxLogRecord{}.queue_idx), uint16_t>);

TEST(LogPrinterTest, workerCpuCanBeConfiguredBeforeStart) {
    LogPrinter printer(1, 4);

    printer.setWorkerCpu(3);

    EXPECT_EQ(printer.m_worker_cpu, 3);
}

TEST(LogPrinterTest, latencyRecordPrintsBatchBoundaryMetrics) {
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

    EXPECT_NE(output.find("LatencyNs[NEG]"), std::string::npos);
    EXPECT_NE(output.find("queue=3"), std::string::npos);
    EXPECT_NE(output.find("event_tag=42"), std::string::npos);
    EXPECT_NE(output.find("frame_start_to_dma_emit_ns=17"), std::string::npos);
    EXPECT_NE(output.find("batch_duration_ns=-5"), std::string::npos);
    EXPECT_NE(output.find("batch_end_to_strategy_start_ns=-11"), std::string::npos);
    EXPECT_EQ(output.find("dma_emit_to_decode_ns="), std::string::npos);
    EXPECT_EQ(output.find("decode_to_strategy_ns="), std::string::npos);
    EXPECT_NE(output.find("strategy_start_to_tx_execution_accepted_ns=13"), std::string::npos);
    EXPECT_NE(output.find("tx_execution_accepted_to_tx_execution_dequeue_ns=-19"), std::string::npos);
    EXPECT_NE(output.find("tx_execution_dequeue_to_tx_order_frame_built_ns=41"), std::string::npos);
    EXPECT_NE(output.find("tx_order_frame_built_to_tx_pending_recorded_ns=101"), std::string::npos);
    EXPECT_NE(output.find("tx_pending_recorded_to_tx_enqueue_ns=47"), std::string::npos);
    EXPECT_EQ(output.find("order_frame_built_to_pending_capacity_handled_ns="), std::string::npos);
    EXPECT_EQ(output.find("pending_capacity_handled_to_pending_tag_recorded_ns="), std::string::npos);
    EXPECT_EQ(output.find("pending_tag_recorded_to_pending_recorded_ns="), std::string::npos);
    EXPECT_EQ(output.find("executor_to_execution_taken_ns="), std::string::npos);
    EXPECT_EQ(output.find("execution_taken_to_execution_accepted_ns="), std::string::npos);
    EXPECT_EQ(output.find("execution_accepted_to_execution_dequeue_ns="), std::string::npos);
    EXPECT_EQ(output.find("strategy_to_executor_ns="), std::string::npos);
    EXPECT_EQ(output.find("executor_to_execution_dequeue_ns="), std::string::npos);
    EXPECT_EQ(output.find("executor_to_tx_enqueue_ns="), std::string::npos);
    EXPECT_NE(output.find("tx_enqueue_to_tx_send_ns=23"), std::string::npos);
}

TEST(LogPrinterTest, TxLogPrintsFormattedTxSentence) {
    LogPrinter printer(2, 4);
    TxLogRecord record {
        .queue_idx = 1,
        .event = TxEventKind::OrderAccepted,
        .user_ref_num = 42,
        .stock_locate = 0x000d,
        .price = 1234,
        .shares = 10,
    };

    testing::internal::CaptureStdout();
    printer.start();
    EXPECT_TRUE(printer.pushTxLog(record));
    printer.stop();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("TxEvent event=OrderAccepted"), std::string::npos);
    EXPECT_NE(output.find("user_ref=42"), std::string::npos);
    EXPECT_NE(output.find("stock_locate=0x000d"), std::string::npos);
    EXPECT_NE(output.find("price=1234"), std::string::npos);
    EXPECT_NE(output.find("shares=10"), std::string::npos);
}

TEST(LogPrinterTest, ExecutionLogPrintsFormattedExecutionSentence) {
    LogPrinter printer(2, 4);
    OrderExecution execution {
        .stock_locate = 0x000d,
        .que_idx = 1,
        .event_tag = 7,
        .order = {.action = OrderIntentAction::Buy, .price = 1234, .shares = 10},
    };

    testing::internal::CaptureStdout();
    printer.start();
    EXPECT_TRUE(printer.pushExecutionLog(execution));
    printer.stop();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Execution action=BUY"), std::string::npos);
    EXPECT_NE(output.find("stock_locate=0x000d"), std::string::npos);
    EXPECT_NE(output.find("price=1234"), std::string::npos);
    EXPECT_NE(output.find("shares=10"), std::string::npos);
}

TEST(LogPrinterTest, fullTxLogQueueIncrementsDropCount) {
    LogPrinter printer(1, 1);
    TxLogRecord record {
        .queue_idx = 0,
        .event = TxEventKind::ConnectionEstablished,
    };

    EXPECT_TRUE(printer.pushTxLog(record));
    EXPECT_FALSE(printer.pushTxLog(record));
    EXPECT_EQ(printer.readDropCount(), 1U);
}
