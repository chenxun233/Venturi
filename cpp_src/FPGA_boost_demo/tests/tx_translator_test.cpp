#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <stdexcept>
#include <sys/socket.h>
#include <type_traits>
#include <unistd.h>
#include <vector>

#include "../common/shared_types.h"
#define private public
#include "../tx_engine/tx_sender.h"
#include "../latency/latency_analyzer.h"
#include "../latency/latency_tracker.h"
#undef private

static_assert(std::is_member_function_pointer_v<decltype(&TxSender::acceptExecution)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::updateConnectionInfo)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::runOnce)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::trySendOutbound)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::buildOutboundFrames)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::queueHeartbeat)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::popReadyOutbound)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::restoreReadyOutbound)>);

namespace {

OrderExecution makeExecution(uint16_t stock_locate,
                             OrderIntentAction action,
                             uint32_t price,
                             uint32_t shares,
                             uint16_t que_idx = 0,
                             uint64_t event_tag = 0,
                             uint32_t trace_id = 0U) {
    return OrderExecution {
        .stock_locate = stock_locate,
        .que_idx = que_idx,
        .event_tag = event_tag,
        .trace_id = trace_id,
        .order = {.action = action, .price = price, .shares = shares},
    };
}

void connectSender(TxSender& sender, uint64_t generation, int fd = -1) {
    sender.updateConnectionInfo(TxConnectionInfo {
        .kind = TxConnectionKind::Connected,
        .generation = generation,
        .fd = fd,
    });
}

void disconnectSender(TxSender& sender, uint64_t generation) {
    sender.updateConnectionInfo(TxConnectionInfo {
        .kind = TxConnectionKind::Disconnected,
        .generation = generation,
        .fd = -1,
    });
}

void seedTrackedLatencyFlow(LatencyTracker& tracker,
                            uint16_t que_idx,
                            uint64_t event_tag,
                            uint32_t trace_id = 1U) {
    tracker.m_active_trace_ids[que_idx].store(trace_id, std::memory_order_relaxed);
    tracker.pushRecord(TimeRecord {
        .que_idx = que_idx,
        .event_tag = event_tag,
        .trace_id = trace_id,
        .event_stage = stage::FRAME_START,
        .time_captured = 100U,
    });
    tracker.pushRecord(TimeRecord {
        .que_idx = que_idx,
        .event_tag = event_tag,
        .trace_id = trace_id,
        .event_stage = stage::DMA_EMIT,
        .time_captured = 110U,
    });
    tracker.pushRecord(TimeRecord {
        .que_idx = que_idx,
        .event_tag = event_tag,
        .trace_id = trace_id,
        .event_stage = stage::BATCH_START,
        .time_captured = 200U,
    });
    tracker.pushRecord(TimeRecord {
        .que_idx = que_idx,
        .event_tag = event_tag,
        .trace_id = trace_id,
        .event_stage = stage::BATCH_END,
        .time_captured = 240U,
    });
    tracker.pushRecord(TimeRecord {
        .que_idx = que_idx,
        .event_tag = event_tag,
        .trace_id = trace_id,
        .event_stage = stage::STRATEGY_START,
        .time_captured = 300U,
    });
}

} // namespace

TEST(TxTranslatorTest, heartbeatTimingIsBasedOnSuccessfulSendsNotPopOrQueueTime) {
    using namespace std::chrono_literals;

    TxSender sender(TxSenderConfig {
        .heartbeat_interval = 1ms,
        .pending_capacity = 8,
    });

    sender.m_logged_in = true;
    sender.m_last_successful_send = std::chrono::steady_clock::now() - 2ms;

    ASSERT_TRUE(sender.queueHeartbeat());

    TxOutboundRecord heartbeat {};
    ASSERT_TRUE(sender.popReadyOutbound(heartbeat));
    ASSERT_GE(heartbeat.payload_length, 3U);
    ASSERT_EQ(heartbeat.payload[2], static_cast<uint8_t>('R'));

    ASSERT_TRUE(sender.queueHeartbeat());

    sender.restoreReadyOutbound(heartbeat);
    EXPECT_FALSE(sender.queueHeartbeat());

    ASSERT_TRUE(sender.popReadyOutbound(heartbeat));
    sender.noteOutboundSent(heartbeat);
    EXPECT_FALSE(sender.queueHeartbeat());
}

