#include "../common/shared_types.h"
#include "../tx_engine/tx_sender.h"

#define private public
#include "../latency/latency_tracker.h"
#undef private

#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <thread>
#include <type_traits>
#include <vector>

static_assert(std::is_member_function_pointer_v<decltype(&TxSender::acceptIntent)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::acceptInboundFrame)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::acceptTransportControl)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::buildOutboundFrame)>);

namespace {

void writeBigEndian16(uint8_t* out, uint16_t value) {
    out[0] = static_cast<uint8_t>((value >> 8) & 0xffU);
    out[1] = static_cast<uint8_t>(value & 0xffU);
}

void writeBigEndian32(uint8_t* out, uint32_t value) {
    out[0] = static_cast<uint8_t>((value >> 24) & 0xffU);
    out[1] = static_cast<uint8_t>((value >> 16) & 0xffU);
    out[2] = static_cast<uint8_t>((value >> 8) & 0xffU);
    out[3] = static_cast<uint8_t>(value & 0xffU);
}

void writeBigEndian64(uint8_t* out, uint64_t value) {
    out[0] = static_cast<uint8_t>((value >> 56) & 0xffU);
    out[1] = static_cast<uint8_t>((value >> 48) & 0xffU);
    out[2] = static_cast<uint8_t>((value >> 40) & 0xffU);
    out[3] = static_cast<uint8_t>((value >> 32) & 0xffU);
    out[4] = static_cast<uint8_t>((value >> 24) & 0xffU);
    out[5] = static_cast<uint8_t>((value >> 16) & 0xffU);
    out[6] = static_cast<uint8_t>((value >> 8) & 0xffU);
    out[7] = static_cast<uint8_t>(value & 0xffU);
}

std::vector<uint8_t> makeLoginAcceptedFrame() {
    std::vector<uint8_t> bytes(33, 0);
    bytes[0] = 0x00;
    bytes[1] = 0x1f;
    bytes[2] = static_cast<uint8_t>('A');

    const std::array<uint8_t, 10> session {'S', 'E', 'S', 'S', 'I', 'O', 'N', '0', '1', ' '};
    for (std::size_t idx = 0; idx < session.size(); ++idx) {
        bytes[3 + idx] = session[idx];
    }

    const std::array<uint8_t, 20> sequence {
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '1'
    };
    for (std::size_t idx = 0; idx < sequence.size(); ++idx) {
        bytes[13 + idx] = sequence[idx];
    }

    return bytes;
}

std::vector<uint8_t> makeSequencedAcceptedFrame(uint32_t user_ref_num,
                                                uint16_t stock_locate,
                                                uint32_t shares,
                                                uint32_t price) {
    (void)stock_locate;
    std::vector<uint8_t> bytes(67, 0);
    bytes[0] = 0x00;
    bytes[1] = 0x41;
    bytes[2] = static_cast<uint8_t>('S');
    bytes[3] = static_cast<uint8_t>('A');

    writeBigEndian32(bytes.data() + 12, user_ref_num);
    bytes[16] = static_cast<uint8_t>('B');
    writeBigEndian32(bytes.data() + 17, shares);
    const std::array<uint8_t, 8> symbol {'A', 'A', 'P', 'L', ' ', ' ', ' ', ' '};
    for (std::size_t idx = 0; idx < symbol.size(); ++idx) {
        bytes[21 + idx] = symbol[idx];
    }
    writeBigEndian64(bytes.data() + 29, static_cast<uint64_t>(price));
    bytes[37] = static_cast<uint8_t>('0');
    bytes[38] = static_cast<uint8_t>('Y');
    writeBigEndian64(bytes.data() + 39, 1U);
    bytes[47] = static_cast<uint8_t>('A');
    bytes[48] = static_cast<uint8_t>('N');
    bytes[49] = static_cast<uint8_t>('N');
    bytes[50] = static_cast<uint8_t>('L');
    bytes[65] = 0x00;
    bytes[66] = 0x00;
    return bytes;
}

std::vector<uint8_t> makeSequencedExecutedFrame(uint32_t user_ref_num,
                                                uint32_t executed_shares,
                                                uint32_t price,
                                                uint64_t match_number) {
    std::vector<uint8_t> bytes(39, 0);
    bytes[0] = 0x00;
    bytes[1] = 0x25;
    bytes[2] = static_cast<uint8_t>('S');
    bytes[3] = static_cast<uint8_t>('E');
    writeBigEndian32(bytes.data() + 12, user_ref_num);
    writeBigEndian32(bytes.data() + 16, executed_shares);
    writeBigEndian64(bytes.data() + 20, static_cast<uint64_t>(price));
    bytes[28] = static_cast<uint8_t>('A');
    writeBigEndian64(bytes.data() + 29, match_number);
    writeBigEndian16(bytes.data() + 37, 0);
    return bytes;
}

std::vector<uint8_t> makeSequencedRejectedFrame(uint32_t user_ref_num, uint16_t reason) {
    std::vector<uint8_t> bytes(34, 0);
    bytes[0] = 0x00;
    bytes[1] = 0x20;
    bytes[2] = static_cast<uint8_t>('S');
    bytes[3] = static_cast<uint8_t>('J');
    writeBigEndian32(bytes.data() + 12, user_ref_num);
    writeBigEndian16(bytes.data() + 16, reason);
    writeBigEndian16(bytes.data() + 32, 0);
    return bytes;
}

TxInboundFrame toInboundFrame(const std::vector<uint8_t>& bytes) {
    TxInboundFrame frame {};
    const std::size_t copy_size = std::min(bytes.size(), frame.payload.size());
    std::copy_n(bytes.begin(), static_cast<std::ptrdiff_t>(copy_size), frame.payload.begin());
    frame.payload_length = static_cast<uint8_t>(copy_size);
    return frame;
}

} // namespace

