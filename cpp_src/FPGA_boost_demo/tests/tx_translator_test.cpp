#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <fcntl.h>
#include <stdexcept>
#include <sys/socket.h>
#include <type_traits>
#include <unistd.h>
#include <vector>

#include "../common/shared_types.h"
#define private public
#include "../tx_engine/tx_client.h"
#include "../tx_engine/tx_receiver.h"
#include "../tx_engine/tx_sender.h"
#include "../latency/latency_analyzer.h"
#include "../latency/latency_tracker.h"
#undef private
#define buildOutboundFrames _queueOutFrames
#define queueHeartbeat _queueHeartbeat

static_assert(std::is_member_function_pointer_v<decltype(&TxSender::acceptExecution)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::updateConnectionInfo)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::runOnce)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::buildOutboundFrames)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::queueHeartbeat)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::_popReadyOutbound)>);

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

bool takeReadyOutbound(TxSender& sender, TxOutboundRecord& record) {
    if (sender.m_ready_outbound.isEmpty()) {
        return false;
    }

    if (!sender._popReadyOutbound(record)) {
        return false;
    }

    return sender.m_ready_outbound.eraseFront();
}

} // namespace

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
    ASSERT_TRUE(sender._sendPayload(outbound));

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
    ASSERT_TRUE(sender._sendPayload(outbound));

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
    ASSERT_TRUE(takeReadyOutbound(sender, record));
    EXPECT_EQ(record.user_ref_num, 0U);
    ASSERT_GE(record.payload_length, 3U);
    EXPECT_EQ(record.payload[2], static_cast<uint8_t>('L'));
}

TEST(TxTranslatorTest, successfulSendClearsPendingOrderWithoutReceiverFlow) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(TxSenderConfig {
        .pending_capacity = 1,
    });
    sender.m_send_fd = sockets[0];
    sender.m_transport_generation = 42;

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::Buy, 100000, 10, 0, 1ULL)));
    ASSERT_TRUE(sender.buildOutboundFrames());
    ASSERT_EQ(sender.m_pending_orders.live_count, 1U);

    TxOutboundRecord first {};
    ASSERT_TRUE(takeReadyOutbound(sender, first));
    ASSERT_TRUE(sender._sendPayload(first));
    EXPECT_EQ(sender.m_pending_orders.live_count, 0U);

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::Buy, 100100, 11, 0, 2ULL)));
    EXPECT_TRUE(sender.buildOutboundFrames());
    EXPECT_EQ(sender.m_pending_orders.live_count, 1U);

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, runOncePollsReceiverFeedbackInlineOnSharedNonBlockingSocket) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
    ASSERT_GE(::fcntl(sockets[0], F_SETFL, ::fcntl(sockets[0], F_GETFL, 0) | O_NONBLOCK), 0);

    TxClient client(GatewayClientConfig {},
                    TxSenderConfig {
                        .pending_capacity = 8,
                    },
                    8);
    TxSender& sender = client.m_sender;
    TxReceiver& receiver = client.m_receiver;
    connectSender(sender, 7, sockets[0]);
    sender.m_logged_in = true;
    sender.m_login_pending = false;
    sender.m_ready_outbound.clear();

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::Buy, 100000, 10, 0, 1ULL)));

    ASSERT_TRUE(client.runOnce());

    std::array<uint8_t, 19> outbound {};
    ASSERT_EQ(::recv(sockets[1], outbound.data(), outbound.size(), 0),
              static_cast<ssize_t>(outbound.size()));
    EXPECT_EQ(outbound[2], static_cast<uint8_t>('U'));

    const std::vector<uint8_t> accepted_frame {
        0x00, 0x0e, static_cast<uint8_t>('S'), static_cast<uint8_t>('A'),
        0, 0, 0, 0, 0, 0, 0, 0,
        0x00, 0x00, 0x00, 0x01,
    };
    ASSERT_EQ(::write(sockets[1], accepted_frame.data(), accepted_frame.size()),
              static_cast<ssize_t>(accepted_frame.size()));

    ASSERT_TRUE(client.runOnce());

    const TxReceiverStats stats = receiver.readStats();
    EXPECT_EQ(stats.sent, 1U);
    EXPECT_EQ(stats.accepted, 1U);
    EXPECT_EQ(stats.malformed, 0U);

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, activeGenerationDisconnectRebuildsReplayAndSessionState) {
    TxSender sender(8);
    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::Buy, 123450, 100, 1, 0x1122334455667788ULL)));
    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord first_send {};
    ASSERT_TRUE(takeReadyOutbound(sender, first_send));
    EXPECT_EQ(first_send.que_idx, 1U);
    EXPECT_EQ(first_send.event_tag, 0x1122334455667788ULL);

    connectSender(sender, 21);
    disconnectSender(sender, 21);


    connectSender(sender, 22);
    TxOutboundRecord login {};
    ASSERT_TRUE(takeReadyOutbound(sender, login));
    EXPECT_EQ(login.payload[2], static_cast<uint8_t>('L'));


    TxOutboundRecord replayed {};
    ASSERT_TRUE(takeReadyOutbound(sender, replayed));
    EXPECT_EQ(replayed.que_idx, 1U);
    EXPECT_EQ(replayed.event_tag, 0x1122334455667788ULL);
}

