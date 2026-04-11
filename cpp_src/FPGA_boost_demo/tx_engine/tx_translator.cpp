#include "tx_translator.h"

#include "../common/time_utils.h"
#include "../latency/latency_tracker.h"
#include "../latency/log_printer.h"

#include <algorithm>
#include <array>
#include <charconv>
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

bool parseSoupFrame(const std::vector<uint8_t>& bytes,
                    uint8_t& packet_type,
                    const uint8_t*& payload,
                    std::size_t& payload_size) {
    if (bytes.size() < kSoupHeaderSize) {
        return false;
    }

    const uint16_t encoded_length = readBigEndian16(bytes.data());
    if (encoded_length == 0 || bytes.size() != static_cast<std::size_t>(encoded_length + 2U)) {
        return false;
    }

    packet_type = bytes[2];
    payload = bytes.data() + static_cast<std::ptrdiff_t>(kSoupHeaderSize);
    payload_size = bytes.size() - kSoupHeaderSize;
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

void writeOrderFrame(TxOutboundRecord& record,
                     uint32_t user_ref_num,
                     uint16_t stock_locate,
                     uint32_t shares,
                     uint32_t price,
                     char side) {
    writeSoupHeader(record, kSoupUnsequencedDataType, kOuchEnterOrderSize);

    uint8_t* payload = record.payload.data() + static_cast<std::ptrdiff_t>(kSoupHeaderSize);
    payload[0] = kOuchEnterOrderType;
    writeBigEndian32(payload + 1, user_ref_num);
    payload[5] = static_cast<uint8_t>(side);
    writeBigEndian16(payload + 6, stock_locate);
    writeBigEndian32(payload + 8, shares);
    writeBigEndian32(payload + 12, price);
}

} // namespace

TxTranslator::TxTranslator(std::size_t pending_capacity)
    : TxTranslator(TxTranslatorConfig {
          .intent_capacity = pending_capacity,
          .pending_capacity = pending_capacity,
      }) {}

TxTranslator::TxTranslator(TxTranslatorConfig config)
    : m_config(std::move(config)),
      m_pending_orders {
          .capacity = m_config.pending_capacity,
          .records_by_tag = {},
          .ordered_tags = {},
      },
      m_intent_buffer(std::make_unique<TraceBuffer<OrderIntent>>(m_config.intent_capacity)),
      m_ready_outbound {},
      m_blocked_outbound {} {
    m_pending_orders.records_by_tag.reserve(m_config.pending_capacity);
    m_pending_orders.ordered_tags.reserve(m_config.pending_capacity);
    m_ready_outbound.reserve(m_config.pending_capacity + 4U);
    m_blocked_outbound.reserve(m_config.pending_capacity);
    m_last_send = std::chrono::steady_clock::now();
}

void TxTranslator::attachLogPrinter(LogPrinter* log_printer) {
    m_log_printer = log_printer;
}

void TxTranslator::attachLatenyTracker(LatencyTracker* latency_tracker) {
    m_latency_tracker = latency_tracker;
}

bool TxTranslator::acceptIntent(const OrderIntent& intent) {
    if (m_intent_buffer == nullptr) {
        return false;
    }
    return m_intent_buffer->push(intent);
}

bool TxTranslator::popReadyOutbound(TxOutboundRecord& record) {
    _normalizeReadyRecords();
    if (m_ready_head >= m_ready_outbound.size()) {
        return false;
    }

    record = m_ready_outbound[m_ready_head];
    ++m_ready_head;
    m_last_send = std::chrono::steady_clock::now();
    return true;
}

void TxTranslator::restoreReadyOutbound(const TxOutboundRecord& record) {
    if (m_ready_head > 0) {
        --m_ready_head;
        m_ready_outbound[m_ready_head] = record;
        return;
    }

    _normalizeReadyRecords();
    m_ready_outbound.push_back(record);
}

void TxTranslator::onTransportConnected() {
    _clearReadyRecords();
    TxOutboundRecord login {};
    writeLoginRequestFrame(login,
                           m_config.username,
                           m_config.password,
                           m_active_session.empty() ? m_config.requested_session : m_active_session,
                           m_next_expected_sequence);
    _queueReadyRecord(login);
    m_login_pending = true;
    m_session_established = false;
}

