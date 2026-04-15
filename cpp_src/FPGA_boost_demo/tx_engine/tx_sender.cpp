#include "tx_sender.h"

#include "../common/time_utils.h"
#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"
#include "tx_connection.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace {

constexpr uint8_t kSoupLoginRequestType = static_cast<uint8_t>('L');
constexpr uint8_t kSoupUnsequencedDataType = static_cast<uint8_t>('U');
constexpr uint8_t kSoupClientHeartbeatType = static_cast<uint8_t>('R');
constexpr uint8_t kOuchEnterOrderType = static_cast<uint8_t>('O');

constexpr std::size_t kSoupHeaderSize = 3;
constexpr std::size_t kSessionWidth = 10;
constexpr std::size_t kSequenceWidth = 20;
constexpr std::size_t kUsernameWidth = 6;
constexpr std::size_t kPasswordWidth = 10;
constexpr std::size_t kOuchEnterOrderSize = 16;

bool isPowerOfTwo(std::size_t value) {
    return value != 0 && (value & (value - 1U)) == 0;
}

std::size_t roundUpToPowerOfTwo(std::size_t value) {
    if (value <= 1U) {
        return 1U;
    }

    std::size_t rounded = 1U;
    while (rounded < value && rounded <= (std::numeric_limits<std::size_t>::max() >> 1U)) {
        rounded <<= 1U;
    }
    return rounded;
}

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

void writeSoupHeader(TxOutboundRecord& record,
                     uint8_t packet_type,
                     std::size_t payload_size) {
    record.payload_length = static_cast<uint8_t>(kSoupHeaderSize + payload_size);
    writeBigEndian16(record.payload.data(), static_cast<uint16_t>(payload_size + 1U));
    record.payload[2] = packet_type;
}

void writePaddedField(uint8_t* out,
                      std::size_t width,
                      std::string_view value,
                      bool left_pad) {
    std::fill_n(out, static_cast<std::ptrdiff_t>(width), static_cast<uint8_t>(' '));
    const std::size_t copy_size = std::min(width, value.size());
    if (left_pad) {
        std::copy_n(value.end() - static_cast<std::ptrdiff_t>(copy_size),
                    static_cast<std::ptrdiff_t>(copy_size),
                    out + static_cast<std::ptrdiff_t>(width - copy_size));
        return;
    }

    std::copy_n(value.begin(), static_cast<std::ptrdiff_t>(copy_size), out);
}

void writeSequenceField(uint8_t* out, std::size_t width, uint64_t value) {
    std::fill_n(out, static_cast<std::ptrdiff_t>(width), static_cast<uint8_t>(' '));
    const std::string text = std::to_string(value);
    const std::size_t copy_size = std::min(width, text.size());
    std::copy_n(text.end() - static_cast<std::ptrdiff_t>(copy_size),
                static_cast<std::ptrdiff_t>(copy_size),
                out + static_cast<std::ptrdiff_t>(width - copy_size));
}

void writeLoginRequestFrame(TxOutboundRecord& record,
                            std::string_view username,
                            std::string_view password,
                            std::string_view requested_session,
                            uint64_t requested_sequence) {
    constexpr std::size_t kPayloadSize =
        kUsernameWidth + kPasswordWidth + kSessionWidth + kSequenceWidth;
    writeSoupHeader(record, kSoupLoginRequestType, kPayloadSize);

    uint8_t* payload = record.payload.data() + static_cast<std::ptrdiff_t>(kSoupHeaderSize);
    writePaddedField(payload, kUsernameWidth, username, false);
    writePaddedField(payload + static_cast<std::ptrdiff_t>(kUsernameWidth),
                     kPasswordWidth,
                     password,
                     false);
    writePaddedField(payload + static_cast<std::ptrdiff_t>(kUsernameWidth + kPasswordWidth),
                     kSessionWidth,
                     requested_session,
                     false);
    writeSequenceField(payload + static_cast<std::ptrdiff_t>(
                           kUsernameWidth + kPasswordWidth + kSessionWidth),
                       kSequenceWidth,
                       requested_sequence);
}

void writeClientHeartbeatFrame(TxOutboundRecord& record) {
    writeSoupHeader(record, kSoupClientHeartbeatType, 0);
}

void writeFramePayload(TxOutboundRecord& record, char side) {
    writeSoupHeader(record, kSoupUnsequencedDataType, kOuchEnterOrderSize);

    uint8_t* payload = record.payload.data() + static_cast<std::ptrdiff_t>(kSoupHeaderSize);
    payload[0] = kOuchEnterOrderType;
    writeBigEndian32(payload + 1, record.user_ref_num);
    payload[5] = static_cast<uint8_t>(side);
    writeBigEndian16(payload + 6, record.stock_locate);
    writeBigEndian32(payload + 8, record.shares);
    writeBigEndian32(payload + 12, record.price);
}

} // namespace