TEST(TxTranslatorTest, staleDisconnectDoesNotRetireNewerInstalledSendFd) {
    int sockets1[2] {-1, -1};
    int sockets2[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets1), 0);
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets2), 0);

    TxSender sender(8);
    connectSender(sender, 9, sockets1[0]);
    connectSender(sender, 10, sockets2[0]);
    disconnectSender(sender, 9);

    TxOutboundRecord outbound {};
    outbound.payload[0] = static_cast<uint8_t>('O');
    outbound.payload[1] = static_cast<uint8_t>('K');
    outbound.payload_length = 2;
    ASSERT_TRUE(sender.trySendOutbound(outbound));

    std::array<uint8_t, 2> received {};
    ASSERT_EQ(::recv(sockets2[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));
    EXPECT_EQ(received[0], outbound.payload[0]);
    EXPECT_EQ(received[1], outbound.payload[1]);

    ::close(sockets1[1]);
    ::close(sockets2[1]);
}

TEST(TxTranslatorTest, trySendOutboundSucceedsAfterConnectedInfoInstallsSendFd) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(8);
    connectSender(sender, 42, sockets[0]);

    TxOutboundRecord outbound {};
    outbound.payload[0] = static_cast<uint8_t>('H');
    outbound.payload[1] = static_cast<uint8_t>('I');
    outbound.payload_length = 2;
    ASSERT_TRUE(sender.trySendOutbound(outbound));

    std::array<uint8_t, 2> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));
    EXPECT_EQ(received[0], outbound.payload[0]);
    EXPECT_EQ(received[1], outbound.payload[1]);

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, transportConnectQueuesLoginRequestFrame) {
    TxSender sender(4);
    connectSender(sender, 1);

    TxOutboundRecord record {};
    ASSERT_TRUE(sender.popReadyOutbound(record));
    EXPECT_EQ(record.user_ref_num, 0U);
    ASSERT_GE(record.payload_length, 3U);
    EXPECT_EQ(record.payload[2], static_cast<uint8_t>('L'));
}

TEST(TxTranslatorTest, successfulSendClearsPendingOrderWithoutReceiverFlow) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(TxSenderConfig {
        .pending_capacity = 1,
        .pending_slot_count = 16,
    });
    sender.m_send_fd = sockets[0];
    sender.m_transport_generation = 42;

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::Buy, 100000, 10, 0, 1ULL)));
    ASSERT_TRUE(sender.buildOutboundFrames());
    ASSERT_EQ(sender.m_pending_orders.live_count, 1U);

    TxOutboundRecord first {};
    ASSERT_TRUE(sender.popReadyOutbound(first));
    ASSERT_TRUE(sender.trySendOutbound(first));
    EXPECT_EQ(sender.m_pending_orders.live_count, 0U);

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::Buy, 100100, 11, 0, 2ULL)));
    EXPECT_TRUE(sender.buildOutboundFrames());
    EXPECT_EQ(sender.m_pending_orders.live_count, 1U);

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, activeGenerationDisconnectRebuildsReplayAndSessionState) {
    TxSender sender(8);
    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::Buy, 123450, 100, 1, 0x1122334455667788ULL)));
    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord first_send {};
    ASSERT_TRUE(sender.popReadyOutbound(first_send));
    EXPECT_EQ(first_send.que_idx, 1U);
    EXPECT_EQ(first_send.event_tag, 0x1122334455667788ULL);

    connectSender(sender, 21);
    disconnectSender(sender, 21);
    ASSERT_EQ(sender.m_blocked_outbound.size(), 1U);

    connectSender(sender, 22);
    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    EXPECT_EQ(login.payload[2], static_cast<uint8_t>('L'));

    sender._flushBlockedRecords();
    TxOutboundRecord replayed {};
    ASSERT_TRUE(sender.popReadyOutbound(replayed));
    EXPECT_EQ(replayed.que_idx, 1U);
    EXPECT_EQ(replayed.event_tag, 0x1122334455667788ULL);
}