TEST(TxTranslatorTest, heartbeatTimingIsBasedOnSuccessfulSendsNotPopOrQueueTime) {
    using namespace std::chrono_literals;

    TxSender sender(TxSenderConfig {
        .username = "client",
        .password = "secret",
        .requested_session = "SESSION01",
        .heartbeat_interval = 1ms,
        .intent_capacity = 8,
        .pending_capacity = 8,
        .inbound_capacity = 8,
        .transport_capacity = 8,
    });

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    sender.noteOutboundSent(login);

    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());

    std::this_thread::sleep_for(2ms);
    ASSERT_TRUE(sender.queueHeartbeatIfDue());

    TxOutboundRecord heartbeat {};
    ASSERT_TRUE(sender.popReadyOutbound(heartbeat));
    ASSERT_GE(heartbeat.payload_length, 3U);
    ASSERT_EQ(heartbeat.payload[2], static_cast<uint8_t>('R'));

    // If a heartbeat is popped but not successfully sent (and not restored), the next heartbeat is still due.
    ASSERT_TRUE(sender.queueHeartbeatIfDue());

    // Restoring the popped heartbeat suppresses enqueueing duplicates until it is sent.
    sender.restoreReadyOutbound(heartbeat);
    EXPECT_FALSE(sender.queueHeartbeatIfDue());

    // A successful send updates the heartbeat timer.
    sender.popReadyOutbound(heartbeat);
    sender.noteOutboundSent(heartbeat);
    EXPECT_FALSE(sender.queueHeartbeatIfDue());
}

TEST(TxTranslatorTest, senderAcceptsInboundFramesThroughTraceBufferBackedQueue) {
    TxSender sender(4);
    TxInboundFrame frame {};
    frame.payload[0] = 0x00;
    frame.payload[1] = 0x1f;
    frame.payload[2] = static_cast<uint8_t>('A');

    ASSERT_TRUE(sender.acceptInboundFrame(frame));
}

TEST(TxTranslatorTest, senderAcceptsTransportControlsThroughTraceBufferBackedQueue) {
    TxSender sender(4);
    TxTransportControl control {
        .kind = TxTransportControlKind::Connected,
        .generation = 0,
        .tx_fd = -1,
    };

    ASSERT_TRUE(sender.acceptTransportControl(control));
}