extern "C" ssize_t __attribute__((weak))
__wrap_send(int sockfd, const void* buffer, size_t length, int flags) {
    return ::sendto(sockfd, buffer, length, flags, nullptr, 0);
}

TxSender::TxSender(std::size_t pending_capacity)
    : TxSender(TxSenderConfig {
          .intent_capacity = pending_capacity,
          .pending_capacity = pending_capacity,
          .pending_slot_count = roundUpToPowerOfTwo(pending_capacity),
          .transport_capacity = pending_capacity,
      }) {}

TxSender::TxSender(TxSenderConfig config)
    : m_config(std::move(config)),
      m_pending_orders {
          .capacity = m_config.pending_capacity,
          .live_count = 0,
          .slots = std::vector<PendingSlot>(m_config.pending_slot_count),
      },
      m_ready_outbound {},
      m_blocked_outbound {} {
    if (!isPowerOfTwo(m_config.pending_slot_count)) {
        throw std::invalid_argument("pending_slot_count must be a non-zero power-of-two");
    }

    m_ready_outbound.reserve(m_config.pending_capacity + 4U);
    m_blocked_outbound.reserve(m_config.pending_capacity);
    m_last_successful_send = std::chrono::steady_clock::now();
}

TxSender::~TxSender() {
    _closeSendFd();
}

void TxSender::attachLogPrinter(LogPrinter* log_printer) {
    m_log_printer = log_printer;
}

void TxSender::attachLatenyTracker(LatencyTracker* latency_tracker) {
    m_latency_tracker = latency_tracker;
}

void TxSender::attachConnection(TxConnection* connection) {
    m_connection = connection;
}

bool TxSender::acceptExecution(const OrderExecution& execution) noexcept {
    const bool pushed = m_execution_buffer.pushBack(execution);
    if (!pushed) {
        return false;
    }
    return true;
}

void TxSender::updateConnectionInfo(const TxConnectionInfo& info) {
#ifdef ISO
    (void)info;
#else
    switch (info.kind) {
        case TxConnectionKind::Connected:
            _updateConnectionInfo(info);
            login();
            return;
        case TxConnectionKind::Disconnected:
            if (info.generation == m_transport_generation) {
                _retireGeneration(info.generation);
                onTransportDisconnected();
            }
            return;
    }
#endif
    return;
}

bool TxSender::runOnce() {

    buildOutboundFrames();
    queueHeartbeat();
#ifndef ISO
    if (m_send_fd < 0) {
        return false;
    }
#endif
    TxOutboundRecord record {};
    while (popReadyOutbound(record)) {
        if (!trySendOutbound(record)) {
            restoreReadyOutbound(record);

            const bool transport_dropped =
                (m_transport_generation != 0) && (m_send_fd < 0);
            if ((errno == EAGAIN || errno == EWOULDBLOCK) && !transport_dropped) {
                break;
            }

            onTransportDisconnected();
            if (m_connection != nullptr && m_transport_generation != 0) {
                (void)m_connection->pushSenderDisconNotice(TxDisconnectNotice {
                    .generation = m_transport_generation,
                });
            }
            break;
        }
        noteOutboundSent(record);
    }

    return true;
}

bool TxSender::popReadyOutbound(TxOutboundRecord& record) {
    _normalizeReadyRecords();
    if (m_ready_head >= m_ready_outbound.size()) {
        return false;
    }

    record = m_ready_outbound[m_ready_head];
    ++m_ready_head;
    if (record.payload_length >= 3 &&
        record.payload[2] == static_cast<uint8_t>('R') &&
        m_heartbeat_ready_count > 0) {
        m_heartbeat_ready_count -= 1;
    }
    return true;
}

void TxSender::restoreReadyOutbound(const TxOutboundRecord& record) {
    if (m_ready_head > 0) {
        --m_ready_head;
        m_ready_outbound[m_ready_head] = record;
        if (record.payload_length >= 3 && record.payload[2] == static_cast<uint8_t>('R')) {
            m_heartbeat_ready_count += 1;
        }
        return;
    }

    _normalizeReadyRecords();
    m_ready_outbound.push_back(record);
    if (record.payload_length >= 3 && record.payload[2] == static_cast<uint8_t>('R')) {
        m_heartbeat_ready_count += 1;
    }
}

bool TxSender::trySendOutbound(const TxOutboundRecord& record) {
    return _sendPayload(record);
}