TEST(TxTranslatorTest, acceptedExecutionMetadataIsPreservedInOutboundRecord) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x0ee8, OrderIntentAction::Sell, 223450, 200, 1, 0x123456789abcdef0ULL)));
    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord record {};
    ASSERT_TRUE(sender.popReadyOutbound(record));
    EXPECT_EQ(record.que_idx, 1U);
    EXPECT_EQ(record.event_tag, 0x123456789abcdef0ULL);
}

TEST(TxTranslatorTest, invalidExecutionActionDoesNotProduceOrderFrame) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::None, 123450, 100)));
    EXPECT_FALSE(sender.buildOutboundFrames());

    TxOutboundRecord record {};
    EXPECT_FALSE(sender.popReadyOutbound(record));
}

TEST(TxTranslatorTest, pendingCapacityRejectionPreservesExistingReadyQueueRecords) {
    TxSender sender(TxSenderConfig {
        .intent_capacity = 4,
        .pending_capacity = 2,
        .pending_slot_count = 16,
    });

    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100000, 10)));
    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x0ee8, OrderIntentAction::Sell, 100100, 11)));
    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100200, 12)));
    EXPECT_FALSE(sender.buildOutboundFrames());

    TxOutboundRecord first {};
    TxOutboundRecord second {};
    ASSERT_TRUE(sender.popReadyOutbound(first));
    ASSERT_TRUE(sender.popReadyOutbound(second));
    EXPECT_EQ(first.user_ref_num, 1U);
    EXPECT_EQ(second.user_ref_num, 2U);
    EXPECT_FALSE(sender.popReadyOutbound(second));
}

TEST(TxTranslatorTest, pendingCapacityRejectionDoesNotSkipRemainingReadyRecord) {
    TxSender sender(TxSenderConfig {
        .intent_capacity = 4,
        .pending_capacity = 2,
        .pending_slot_count = 16,
    });

    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100000, 10)));
    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x0ee8, OrderIntentAction::Sell, 100100, 11)));
    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord first {};
    ASSERT_TRUE(sender.popReadyOutbound(first));
    EXPECT_EQ(first.user_ref_num, 1U);

    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100200, 12)));
    EXPECT_FALSE(sender.buildOutboundFrames());

    TxOutboundRecord second {};
    ASSERT_TRUE(sender.popReadyOutbound(second));
    EXPECT_EQ(second.user_ref_num, 2U);
    EXPECT_FALSE(sender.popReadyOutbound(second));
}

TEST(TxTranslatorTest, pendingCapacityRejectionPreservesExistingBlockedQueueRecords) {
    TxSender sender(TxSenderConfig {
        .intent_capacity = 4,
        .pending_capacity = 2,
        .pending_slot_count = 16,
    });

    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100000, 10)));
    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x0ee8, OrderIntentAction::Sell, 100100, 11)));
    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100200, 12)));
    EXPECT_FALSE(sender.buildOutboundFrames());

    sender.onTransportDisconnected();
    sender._flushBlockedRecords();

    TxOutboundRecord first {};
    TxOutboundRecord second {};
    ASSERT_TRUE(sender.popReadyOutbound(first));
    ASSERT_TRUE(sender.popReadyOutbound(second));
    EXPECT_EQ(first.user_ref_num, 1U);
    EXPECT_EQ(second.user_ref_num, 2U);
    EXPECT_FALSE(sender.popReadyOutbound(second));
}

TEST(TxTranslatorTest, replayedOutboundPreservesMetadataAfterDisconnect) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::Buy, 123450, 100, 1, 0x1122334455667788ULL)));
    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord first_send {};
    ASSERT_TRUE(sender.popReadyOutbound(first_send));
    EXPECT_EQ(first_send.que_idx, 1U);
    EXPECT_EQ(first_send.event_tag, 0x1122334455667788ULL);

    sender.onTransportDisconnected();
    connectSender(sender, 2);
    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    EXPECT_EQ(login.payload[2], static_cast<uint8_t>('L'));

    sender._flushBlockedRecords();
    TxOutboundRecord replayed {};
    ASSERT_TRUE(sender.popReadyOutbound(replayed));
    EXPECT_EQ(replayed.que_idx, 1U);
    EXPECT_EQ(replayed.event_tag, 0x1122334455667788ULL);
}