TEST(TxTranslatorTest, transportConnectQueuesLoginRequestFrame) {
    TxSender sender(4);
    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord record {};
    ASSERT_TRUE(sender.popReadyOutbound(record));
    EXPECT_EQ(record.user_ref_num, 0U);
    ASSERT_GE(record.payload_length, 3U);
    EXPECT_EQ(record.payload[2], static_cast<uint8_t>('L'));
}

TEST(TxTranslatorTest, fullInboundBacklogStillAllowsTransportBudgetEnqueue) {
    TxSender sender(TxSenderConfig {
        .intent_capacity = 8,
        .pending_capacity = 8,
        .inbound_capacity = 4,
        .transport_capacity = 4,
    });

    TxInboundFrame frame {};
    frame.payload[0] = 0x00;
    frame.payload[1] = 0x01;
    frame.payload[2] = static_cast<uint8_t>('H');
    frame.payload_length = 3;

    for (std::size_t idx = 0; idx < 4; ++idx) {
        ASSERT_TRUE(sender.acceptInboundFrame(frame));
    }

    for (std::size_t idx = 0; idx < 4; ++idx) {
        ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    }

    EXPECT_FALSE(sender.acceptTransportEvent(TxTransportEvent::Disconnected));
}

TEST(TxTranslatorTest, zeroMergedIngressCapacityIsRejected) {
    EXPECT_THROW((void)TxSender(TxSenderConfig {
                     .intent_capacity = 8,
                     .pending_capacity = 8,
                     .inbound_capacity = 0,
                     .transport_capacity = 0,
                 }),
                 std::invalid_argument);
}

TEST(TxTranslatorTest, inboundFrameAndDisconnectAreAppliedInArrivalOrder) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));
    ASSERT_TRUE(sender.buildOutboundFrame());

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    EXPECT_EQ(login.user_ref_num, 0U);
    ASSERT_GE(login.payload_length, 3U);
    EXPECT_EQ(login.payload[2], static_cast<uint8_t>('L'));

    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Disconnected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord ready {};
    EXPECT_FALSE(sender.popReadyOutbound(ready));
    EXPECT_FALSE(sender.queueHeartbeatIfDue());
}

TEST(TxTranslatorTest, loginAcceptReleasesBufferedOrdersWithIncreasingTags) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));
    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x0ee8,
        .intent = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    EXPECT_EQ(login.payload[2], static_cast<uint8_t>('L'));

    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.buildOutboundFrame());

    TxOutboundRecord first {};
    TxOutboundRecord second {};
    ASSERT_TRUE(sender.popReadyOutbound(first));
    ASSERT_TRUE(sender.popReadyOutbound(second));

    EXPECT_LT(first.user_ref_num, second.user_ref_num);
    EXPECT_EQ(first.payload[2], static_cast<uint8_t>('U'));
    EXPECT_EQ(first.payload[3], static_cast<uint8_t>('O'));
    EXPECT_EQ(second.payload[2], static_cast<uint8_t>('U'));
    EXPECT_EQ(second.payload[3], static_cast<uint8_t>('O'));
}

TEST(TxTranslatorTest, acceptedIntentMetadataIsPreservedInOutboundRecord) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x123456789abcdef0ULL,
        .intent = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.buildOutboundFrame());

    TxOutboundRecord record {};
    ASSERT_TRUE(sender.popReadyOutbound(record));
    EXPECT_EQ(record.que_idx, 1U);
    EXPECT_EQ(record.event_tag, 0x123456789abcdef0ULL);
}

TEST(TxTranslatorTest, trackedReadyOutboundPushesTxEnqueueRecord) {
    TxSender sender(4);
    LatencyTracker tracker(2, 8);
    sender.attachLatenyTracker(&tracker);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x12345678ULL,
        .intent = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());
    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.buildOutboundFrame());

    TimeRecord record {};
    ASSERT_TRUE(tracker.m_trace_buffer[1]->pop(record));
    EXPECT_EQ(record.event_stage, stage::TX_ENQUEUE);
    EXPECT_EQ(record.que_idx, 1U);
    EXPECT_EQ(record.event_tag, 0x12345678ULL);
    EXPECT_GT(record.time_captured, 0U);
    EXPECT_FALSE(tracker.m_trace_buffer[1]->pop(record));
}

