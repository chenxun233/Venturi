#include <gtest/gtest.h>

#include <array>
#include <cerrno>
#include <chrono>
#include <stdexcept>
#include <sys/socket.h>
#include <type_traits>
#include <unordered_map>
#include <unistd.h>

#define private public
#include "../tx_engine/tx_connection.h"
#include "../tx_engine/tx_sender.h"
#include "../latency/latency_tracker.h"
#undef private

#include "../common/shared_types.h"

namespace {

enum class WrappedSendMode : uint8_t {
    PassThrough,
    PartialThenEagain,
};

WrappedSendMode g_wrapped_send_mode = WrappedSendMode::PassThrough;
int g_partial_send_call_count = 0;

} // namespace

extern "C" ssize_t __real_send(int socket_fd, const void* buffer, size_t length, int flags);

extern "C" ssize_t __wrap_send(int socket_fd, const void* buffer, size_t length, int flags) {
    if (g_wrapped_send_mode != WrappedSendMode::PartialThenEagain) {
        return __real_send(socket_fd, buffer, length, flags);
    }

    ++g_partial_send_call_count;
    if (g_partial_send_call_count == 1) {
        return length > 0 ? 1 : 0;
    }

    errno = EAGAIN;
    return -1;
}

static_assert(std::is_member_function_pointer_v<decltype(&TxSender::acceptExecution)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::updateConnectionInfo)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::runOnce)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::trySendOutbound)>);
static_assert(std::is_member_function_pointer_v<decltype(&TxSender::buildOutboundFrames)>);

TEST(TxTranslatorTest, senderAcceptsConnectionInfoThroughSenderOwnedQueue) {
    TxSender sender(4);
    TxConnectionInfo info {
        .kind = TxConnectionKind::Connected,
        .generation = 1,
        .fd = -1,
    };

    sender.updateConnectionInfo(info);
    (void)sender.runOnce();

    TxOutboundRecord login {};
    ASSERT_TRUE(sender.popReadyOutbound(login));
    EXPECT_EQ(login.user_ref_num, 0U);
    ASSERT_GE(login.payload_length, 3U);
    EXPECT_EQ(login.payload[2], static_cast<uint8_t>('L'));
}

TEST(TxTranslatorTest, senderInstallsSendFdAndSendsLoginImmediatelyAfterConnectedInfo) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(8);
    sender.updateConnectionInfo(TxConnectionInfo {
        .kind = TxConnectionKind::Connected,
        .generation = 42,
        .fd = sockets[0],
    });
    ASSERT_TRUE(sender.runOnce());

    std::array<uint8_t, 39> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));
    EXPECT_EQ(received[2], static_cast<uint8_t>('L'));

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, senderTrySendOutboundUsesSenderOwnedFdDirectly) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(8);
    sender.updateConnectionInfo(TxConnectionInfo {
        .kind = TxConnectionKind::Connected,
        .generation = 42,
        .fd = sockets[0],
    });
    ASSERT_TRUE(sender.runOnce());

    std::array<uint8_t, 128> login_bytes {};
    const ssize_t login_size = ::recv(sockets[1], login_bytes.data(), login_bytes.size(), 0);
    ASSERT_GT(login_size, 0);
    ASSERT_GE(login_size, 3);
    EXPECT_EQ(login_bytes[2], static_cast<uint8_t>('L'));

    TxOutboundRecord outbound {};
    outbound.payload[0] = static_cast<uint8_t>('O');
    outbound.payload[1] = static_cast<uint8_t>('K');
    outbound.payload_length = 2;
    ASSERT_TRUE(sender.trySendOutbound(outbound));

    std::array<uint8_t, 2> received {};
    ASSERT_EQ(::recv(sockets[1], received.data(), received.size(), 0),
              static_cast<ssize_t>(received.size()));
    EXPECT_EQ(received[0], static_cast<uint8_t>('O'));
    EXPECT_EQ(received[1], static_cast<uint8_t>('K'));

    ::close(sockets[1]);
}

TEST(TxTranslatorTest, senderDoesNotReleaseOrderFramesBeforeLoginAcceptedFeedbackExists) {
    TxSender sender(8);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x000d,
        .que_idx = 1,
        .event_tag = 0x1234ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));
    ASSERT_TRUE(sender.buildOutboundFrames());

    sender.updateConnectionInfo(TxConnectionInfo {
        .kind = TxConnectionKind::Connected,
        .generation = 7,
        .fd = -1,
    });
    (void)sender.runOnce();

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    EXPECT_EQ(outbound.user_ref_num, 0U);
    EXPECT_FALSE(sender.popReadyOutbound(outbound));
}

