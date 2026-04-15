#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"

#include <gtest/gtest.h>

#include <cerrno>
#include <cstdio>
#include <string>
#include <system_error>
#include <unistd.h>
#include <vector>

namespace {

TimeRecord makeRecord(uint16_t que_idx, uint64_t event_tag, stage event_stage, uint64_t time_captured) {
    return TimeRecord {
        .que_idx = que_idx,
        .event_tag = event_tag,
        .event_stage = event_stage,
        .time_captured = time_captured,
    };
}

class StdoutPipeCapture {
public:
    StdoutPipeCapture() {
        if (::pipe(m_pipe_fd) == -1) {
            throw std::system_error(errno, std::generic_category(), "pipe");
        }
        std::fflush(stdout);
        const int saved_stdout = ::dup(fileno(stdout));
        if (saved_stdout == -1) {
            cleanupPipe();
            throw std::system_error(errno, std::generic_category(), "dup");
        }
        if (::dup2(m_pipe_fd[1], fileno(stdout)) == -1) {
            ::close(saved_stdout);
            cleanupPipe();
            throw std::system_error(errno, std::generic_category(), "dup2");
        }
        ::close(m_pipe_fd[1]);
        m_pipe_fd[1] = -1;
        m_old_stdout = saved_stdout;
        m_read_fd = m_pipe_fd[0];
        m_pipe_fd[0] = -1;
    }

    ~StdoutPipeCapture() {
        restoreStdout();
        if (m_read_fd != -1) {
            ::close(m_read_fd);
            m_read_fd = -1;
        }
    }

    std::string read() {
        restoreStdout();
        std::string output;
        char buffer[256];
        ssize_t count;
        while ((count = ::read(m_read_fd, buffer, sizeof(buffer))) > 0) {
            output.append(buffer, buffer + count);
        }
        if (count == -1) {
            throw std::system_error(errno, std::generic_category(), "read");
        }
        ::close(m_read_fd);
        m_read_fd = -1;
        return output;
    }

private:
    void restoreStdout() {
        if (m_old_stdout == -1) {
            return;
        }
        std::fflush(stdout);
        ::dup2(m_old_stdout, fileno(stdout));
        ::close(m_old_stdout);
        m_old_stdout = -1;
    }

    void cleanupPipe() noexcept {
        if (m_pipe_fd[0] != -1) {
            ::close(m_pipe_fd[0]);
            m_pipe_fd[0] = -1;
        }
        if (m_pipe_fd[1] != -1) {
            ::close(m_pipe_fd[1]);
            m_pipe_fd[1] = -1;
        }
    }

    int m_pipe_fd[2] = {-1, -1};
    int m_read_fd = -1;
    int m_old_stdout = -1;
};

std::string runTrackerAndCaptureOutput(const std::vector<TimeRecord>& records,
                                       std::size_t capacity = 32U) {
    LatencyTracker tracker(1, capacity);
    LogPrinter log_printer(1, capacity);
    tracker.attachLogPrinter(&log_printer);

    StdoutPipeCapture capture;
    log_printer.start();
    for (const TimeRecord& record : records) {
        tracker.pushRecord(record);
    }
    EXPECT_EQ(tracker.run(), records.size());
    log_printer.stop();
    return capture.read();
}

} // namespace

