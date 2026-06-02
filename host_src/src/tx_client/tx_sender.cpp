#include "tx_sender.h"

#include "../common/time_utils.h"
#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"
#include "tx_connector.h"
#include "tx_receiver.h"

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
// comment
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

TxSender::TxSender(std::size_t pending_capacity)
    : TxSender(TxSenderConfig {
          .pending_capacity = roundUpToPowerOfTwo(pending_capacity)
      }) {}

TxSender::TxSender(TxSenderConfig config)
    : m_config(std::move(config)),
      m_pending_orders {
          .capacity = m_config.pending_capacity,
          .live_count = 0,
          .slots = std::vector<PendingSlot>(m_config.pending_capacity),
      }
{
    if (!isPowerOfTwo(m_config.pending_capacity)) {
        throw std::invalid_argument("pending_capacity must be a non-zero power-of-two");
    }
    m_last_successful_send = std::chrono::steady_clock::now();
}

TxSender::~TxSender() {
    _closeSendFd();
}



void TxSender::attachLatencyTracker(LatencyTracker* latency_tracker) {
    p_latency_tracker = latency_tracker;
}

void TxSender::attachConnection(TxConnector* connection) {
    p_connection = connection;
}

void TxSender::attachReceiver(TxReceiver* receiver) {
    p_receiver = receiver;
}

bool TxSender::acceptExecution(const OrderExecution& execution) noexcept {
    const bool pushed = m_execution_buffer.pushBack(execution);
    if (pushed && execution.trace_id != 0U && p_latency_tracker != nullptr) {

        try {
            p_latency_tracker->pushRecord(TimeRecord {
                .que_idx = execution.que_idx,
                .event_tag = execution.event_tag,
                .trace_id = execution.trace_id,
                .event_stage = stage::TX_SENDER_EXECUTION_ACCEPTED,
                .time_captured = readMonotonicRawNs(),
            });
        } catch (...) {
        }
    }
    return pushed;
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
                _onTransportDisconnected();
            }
            return;
    }
#endif
    return;
}

bool TxSender::runOnce() {

    _queueOutFrames();
    _queueHeartbeat();
#ifndef ISO
    if (m_send_fd < 0) {
        return false;
    }
#endif
    while (!m_ready_outbound.isEmpty()) {
        TxOutboundRecord& record = m_ready_outbound.readFront();
        const bool is_heartbeat =
            record.payload_length >= 3 &&
            record.payload[2] == static_cast<uint8_t>('R');
        if (is_heartbeat && record.send_offset == 0U && m_heartbeat_ready_count > 0) {
            m_heartbeat_ready_count -= 1;
        }
        if (!_sendPayload(record)) {
            if (is_heartbeat && record.send_offset == 0U) {
                m_heartbeat_ready_count += 1;
            }
            const bool transport_dropped =
                (m_transport_generation != 0) && (m_send_fd < 0);
#ifdef VENTURI_STABLE_LINK
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR || transport_dropped) {
                break;
            }
            break;
#else
            if ((errno == EAGAIN || errno == EWOULDBLOCK) && !transport_dropped) {
                break;
            }

            _onTransportDisconnected();
            if (p_connection != nullptr && m_transport_generation != 0) {
                (void)p_connection->recvSenderDisconNotice(TxDisconnectNotice {
                    .generation = m_transport_generation,
                });
            }
            break;
#endif
        }
        m_ready_outbound.eraseFront();
        m_last_successful_send = std::chrono::steady_clock::now();
    }
    return true;
}

int TxSender::readSendFd() const noexcept {
    return m_send_fd;
}

bool TxSender::_popReadyOutbound(TxOutboundRecord& record) {
    if (m_ready_outbound.isEmpty()) {
        return false;
    }
    record = m_ready_outbound.readFront();
    if (record.payload_length >= 3 &&
        record.payload[2] == static_cast<uint8_t>('R') &&
        m_heartbeat_ready_count > 0) {
        m_heartbeat_ready_count -= 1;
    }
    return true;
}



