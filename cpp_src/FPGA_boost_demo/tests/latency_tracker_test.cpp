#define private public
#include "../latency/latency_tracker.h"
#include "../latency/latency_analyzer.h"
#undef private
#include "../common/time_utils.h"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <type_traits>
#include <vector>

namespace {

TimeRecord makeRecord(uint16_t que_idx,
                      uint64_t event_tag,
                      uint32_t trace_id,
                      stage event_stage,
                      uint64_t time_captured) {
    return TimeRecord {
        .que_idx = que_idx,
        .event_tag = event_tag,
        .trace_id = trace_id,
        .event_stage = event_stage,
        .time_captured = time_captured,
    };
}

template <typename TraceIdSlot>
uint32_t readActiveTraceId(const TraceIdSlot& trace_id) {
    using SlotType = std::remove_cv_t<std::remove_reference_t<TraceIdSlot>>;
    if constexpr (std::is_same_v<SlotType, std::atomic<uint32_t>>) {
        return trace_id.load();
    }
    return trace_id;
}

void pushCompleteTraceRecords(LatencyTracker& tracker,
                              uint16_t que_idx,
                              uint64_t event_tag,
                              uint32_t trace_id) {
    tracker.pushRecord(makeRecord(que_idx, event_tag, trace_id, stage::FRAME_START, 100U));
    tracker.pushRecord(makeRecord(que_idx, event_tag, trace_id, stage::DMA_EMIT, 110U));
    tracker.pushRecord(makeRecord(que_idx, event_tag, trace_id, stage::BATCH_START, 1000U));
    tracker.pushRecord(makeRecord(que_idx, event_tag, trace_id, stage::BATCH_END, 1300U));
    tracker.pushRecord(makeRecord(que_idx, event_tag, trace_id, stage::STRATEGY_START, 1400U));
    tracker.pushRecord(makeRecord(que_idx,
                                  event_tag,
                                  trace_id,
                                  stage::TX_EXECUTION_ACCEPTED,
                                  1500U));
    tracker.pushRecord(makeRecord(que_idx, event_tag, trace_id, stage::TX_ENQUEUE, 1540U));
    tracker.pushRecord(makeRecord(que_idx, event_tag, trace_id, stage::TX_SEND_ENTER, 1550U));
    tracker.pushRecord(makeRecord(que_idx,
                                  event_tag,
                                  trace_id,
                                  stage::TX_SEND_SYSCALL_ENTER,
                                  1552U));
    tracker.pushRecord(makeRecord(que_idx, event_tag, trace_id, stage::TX_SEND, 1555U));
}

TraceCommand decodeOverflowCommand(uint64_t encoded_command) {
    TraceCommand command {};
    command.que_idx = static_cast<uint16_t>((encoded_command >> 48U) & 0xFFFFU);
    command.trace_id = static_cast<uint32_t>((encoded_command >> 16U) & 0xFFFFFFFFULL);
    command.op = static_cast<TraceCommandOp>(encoded_command & 0xFFU);
    return command;
}

} // namespace

TEST(LatencyTrackerTest, monotonicRawReadRemainsMonotonic) {
    const uint64_t first = readMonotonicRawNs();
    const uint64_t second = readMonotonicRawNs();

    EXPECT_GT(first, 0U);
    EXPECT_GT(second, 0U);
    EXPECT_LE(first, second);
}

TEST(LatencyTrackerTest, nonFirstEventCannotAllocateTraceId) {
    LatencyTracker tracker(1, 8);
    EXPECT_EQ(tracker.tryAllocateTraceId(0, false), 0U);
}

TEST(LatencyTrackerTest, traceCommandCarriesQueueTraceIdAndOperation) {
    const TraceCommand command {
        .que_idx = 1U,
        .trace_id = 42U,
        .op = TraceCommandOp::Finalize,
    };

    EXPECT_EQ(command.que_idx, 1U);
    EXPECT_EQ(command.trace_id, 42U);
    EXPECT_EQ(command.op, TraceCommandOp::Finalize);
}

TEST(LatencyTrackerTest, queueRejectsLaterFirstEventWhileTraceIsActive) {
    LatencyTracker tracker(1, 8);
    const uint32_t trace_id = tracker.tryAllocateTraceId(0, true);
    ASSERT_NE(trace_id, 0U);
    EXPECT_EQ(tracker.tryAllocateTraceId(0, true), 0U);
}