TEST(LatencyTrackerTest, emitsBatchDerivedLatencyWithoutRegression) {
    LatencyTracker tracker(1, 32);
    LogPrinter log_printer(1, 32);
    tracker.attachLogPrinter(&log_printer);

    const uint16_t que_idx = 0;
    const uint64_t event_tag = 1234ULL;
    const uint64_t frame_start_tick = 100ULL;
    const uint64_t dma_emit_tick = 110ULL;
    const uint64_t batch_start_ns = 1000ULL;
    const uint64_t batch_end_ns = 1300ULL;
    const uint64_t strategy_start_ns = 1400ULL;
    const uint64_t executor_ns = 1500ULL;
    const uint64_t tx_execution_accepted_ns = 1500ULL;
    const uint64_t execution_dequeue_ns = 1510ULL;
    const uint64_t order_frame_built_ns = 1520ULL;
    const uint64_t pending_recorded_ns = 1530ULL;
    const uint64_t tx_enqueue_ns = 1540ULL;
    const uint64_t tx_send_ns = 1550ULL;

    StdoutPipeCapture capture;
    log_printer.start();
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::FRAME_START, frame_start_tick));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::DMA_EMIT, dma_emit_tick));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::BATCH_START, batch_start_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::BATCH_END, batch_end_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::STRATEGY_START, strategy_start_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::EXECUTOR, executor_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_EXECUTION_ACCEPTED, tx_execution_accepted_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_EXECUTION_DEQUEUE, execution_dequeue_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_ORDER_FRAME_BUILT, order_frame_built_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_PENDING_RECORDED, pending_recorded_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_ENQUEUE, tx_enqueue_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_SEND, tx_send_ns));
    EXPECT_EQ(tracker.run(), 12U);
    log_printer.stop();

    const std::string output = capture.read();
    EXPECT_NE(output.find("LatencyNs"), std::string::npos);
    EXPECT_NE(output.find("frame_start_to_dma_emit_ns=64"), std::string::npos);
    EXPECT_NE(output.find("batch_duration_ns=300"), std::string::npos);
    EXPECT_NE(output.find("batch_end_to_strategy_start_ns=100"), std::string::npos);
    EXPECT_NE(output.find("strategy_start_to_tx_execution_accepted_ns=100"), std::string::npos);
    EXPECT_NE(output.find("tx_execution_accepted_to_tx_execution_dequeue_ns=10"), std::string::npos);
    EXPECT_NE(output.find("tx_execution_dequeue_to_tx_order_frame_built_ns=10"), std::string::npos);
    EXPECT_NE(output.find("tx_order_frame_built_to_tx_pending_recorded_ns=10"), std::string::npos);
    EXPECT_NE(output.find("tx_pending_recorded_to_tx_enqueue_ns=10"), std::string::npos);
    EXPECT_NE(output.find("tx_enqueue_to_tx_send_ns=10"), std::string::npos);
    EXPECT_EQ(output.find("dma_emit_to_decode_ns="), std::string::npos);
    EXPECT_EQ(output.find("decode_to_strategy_ns="), std::string::npos);
    EXPECT_EQ(output.find("strategy_to_executor_ns="), std::string::npos);
    EXPECT_EQ(output.find("executor_to_execution_dequeue_ns="), std::string::npos);
    EXPECT_EQ(output.find("executor_to_tx_enqueue_ns="), std::string::npos);
}

TEST(LatencyTrackerTest, emitsCompleteChainOnlyAfterTxSend) {
    LatencyTracker tracker(1, 8);
    LogPrinter printer(1, 8);
    tracker.attachLogPrinter(&printer);

    const uint16_t que_idx = 0;
    const uint64_t event_tag = 777ULL;
    const uint64_t frame_start_tick = 1000ULL;
    const uint64_t dma_emit_tick = 1010ULL;
    const uint64_t batch_start_ns = 2000ULL;
    const uint64_t batch_end_ns = 2200ULL;
    const uint64_t strategy_start_ns = 2210ULL;
    const uint64_t executor_ns = 2230ULL;
    const uint64_t tx_execution_accepted_ns = 2235ULL;
    const uint64_t execution_dequeue_ns = 2240ULL;
    const uint64_t order_frame_built_ns = 2245ULL;
    const uint64_t pending_recorded_ns = 2250ULL;
    const uint64_t tx_enqueue_ns = 2255ULL;
    const uint64_t tx_send_ns = 2260ULL;

    StdoutPipeCapture capture_before_tx;
    printer.start();

    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::FRAME_START, frame_start_tick));
    EXPECT_EQ(tracker.run(), 1U);
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::DMA_EMIT, dma_emit_tick));
    EXPECT_EQ(tracker.run(), 1U);
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::BATCH_START, batch_start_ns));
    EXPECT_EQ(tracker.run(), 1U);
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::BATCH_END, batch_end_ns));
    EXPECT_EQ(tracker.run(), 1U);
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::STRATEGY_START, strategy_start_ns));
    EXPECT_EQ(tracker.run(), 1U);
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::EXECUTOR, executor_ns));
    EXPECT_EQ(tracker.run(), 1U);
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_EXECUTION_ACCEPTED, tx_execution_accepted_ns));
    EXPECT_EQ(tracker.run(), 1U);
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_EXECUTION_DEQUEUE, execution_dequeue_ns));
    EXPECT_EQ(tracker.run(), 1U);
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_ORDER_FRAME_BUILT, order_frame_built_ns));
    EXPECT_EQ(tracker.run(), 1U);
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_PENDING_RECORDED, pending_recorded_ns));
    EXPECT_EQ(tracker.run(), 1U);
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_ENQUEUE, tx_enqueue_ns));
    EXPECT_EQ(tracker.run(), 1U);
    printer.stop();
    const std::string pre_tx_send_output = capture_before_tx.read();
    EXPECT_TRUE(pre_tx_send_output.empty());

    StdoutPipeCapture capture_tx;
    printer.start();
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_SEND, tx_send_ns));
    EXPECT_EQ(tracker.run(), 1U);
    printer.stop();
    const std::string output = capture_tx.read();

    EXPECT_NE(output.find("LatencyNs"), std::string::npos);
    EXPECT_NE(output.find("frame_start_to_dma_emit_ns=64"), std::string::npos);
    EXPECT_NE(output.find("batch_duration_ns=200"), std::string::npos);
    EXPECT_NE(output.find("batch_end_to_strategy_start_ns=10"), std::string::npos);
    EXPECT_NE(output.find("strategy_start_to_tx_execution_accepted_ns=25"), std::string::npos);
    EXPECT_NE(output.find("tx_execution_accepted_to_tx_execution_dequeue_ns=5"), std::string::npos);
    EXPECT_NE(output.find("tx_execution_dequeue_to_tx_order_frame_built_ns=5"), std::string::npos);
    EXPECT_NE(output.find("tx_order_frame_built_to_tx_pending_recorded_ns=5"), std::string::npos);
    EXPECT_NE(output.find("tx_pending_recorded_to_tx_enqueue_ns=5"), std::string::npos);
    EXPECT_NE(output.find("tx_enqueue_to_tx_send_ns=5"), std::string::npos);
}

