#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"

#include <gtest/gtest.h>

#include <cerrno>
#include <cstdio>
#include <string>
#include <system_error>
#include <unistd.h>

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

} // namespace

TEST(LatencyTrackerTest, emitsRenamedStageAndLatencyFields) {
    LatencyTracker tracker(1, 32);
    LogPrinter printer(1, 32);
    tracker.attachLogPrinter(&printer);

    const uint16_t que_idx = 0;
    const uint64_t event_tag = 1234ULL;
    const uint64_t frame_start_tick = 100ULL;
    const uint64_t dma_emit_tick = 110ULL;
    const uint64_t batch_start_ns = 1000ULL;
    const uint64_t batch_end_ns = 1300ULL;
    const uint64_t strategy_start_ns = 1400ULL;
    const uint64_t tx_execution_accepted_ns = 1500ULL;
    const uint64_t tx_execution_dequeue_ns = 1510ULL;
    const uint64_t tx_order_frame_built_ns = 1520ULL;
    const uint64_t tx_pending_recorded_ns = 1530ULL;
    const uint64_t tx_enqueue_ns = 1540ULL;
    const uint64_t tx_send_ns = 1550ULL;

    StdoutPipeCapture capture;
    printer.start();
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::FRAME_START, frame_start_tick));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::DMA_EMIT, dma_emit_tick));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::BATCH_START, batch_start_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::BATCH_END, batch_end_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::STRATEGY_START, strategy_start_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_EXECUTION_ACCEPTED, tx_execution_accepted_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_EXECUTION_DEQUEUE, tx_execution_dequeue_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_ORDER_FRAME_BUILT, tx_order_frame_built_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_PENDING_RECORDED, tx_pending_recorded_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_ENQUEUE, tx_enqueue_ns));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_SEND, tx_send_ns));
    EXPECT_EQ(tracker.run(), 11U);
    printer.stop();

    const std::string output = capture.read();
    EXPECT_NE(output.find("batch_end_to_strategy_start_ns=100"), std::string::npos);
    EXPECT_NE(output.find("strategy_start_to_tx_execution_accepted_ns=100"), std::string::npos);
    EXPECT_NE(output.find("tx_execution_accepted_to_tx_execution_dequeue_ns=10"), std::string::npos);
    EXPECT_EQ(output.find("strategy_to_executor_ns="), std::string::npos);
    EXPECT_EQ(output.find("executor_to_execution_dequeue_ns="), std::string::npos);
    EXPECT_EQ(output.find("executor_to_tx_enqueue_ns="), std::string::npos);
}