TEST(LatencyTrackerTest, requestFinalizeQueuesCommandWithoutRunningFinalizeInline) {
    LatencyTracker tracker(1, 32, 8);
    LatencyAnalyzer analyzer(1);
    tracker.attachAnalyzer(&analyzer);

    const uint32_t trace_id = tracker.tryAllocateTraceId(0, true);
    ASSERT_NE(trace_id, 0U);

    pushCompleteTraceRecords(tracker, 0, 1001ULL, trace_id);

    EXPECT_TRUE(tracker.requestFinalize(0, trace_id));
    EXPECT_TRUE(analyzer.m_completed_records[0].empty());
    EXPECT_EQ(readActiveTraceId(tracker.m_active_trace_ids[0]), trace_id);

    TraceCommand command {};
    ASSERT_TRUE(tracker.m_trace_command_queues[0]->pop(command));
    EXPECT_EQ(command.que_idx, 0U);
    EXPECT_EQ(command.trace_id, trace_id);
    EXPECT_EQ(command.op, TraceCommandOp::Finalize);
}

TEST(LatencyTrackerTest, requestDropCanUseProducerQueueDifferentFromTargetQueue) {
    LatencyTracker tracker(2, 16, 8);
    const uint32_t trace_id = tracker.tryAllocateTraceId(1, true);
    ASSERT_NE(trace_id, 0U);

    tracker.pushRecord(makeRecord(1, 1001ULL, trace_id, stage::FRAME_START, 100U));

    EXPECT_TRUE(tracker.requestDrop(0, 1, trace_id));
    EXPECT_EQ(readActiveTraceId(tracker.m_active_trace_ids[1]), trace_id);

    TraceCommand command {};
    ASSERT_TRUE(tracker.m_trace_command_queues[0]->pop(command));
    EXPECT_EQ(command.que_idx, 1U);
    EXPECT_EQ(command.trace_id, trace_id);
    EXPECT_EQ(command.op, TraceCommandOp::Drop);
    EXPECT_FALSE(tracker.m_trace_command_queues[1]->pop(command));
}

TEST(LatencyTrackerTest, requestFinalizePreservesQueuedProgressWhenCommandQueueIsFull) {
    LatencyTracker tracker(1, 32, 1);
    LatencyAnalyzer analyzer(1);
    tracker.attachAnalyzer(&analyzer);

    const uint32_t trace_id = tracker.tryAllocateTraceId(0, true);
    ASSERT_NE(trace_id, 0U);
    pushCompleteTraceRecords(tracker, 0, 1001ULL, trace_id);

    const TraceCommand stale_command {
        .que_idx = 0U,
        .trace_id = trace_id + 99U,
        .op = TraceCommandOp::Drop,
    };
    ASSERT_TRUE(tracker.m_trace_command_queues[0]->push(stale_command));

    EXPECT_TRUE(tracker.requestFinalize(0, trace_id));

    TraceCommand queued_command {};
    ASSERT_TRUE(tracker.m_trace_command_queues[0]->pop(queued_command));
    EXPECT_EQ(queued_command.que_idx, stale_command.que_idx);
    EXPECT_EQ(queued_command.trace_id, stale_command.trace_id);
    EXPECT_EQ(queued_command.op, stale_command.op);
    EXPECT_FALSE(tracker.m_trace_command_queues[0]->pop(queued_command));

    const uint64_t encoded_overflow_command =
        tracker.m_trace_command_overflow_slots[0].load();
    ASSERT_NE(encoded_overflow_command, 0ULL);
    const TraceCommand overflow_command = decodeOverflowCommand(encoded_overflow_command);
    EXPECT_EQ(overflow_command.que_idx, 0U);
    EXPECT_EQ(overflow_command.trace_id, trace_id);
    EXPECT_EQ(overflow_command.op, TraceCommandOp::Finalize);

    const TraceCommand later_command {
        .que_idx = 0U,
        .trace_id = trace_id + 123U,
        .op = TraceCommandOp::Drop,
    };
    EXPECT_TRUE(tracker.requestDrop(0, later_command.trace_id));

    TraceCommand retried_ring_command {};
    ASSERT_TRUE(tracker.m_trace_command_queues[0]->pop(retried_ring_command));
    EXPECT_EQ(retried_ring_command.que_idx, later_command.que_idx);
    EXPECT_EQ(retried_ring_command.trace_id, later_command.trace_id);
    EXPECT_EQ(retried_ring_command.op, later_command.op);
    ASSERT_TRUE(tracker.m_trace_command_queues[0]->push(retried_ring_command));

    tracker.stop();
    tracker.run();

    ASSERT_EQ(analyzer.m_completed_records[0].size(), 1U);
    EXPECT_EQ(analyzer.m_completed_records[0][0].event_tag, 1001ULL);
    EXPECT_EQ(readActiveTraceId(tracker.m_active_trace_ids[0]), 0U);
}