TEST(LatencyTrackerTest, rejectsMissingBatchStartBeforeBatchEnd) {
    const std::vector<TimeRecord> records {
        makeRecord(0, 900ULL, stage::FRAME_START, 100ULL),
        makeRecord(0, 900ULL, stage::DMA_EMIT, 110ULL),
        makeRecord(0, 900ULL, stage::BATCH_END, 1300ULL),
        makeRecord(0, 900ULL, stage::STRATEGY_START, 1400ULL),
        makeRecord(0, 900ULL, stage::EXECUTOR, 1500ULL),
        makeRecord(0, 900ULL, stage::TX_EXECUTION_DEQUEUE, 1510ULL),
        makeRecord(0, 900ULL, stage::TX_ORDER_FRAME_BUILT, 1520ULL),
        makeRecord(0, 900ULL, stage::TX_PENDING_RECORDED, 1530ULL),
        makeRecord(0, 900ULL, stage::TX_ENQUEUE, 1540ULL),
        makeRecord(0, 900ULL, stage::TX_SEND, 1550ULL),
    };

    const std::string output = runTrackerAndCaptureOutput(records);
    EXPECT_TRUE(output.empty());
}

TEST(LatencyTrackerTest, rejectsBatchStartWithoutDmaEmit) {
    const std::vector<TimeRecord> records {
        makeRecord(0, 905ULL, stage::FRAME_START, 100ULL),
        makeRecord(0, 905ULL, stage::BATCH_START, 1000ULL),
        makeRecord(0, 905ULL, stage::BATCH_END, 1300ULL),
        makeRecord(0, 905ULL, stage::STRATEGY_START, 1400ULL),
        makeRecord(0, 905ULL, stage::EXECUTOR, 1500ULL),
        makeRecord(0, 905ULL, stage::TX_EXECUTION_DEQUEUE, 1510ULL),
        makeRecord(0, 905ULL, stage::TX_ORDER_FRAME_BUILT, 1520ULL),
        makeRecord(0, 905ULL, stage::TX_PENDING_RECORDED, 1530ULL),
        makeRecord(0, 905ULL, stage::TX_ENQUEUE, 1540ULL),
        makeRecord(0, 905ULL, stage::TX_SEND, 1550ULL),
    };

    const std::string output = runTrackerAndCaptureOutput(records);
    EXPECT_TRUE(output.empty());
}

TEST(LatencyTrackerTest, rejectsMissingBatchEndBeforeStrategy) {
    const std::vector<TimeRecord> records {
        makeRecord(0, 901ULL, stage::FRAME_START, 100ULL),
        makeRecord(0, 901ULL, stage::DMA_EMIT, 110ULL),
        makeRecord(0, 901ULL, stage::BATCH_START, 1000ULL),
        makeRecord(0, 901ULL, stage::STRATEGY_START, 1400ULL),
        makeRecord(0, 901ULL, stage::EXECUTOR, 1500ULL),
        makeRecord(0, 901ULL, stage::TX_EXECUTION_DEQUEUE, 1510ULL),
        makeRecord(0, 901ULL, stage::TX_ORDER_FRAME_BUILT, 1520ULL),
        makeRecord(0, 901ULL, stage::TX_PENDING_RECORDED, 1530ULL),
        makeRecord(0, 901ULL, stage::TX_ENQUEUE, 1540ULL),
        makeRecord(0, 901ULL, stage::TX_SEND, 1550ULL),
    };

    const std::string output = runTrackerAndCaptureOutput(records);
    EXPECT_TRUE(output.empty());
}