void TxSender::login() {
    m_ready_outbound.clear();
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

bool TxSender::_sendPayload(TxOutboundRecord& record) {
    const bool should_track_latency =
        (p_latency_tracker != nullptr && record.trace_id != 0U);
#ifndef ISO
    if (record.payload_length == 0 ||
        record.payload_length > record.payload.size() ||
        m_send_fd < 0) {
        return false;
    }
    std::size_t offset = record.send_offset;
    while (offset < static_cast<std::size_t>(record.payload_length)) {
        if (offset == static_cast<std::size_t>(record.send_offset) &&
            record.send_offset == 0U &&
            should_track_latency) {

            try {
                p_latency_tracker->pushRecord(TimeRecord {
                    .que_idx = record.que_idx,
                    .event_tag = record.event_tag,
                    .trace_id = record.trace_id,
                    .event_stage = stage::TX_SEND_SYSCALL_ENTER,
                    .time_captured = readMonotonicRawNs(),
                });
            } catch (...) {
            }
        }
        const std::size_t bytes_remaining =
            static_cast<std::size_t>(record.payload_length) - offset;
        const ssize_t written = ::send(
            m_send_fd,
            record.payload.data() + static_cast<std::ptrdiff_t>(offset),
            bytes_remaining,
            MSG_NOSIGNAL);
        if (written > 0) {
            offset += static_cast<std::size_t>(written);
            continue;
        }
        if (written < 0 && errno == EINTR) {
            continue;
        }
        if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            record.send_offset = static_cast<uint8_t>(offset);
            return false;
        }
#ifdef VENTURI_STABLE_LINK
        if (offset > 0U) {
            record.send_offset = static_cast<uint8_t>(offset);
        }
        return false;
#else
        _closeSendFd(false);
        return false;
#endif
    }
#endif
    record.send_offset = 0U;
    if (should_track_latency) {

        try {
            p_latency_tracker->pushRecord(TimeRecord {
                .que_idx = record.que_idx,
                .event_tag = record.event_tag,
                .trace_id = record.trace_id,
                .event_stage = stage::TX_SEND,
                .time_captured = readMonotonicRawNs(),
            });
            (void)p_latency_tracker->requestFinalize(record.que_idx, record.trace_id);
        } catch (...) {
        }
    }

    // In split sender/receiver mode, sender-side progress ends at a successful send.
    if (p_receiver != nullptr &&
        record.user_ref_num != 0 &&
        record.payload_length >= 4 &&
        record.payload[2] == kSoupUnsequencedDataType) {
        (void)p_receiver->acceptSentOrder(TxSentOrderRecord {
            .user_ref_num = record.user_ref_num,
            .stock_locate = record.stock_locate,
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .trace_id = record.trace_id,
            .price = record.price,
            .shares = record.shares,
        });
    }
    _erasePendingOrder(record.user_ref_num);

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

void TxSender::_onTransportDisconnected() {
    m_ready_outbound.clear();
    m_login_pending = false;
    m_logged_in = false;
}

bool TxSender::_queueHeartbeat() {
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

bool TxSender::_queueOutFrames() {

    bool did_work = false;
    bool did_reject_pending = false;
    TxOutboundRecord record {};
    while (!m_execution_buffer.isEmpty()) {
        OrderExecution execution = m_execution_buffer.readFront();
        (void)m_execution_buffer.eraseFront();

        if (!_buildOrderFrame(execution, record)) {
            if (execution.trace_id != 0U && p_latency_tracker != nullptr) {
                (void)p_latency_tracker->requestDrop(execution.que_idx, execution.trace_id);
            }
            continue;
        }

        if (!_recordPendingOrder(record)) {
            if (record.trace_id != 0U && p_latency_tracker != nullptr) {
                (void)p_latency_tracker->requestDrop(record.que_idx, record.trace_id);
            }
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
    record.trace_id = execution.trace_id;
    record.shares = execution.order.shares;
    record.price = execution.order.price;
    writeFramePayload(record, side);

    return true;
}

void TxSender::_queueReadyRecord(const TxOutboundRecord& record) {
    m_ready_outbound.pushBack(record);
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
    if (m_pending_orders.live_count >= m_pending_orders.capacity) {
        return false;
    }

    PendingSlot& slot = m_pending_orders.slots[_computePendingSlotIndex(record.user_ref_num)];
    if (slot.occupied) {
        return false;
    }
    slot.occupied = true;
    slot.record = record;
    m_pending_orders.live_count += 1U;
    return true;
}

void TxSender::_erasePendingOrder(uint32_t user_ref_num) {
    PendingSlot* slot = _lookupPendingSlot(user_ref_num);
    if (slot == nullptr) {
        return;
    }
    _clearPendingSlot(*slot);
}