void TxSender::noteOutboundSent(const TxOutboundRecord& record) {
    (void)record;
    m_last_successful_send = std::chrono::steady_clock::now();
}

void TxSender::login() {
    _clearReadyRecords();
    TxOutboundRecord login_record {};
    writeLoginRequestFrame(login_record,
                           m_config.username,
                           m_config.password,
                           m_active_session.empty() ? m_config.requested_session : m_active_session,
                           m_next_expected_sequence);
    _queueReadyRecord(login_record);
    m_login_pending = true;
    m_logged_in = false;
}

void TxSender::_updateConnectionInfo(const TxConnectionInfo& info) {
    if (info.kind != TxConnectionKind::Connected) {
        return;
    }
    if (info.generation == m_transport_generation && info.fd == m_send_fd) {
        return;
    }
    _closeSendFd();
    m_send_fd = info.fd;
    m_transport_generation = info.generation;
}

void TxSender::_retireGeneration(uint64_t generation) {
    if (generation != m_transport_generation) {
        return;
    }
    _closeSendFd();
}

bool TxSender::_sendPayload(const TxOutboundRecord& record) {
#ifndef ISO
    if (record.payload_length == 0 ||
        record.payload_length > record.payload.size() ||
        m_send_fd < 0) {
        return false;
    }
    std::size_t offset = 0;
    while (offset < static_cast<std::size_t>(record.payload_length)) {
        const ssize_t written = ::send(
            m_send_fd,
            record.payload.data() + static_cast<std::ptrdiff_t>(offset),
            static_cast<std::size_t>(record.payload_length) - offset,
            MSG_NOSIGNAL);
        if (written > 0) {
            offset += static_cast<std::size_t>(written);
            continue;
        }
        if (written < 0 && errno == EINTR) {
            continue;
        }
        if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            if (offset == 0) {
                return false;
            }
            _closeSendFd(false);
            return false;
        }
        _closeSendFd(false);
        return false;
    }
#endif

    if (record.event_tag != 0 && m_latency_tracker != nullptr) {
        try {
            m_latency_tracker->pushRecord(TimeRecord {
                .que_idx = record.que_idx,
                .event_tag = record.event_tag,
                .event_stage = stage::TX_SEND,
                .time_captured = readMonotonicRawNs(),
            });
        } catch (...) {
        }
    }

    if (record.user_ref_num != 0) {
        _pushTxEvent(record.que_idx, TxLogRecord {
            .event = TxEventKind::OrderSent,
            .user_ref_num = record.user_ref_num,
            .stock_locate = record.stock_locate,
            .price = record.price,
            .shares = record.shares,
        });
    }

    return true;
}

void TxSender::_closeSendFd(bool clear_generation) {
    if (m_send_fd >= 0) {
        ::close(m_send_fd);
        m_send_fd = -1;
    }
    if (clear_generation) {
        m_transport_generation = 0;
    }
}

void TxSender::onTransportDisconnected() {
    _clearReadyRecords();
    m_login_pending = false;
    m_logged_in = false;
    _rebuildBlockedRecords();
}

bool TxSender::queueHeartbeat() {
    if (!m_logged_in) {
        return false;
    }
    if (m_heartbeat_ready_count > 0) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    if (now - m_last_successful_send < m_config.heartbeat_interval) {
        return false;
    }

    TxOutboundRecord heartbeat {};
    writeClientHeartbeatFrame(heartbeat);
    _queueReadyRecord(heartbeat);
    m_heartbeat_ready_count += 1;
    return true;
}

bool TxSender::buildOutboundFrames() {
    bool did_work = false;
    bool did_reject_pending = false;
    TxOutboundRecord record {};
    while (!m_execution_buffer.isEmpty()) {
        OrderExecution execution = m_execution_buffer.readFront();
        (void)m_execution_buffer.eraseFront();
        if (execution.event_tag != 0 && m_latency_tracker != nullptr) {
            try {
                m_latency_tracker->pushRecord(TimeRecord {
                    .que_idx = execution.que_idx,
                    .event_tag = execution.event_tag,
                    .event_stage = stage::EXECUTION_DEQUEUE,
                    .time_captured = readMonotonicRawNs(),
                });
            } catch (...) {
            }
        }

        if (!_buildOrderFrame(execution, record)) {
            continue;
        }

        if (!_recordPendingOrder(record)) {
            did_reject_pending = true;
            continue;
        }

        did_work = true;
        _queueReadyRecord(record);
    }

    return did_work && !did_reject_pending;
}