TEST(LatencyTrackerTest, rejectsBatchEndEarlierThanBatchStart) {
    const std::vector<TimeRecord> records {
        makeRecord(0, 902ULL, stage::FRAME_START, 100ULL),
        makeRecord(0, 902ULL, stage::DMA_EMIT, 110ULL),
        makeRecord(0, 902ULL, stage::BATCH_START, 1300ULL),
        makeRecord(0, 902ULL, stage::BATCH_END, 1200ULL),
        makeRecord(0, 902ULL, stage::STRATEGY_START, 1400ULL),
        makeRecord(0, 902ULL, stage::EXECUTOR, 1500ULL),
        makeRecord(0, 902ULL, stage::TX_EXECUTION_DEQUEUE, 1510ULL),
        makeRecord(0, 902ULL, stage::TX_ORDER_FRAME_BUILT, 1520ULL),
        makeRecord(0, 902ULL, stage::TX_PENDING_RECORDED, 1530ULL),
        makeRecord(0, 902ULL, stage::TX_ENQUEUE, 1540ULL),
        makeRecord(0, 902ULL, stage::TX_SEND, 1550ULL),
    };

    const std::string output = runTrackerAndCaptureOutput(records);
    EXPECT_TRUE(output.empty());
}

TEST(LatencyTrackerTest, rejectsStrategyEarlierThanBatchEnd) {
    const std::vector<TimeRecord> records {
        makeRecord(0, 906ULL, stage::FRAME_START, 100ULL),
        makeRecord(0, 906ULL, stage::DMA_EMIT, 110ULL),
        makeRecord(0, 906ULL, stage::BATCH_START, 1100ULL),
        makeRecord(0, 906ULL, stage::BATCH_END, 1300ULL),
        makeRecord(0, 906ULL, stage::STRATEGY_START, 1200ULL),
        makeRecord(0, 906ULL, stage::EXECUTOR, 1500ULL),
        makeRecord(0, 906ULL, stage::TX_EXECUTION_DEQUEUE, 1510ULL),
        makeRecord(0, 906ULL, stage::TX_ORDER_FRAME_BUILT, 1520ULL),
        makeRecord(0, 906ULL, stage::TX_PENDING_RECORDED, 1530ULL),
        makeRecord(0, 906ULL, stage::TX_ENQUEUE, 1540ULL),
        makeRecord(0, 906ULL, stage::TX_SEND, 1550ULL),
    };

    const std::string output = runTrackerAndCaptureOutput(records);
    EXPECT_TRUE(output.empty());
}

TEST(LatencyTrackerTest, rejectsDuplicateBatchStartMarker) {
    const std::vector<TimeRecord> records {
        makeRecord(0, 903ULL, stage::FRAME_START, 100ULL),
        makeRecord(0, 903ULL, stage::DMA_EMIT, 110ULL),
        makeRecord(0, 903ULL, stage::BATCH_START, 1000ULL),
        makeRecord(0, 903ULL, stage::BATCH_START, 1010ULL),
        makeRecord(0, 903ULL, stage::BATCH_END, 1300ULL),
        makeRecord(0, 903ULL, stage::STRATEGY_START, 1400ULL),
        makeRecord(0, 903ULL, stage::EXECUTOR, 1500ULL),
        makeRecord(0, 903ULL, stage::TX_EXECUTION_DEQUEUE, 1510ULL),
        makeRecord(0, 903ULL, stage::TX_ORDER_FRAME_BUILT, 1520ULL),
        makeRecord(0, 903ULL, stage::TX_PENDING_RECORDED, 1530ULL),
        makeRecord(0, 903ULL, stage::TX_ENQUEUE, 1540ULL),
        makeRecord(0, 903ULL, stage::TX_SEND, 1550ULL),
    };

    const std::string output = runTrackerAndCaptureOutput(records);
    EXPECT_TRUE(output.empty());
}

TEST(LatencyTrackerTest, rejectsDuplicateBatchEndMarker) {
    const std::vector<TimeRecord> records {
        makeRecord(0, 904ULL, stage::FRAME_START, 100ULL),
        makeRecord(0, 904ULL, stage::DMA_EMIT, 110ULL),
        makeRecord(0, 904ULL, stage::BATCH_START, 1000ULL),
        makeRecord(0, 904ULL, stage::BATCH_END, 1300ULL),
        makeRecord(0, 904ULL, stage::BATCH_END, 1310ULL),
        makeRecord(0, 904ULL, stage::STRATEGY_START, 1400ULL),
        makeRecord(0, 904ULL, stage::EXECUTOR, 1500ULL),
        makeRecord(0, 904ULL, stage::TX_EXECUTION_DEQUEUE, 1510ULL),
        makeRecord(0, 904ULL, stage::TX_ORDER_FRAME_BUILT, 1520ULL),
        makeRecord(0, 904ULL, stage::TX_PENDING_RECORDED, 1530ULL),
        makeRecord(0, 904ULL, stage::TX_ENQUEUE, 1540ULL),
        makeRecord(0, 904ULL, stage::TX_SEND, 1550ULL),
    };

    const std::string output = runTrackerAndCaptureOutput(records);
    EXPECT_TRUE(output.empty());
}