TEST(TxTranslatorTest, enqueueStillSucceedsWhenLatencyTrackerThrows) {
    TxSender sender(4);
    LatencyTracker tracker(1, 8);
    sender.attachLatenyTracker(&tracker);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x0ee8,
        .que_idx = 1,
        .event_tag = 0x12345678ULL,
        .intent = {.action = OrderIntentAction::Sell, .price = 223450, .shares = 200},
    }));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());
    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.buildOutboundFrame());

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    EXPECT_EQ(outbound.que_idx, 1U);
}

TEST(TxTranslatorTest, replayedOutboundPreservesMetadataAfterDisconnect) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .que_idx = 1,
        .event_tag = 0x1122334455667788ULL,
        .intent = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.buildOutboundFrame());

    TxOutboundRecord first_send {};
    ASSERT_TRUE(sender.popReadyOutbound(first_send));
    EXPECT_EQ(first_send.que_idx, 1U);
    EXPECT_EQ(first_send.event_tag, 0x1122334455667788ULL);

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Disconnected));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord replayed {};
    ASSERT_TRUE(sender.popReadyOutbound(replayed));
    EXPECT_EQ(replayed.que_idx, 1U);
    EXPECT_EQ(replayed.event_tag, 0x1122334455667788ULL);
}

TEST(TxTranslatorTest, invalidIntentActionDoesNotProduceOrderFrame) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::None, .price = 123450, .shares = 100},
    }));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    EXPECT_FALSE(sender.buildOutboundFrame());

    TxOutboundRecord record {};
    EXPECT_FALSE(sender.popReadyOutbound(record));
}

TEST(TxTranslatorTest, acceptedOrderStopsBeingReplayCandidateAfterDisconnect) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());
    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.buildOutboundFrame());

    TxOutboundRecord order {};
    ASSERT_TRUE(sender.popReadyOutbound(order));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeSequencedAcceptedFrame(order.user_ref_num,
                                                                                  order.stock_locate,
                                                                                  order.shares,
                                                                                  order.price))));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Disconnected));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    EXPECT_FALSE(sender.popReadyOutbound(order));
}

TEST(TxTranslatorTest, executedOrderStopsBeingReplayCandidateAfterDisconnect) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());
    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.buildOutboundFrame());

    TxOutboundRecord order {};
    ASSERT_TRUE(sender.popReadyOutbound(order));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeSequencedExecutedFrame(order.user_ref_num,
                                                                                   order.shares,
                                                                                   order.price,
                                                                                   77U))));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Disconnected));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    EXPECT_FALSE(sender.popReadyOutbound(order));
}

TEST(TxTranslatorTest, rejectedOrderStopsBeingReplayCandidateAfterDisconnect) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());
    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.buildOutboundFrame());

    TxOutboundRecord order {};
    ASSERT_TRUE(sender.popReadyOutbound(order));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeSequencedRejectedFrame(order.user_ref_num, 0x0015))));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Disconnected));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    EXPECT_FALSE(sender.popReadyOutbound(order));
}

TEST(TxTranslatorTest, pendingCapacityDropsOldestRecordOnDisconnectRestore) {
    TxSender sender(TxSenderConfig {
        .intent_capacity = 4,
        .pending_capacity = 2,
    });

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 100000, .shares = 10},
    }));
    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x0ee8,
        .intent = {.action = OrderIntentAction::Sell, .price = 100100, .shares = 11},
    }));
    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 100200, .shares = 12},
    }));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.buildOutboundFrame());

    TxOutboundRecord first {};
    TxOutboundRecord second {};
    ASSERT_TRUE(sender.popReadyOutbound(first));
    ASSERT_TRUE(sender.popReadyOutbound(second));
    EXPECT_FALSE(sender.popReadyOutbound(second));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Disconnected));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord resend_first {};
    TxOutboundRecord resend_second {};
    ASSERT_TRUE(sender.popReadyOutbound(resend_first));
    ASSERT_TRUE(sender.popReadyOutbound(resend_second));
    EXPECT_EQ(resend_first.user_ref_num, first.user_ref_num);
    EXPECT_EQ(resend_second.user_ref_num, 3U);
    EXPECT_FALSE(sender.popReadyOutbound(resend_second));
}

