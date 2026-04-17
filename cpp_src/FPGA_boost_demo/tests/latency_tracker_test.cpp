#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"
#include "../common/time_utils.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <string>

namespace {

TimeRecord makeRecord(uint16_t que_idx,
                      uint64_t event_tag,
                      stage event_stage,
                      uint64_t time_captured,
                      uint32_t sender_backlog_depth = 0,
                      uint32_t tx_send_call_count = 0,
                      uint32_t tx_send_bytes_total = 0,
                      uint32_t tx_send_eintr_retry_count = 0,
                      uint32_t tx_send_had_partial_write = 0) {
    return TimeRecord {
        .que_idx = que_idx,
        .event_tag = event_tag,
        .event_stage = event_stage,
        .time_captured = time_captured,
        .sender_backlog_depth = sender_backlog_depth,
        .tx_send_call_count = tx_send_call_count,
        .tx_send_bytes_total = tx_send_bytes_total,
        .tx_send_eintr_retry_count = tx_send_eintr_retry_count,
        .tx_send_had_partial_write = tx_send_had_partial_write,
    };
}

} // namespace

TEST(LatencyTrackerTest, monotonicRawReadRemainsMonotonic) {
    const uint64_t first = readMonotonicRawNs();
    const uint64_t second = readMonotonicRawNs();

    EXPECT_GT(first, 0U);
    EXPECT_GT(second, 0U);
    EXPECT_LE(first, second);
}