TEST(TxTranslatorTest, acceptedExecutionMetadataIsPreservedInOutboundRecord) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x0ee8, OrderIntentAction::Sell, 223450, 200, 1, 0x123456789abcdef0ULL)));
    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord record {};
    ASSERT_TRUE(takeReadyOutbound(sender, record));
    EXPECT_EQ(record.que_idx, 1U);
    EXPECT_EQ(record.event_tag, 0x123456789abcdef0ULL);
}

TEST(TxTranslatorTest, invalidExecutionActionDoesNotProduceOrderFrame) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::None, 123450, 100)));
    EXPECT_FALSE(sender.buildOutboundFrames());

    EXPECT_TRUE(sender.m_ready_outbound.isEmpty());
}

TEST(TxTranslatorTest, pendingCapacityRejectionPreservesExistingReadyQueueRecords) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 2,
    });

    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100000, 10)));
    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x0ee8, OrderIntentAction::Sell, 100100, 11)));
    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100200, 12)));
    EXPECT_FALSE(sender.buildOutboundFrames());

    TxOutboundRecord first {};
    TxOutboundRecord second {};
    ASSERT_TRUE(takeReadyOutbound(sender, first));
    ASSERT_TRUE(takeReadyOutbound(sender, second));
    EXPECT_EQ(first.user_ref_num, 1U);
    EXPECT_EQ(second.user_ref_num, 2U);
    EXPECT_FALSE(takeReadyOutbound(sender, second));
}

TEST(TxTranslatorTest, pendingCapacityRejectionDoesNotSkipRemainingReadyRecord) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 2,
    });

    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100000, 10)));
    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x0ee8, OrderIntentAction::Sell, 100100, 11)));
    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord first {};
    ASSERT_TRUE(takeReadyOutbound(sender, first));
    EXPECT_EQ(first.user_ref_num, 1U);

    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100200, 12)));
    EXPECT_FALSE(sender.buildOutboundFrames());

    TxOutboundRecord second {};
    ASSERT_TRUE(takeReadyOutbound(sender, second));
    EXPECT_EQ(second.user_ref_num, 2U);
    EXPECT_FALSE(takeReadyOutbound(sender, second));
}

TEST(TxTranslatorTest, pendingCapacityRejectionClearsBlockedQueueRecordsOnDisconnect) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 2,
    });

    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100000, 10)));
    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x0ee8, OrderIntentAction::Sell, 100100, 11)));
    ASSERT_TRUE(sender.acceptExecution(makeExecution(0x000d, OrderIntentAction::Buy, 100200, 12)));
    EXPECT_FALSE(sender.buildOutboundFrames());

    sender._onTransportDisconnected();

    TxOutboundRecord first {};
    EXPECT_FALSE(takeReadyOutbound(sender, first));
}

TEST(TxTranslatorTest, replayedOutboundPreservesMetadataAfterDisconnect) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::Buy, 123450, 100, 1, 0x1122334455667788ULL)));
    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord first_send {};
    ASSERT_TRUE(takeReadyOutbound(sender, first_send));
    EXPECT_EQ(first_send.que_idx, 1U);
    EXPECT_EQ(first_send.event_tag, 0x1122334455667788ULL);

    sender._onTransportDisconnected();
    connectSender(sender, 2);
    TxOutboundRecord login {};
    ASSERT_TRUE(takeReadyOutbound(sender, login));
    EXPECT_EQ(login.payload[2], static_cast<uint8_t>('L'));

    TxOutboundRecord replayed {};
    ASSERT_TRUE(takeReadyOutbound(sender, replayed));
    EXPECT_EQ(replayed.que_idx, 1U);
    EXPECT_EQ(replayed.event_tag, 0x1122334455667788ULL);
}