bool TxSender::_buildOrderFrame(const OrderExecution& execution, TxOutboundRecord& record) {
    char side = '\0';
    switch (execution.order.action) {
        case OrderIntentAction::Buy:
            side = 'B';
            break;
        case OrderIntentAction::Sell:
            side = 'S';
            break;
        default:
            return false;
    }

    record = TxOutboundRecord {};
    record.user_ref_num = m_next_tag++;
    record.stock_locate = execution.stock_locate;
    record.que_idx = execution.que_idx;
    record.event_tag = execution.event_tag;
    record.shares = execution.order.shares;
    record.price = execution.order.price;
    writeFramePayload(record, side);
    if (execution.event_tag != 0 && m_latency_tracker != nullptr) {
        try {
            m_latency_tracker->pushRecord(TimeRecord {
                .que_idx = execution.que_idx,
                .event_tag = execution.event_tag,
                .event_stage = stage::ORDER_FRAME_BUILT,
                .time_captured = readMonotonicRawNs(),
            });
        } catch (...) {
        }
    }

    return true;
}

void TxSender::_queueReadyRecord(const TxOutboundRecord& record) {
    _normalizeReadyRecords();
    m_ready_outbound.push_back(record);
    if (record.event_tag != 0 && m_latency_tracker != nullptr) {
        try {
            m_latency_tracker->pushRecord(TimeRecord {
                .que_idx = record.que_idx,
                .event_tag = record.event_tag,
                .event_stage = stage::TX_ENQUEUE,
                .time_captured = readMonotonicRawNs(),
            });
        } catch (...) {
        }
    }
}

void TxSender::_queueBlockedRecord(const TxOutboundRecord& record) {
    m_blocked_outbound.push_back(record);
}

void TxSender::_flushBlockedRecords() {
    for (const TxOutboundRecord& record : m_blocked_outbound) {
        _queueReadyRecord(record);
    }
    m_blocked_outbound.clear();
}

void TxSender::_rebuildBlockedRecords() {
    m_blocked_outbound.clear();
    m_blocked_outbound.reserve(m_pending_orders.live_count);
    for (const PendingSlot& slot : m_pending_orders.slots) {
        if (slot.occupied) {
            m_blocked_outbound.push_back(slot.record);
        }
    }

    std::sort(m_blocked_outbound.begin(),
              m_blocked_outbound.end(),
              [](const TxOutboundRecord& lhs, const TxOutboundRecord& rhs) {
                  return lhs.user_ref_num < rhs.user_ref_num;
              });
}

std::size_t TxSender::_computePendingSlotIndex(uint32_t user_ref_num) const noexcept {
    if (m_pending_orders.slots.empty()) {
        return 0;
    }
    return static_cast<std::size_t>(user_ref_num) & (m_pending_orders.slots.size() - 1U);
}

TxSender::PendingSlot* TxSender::_lookupPendingSlot(uint32_t user_ref_num) noexcept {
    if (m_pending_orders.slots.empty()) {
        return nullptr;
    }

    PendingSlot& slot = m_pending_orders.slots[_computePendingSlotIndex(user_ref_num)];
    if (!slot.occupied || slot.record.user_ref_num != user_ref_num) {
        return nullptr;
    }

    return &slot;
}

const TxSender::PendingSlot* TxSender::_lookupPendingSlot(uint32_t user_ref_num) const noexcept {
    if (m_pending_orders.slots.empty()) {
        return nullptr;
    }

    const PendingSlot& slot = m_pending_orders.slots[_computePendingSlotIndex(user_ref_num)];
    if (!slot.occupied || slot.record.user_ref_num != user_ref_num) {
        return nullptr;
    }

    return &slot;
}

void TxSender::_clearPendingSlot(PendingSlot& slot) noexcept {
    if (!slot.occupied) {
        return;
    }

    slot.occupied = false;
    slot.record = TxOutboundRecord {};
    if (m_pending_orders.live_count > 0) {
        --m_pending_orders.live_count;
    }
}

bool TxSender::_recordPendingOrder(const TxOutboundRecord& record) {
    if (m_pending_orders.capacity == 0 || m_pending_orders.slots.empty()) {
        return false;
    }

    const auto push_pending_stage = [this, &record](stage event_stage) {
        if (record.event_tag == 0 || m_latency_tracker == nullptr) {
            return;
        }
        try {
            m_latency_tracker->pushRecord(TimeRecord {
                .que_idx = record.que_idx,
                .event_tag = record.event_tag,
                .event_stage = event_stage,
                .time_captured = readMonotonicRawNs(),
            });
        } catch (...) {
        }
    };

    push_pending_stage(stage::PENDING_CAPACITY_HANDLED);
    if (m_pending_orders.live_count >= m_pending_orders.capacity) {
        return false;
    }

    PendingSlot& slot = m_pending_orders.slots[_computePendingSlotIndex(record.user_ref_num)];
    if (slot.occupied) {
        return false;
    }

    push_pending_stage(stage::PENDING_TAG_RECORDED);
    slot.occupied = true;
    slot.record = record;
    m_pending_orders.live_count += 1U;
    push_pending_stage(stage::PENDING_RECORDED);
    return true;
}