TEST(TxTranslatorTest, acceptExecutionMakesExecutionAvailableWithoutThreadBoundaryQueue) {
    TxSender sender(8);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 99ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));

    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    EXPECT_EQ(outbound.que_idx, 0U);
    EXPECT_EQ(outbound.event_tag, 99ULL);
}

TEST(TxTranslatorTest, acceptExecutionReturnsFalseWhenFixedExecutionBufferIsFull) {
    TxSender sender(8);

    OrderExecution execution {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 1ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 100, .shares = 10},
    };

    for (std::size_t idx = 0; idx < 1024; ++idx) {
        execution.event_tag = idx + 1;
        ASSERT_TRUE(sender.acceptExecution(execution));
    }

    execution.event_tag = 2048ULL;
    EXPECT_FALSE(sender.acceptExecution(execution));
}

TEST(TxTranslatorTest, fixedExecutionBufferPreservesFifoOrderIntoOutboundFrames) {
    TxSender sender(8);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 10ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 100, .shares = 10},
    }));
    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 20ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 200, .shares = 20},
    }));

    ASSERT_TRUE(sender.buildOutboundFrames());

    TxOutboundRecord outbound {};
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    EXPECT_EQ(outbound.event_tag, 10ULL);
    ASSERT_TRUE(sender.popReadyOutbound(outbound));
    EXPECT_EQ(outbound.event_tag, 20ULL);
}

TEST(TxTranslatorTest, senderRejectsNonPowerOfTwoPendingSlotCount) {
    EXPECT_THROW((void)TxSender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 12,
    }), std::invalid_argument);
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

TEST(TxTranslatorTest, senderEmitsExecutionDequeueFrameBuiltAndPendingRecordedStages) {
    TxSender sender(TxSenderConfig {
        .pending_capacity = 8,
        .pending_slot_count = 64,
    });
    LatencyTracker tracker(1, 32);
    sender.attachLatenyTracker(&tracker);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 99ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));

    ASSERT_TRUE(sender.buildOutboundFrames());
    EXPECT_GT(tracker.run(), 0U);
}

TEST(TxTranslatorTest, acceptExecutionDoesNotEmitExecutionAcceptedStage) {
    TxSender sender(8);
    LatencyTracker tracker(1, 32);
    sender.attachLatenyTracker(&tracker);

    ASSERT_TRUE(sender.acceptExecution(OrderExecution {
        .stock_locate = 0x000d,
        .que_idx = 0,
        .event_tag = 88ULL,
        .order = {.action = OrderIntentAction::Buy, .price = 123450, .shares = 100},
    }));

    EXPECT_EQ(tracker.run(), 0U);
}

TEST(TxTranslatorTest, partialWouldBlockSendQueuesDisconnectNoticeAndClearsSenderFd) {
    int sockets[2] {-1, -1};
    ASSERT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

    TxSender sender(8);
    TxConnection connection;
    sender.attachConnection(&connection);
    sender.updateConnectionInfo(TxConnectionInfo {
        .kind = TxConnectionKind::Connected,
        .generation = 11,
        .fd = sockets[0],
    });
    ASSERT_TRUE(sender.runOnce());

    std::array<uint8_t, 128> login_bytes {};
    ASSERT_GT(::recv(sockets[1], login_bytes.data(), login_bytes.size(), 0), 0);

    TxOutboundRecord outbound {};
    outbound.payload_length = 2;
    outbound.payload[0] = static_cast<uint8_t>('O');
    outbound.payload[1] = static_cast<uint8_t>('K');
    sender.m_ready_outbound.push_back(outbound);

    g_wrapped_send_mode = WrappedSendMode::PartialThenEagain;
    g_partial_send_call_count = 0;
    errno = 0;
    ASSERT_TRUE(sender.runOnce());
    g_wrapped_send_mode = WrappedSendMode::PassThrough;

    EXPECT_EQ(sender.m_send_fd, -1);
    EXPECT_TRUE(sender.m_ready_outbound.empty());
    TxDisconnectNotice notice {};
    ASSERT_TRUE(connection.m_sender_disconnect_notices.pop(notice));
    EXPECT_EQ(notice.generation, 11U);
    EXPECT_FALSE(connection.m_sender_disconnect_notices.pop(notice));

    ::close(sockets[1]);
}