TEST(TxTranslatorTest, readyOutboundDoesNotDrainAcceptedExecutionsUntilExplicitStepRuns) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::Buy, 123450, 100)));

    TxOutboundRecord record {};
    EXPECT_FALSE(sender.popReadyOutbound(record));
    EXPECT_TRUE(sender.buildOutboundFrames());
    ASSERT_TRUE(sender.popReadyOutbound(record));
}

TEST(TxTranslatorTest, senderRejectsNonPowerOfTwoPendingSlotCount) {
    EXPECT_THROW((void)TxSender(TxSenderConfig {
                     .pending_capacity = 8,
                     .pending_slot_count = 12,
                 }),
                 std::invalid_argument);
}

TEST(TxTranslatorTest, senderRejectsNewPendingOrderWhenPendingCapacityReached) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 2,
        .pending_slot_count = 16,
    });

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 1ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 100, .shares = 10},
    }));
    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 2ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 101, .shares = 11},
    }));
    ASSERT_TRUE(sender.buildOutboundFrames());

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 3ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 102, .shares = 12},
    }));
    EXPECT_FALSE(sender.buildOutboundFrames());
}

TEST(TxTranslatorTest, senderRejectsPendingInsertWhenMaskedSlotCollides) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 8,
    });
    sender.m_next_tag = 1U;

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 11ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 100, .shares = 10},
    }));
    ASSERT_TRUE(sender.buildOutboundFrames());

    sender.m_next_tag = 9U;
    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 22ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 101, .shares = 11},
    }));
    EXPECT_FALSE(sender.buildOutboundFrames());
}

TEST(TxTranslatorTest, rebuildBlockedRecordsRestoresAscendingUserRefOrder) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 32,
    });

    sender.m_pending_orders.live_count = 3;
    sender.m_pending_orders.slots[7] = TxSender::PendingSlot {
        .occupied = true,
        .record = TxOutboundRecord {.user_ref_num = 7, .price = 700},
    };
    sender.m_pending_orders.slots[2] = TxSender::PendingSlot {
        .occupied = true,
        .record = TxOutboundRecord {.user_ref_num = 2, .price = 200},
    };
    sender.m_pending_orders.slots[11] = TxSender::PendingSlot {
        .occupied = true,
        .record = TxOutboundRecord {.user_ref_num = 11, .price = 1100},
    };

    sender._rebuildBlockedRecords();

    ASSERT_EQ(sender.m_blocked_outbound.size(), 3U);
    EXPECT_EQ(sender.m_blocked_outbound[0].user_ref_num, 2U);
    EXPECT_EQ(sender.m_blocked_outbound[1].user_ref_num, 7U);
    EXPECT_EQ(sender.m_blocked_outbound[2].user_ref_num, 11U);
}

TEST(TxTranslatorTest, trackedAcceptedExecutionPushesTxExecutionAcceptedRecord) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(2, 8);
    sender.attachLatenyTracker(&tracker);
    seedTrackedLatencyFlow(tracker, 1, 0x12345678ULL);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x12345678ULL,
        .trace_id = 1U,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));
    EXPECT_EQ(static_cast<int>(stage::TX_EXECUTION_DEQUEUE), 7);
    EXPECT_EQ(static_cast<int>(stage::TX_ORDER_FRAME_BUILT), 8);
    EXPECT_EQ(static_cast<int>(stage::TX_PENDING_RECORDED), 9);
    std::vector<TimeRecord> records;
    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
    while (tracker.m_latency_queues[1]->pop(record)) {
        records.push_back(record);
    }
    ASSERT_FALSE(records.empty());
    EXPECT_EQ(records.back().event_stage, stage::TX_EXECUTION_ACCEPTED);
    EXPECT_EQ(records.back().que_idx, 1U);
    EXPECT_EQ(records.back().event_tag, 0x12345678ULL);
    EXPECT_EQ(records.back().trace_id, 1U);
    EXPECT_GT(records.back().time_captured, 0U);

    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    EXPECT_EQ(outbound.que_idx, 1U);
    EXPECT_EQ(outbound.event_tag, 0x12345678ULL);
    EXPECT_EQ(outbound.trace_id, 1U);
}