void TxTranslator::acceptInboundPayload(const std::vector<uint8_t>& payload) {
    uint8_t packet_type = 0;
    const uint8_t* frame_payload = nullptr;
    std::size_t frame_payload_size = 0;
    if (!parseSoupFrame(payload, packet_type, frame_payload, frame_payload_size)) {
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
            m_session_established = true;
            _flushBlockedRecords();
            return;
        }
        case kSoupLoginRejectedType:
            m_login_pending = false;
            m_session_established = false;
            return;
        case kSoupHeartbeatType:
            return;
        case kSoupEndOfSessionType:
            m_login_pending = false;
            m_session_established = false;
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

void TxTranslator::onTransportDisconnected() {
    _clearReadyRecords();
    m_login_pending = false;
    m_session_established = false;
    _rebuildBlockedRecords();
}

bool TxTranslator::queueHeartbeatIfDue() {
    if (!m_session_established) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    if (now - m_last_send < m_config.heartbeat_interval) {
        return false;
    }

    TxOutboundRecord heartbeat {};
    writeClientHeartbeatFrame(heartbeat);
    _queueReadyRecord(heartbeat);
    m_last_send = now;
    return true;
}

bool TxTranslator::buildReadyOutboundFromAcceptedIntents() {
    if (m_intent_buffer == nullptr) {
        return false;
    }

    bool did_work = false;
    OrderIntent intent {};
    while (m_intent_buffer->pop(intent)) {
        TxOutboundRecord record {};
        if (!_buildOrderFrame(intent, record)) {
            continue;
        }

        did_work = true;
        _recordPendingOrder(record);
        if (m_session_established && !m_login_pending) {
            _queueReadyRecord(record);
            continue;
        }

        _queueBlockedRecord(record);
    }

    return did_work;
}

bool TxTranslator::_buildOrderFrame(const OrderIntent& intent, TxOutboundRecord& record) {
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

    record.user_ref_num = m_next_tag++;
    record.stock_locate = intent.stock_locate;
    record.que_idx = intent.que_idx;
    record.event_ts = intent.event_ts;
    record.shares = intent.intent.shares;
    record.price = intent.intent.price;
    writeOrderFrame(record, record.user_ref_num, record.stock_locate, record.shares, record.price, side);
    return true;
}

void TxTranslator::_queueReadyRecord(const TxOutboundRecord& record) {
    _normalizeReadyRecords();
    m_ready_outbound.push_back(record);
    if (record.event_ts != 0 && m_latency_tracker != nullptr) {
        try {
            m_latency_tracker->pushRecord(TimeRecord {
                .que_idx = record.que_idx,
                .event_ts = record.event_ts,
                .event_stage = stage::TX_ENQUEUE,
                .time_captured = readMonotonicRawNs(),
            });
        } catch (...) {
            // Latency tracking failure must not affect enqueue success.
        }
    }
}

void TxTranslator::_queueBlockedRecord(const TxOutboundRecord& record) {
    m_blocked_outbound.push_back(record);
}

void TxTranslator::_flushBlockedRecords() {
    for (const TxOutboundRecord& record : m_blocked_outbound) {
        _queueReadyRecord(record);
    }
    m_blocked_outbound.clear();
}

void TxTranslator::_rebuildBlockedRecords() {
    m_blocked_outbound.clear();
    for (uint32_t user_ref_num : m_pending_orders.ordered_tags) {
        const auto it = m_pending_orders.records_by_tag.find(user_ref_num);
        if (it != m_pending_orders.records_by_tag.end()) {
            m_blocked_outbound.push_back(it->second);
        }
    }
}

void TxTranslator::_recordPendingOrder(const TxOutboundRecord& record) {
    if (m_pending_orders.capacity == 0) {
        return;
    }

    if (m_pending_orders.ordered_tags.size() == m_pending_orders.capacity &&
        !m_pending_orders.ordered_tags.empty()) {
        const uint32_t dropped_tag = m_pending_orders.ordered_tags.front();
        m_pending_orders.ordered_tags.erase(m_pending_orders.ordered_tags.begin());
        m_pending_orders.records_by_tag.erase(dropped_tag);
        _logOrderDropped(dropped_tag);
    }

    m_pending_orders.ordered_tags.push_back(record.user_ref_num);
    m_pending_orders.records_by_tag[record.user_ref_num] = record;
}

void TxTranslator::_erasePendingOrder(uint32_t user_ref_num) {
    m_pending_orders.records_by_tag.erase(user_ref_num);
    const auto it = std::find(m_pending_orders.ordered_tags.begin(),
                              m_pending_orders.ordered_tags.end(),
                              user_ref_num);
    if (it != m_pending_orders.ordered_tags.end()) {
        m_pending_orders.ordered_tags.erase(it);
    }
}

void TxTranslator::_handleAccepted(uint32_t user_ref_num,
                                   uint32_t shares,
                                   uint32_t price) {
    uint16_t stock_locate = 0;
    const auto found = m_pending_orders.records_by_tag.find(user_ref_num);
    if (found != m_pending_orders.records_by_tag.end()) {
        stock_locate = found->second.stock_locate;
    }
    _erasePendingOrder(user_ref_num);
    _logOrderAccepted(user_ref_num, stock_locate, shares, price);
}

void TxTranslator::_handleExecuted(uint32_t user_ref_num,
                                   uint32_t executed_shares,
                                   uint32_t price,
                                   uint64_t match_number) {
    _erasePendingOrder(user_ref_num);
    _logOrderFilled(user_ref_num, executed_shares, price, match_number);
}

void TxTranslator::_handleRejected(uint32_t user_ref_num, uint16_t reason) {
    _erasePendingOrder(user_ref_num);
    _logOrderRejected(user_ref_num, reason);
}

void TxTranslator::_logOrderAccepted(uint32_t user_ref_num,
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

void TxTranslator::_logOrderRejected(uint32_t user_ref_num, uint16_t reason) {
    _pushTxEvent(TxLogRecord {
        .event = TxEventKind::OrderRejected,
        .user_ref_num = user_ref_num,
        .reason = reason,
    });
}

void TxTranslator::_logOrderFilled(uint32_t user_ref_num,
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

void TxTranslator::_logOrderDropped(uint32_t user_ref_num) {
    _pushTxEvent(TxLogRecord {
        .event = TxEventKind::OrderDropped,
        .user_ref_num = user_ref_num,
    });
}

void TxTranslator::_pushTxEvent(const TxLogRecord& record) {
    if (m_log_printer != nullptr) {
        (void)m_log_printer->pushTxEvent(record);
    }
}

void TxTranslator::_clearReadyRecords() {
    m_ready_outbound.clear();
    m_ready_head = 0;
}

void TxTranslator::_normalizeReadyRecords() {
    if (m_ready_head >= m_ready_outbound.size()) {
        _clearReadyRecords();
    }
}