void TxSender::_erasePendingOrder(uint32_t user_ref_num) {
    PendingSlot* slot = _lookupPendingSlot(user_ref_num);
    if (slot == nullptr) {
        return;
    }
    _clearPendingSlot(*slot);
}

void TxSender::_handleAccepted(uint32_t user_ref_num, uint32_t shares, uint32_t price) {
    PendingSlot* found = _lookupPendingSlot(user_ref_num);
    if (found == nullptr) {
        return;
    }
    const uint16_t queue_idx = found->record.que_idx;
    const uint16_t stock_locate = found->record.stock_locate;
    _clearPendingSlot(*found);
    _logOrderAccepted(queue_idx, user_ref_num, stock_locate, shares, price);
}

void TxSender::_handleExecuted(uint32_t user_ref_num,
                               uint32_t executed_shares,
                               uint32_t price,
                               uint64_t match_number) {
    PendingSlot* found = _lookupPendingSlot(user_ref_num);
    if (found == nullptr) {
        return;
    }
    const uint16_t queue_idx = found->record.que_idx;
    _clearPendingSlot(*found);
    _logOrderFilled(queue_idx, user_ref_num, executed_shares, price, match_number);
}

void TxSender::_handleRejected(uint32_t user_ref_num, uint16_t reason) {
    PendingSlot* found = _lookupPendingSlot(user_ref_num);
    if (found == nullptr) {
        return;
    }
    const uint16_t queue_idx = found->record.que_idx;
    _clearPendingSlot(*found);
    _logOrderRejected(queue_idx, user_ref_num, reason);
}

void TxSender::_logOrderAccepted(uint16_t queue_idx,
                                 uint32_t user_ref_num,
                                 uint16_t stock_locate,
                                 uint32_t shares,
                                 uint32_t price) {
    _pushTxEvent(queue_idx, TxLogRecord {
        .event = TxEventKind::OrderAccepted,
        .user_ref_num = user_ref_num,
        .stock_locate = stock_locate,
        .price = price,
        .shares = shares,
    });
}

void TxSender::_logOrderRejected(uint16_t queue_idx, uint32_t user_ref_num, uint16_t reason) {
    _pushTxEvent(queue_idx, TxLogRecord {
        .event = TxEventKind::OrderRejected,
        .user_ref_num = user_ref_num,
        .reason = reason,
    });
}

void TxSender::_logOrderFilled(uint16_t queue_idx,
                               uint32_t user_ref_num,
                               uint32_t shares,
                               uint32_t price,
                               uint64_t match_number) {
    _pushTxEvent(queue_idx, TxLogRecord {
        .event = TxEventKind::OrderFilled,
        .user_ref_num = user_ref_num,
        .price = price,
        .shares = shares,
        .match_number = match_number,
    });
}

void TxSender::_logOrderDropped(uint16_t queue_idx,
                                uint32_t user_ref_num,
                                uint16_t stock_locate,
                                uint32_t shares,
                                uint32_t price) {
    _pushTxEvent(queue_idx, TxLogRecord {
        .event = TxEventKind::OrderDropped,
        .user_ref_num = user_ref_num,
        .stock_locate = stock_locate,
        .price = price,
        .shares = shares,
    });
}

void TxSender::_pushTxEvent(uint16_t queue_idx, const TxLogRecord& record) {
    if (m_log_printer == nullptr) {
        return;
    }
    TxLogRecord queued_record = record;
    queued_record.queue_idx = queue_idx;
    (void)m_log_printer->pushTxLog(queued_record);
}

void TxSender::_clearReadyRecords() {
    m_ready_outbound.clear();
    m_ready_head = 0;
    m_heartbeat_ready_count = 0;
}

void TxSender::_normalizeReadyRecords() {
    if (m_ready_head == 0) {
        return;
    }
    if (m_ready_head >= m_ready_outbound.size()) {
        m_ready_outbound.clear();
        m_ready_head = 0;
        return;
    }

    m_ready_outbound.erase(m_ready_outbound.begin(),
                           m_ready_outbound.begin() + static_cast<std::ptrdiff_t>(m_ready_head));
    m_ready_head = 0;
}