TEST(LatencyTrackerTest, requestFinalizeOverflowStillDrainsWhenRingPassHitsWorkBound) {
    LatencyTracker tracker(1, 32, 4);
    LatencyAnalyzer analyzer(1);
    tracker.attachAnalyzer(&analyzer);

    const uint32_t trace_id = tracker.tryAllocateTraceId(0, true);
    ASSERT_NE(trace_id, 0U);
    pushCompleteTraceRecords(tracker, 0, 1001ULL, trace_id);

    for (uint32_t stale_offset = 0U; stale_offset < 4U; ++stale_offset) {
        const TraceCommand stale_command {
            .que_idx = 0U,
            .trace_id = trace_id + 100U + stale_offset,
            .op = TraceCommandOp::Drop,
        };
        ASSERT_TRUE(tracker.m_trace_command_queues[0]->push(stale_command));
    }

    ASSERT_TRUE(tracker.requestFinalize(0, trace_id));
    EXPECT_NE(tracker.m_trace_command_overflow_slots[0].load(), 0ULL);

    EXPECT_TRUE(tracker._drainCommandPass());

    EXPECT_EQ(tracker.m_trace_command_overflow_slots[0].load(), 0ULL);
    ASSERT_EQ(analyzer.m_completed_records[0].size(), 1U);
    EXPECT_EQ(analyzer.m_completed_records[0][0].event_tag, 1001ULL);
    EXPECT_EQ(readActiveTraceId(tracker.m_active_trace_ids[0]), 0U);
}

TEST(LatencyTrackerTest, stopCausesRunToDrainPendingCommandsBeforeExit) {
    LatencyTracker tracker(1, 32, 8);
    LatencyAnalyzer analyzer(1);
    tracker.attachAnalyzer(&analyzer);

    const uint32_t trace_id = tracker.tryAllocateTraceId(0, true);
    ASSERT_NE(trace_id, 0U);
    pushCompleteTraceRecords(tracker, 0, 1001ULL, trace_id);

    ASSERT_TRUE(tracker.requestFinalize(0, trace_id));

    tracker.stop();
    tracker.run();

    ASSERT_EQ(analyzer.m_completed_records[0].size(), 1U);
    EXPECT_EQ(analyzer.m_completed_records[0][0].event_tag, 1001ULL);
    EXPECT_EQ(analyzer.m_completed_records[0][0].frame_start_to_dma_emit_ns, 64U);
    EXPECT_EQ(readActiveTraceId(tracker.m_active_trace_ids[0]), 0U);

    TimeRecord leftover {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(leftover));
}

TEST(LatencyTrackerTest, runProcessesQueuedFinalizeBeforeStopIsCalled) {
    LatencyTracker tracker(1, 32, 8);
    LatencyAnalyzer analyzer(1);
    tracker.attachAnalyzer(&analyzer);

    const uint32_t trace_id = tracker.tryAllocateTraceId(0, true);
    ASSERT_NE(trace_id, 0U);
    pushCompleteTraceRecords(tracker, 0, 1001ULL, trace_id);
    ASSERT_TRUE(tracker.requestFinalize(0, trace_id));

    std::thread run_thread([&tracker]() {
        tracker.run();
    });

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    while (readActiveTraceId(tracker.m_active_trace_ids[0]) != 0U &&
           std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    EXPECT_EQ(readActiveTraceId(tracker.m_active_trace_ids[0]), 0U);

    tracker.stop();
    run_thread.join();

    ASSERT_EQ(analyzer.m_completed_records[0].size(), 1U);
    EXPECT_EQ(analyzer.m_completed_records[0][0].event_tag, 1001ULL);
    EXPECT_EQ(analyzer.m_completed_records[0][0].frame_start_to_dma_emit_ns, 64U);
}

TEST(LatencyTrackerTest, runConsumesQueuedDropAndClearsActiveTrace) {
    LatencyTracker tracker(1, 16, 8);
    const uint32_t trace_id = tracker.tryAllocateTraceId(0, true);
    ASSERT_NE(trace_id, 0U);

    tracker.pushRecord(makeRecord(0, 1001ULL, trace_id, stage::FRAME_START, 100U));
    tracker.pushRecord(makeRecord(0, 1001ULL, trace_id, stage::DMA_EMIT, 110U));

    ASSERT_TRUE(tracker.requestDrop(0, trace_id));

    tracker.stop();
    tracker.run();

    EXPECT_EQ(readActiveTraceId(tracker.m_active_trace_ids[0]), 0U);
    TimeRecord leftover {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(leftover));
}
