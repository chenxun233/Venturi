#include <gtest/gtest.h>

#include "../common/shared_types.h"
#include "../latency/log_printer.h"

#include <string>
#include <type_traits>

static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::pushTxLog)>);
static_assert(std::is_member_function_pointer_v<decltype(&LogPrinter::setWorkerCpu)>);
static_assert(std::is_same_v<decltype(TxLogRecord{}.que_idx), uint16_t>);
static_assert(std::is_same_v<decltype(TxLogRecord{}.event), TxEventKind>);
static_assert(sizeof(TxLogRecord) <= 4);

TEST(LogPrinterTest, txLogsRouteToIndependentQueues) {
    LogPrinter printer(2, 1);

    TxLogRecord q0 {
        .que_idx = 0,
        .event = TxEventKind::ConnectionEstablished,
    };
    TxLogRecord q1 {
        .que_idx = 1,
        .event = TxEventKind::ConnectionLost,
    };

    EXPECT_TRUE(printer.pushTxLog(q0));
    EXPECT_TRUE(printer.pushTxLog(q1));
}

TEST(LogPrinterTest, rejectsTxLogsForOutOfRangeQueue) {
    LogPrinter printer(1, 8);

    TxLogRecord invalid {
        .que_idx = 1,
        .event = TxEventKind::ConnectionEstablished,
    };

    EXPECT_FALSE(printer.pushTxLog(invalid));
}

TEST(LogPrinterTest, connectionEstablishedLogsPrintQueueAndEventOnly) {
    LogPrinter printer(1, 8);

    TxLogRecord event {
        .que_idx = 0,
        .event = TxEventKind::ConnectionEstablished,
    };

    testing::internal::CaptureStdout();
    printer.start();
    EXPECT_TRUE(printer.pushTxLog(event));
    printer.stop();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "TxEvent queue=0 event=ConnectionEstablished\n");
}

TEST(LogPrinterTest, connectionLostLogsPrintQueueAndEventOnly) {
    LogPrinter printer(1, 8);

    TxLogRecord event {
        .que_idx = 0,
        .event = TxEventKind::ConnectionLost,
    };

    testing::internal::CaptureStdout();
    printer.start();
    EXPECT_TRUE(printer.pushTxLog(event));
    printer.stop();
    const std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(output, "TxEvent queue=0 event=ConnectionLost\n");
}