TEST(TxTranslatorTest, buildOutboundFramesPushesTxEnqueueWithTraceId) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(2, 8);
    sender.attachLatenyTracker(&tracker);
    seedTrackedLatencyFlow(tracker, 1, 0x12345678ULL);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x12345678ULL,
        .trace_id = 1U,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    TimeRecord record {};
    while (tracker.m_latency_queues[1]->pop(record)) {
    }

    ASSERT_TRUE(sender.buildOutboundFrames());

    ASSERT_TRUE(tracker.m_latency_queues[1]->pop(record));
    EXPECT_EQ(record.event_stage, stage::TX_ENQUEUE);
    EXPECT_EQ(record.trace_id, 1U);
    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));
}

TEST(TxTranslatorTest, untrackedExecutionDoesNotPushSenderLatencyStages) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatenyTracker(&tracker);
    sender.m_send_fd = sockets[0];
    sender.m_transport_generation = 42;

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x88776655ULL,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));
    EXPECT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    ASSERT_TRUE(sender.trySendOutbound(outbound));

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));

    std::array<uint8_t, 64> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), outbound.payload_length, 0),
              static_cast<ssize_t>(outbound.payload_length));

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, trySendOutboundEmitsTxSendEnterOnlyAfterPayloadGuardsPass) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatenyTracker(&tracker);
    seedTrackedLatencyFlow(tracker, 1, 0x55667788ULL);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x55667788ULL,
        .trace_id = 1U,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    EXPECT_FALSE(sender.trySendOutbound(outbound));

    TimeRecord record {};
    std::vector<stage> failed_send_stages;
    while (tracker.m_latency_queues[1]->pop(record)) {
        failed_send_stages.push_back(record.event_stage);
    }

    EXPECT_NE(std::find(failed_send_stages.begin(), failed_send_stages.end(), stage::TX_ENQUEUE),
              failed_send_stages.end());
    EXPECT_EQ(std::find(failed_send_stages.begin(),
                        failed_send_stages.end(),
                        stage::TX_SEND_ENTER),
              failed_send_stages.end());
    EXPECT_EQ(std::find(failed_send_stages.begin(),
                        failed_send_stages.end(),
                        stage::TX_SEND_SYSCALL_ENTER),
              failed_send_stages.end());
    EXPECT_EQ(std::find(failed_send_stages.begin(), failed_send_stages.end(), stage::TX_SEND),
              failed_send_stages.end());
}

TEST(TxTranslatorTest, tracedEnqueuePreservesTraceId) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatenyTracker(&tracker);
    seedTrackedLatencyFlow(tracker, 1, 0x1001ULL, 1U);
    seedTrackedLatencyFlow(tracker, 1, 0x1002ULL, 2U);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x1001ULL,
        .trace_id = 1U,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));
    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x1002ULL,
        .trace_id = 2U,
        .order = {.action = OrderIntentAction::Sell, .price = 223451, .shares = 200},
    }));

    ASSERT_TRUE(sender.buildOutboundFrames());

    TimeRecord record {};
    std::vector<TimeRecord> enqueue_records;
    while (tracker.m_latency_queues[1]->pop(record)) {
        if (record.event_stage == stage::TX_ENQUEUE) {
            enqueue_records.push_back(record);
        }
    }

    ASSERT_EQ(enqueue_records.size(), 2U);
    EXPECT_EQ(enqueue_records[0].trace_id, 1U);
    EXPECT_EQ(enqueue_records[1].trace_id, 2U);
}

