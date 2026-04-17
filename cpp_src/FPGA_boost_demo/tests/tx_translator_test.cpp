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
                             uint64_t event_tag = 0) {
    return OrderExecution {
        .stock_locate = stock_locate,
        .que_idx = que_idx,
        .event_tag = event_tag,
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

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x12345678ULL,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));
    EXPECT_EQ(static_cast<int>(stage::TX_EXECUTION_DEQUEUE), 7);
    EXPECT_EQ(static_cast<int>(stage::TX_ORDER_FRAME_BUILT), 8);
    EXPECT_EQ(static_cast<int>(stage::TX_PENDING_RECORDED), 9);
    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[0]->pop(record));
    ASSERT_TRUE(tracker.m_latency_queues[1]->pop(record));
    EXPECT_EQ(record.event_stage, stage::TX_EXECUTION_ACCEPTED);
    EXPECT_EQ(record.que_idx, 1U);
    EXPECT_EQ(record.event_tag, 0x12345678ULL);
    EXPECT_GT(record.time_captured, 0U);
    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));

    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    EXPECT_EQ(outbound.que_idx, 1U);
    EXPECT_EQ(outbound.event_tag, 0x12345678ULL);
}

TEST(TxTranslatorTest, trackedAcceptedExecutionStillFlowsThroughSenderStages) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(2, 8, 8);
    sender.attachLatenyTracker(&tracker);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x12345678ULL,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    TimeRecord record {};
    ASSERT_TRUE(tracker.m_latency_queues[1]->pop(record));
    EXPECT_EQ(record.event_stage, stage::TX_EXECUTION_ACCEPTED);
    EXPECT_EQ(record.trace_id, 0U);
}

TEST(TxTranslatorTest, buildOutboundFramesPushesOnlyEnqueueSenderLocalLatencyStage) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatenyTracker(&tracker);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x10203040ULL,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    TimeRecord record {};
    ASSERT_TRUE(tracker.m_latency_queues[1]->pop(record));
    EXPECT_EQ(record.event_stage, stage::TX_EXECUTION_ACCEPTED);

    ASSERT_TRUE(sender.buildOutboundFrames());

    bool saw_tx_enqueue = false;
    while (tracker.m_latency_queues[1]->pop(record)) {
        saw_tx_enqueue = saw_tx_enqueue || (record.event_stage == stage::TX_ENQUEUE);
        EXPECT_NE(record.event_stage, stage::TX_EXECUTION_DEQUEUE);
        EXPECT_NE(record.event_stage, stage::TX_ORDER_FRAME_BUILT);
        EXPECT_NE(record.event_stage, stage::TX_PENDING_RECORDED);
    }

    EXPECT_TRUE(saw_tx_enqueue);
}

