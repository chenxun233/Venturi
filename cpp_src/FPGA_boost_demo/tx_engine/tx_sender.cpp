#include "tx_sender.h"

#include "../common/time_utils.h"
#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace {

constexpr uint8_t kSoupLoginAcceptedType = static_cast<uint8_t>('A');
constexpr uint8_t kSoupLoginRejectedType = static_cast<uint8_t>('J');
constexpr uint8_t kSoupSequencedDataType = static_cast<uint8_t>('S');
constexpr uint8_t kSoupHeartbeatType = static_cast<uint8_t>('H');
constexpr uint8_t kSoupEndOfSessionType = static_cast<uint8_t>('Z');
constexpr uint8_t kSoupLoginRequestType = static_cast<uint8_t>('L');
constexpr uint8_t kSoupUnsequencedDataType = static_cast<uint8_t>('U');
constexpr uint8_t kSoupClientHeartbeatType = static_cast<uint8_t>('R');

constexpr uint8_t kOuchEnterOrderType = static_cast<uint8_t>('O');
constexpr uint8_t kOuchAcceptedType = static_cast<uint8_t>('A');
constexpr uint8_t kOuchExecutedType = static_cast<uint8_t>('E');
constexpr uint8_t kOuchRejectedType = static_cast<uint8_t>('J');

constexpr std::size_t kSoupHeaderSize = 3;
constexpr std::size_t kSessionWidth = 10;
constexpr std::size_t kSequenceWidth = 20;
constexpr std::size_t kUsernameWidth = 6;
constexpr std::size_t kPasswordWidth = 10;

constexpr std::size_t kOuchEnterOrderSize = 16;
constexpr std::size_t kOuchAcceptedSize = 64;
constexpr std::size_t kOuchExecutedSize = 36;
constexpr std::size_t kOuchRejectedSize = 31;
constexpr int kLegacyTransportFdSentinel = -2;

uint16_t readBigEndian16(const uint8_t* bytes) {
    return static_cast<uint16_t>((static_cast<uint16_t>(bytes[0]) << 8) |
                                 static_cast<uint16_t>(bytes[1]));
}

uint32_t readBigEndian32(const uint8_t* bytes) {
    return (static_cast<uint32_t>(bytes[0]) << 24) |
           (static_cast<uint32_t>(bytes[1]) << 16) |
           (static_cast<uint32_t>(bytes[2]) << 8) |
           static_cast<uint32_t>(bytes[3]);
}