TEST(LatencyTrackerTest, emitsRenamedStageAndLatencyFields) {
    LatencyTracker tracker(1, 32);
    LogPrinter printer(1, 32);
    tracker.attachLogPrinter(&printer);

    const uint16_t que_idx = 0;
    const uint64_t event_tag = 1234ULL;
    const uint64_t frame_start_tick = 100ULL;
    const uint64_t dma_emit_tick = 110ULL;
    const uint64_t frame_start_to_dma_emit_ns =
        ((dma_emit_tick - frame_start_tick) * 64ULL) / 10ULL;
    const HostTickScale& host_scale = readHostTickScale();
    auto hostNsToTick = [&host_scale](uint64_t host_ns) {
        if (host_scale.use_clock_fallback || host_scale.tsc_hz == 0) {
            return host_ns;
        }
        return static_cast<uint64_t>(
            (static_cast<__uint128_t>(host_ns) * host_scale.tsc_hz) / 1000000000ULL);
    };
    auto hostTickDeltaToNs = [&host_scale](uint64_t later_tick, uint64_t earlier_tick) {
        const uint64_t delta_tick = later_tick - earlier_tick;
        if (host_scale.use_clock_fallback || host_scale.tsc_hz == 0) {
            return static_cast<long long>(delta_tick);
        }
        return static_cast<long long>(
            (static_cast<__uint128_t>(delta_tick) * 1000000000ULL) / host_scale.tsc_hz);
    };
    const uint64_t batch_start_tick = hostNsToTick(1000ULL);
    const uint64_t batch_end_tick = hostNsToTick(1300ULL);
    const uint64_t strategy_start_tick = hostNsToTick(1400ULL);
    const uint64_t tx_execution_accepted_tick = hostNsToTick(1500ULL);
    const uint64_t tx_enqueue_tick = hostNsToTick(1540ULL);
    const uint64_t tx_send_enter_tick = hostNsToTick(1550ULL);
    const uint64_t tx_send_syscall_enter_tick = hostNsToTick(1552ULL);
    const uint64_t tx_send_tick = hostNsToTick(1555ULL);

    printer.start();
    testing::internal::CaptureStdout();
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::FRAME_START, frame_start_tick));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::DMA_EMIT, dma_emit_tick));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::BATCH_START, batch_start_tick));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::BATCH_END, batch_end_tick));
    tracker.pushRecord(makeRecord(que_idx, event_tag, stage::STRATEGY_START, strategy_start_tick));
    tracker.pushRecord(makeRecord(que_idx,
                                  event_tag,
                                  stage::TX_EXECUTION_ACCEPTED,
                                  tx_execution_accepted_tick));
    tracker.pushRecord(makeRecord(que_idx,
                                  event_tag,
                                  stage::TX_ENQUEUE,
                                  tx_enqueue_tick,
                                  7U));
    tracker.run();
    printer.stop();
    const std::string pre_tx_send_output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(pre_tx_send_output.empty());

    printer.start();
    testing::internal::CaptureStdout();
    tracker.pushRecord(makeRecord(que_idx,
                                  event_tag,
                                  stage::TX_SEND_ENTER,
                                  tx_send_enter_tick,
                                  3U));
    tracker.pushRecord(makeRecord(que_idx,
                                  event_tag,
                                  stage::TX_SEND_SYSCALL_ENTER,
                                  tx_send_syscall_enter_tick));
    tracker.pushRecord(makeRecord(que_idx,
                                  event_tag,
                                  stage::TX_SEND,
                                  tx_send_tick,
                                  0U,
                                  2U,
                                  64U,
                                  1U,
                                  1U));
    tracker.run();
    printer.stop();

    const std::string output = testing::internal::GetCapturedStdout();
    auto formatSummaryLine = [](const char* label, long long value) {
        char line[128];
        std::snprintf(line,
                      sizeof(line),
                      "%-48s count=1 min=%lld p50=%lld p99=%lld max=%lld\n",
                      label,
                      value,
                      value,
                      value,
                      value);
        return std::string(line);
    };
    std::string expected = "\nLatencySummary queue=0\n";
    expected += formatSummaryLine("frame_start -> dma_emit_ns",
                                  static_cast<long long>(frame_start_to_dma_emit_ns));
    expected += formatSummaryLine("batch_duration_ns",
                                  hostTickDeltaToNs(batch_end_tick, batch_start_tick));
    expected += formatSummaryLine("batch_end -> strategy_start_ns",
                                  hostTickDeltaToNs(strategy_start_tick, batch_end_tick));
    expected += formatSummaryLine(
        "strategy_start -> tx_execution_accepted_ns",
        hostTickDeltaToNs(tx_execution_accepted_tick, strategy_start_tick));
    expected += formatSummaryLine("tx_execution_accepted -> tx_enqueue_ns",
                                  hostTickDeltaToNs(tx_enqueue_tick,
                                                    tx_execution_accepted_tick));
    expected += formatSummaryLine("tx_enqueue -> tx_send_enter_ns",
                                  hostTickDeltaToNs(tx_send_enter_tick, tx_enqueue_tick));
    expected += formatSummaryLine("tx_send_enter -> tx_send_syscall_enter_ns",
                                  hostTickDeltaToNs(tx_send_syscall_enter_tick,
                                                    tx_send_enter_tick));
    expected += formatSummaryLine("tx_send_syscall_enter -> tx_send_ns",
                                  hostTickDeltaToNs(tx_send_tick,
                                                    tx_send_syscall_enter_tick));

    EXPECT_EQ(output, expected);
    EXPECT_EQ(output.find("event_tag="), std::string::npos);
    EXPECT_EQ(output.find("tx_enqueue_backlog_depth"), std::string::npos);
    EXPECT_EQ(output.find("tx_send_enter_backlog_depth"), std::string::npos);
    EXPECT_EQ(output.find("tx_send_call_count"), std::string::npos);
    EXPECT_EQ(output.find("tx_send_bytes_total"), std::string::npos);
    EXPECT_EQ(output.find("tx_send_eintr_retry_count"), std::string::npos);
    EXPECT_EQ(output.find("tx_send_had_partial_write"), std::string::npos);
    EXPECT_EQ(output.find("tx_execution_accepted -> tx_execution_dequeue_ns"), std::string::npos);
    EXPECT_EQ(output.find("tx_execution_dequeue -> tx_order_frame_built_ns"), std::string::npos);
    EXPECT_EQ(output.find("tx_order_frame_built -> tx_pending_recorded_ns"), std::string::npos);
    EXPECT_EQ(output.find("tx_pending_recorded -> tx_enqueue_ns"), std::string::npos);
    EXPECT_EQ(output.find("tx_enqueue -> tx_send_ns"), std::string::npos);
    EXPECT_NE(output.find("tx_execution_accepted -> tx_enqueue_ns"), std::string::npos);
}