TEST(TxTranslatorTest, pendingCapacityEvictionRemovesOldestRecordFromReadyQueue) {
    TxSender sender(TxSenderConfig {
        .intent_capacity = 4,
        .pending_capacity = 2,
        .inbound_capacity = 4,
        .transport_capacity = 4,
    });

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 100000, .shares = 10},
    }));
    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x0ee8,
        .intent = {.action = OrderIntentAction::Sell, .price = 100100, .shares = 11},
    }));
    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 100200, .shares = 12},
    }));
    ASSERT_TRUE(sender.buildOutboundFrame());

    TxOutboundRecord resend_first {};
    TxOutboundRecord resend_second {};
    ASSERT_TRUE(sender.popReadyOutbound(resend_first));
    ASSERT_TRUE(sender.popReadyOutbound(resend_second));
    EXPECT_EQ(resend_first.user_ref_num, 2U);
    EXPECT_EQ(resend_second.user_ref_num, 3U);
    EXPECT_FALSE(sender.popReadyOutbound(resend_second));
}

TEST(TxTranslatorTest, pendingCapacityEvictionBeforeReadyHeadDoesNotSkipUnsentRecord) {
    TxSender sender(TxSenderConfig {
        .intent_capacity = 4,
        .pending_capacity = 2,
        .inbound_capacity = 4,
        .transport_capacity = 4,
    });

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 100000, .shares = 10},
    }));
    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x0ee8,
        .intent = {.action = OrderIntentAction::Sell, .price = 100100, .shares = 11},
    }));
    ASSERT_TRUE(sender.buildOutboundFrame());

    TxOutboundRecord first {};
    ASSERT_TRUE(sender.popReadyOutbound(first));
    EXPECT_EQ(first.user_ref_num, 1U);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 100200, .shares = 12},
    }));
    ASSERT_TRUE(sender.buildOutboundFrame());

    TxOutboundRecord second {};
    TxOutboundRecord third {};
    ASSERT_TRUE(sender.popReadyOutbound(second));
    ASSERT_TRUE(sender.popReadyOutbound(third));
    EXPECT_EQ(second.user_ref_num, 2U);
    EXPECT_EQ(third.user_ref_num, 3U);
    EXPECT_FALSE(sender.popReadyOutbound(third));
}

TEST(TxTranslatorTest, pendingCapacityEvictionRemovesOldestRecordFromBlockedQueue) {
    TxSender sender(TxSenderConfig {
        .intent_capacity = 4,
        .pending_capacity = 2,
        .inbound_capacity = 4,
        .transport_capacity = 4,
    });

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 100000, .shares = 10},
    }));
    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x0ee8,
        .intent = {.action = OrderIntentAction::Sell, .price = 100100, .shares = 11},
    }));
    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 100200, .shares = 12},
    }));
    ASSERT_TRUE(sender.buildOutboundFrame());

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord resend_first {};
    TxOutboundRecord resend_second {};
    ASSERT_TRUE(sender.popReadyOutbound(resend_first));
    ASSERT_TRUE(sender.popReadyOutbound(resend_second));
    EXPECT_EQ(resend_first.user_ref_num, 2U);
    EXPECT_EQ(resend_second.user_ref_num, 3U);
    EXPECT_FALSE(sender.popReadyOutbound(resend_second));
}

TEST(TxTranslatorTest, readyOutboundDoesNotDrainAcceptedIntentsUntilExplicitStepRuns) {
    TxSender sender(4);

    ASSERT_TRUE(sender.acceptIntent(OrderIntent {
        .stock_locate = 0x000d,
        .intent = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));

    ASSERT_TRUE(sender.acceptTransportEvent(TxTransportEvent::Connected));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    ASSERT_TRUE(sender.acceptInboundFrame(toInboundFrame(makeLoginAcceptedFrame())));
    ASSERT_TRUE(sender.processInboundQueues());

    TxOutboundRecord record {};
    EXPECT_FALSE(sender.popReadyOutbound(record));
    EXPECT_TRUE(sender.buildOutboundFrame());
    ASSERT_TRUE(sender.popReadyOutbound(record));
}