TEST(TxTranslatorTest, readyOutboundDoesNotDrainAcceptedExecutionsUntilExplicitStepRuns) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptExecution(
        makeExecution(0x000d, OrderIntentAction::Buy, 123450, 100)));

    EXPECT_TRUE(sender.m_ready_outbound.isEmpty());
    EXPECT_TRUE(sender.buildOutboundFrames());
    TxOutboundRecord record {};
    ASSERT_TRUE(takeReadyOutbound(sender, record));
}

TEST(TxTranslatorTest, senderRejectsNonPowerOfTwoPendingSlotCount) {
    EXPECT_THROW((void)TxSender(TxSenderConfig {
                     .pending_capacity = 8,
                 }),
                 std::invalid_argument);
}

TEST(TxTranslatorTest, senderRejectsNewPendingOrderWhenPendingCapacityReached) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 2,
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

}

TEST(TxTranslatorTest, trackedAcceptedExecutionPushesTxExecutionAcceptedRecord) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
    });
    LatencyTracker tracker(2, 8);
    sender.attachLatencyTracker(&tracker);
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
    EXPECT_EQ(records.back().event_stage, stage::TX_SENDER_EXECUTION_ACCEPTED);
    EXPECT_EQ(records.back().que_idx, 1U);
    EXPECT_EQ(records.back().event_tag, 0x12345678ULL);
    EXPECT_EQ(records.back().trace_id, 1U);
    EXPECT_GT(records.back().time_captured, 0U);

    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord outbound {};
    ASSERT_TRUE(takeReadyOutbound(sender, outbound));
    EXPECT_EQ(outbound.que_idx, 1U);
    EXPECT_EQ(outbound.event_tag, 0x12345678ULL);
    EXPECT_EQ(outbound.trace_id, 1U);
}

TEST(TxTranslatorTest, buildOutboundFramesDoesNotPushLatencyRecords) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
    });
    LatencyTracker tracker(2, 8);
    sender.attachLatencyTracker(&tracker);
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

    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));
}

TEST(TxTranslatorTest, untrackedExecutionDoesNotPushSenderLatencyStages) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatencyTracker(&tracker);
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
    ASSERT_TRUE(takeReadyOutbound(sender, outbound));

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));

    std::array<uint8_t, 64> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), outbound.payload_length, 0),
              static_cast<ssize_t>(outbound.payload_length));

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, trySendOutboundDoesNotEmitSendStagesWhenPayloadGuardsFail) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatencyTracker(&tracker);
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
    ASSERT_TRUE(takeReadyOutbound(sender, outbound));

    TimeRecord record {};
    std::vector<stage> failed_send_stages;
    while (tracker.m_latency_queues[1]->pop(record)) {
        failed_send_stages.push_back(record.event_stage);
    }

    EXPECT_EQ(std::find(failed_send_stages.begin(),
                        failed_send_stages.end(),
                        stage::TX_SEND_SYSCALL_ENTER),
              failed_send_stages.end());
    EXPECT_EQ(std::find(failed_send_stages.begin(), failed_send_stages.end(), stage::TX_SEND),
              failed_send_stages.end());
}

TEST(TxTranslatorTest, buildOutboundFramesDoesNotEmitTxSendEnqueue) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatencyTracker(&tracker);
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

    TimeRecord record {};
    while (tracker.m_latency_queues[1]->pop(record)) {
    }

    ASSERT_TRUE(sender.buildOutboundFrames());

    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));
}

TEST(TxTranslatorTest, successfulTrackedSendQueuesFinalizeWithoutPublishingInline) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
    });
    LatencyAnalyzer analyzer(2);
    LatencyTracker tracker(2, 16);
    tracker.attachAnalyzer(&analyzer);
    sender.attachLatencyTracker(&tracker);
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

    EXPECT_TRUE(sender.runOnce());

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

    std::array<uint8_t, 64> received {};
    ASSERT_GT(::recv(sockets[1], received.data(), received.size(), 0), 0);

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, pendingRejectQueuesDropWithoutDroppingInline) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 0,
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatencyTracker(&tracker);
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
    });
    LatencyTracker tracker(2, 16);
    sender.attachLatencyTracker(&tracker);
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

    EXPECT_TRUE(sender.m_ready_outbound.isEmpty());

    tracker.stop();
    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 1U);
    tracker.run();

    EXPECT_EQ(tracker.m_active_trace_ids[1].load(std::memory_order_acquire), 0U);

    TimeRecord record {};
    EXPECT_FALSE(tracker.m_latency_queues[1]->pop(record));
}
