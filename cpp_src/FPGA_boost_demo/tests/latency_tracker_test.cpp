#define private public
#include "../latency/latency_tracker.h"
#undef private
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
                      uint32_t trace_id = 0,
                      uint32_t sender_backlog_depth = 0,
                      uint32_t tx_send_call_count = 0,
                      uint32_t tx_send_bytes_total = 0,
                      uint32_t tx_send_eintr_retry_count = 0,
                      uint32_t tx_send_had_partial_write = 0) {
    return TimeRecord {
        .que_idx = que_idx,
        .event_tag = event_tag,
        .trace_id = trace_id,
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

TEST(LatencyTrackerTest, rejectsNonPowerOfTwoPendingCapacity) {
    EXPECT_THROW((void)LatencyTracker(2, 8, 12), std::invalid_argument);
    EXPECT_THROW((void)LatencyTracker(2, 8, 0), std::invalid_argument);
}

TEST(LatencyTrackerTest, timeRecordCarriesTraceIdField) {
    const TimeRecord record {
        .que_idx = 1,
        .event_tag = 0x22ULL,
        .trace_id = 7U,
        .event_stage = stage::FRAME_START,
        .time_captured = 123U,
    };

    EXPECT_EQ(record.trace_id, 7U);
    EXPECT_EQ(record.event_tag, 0x22ULL);
}

TEST(LatencyTrackerTest, fairPassConsumesAtMostOneRecordPerQueue) {
    LatencyTracker tracker(2, 8, 8);
    tracker.pushRecord(makeRecord(0, 101ULL, stage::FRAME_START, 10U, 1U));
    tracker.pushRecord(makeRecord(0, 102ULL, stage::FRAME_START, 11U, 2U));
    tracker.pushRecord(makeRecord(1, 201ULL, stage::FRAME_START, 12U, 3U));

    EXPECT_EQ(tracker._drainFairPass(), 2U);
    EXPECT_EQ(tracker.m_pending_tables[0].live_count, 1U);
    EXPECT_EQ(tracker.m_pending_tables[1].live_count, 1U);

    TimeRecord leftover {};
    EXPECT_TRUE(tracker.m_latency_queues[0]->pop(leftover));
}

TEST(LatencyTrackerTest, oldestPendingEntryIsReusedWhenTableIsFull) {
    LatencyTracker tracker(1, 8, 2);
    tracker.pushRecord(makeRecord(0, 11ULL, stage::FRAME_START, 10U, 1U));
    tracker.pushRecord(makeRecord(0, 22ULL, stage::FRAME_START, 20U, 2U));
    tracker.pushRecord(makeRecord(0, 33ULL, stage::FRAME_START, 30U, 3U));

    tracker.run();

    EXPECT_FALSE(tracker._hasPendingRecord(0, 11ULL));
    EXPECT_TRUE(tracker._hasPendingRecord(0, 22ULL));
    EXPECT_TRUE(tracker._hasPendingRecord(0, 33ULL));
}

TEST(LatencyTrackerTest, sameEventTagRemainsIsolatedAcrossQueues) {
    LatencyTracker tracker(2, 8, 8);
    constexpr uint64_t shared_event_tag = 555ULL;
    tracker.pushRecord(makeRecord(0, shared_event_tag, stage::FRAME_START, 100U, 1U));
    tracker.pushRecord(makeRecord(1, shared_event_tag, stage::FRAME_START, 200U, 2U));
    tracker.pushRecord(makeRecord(0, shared_event_tag, stage::DMA_EMIT, 110U, 1U));

    tracker.run();

    EXPECT_TRUE(tracker._hasPendingRecord(0, shared_event_tag));
    EXPECT_TRUE(tracker._hasPendingRecord(1, shared_event_tag));
    EXPECT_EQ(tracker.m_pending_tables[0].live_count, 1U);
    EXPECT_EQ(tracker.m_pending_tables[1].live_count, 1U);

    auto* queue0_slot = tracker._findPendingSlot(0, shared_event_tag);
    auto* queue1_slot = tracker._findPendingSlot(1, shared_event_tag);
    ASSERT_NE(queue0_slot, nullptr);
    ASSERT_NE(queue1_slot, nullptr);
    EXPECT_EQ(queue0_slot->trace_id, 1U);
    EXPECT_EQ(queue1_slot->trace_id, 2U);
    EXPECT_TRUE(queue0_slot->state.has_dma_emit);
    EXPECT_FALSE(queue1_slot->state.has_dma_emit);
    EXPECT_EQ(queue0_slot->state.frame_start_tick, 100U);
    EXPECT_EQ(queue1_slot->state.frame_start_tick, 200U);
}

TEST(LatencyTrackerTest, overflowEvictsOldestLiveEntryAfterFreeSlotReuse) {
    LatencyTracker tracker(1, 8, 2);
    constexpr uint64_t event_a = 1001ULL;
    constexpr uint64_t event_b = 1002ULL;
    constexpr uint64_t event_c = 1003ULL;
    constexpr uint64_t event_d = 1004ULL;

    tracker.pushRecord(makeRecord(0, event_a, stage::FRAME_START, 10U, 1U));
    tracker.pushRecord(makeRecord(0, event_b, stage::FRAME_START, 20U, 2U));
    tracker.run();

    auto* slot_a = tracker._findPendingSlot(0, event_a);
    ASSERT_NE(slot_a, nullptr);
    tracker._erasePendingSlot(0, *slot_a);

    tracker.pushRecord(makeRecord(0, event_c, stage::FRAME_START, 30U, 3U));
    tracker.pushRecord(makeRecord(0, event_d, stage::FRAME_START, 40U, 4U));
    tracker.run();

    EXPECT_FALSE(tracker._hasPendingRecord(0, event_b));
    EXPECT_TRUE(tracker._hasPendingRecord(0, event_c));
    EXPECT_TRUE(tracker._hasPendingRecord(0, event_d));
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
                                  0U,
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
                                  0U,
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