uint64_t readBigEndian64(const uint8_t* bytes) {
    return (static_cast<uint64_t>(bytes[0]) << 56) |
           (static_cast<uint64_t>(bytes[1]) << 48) |
           (static_cast<uint64_t>(bytes[2]) << 40) |
           (static_cast<uint64_t>(bytes[3]) << 32) |
           (static_cast<uint64_t>(bytes[4]) << 24) |
           (static_cast<uint64_t>(bytes[5]) << 16) |
           (static_cast<uint64_t>(bytes[6]) << 8) |
           static_cast<uint64_t>(bytes[7]);
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

std::string trimSpaces(std::string_view text) {
    std::size_t begin = 0;
    while (begin < text.size() && text[begin] == ' ') {
        ++begin;
    }

    std::size_t end = text.size();
    while (end > begin && text[end - 1] == ' ') {
        --end;
    }

    return std::string(text.substr(begin, end - begin));
}

bool readSequenceField(const uint8_t* data, std::size_t width, uint64_t& value) {
    std::string_view text(reinterpret_cast<const char*>(data), width);
    const std::size_t first_non_space = text.find_first_not_of(' ');
    if (first_non_space == std::string_view::npos) {
        value = 0;
        return true;
    }

    const char* begin = text.data() + static_cast<std::ptrdiff_t>(first_non_space);
    const char* end = text.data() + static_cast<std::ptrdiff_t>(text.size());
    const auto [ptr, ec] = std::from_chars(begin, end, value);
    return ec == std::errc() && ptr == end;
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

bool parseSoupFrame(const TxInboundFrame& frame,
                    uint8_t& packet_type,
                    const uint8_t*& payload,
                    std::size_t& payload_size) {
    const std::size_t size = frame.payload_length;
    if (size < kSoupHeaderSize) {
        return false;
    }

    const uint16_t encoded_length = readBigEndian16(frame.payload.data());
    if (encoded_length == 0 || size != static_cast<std::size_t>(encoded_length + 2U)) {
        return false;
    }

    packet_type = frame.payload[2];
    payload = frame.payload.data() + static_cast<std::ptrdiff_t>(kSoupHeaderSize);
    payload_size = size - kSoupHeaderSize;
    return true;
}

void writeSoupHeader(TxOutboundRecord& record,
                     uint8_t packet_type,
                     std::size_t payload_size) {
    record.payload_length = static_cast<uint8_t>(kSoupHeaderSize + payload_size);
    writeBigEndian16(record.payload.data(), static_cast<uint16_t>(payload_size + 1U));
    record.payload[2] = packet_type;
}

void writeLoginRequestFrame(TxOutboundRecord& record,
                            std::string_view username,
                            std::string_view password,
                            std::string_view requested_session,
                            uint64_t requested_sequence) {
    constexpr std::size_t kPayloadSize = kUsernameWidth + kPasswordWidth + kSessionWidth + kSequenceWidth;
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
    writeSequenceField(payload + static_cast<std::ptrdiff_t>(kUsernameWidth + kPasswordWidth + kSessionWidth),
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

std::size_t roundUpPowerOfTwo(std::size_t value) {
    if (value <= 1) {
        return 1;
    }

    std::size_t rounded = 1;
    while (rounded < value) {
        if (rounded > (std::numeric_limits<std::size_t>::max() >> 1U)) {
            throw std::invalid_argument("TxSender merged ingress capacity overflow");
        }
        rounded <<= 1U;
    }

    return rounded;
}

std::size_t computeMergedIngressCapacity(const TxSenderConfig& config) {
    if (config.inbound_capacity >
        (std::numeric_limits<std::size_t>::max() - config.transport_capacity)) {
        throw std::invalid_argument("TxSender merged ingress capacity overflow");
    }
    const std::size_t required = config.inbound_capacity + config.transport_capacity;
    if (required == 0) {
        throw std::invalid_argument("TxSender merged ingress capacity must be non-zero");
    }
    return roundUpPowerOfTwo(required);
}

} // namespace

TxSender::TxSender(std::size_t pending_capacity)
    : TxSender(TxSenderConfig {
          .intent_capacity = pending_capacity,
          .pending_capacity = pending_capacity,
          .inbound_capacity = pending_capacity,
          .transport_capacity = pending_capacity,
      }) {}

TxSender::TxSender(TxSenderConfig config)
    : m_config(std::move(config)),
      m_pending_orders {
          .capacity = m_config.pending_capacity,
          .order_records = {},
          .ordered_tags = {},
      },
      m_intent_buffer(m_config.intent_capacity),
      m_inbound_records(computeMergedIngressCapacity(m_config)),
      m_ready_outbound {},
      m_blocked_outbound {} {
    m_pending_orders.order_records.reserve(m_config.pending_capacity);
    m_pending_orders.ordered_tags.reserve(m_config.pending_capacity);
    m_ready_outbound.reserve(m_config.pending_capacity + 4U);
    m_blocked_outbound.reserve(m_config.pending_capacity);
    m_last_successful_send = std::chrono::steady_clock::now();
}

void TxSender::attachLogPrinter(LogPrinter* log_printer) {
    m_log_printer = log_printer;
    m_send_socket.attachLogPrinter(log_printer);
}

void TxSender::attachLatenyTracker(LatencyTracker* latency_tracker) {
    m_latency_tracker = latency_tracker;
    m_send_socket.attachLatenyTracker(latency_tracker);
}

void TxSender::_assertIngressProducerThread() {
#ifndef NDEBUG
    thread_local uint8_t ingress_thread_token {};
    const std::uintptr_t producer_token =
        reinterpret_cast<std::uintptr_t>(&ingress_thread_token);

    std::uintptr_t expected_token = 0;
    if (m_ingress_producer_thread_token.compare_exchange_strong(expected_token,
                                                                 producer_token,
                                                                 std::memory_order_relaxed,
                                                                 std::memory_order_relaxed)) {
        return;
    }

    assert(expected_token == producer_token &&
           "TxSender ingress queue is SPSC: one producer thread for frame/event APIs");
#endif
}

bool TxSender::acceptIntent(const OrderIntent& intent) {
    return m_intent_buffer.push(intent);
}

bool TxSender::acceptInboundFrame(const TxInboundFrame& frame) {
    _assertIngressProducerThread();
    return m_inbound_records.push(TxSenderInboundRecord {
        .kind = TxSenderInboundKind::Frame,
        .frame = frame,
    });
}

bool TxSender::acceptTransportEvent(TxTransportEvent event) {
    const TxTransportControl control {
        .kind = event == TxTransportEvent::Connected
            ? TxTransportControlKind::Connected
            : TxTransportControlKind::Disconnected,
        .generation = 0,
        .tx_fd = kLegacyTransportFdSentinel,
    };
    return acceptTransportControl(control);
}

bool TxSender::acceptTransportControl(const TxTransportControl& control) {
    _assertIngressProducerThread();
    return m_inbound_records.push(TxSenderInboundRecord {
        .kind = TxSenderInboundKind::TransportEvent,
        .transport_event = control,
    });
}

bool TxSender::processInboundQueues() {
    bool did_work = false;
    const auto handle_transport_control = [this](const TxTransportControl& control) {
        switch (control.kind) {
            case TxTransportControlKind::Connected:
                m_send_socket.install(control);
                login();
                break;
            case TxTransportControlKind::Disconnected: {
                const uint64_t active_generation = m_send_socket.activeGeneration();
                if (control.generation < active_generation) {
                    break;
                }
                if (control.generation != active_generation) {
                    break;
                }
                m_send_socket.retireGeneration(control.generation);
                onTransportDisconnected();
                break;
            }
        }
    };

    TxSenderInboundRecord inbound {};
    while (m_inbound_records.pop(inbound)) {
        did_work = true;
        switch (inbound.kind) {
            case TxSenderInboundKind::Frame:
                _acceptInboundFramePayload(inbound.frame);
                break;
            case TxSenderInboundKind::TransportEvent: {
                TxTransportControl control = inbound.transport_event;
                if (control.tx_fd == kLegacyTransportFdSentinel) {
                    control.generation = m_send_socket.activeGeneration();
                    if (control.kind == TxTransportControlKind::Connected &&
                        control.generation < std::numeric_limits<uint64_t>::max()) {
                        ++control.generation;
                    }
                    control.tx_fd = -1;
                }
                handle_transport_control(control);
                break;
            }
        }
    }

    return did_work;
}

bool TxSender::popReadyOutbound(TxOutboundRecord& record) {
    _normalizeReadyRecords();
    if (m_ready_head >= m_ready_outbound.size()) {
        return false;
    }

    record = m_ready_outbound[m_ready_head];
    ++m_ready_head;
    if (record.payload_length >= 3 && record.payload[2] == static_cast<uint8_t>('R')) {
        if (m_heartbeat_ready_count > 0) {
            m_heartbeat_ready_count -= 1;
        }
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
    return m_send_socket.sendPayload(record);
}

void TxSender::noteOutboundSent(const TxOutboundRecord& record) {
    (void)record;
    m_last_successful_send = std::chrono::steady_clock::now();
}

void TxSender::login() {
    _clearReadyRecords();
    TxOutboundRecord login {};
    writeLoginRequestFrame(login,
                           m_config.username,
                           m_config.password,
                           m_active_session.empty() ? m_config.requested_session : m_active_session,
                           m_next_expected_sequence);
    _queueReadyRecord(login);
    m_login_pending = true;
    m_logged_in = false;
}

void TxSender::_acceptInboundFramePayload(const TxInboundFrame& frame) {
    uint8_t packet_type = 0;
    const uint8_t* frame_payload = nullptr;
    std::size_t frame_payload_size = 0;
    if (!parseSoupFrame(frame, packet_type, frame_payload, frame_payload_size)) {
        return;
    }

    switch (packet_type) {
        case kSoupLoginAcceptedType: {
            if (frame_payload_size != kSessionWidth + kSequenceWidth) {
                return;
            }

            uint64_t next_sequence = 0;
            if (!readSequenceField(frame_payload + static_cast<std::ptrdiff_t>(kSessionWidth),
                                   kSequenceWidth,
                                   next_sequence)) {
                return;
            }

            m_active_session = trimSpaces(std::string_view(
                reinterpret_cast<const char*>(frame_payload),
                kSessionWidth));
            m_next_expected_sequence = next_sequence == 0 ? 1 : next_sequence;
            m_login_pending = false;
            m_logged_in = true;
            _flushBlockedRecords();
            return;
        }
        case kSoupLoginRejectedType:
            m_login_pending = false;
            m_logged_in = false;
            return;
        case kSoupHeartbeatType:
            return;
        case kSoupEndOfSessionType:
            m_login_pending = false;
            m_logged_in = false;
            return;
        case kSoupSequencedDataType:
            break;
        default:
            return;
    }

    ++m_next_expected_sequence;
    if (frame_payload_size == kOuchAcceptedSize && frame_payload[0] == kOuchAcceptedType) {
        _handleAccepted(readBigEndian32(frame_payload + 9),
                        readBigEndian32(frame_payload + 14),
                        static_cast<uint32_t>(readBigEndian64(frame_payload + 26)));
        return;
    }
    if (frame_payload_size == kOuchExecutedSize && frame_payload[0] == kOuchExecutedType) {
        _handleExecuted(readBigEndian32(frame_payload + 9),
                        readBigEndian32(frame_payload + 13),
                        static_cast<uint32_t>(readBigEndian64(frame_payload + 17)),
                        readBigEndian64(frame_payload + 26));
        return;
    }
    if (frame_payload_size == kOuchRejectedSize && frame_payload[0] == kOuchRejectedType) {
        _handleRejected(readBigEndian32(frame_payload + 9),
                        readBigEndian16(frame_payload + 13));
    }
}

void TxSender::onTransportDisconnected() {
    _clearReadyRecords();
    m_login_pending = false;
    m_logged_in = false;
    _rebuildBlockedRecords();
}

bool TxSender::queueHeartbeatIfDue() {
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

bool TxSender::buildOutboundFrame() {
    bool did_work = false;
    OrderIntent intent {};
    TxOutboundRecord record {};
    while (m_intent_buffer.pop(intent)) {
        if (!_buildOrderFrame(intent, record)) {
            continue;
        }

        did_work = true;
        _recordPendingOrder(record);
        if (m_logged_in && !m_login_pending) {
            _queueReadyRecord(record);
            continue;
        }

        _queueBlockedRecord(record);
    }

    return did_work;
}

bool TxSender::_buildOrderFrame(const OrderIntent& intent, TxOutboundRecord& record) {
    char side = '\0';
    switch (intent.intent.action) {
        case OrderIntentAction::Buy:
            side = 'B';
            break;
        case OrderIntentAction::Sell:
            side = 'S';
            break;
        default:
            return false;
    }
    record.user_ref_num     = m_next_tag++;
    record.stock_locate     = intent.stock_locate;
    record.que_idx          = intent.que_idx;
    record.event_tag        = intent.event_tag;
    record.shares           = intent.intent.shares;
    record.price            = intent.intent.price;
    writeFramePayload(record, side);
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
            // Latency tracking failure must not affect enqueue success.
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
    for (uint32_t user_ref_num : m_pending_orders.ordered_tags) {
        const auto it = m_pending_orders.order_records.find(user_ref_num);
        if (it != m_pending_orders.order_records.end()) {
            m_blocked_outbound.push_back(it->second);
        }
    }
}

void TxSender::_dropQueuedRecordByTag(uint32_t user_ref_num) {
    if (!m_ready_outbound.empty()) {
        const std::size_t old_head = std::min(m_ready_head, m_ready_outbound.size());
        std::vector<TxOutboundRecord> filtered_ready {};
        filtered_ready.reserve(m_ready_outbound.size());

        std::size_t new_head = 0;
        for (std::size_t idx = 0; idx < m_ready_outbound.size(); ++idx) {
            const TxOutboundRecord& record = m_ready_outbound[idx];
            if (record.user_ref_num == user_ref_num) {
                continue;
            }
            filtered_ready.push_back(record);
            if (idx < old_head) {
                ++new_head;
            }
        }

        m_ready_outbound = std::move(filtered_ready);
        m_ready_head = std::min(new_head, m_ready_outbound.size());
        _normalizeReadyRecords();
    }

    m_blocked_outbound.erase(std::remove_if(m_blocked_outbound.begin(),
                                            m_blocked_outbound.end(),
                                            [user_ref_num](const TxOutboundRecord& record) {
                                                return record.user_ref_num == user_ref_num;
                                            }),
                             m_blocked_outbound.end());
}

void TxSender::_recordPendingOrder(const TxOutboundRecord& record) {
    if (m_pending_orders.capacity == 0) {
        return;
    }

    if (m_pending_orders.ordered_tags.size() == m_pending_orders.capacity &&
        !m_pending_orders.ordered_tags.empty()) {
        const uint32_t dropped_tag = m_pending_orders.ordered_tags.front();
        m_pending_orders.ordered_tags.erase(m_pending_orders.ordered_tags.begin());
        _dropQueuedRecordByTag(dropped_tag);
        m_pending_orders.order_records.erase(dropped_tag);
        _logOrderDropped(dropped_tag);
    }

    m_pending_orders.ordered_tags.push_back(record.user_ref_num);
    m_pending_orders.order_records[record.user_ref_num] = record;
}

void TxSender::_erasePendingOrder(uint32_t user_ref_num) {
    m_pending_orders.order_records.erase(user_ref_num);
    const auto it = std::find(m_pending_orders.ordered_tags.begin(),
                              m_pending_orders.ordered_tags.end(),
                              user_ref_num);
    if (it != m_pending_orders.ordered_tags.end()) {
        m_pending_orders.ordered_tags.erase(it);
    }
}

void TxSender::_handleAccepted(uint32_t user_ref_num,
                               uint32_t shares,
                               uint32_t price) {
    const auto found = m_pending_orders.order_records.find(user_ref_num);
    if (found == m_pending_orders.order_records.end()) {
        return;
    }
    const uint16_t stock_locate = found->second.stock_locate;
    _erasePendingOrder(user_ref_num);
    _logOrderAccepted(user_ref_num, stock_locate, shares, price);
}

void TxSender::_handleExecuted(uint32_t user_ref_num,
                               uint32_t executed_shares,
                               uint32_t price,
                               uint64_t match_number) {
    if (m_pending_orders.order_records.find(user_ref_num) == m_pending_orders.order_records.end()) {
        return;
    }
    _erasePendingOrder(user_ref_num);
    _logOrderFilled(user_ref_num, executed_shares, price, match_number);
}

void TxSender::_handleRejected(uint32_t user_ref_num, uint16_t reason) {
    if (m_pending_orders.order_records.find(user_ref_num) == m_pending_orders.order_records.end()) {
        return;
    }
    _erasePendingOrder(user_ref_num);
    _logOrderRejected(user_ref_num, reason);
}

void TxSender::_logOrderAccepted(uint32_t user_ref_num,
                                 uint16_t stock_locate,
                                 uint32_t shares,
                                 uint32_t price) {
    _pushTxEvent(TxLogRecord {
        .event = TxEventKind::OrderAccepted,
        .user_ref_num = user_ref_num,
        .stock_locate = stock_locate,
        .price = price,
        .shares = shares,
    });
}

void TxSender::_logOrderRejected(uint32_t user_ref_num, uint16_t reason) {
    _pushTxEvent(TxLogRecord {
        .event = TxEventKind::OrderRejected,
        .user_ref_num = user_ref_num,
        .reason = reason,
    });
}

void TxSender::_logOrderFilled(uint32_t user_ref_num,
                               uint32_t shares,
                               uint32_t price,
                               uint64_t match_number) {
    _pushTxEvent(TxLogRecord {
        .event = TxEventKind::OrderFilled,
        .user_ref_num = user_ref_num,
        .price = price,
        .shares = shares,
        .match_number = match_number,
    });
}

void TxSender::_logOrderDropped(uint32_t user_ref_num) {
    _pushTxEvent(TxLogRecord {
        .event = TxEventKind::OrderDropped,
        .user_ref_num = user_ref_num,
    });
}

void TxSender::_pushTxEvent(const TxLogRecord& record) {
    if (m_log_printer != nullptr) {
        (void)m_log_printer->pushTxEvent(record);
    }
}

void TxSender::_clearReadyRecords() {
    m_ready_outbound.clear();
    m_ready_head = 0;
    m_heartbeat_ready_count = 0;
}

void TxSender::_normalizeReadyRecords() {
    if (m_ready_head >= m_ready_outbound.size()) {
        _clearReadyRecords();
    }
}