TEST(TxTranslatorTest, successfulTrackedSendQueuesFinalizeWithoutPublishingInline) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyAnalyzer analyzer(2);
    LatencyTracker tracker(2, 16);
    tracker.attachAnalyzer(&analyzer);
    sender.attachLatenyTracker(&tracker);
    sender.m_send_fd = sockets[0];
    sender.m_transport_generation = 42;
    seedTrackedLatencyFlow(tracker, 1, 0x2001ULL);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x2001ULL,
        .trace_id = 1U,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    ASSERT_TRUE(sender.trySendOutbound(outbound));

    TimeRecord record {};
    EXPECT_TRUE(analyzer.m_completed_records[1].empty());
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 1U);

    tracker.stop();
    EXPECT_TRUE(analyzer.m_completed_records[1].empty());
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 1U);
    tracker.run();

    ASSERT_EQ(analyzer.m_completed_records[1].size(), 1U);
    EXPECT_EQ(analyzer.m_completed_records[1][0].event_tag, 0x2001ULL);
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 0U);
    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));

    std::array<uint8_t, 2> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, pendingRejectQueuesDropWithoutDroppingInline) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 0,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatenyTracker(&tracker);
    seedTrackedLatencyFlow(tracker, 1, 0x3001ULL);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x3001ULL,
        .trace_id = 1U,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    EXPECT_FALSE(sender.buildOutboundFrames());
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 1U);

    tracker.stop();
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 1U);
    tracker.run();

    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 0U);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));
}

TEST(TxTranslatorTest, invalidTrackedExecutionQueuesDropWithoutDroppingInline) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatenyTracker(&tracker);
    seedTrackedLatencyFlow(tracker, 1, 0x3002ULL);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x3002ULL,
        .trace_id = 1U,
        .order = {.action = OrderIntentAction::None, .price = 223450, .shares = 200},
    }));

    EXPECT_FALSE(sender.buildOutboundFrames());
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 1U);

    TxOutboundRecord outbound {};
    EXPECT_FALSE(sender.popReadyOutbound(outbound));

    tracker.stop();
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 1U);
    tracker.run();

    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 0U);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));
}

TEST(TxTranslatorTest, backpressuredFinalizeIsRetriedLater) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyAnalyzer analyzer(2);
    LatencyTracker tracker(2, 16, 1);
    tracker.attachAnalyzer(&analyzer);
    sender.attachLatenyTracker(&tracker);
    sender.m_send_fd = sockets[0];
    sender.m_transport_generation = 42;
    seedTrackedLatencyFlow(tracker, 1, 0x4001ULL);

    const TraceCommand blocker {
        .que_idx = 1,
        .trace_id = 99U,
        .op = TraceCommandOp::Drop,
    };
    ASSERT_TRUE(tracker.m_trace_command_queues[1]->push(blocker));

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x4001ULL,
        .trace_id = 1U,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));
    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    ASSERT_TRUE(sender.trySendOutbound(outbound));

    EXPECT_TRUE(analyzer.m_completed_records[1].empty());
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 1U);

    tracker.stop();
    tracker.run();
    EXPECT_TRUE(analyzer.m_completed_records[1].empty());
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 1U);

    EXPECT_TRUE(sender.runOnce());
    tracker.run();

    ASSERT_EQ(analyzer.m_completed_records[1].size(), 1U);
    EXPECT_EQ(analyzer.m_completed_records[1][0].event_tag, 0x4001ULL);
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 0U);

    std::array<uint8_t, 2> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, backpressuredDropIsRetriedLater) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 0,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(2, 16, 1);
    sender.attachLatenyTracker(&tracker);
    seedTrackedLatencyFlow(tracker, 1, 0x4002ULL);

    const TraceCommand blocker {
        .que_idx = 1,
        .trace_id = 99U,
        .op = TraceCommandOp::Finalize,
    };
    ASSERT_TRUE(tracker.m_trace_command_queues[1]->push(blocker));

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x4002ULL,
        .trace_id = 1U,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    EXPECT_FALSE(sender.buildOutboundFrames());
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 1U);

    tracker.stop();
    tracker.run();
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 1U);

    EXPECT_FALSE(sender.runOnce());
    tracker.run();

    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 0U);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));
}