TEST(TxTranslatorTest, trySendOutboundEmitsTxSendEnterOnlyAfterPayloadGuardsPass) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatenyTracker(&tracker);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x55667788ULL,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    TimeRecord record {};
    ASSERT_TRUE(tracker.m_latency_queues[1]->pop(record));
    EXPECT_EQ(record.event_stage, stage::TX_EXECUTION_ACCEPTED);

    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    ASSERT_TRUE(sender.trySendOutbound(outbound) == false);

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

    connectSender(sender, 42, sockets[0]);
    ASSERT_TRUE(sender.trySendOutbound(outbound));

    std::vector<stage> successful_send_stages;
    while (tracker.m_latency_queues[1]->pop(record)) {
        successful_send_stages.push_back(record.event_stage);
    }

    EXPECT_EQ(std::find(successful_send_stages.begin(),
                        successful_send_stages.end(),
                        stage::TX_ENQUEUE),
              successful_send_stages.end());

    std::vector<stage> all_seen_stages = failed_send_stages;
    all_seen_stages.insert(all_seen_stages.end(),
                           successful_send_stages.begin(),
                           successful_send_stages.end());

    const auto tx_enqueue_it =
        std::find(all_seen_stages.begin(), all_seen_stages.end(), stage::TX_ENQUEUE);
    const auto tx_send_enter_it =
        std::find(all_seen_stages.begin(), all_seen_stages.end(), stage::TX_SEND_ENTER);
    const auto tx_send_syscall_enter_it =
        std::find(all_seen_stages.begin(), all_seen_stages.end(), stage::TX_SEND_SYSCALL_ENTER);
    const auto tx_send_it =
        std::find(all_seen_stages.begin(), all_seen_stages.end(), stage::TX_SEND);

    EXPECT_NE(tx_enqueue_it, all_seen_stages.end());
    EXPECT_NE(tx_send_enter_it, all_seen_stages.end());
    EXPECT_NE(tx_send_syscall_enter_it, all_seen_stages.end());
    EXPECT_NE(tx_send_it, all_seen_stages.end());
    EXPECT_LT(tx_enqueue_it, tx_send_enter_it);
    EXPECT_LT(tx_send_enter_it, tx_send_syscall_enter_it);
    EXPECT_LT(tx_send_syscall_enter_it, tx_send_it);

    std::array<uint8_t, 2> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, senderBacklogDepthIsCapturedAtEnqueueAndSendEnter) {
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
        .event_tag = 0x1001ULL,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));
    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x1002ULL,
        .order = {.action = OrderIntentAction::Sell, .price = 223451, .shares = 200},
    }));

    TimeRecord record {};
    ASSERT_TRUE(tracker.m_latency_queues[1]->pop(record));
    EXPECT_EQ(record.event_stage, stage::TX_EXECUTION_ACCEPTED);
    ASSERT_TRUE(tracker.m_latency_queues[1]->pop(record));
    EXPECT_EQ(record.event_stage, stage::TX_EXECUTION_ACCEPTED);

    ASSERT_TRUE(sender.buildOutboundFrames());

    std::vector<TimeRecord> enqueue_records;
    while (tracker.m_latency_queues[1]->pop(record)) {
        if (record.event_stage == stage::TX_ENQUEUE) {
            enqueue_records.push_back(record);
        }
    }

    ASSERT_EQ(enqueue_records.size(), 2U);
    EXPECT_EQ(enqueue_records[0].sender_backlog_depth, 0U);
    EXPECT_EQ(enqueue_records[1].sender_backlog_depth, 1U);
    EXPECT_EQ(enqueue_records[0].trace_id, 0U);
    EXPECT_EQ(enqueue_records[1].trace_id, 0U);

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    ASSERT_TRUE(sender.trySendOutbound(outbound));

    bool saw_tx_send_enter = false;
    while (tracker.m_latency_queues[1]->pop(record)) {
        if (record.event_stage != stage::TX_SEND_ENTER) {
            continue;
        }
        saw_tx_send_enter = true;
        EXPECT_EQ(record.event_tag, 0x1001ULL);
        EXPECT_EQ(record.sender_backlog_depth, 1U);
    }

    EXPECT_TRUE(saw_tx_send_enter);

    std::array<uint8_t, 2> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, sendLoopStatsAreCapturedOnTxSendRecord) {
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
        .event_tag = 0x2001ULL,
        .order = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    TimeRecord record {};
    ASSERT_TRUE(tracker.m_latency_queues[1]->pop(record));
    EXPECT_EQ(record.event_stage, stage::TX_EXECUTION_ACCEPTED);

    ASSERT_TRUE(sender.buildOutboundFrames());
    while (tracker.m_latency_queues[1]->pop(record)) {
    }

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    ASSERT_TRUE(sender.trySendOutbound(outbound));

    bool saw_tx_send_syscall_enter = false;
    bool saw_tx_send = false;
    while (tracker.m_latency_queues[1]->pop(record)) {
        if (record.event_stage == stage::TX_SEND_SYSCALL_ENTER) {
            saw_tx_send_syscall_enter = true;
            EXPECT_EQ(record.event_tag, 0x2001ULL);
        }
        if (record.event_stage != stage::TX_SEND) {
            continue;
        }
        saw_tx_send = true;
        EXPECT_EQ(record.event_tag, 0x2001ULL);
        EXPECT_EQ(record.tx_send_call_count, 1U);
        EXPECT_EQ(record.tx_send_bytes_total, outbound.payload_length);
        EXPECT_EQ(record.tx_send_eintr_retry_count, 0U);
        EXPECT_EQ(record.tx_send_had_partial_write, 0U);
    }

    EXPECT_TRUE(saw_tx_send_syscall_enter);
    EXPECT_TRUE(saw_tx_send);

    std::array<uint8_t, 2> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));

    ::close(sockets[1]);
}
