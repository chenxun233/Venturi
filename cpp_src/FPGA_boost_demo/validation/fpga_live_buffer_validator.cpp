#include "fpga_live_buffer_validator.h"

#include "../../common/log.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

constexpr std::size_t kItchMsgCountOffset = 0x3c;
constexpr std::size_t kItchMsgBaseOffset = 0x3e;
constexpr uint8_t kTypeAdd = 0x41;
constexpr uint8_t kTypeCancel = 0x58;
constexpr uint8_t kTypeDelete = 0x44;
constexpr uint8_t kTypeReplace = 0x55;
constexpr uint8_t kTypeExec = 0x45;
constexpr uint8_t kTypeAddMpid = 0x46;
constexpr uint8_t kTypeExecPrice = 0x43;
constexpr std::size_t kCurrentPayloadEventCount = 8;

uint16_t readBe16(const std::vector<uint8_t>& bytes, std::size_t offset) {
    return static_cast<uint16_t>((static_cast<uint16_t>(bytes[offset]) << 8) |
                                 static_cast<uint16_t>(bytes[offset + 1]));
}

uint32_t readBe32(const std::vector<uint8_t>& bytes, std::size_t offset) {
    return (static_cast<uint32_t>(bytes[offset]) << 24) |
           (static_cast<uint32_t>(bytes[offset + 1]) << 16) |
           (static_cast<uint32_t>(bytes[offset + 2]) << 8) |
           static_cast<uint32_t>(bytes[offset + 3]);
}

uint64_t readBe64(const std::vector<uint8_t>& bytes, std::size_t offset) {
    uint64_t value = 0;
    for (int byte_idx = 0; byte_idx < 8; ++byte_idx) {
        value = (value << 8) | static_cast<uint64_t>(bytes[offset + byte_idx]);
    }
    return value;
}

std::string stripUnderscores(std::string text) {
    text.erase(std::remove(text.begin(), text.end(), '_'), text.end());
    return text;
}

uint64_t parseHexU64(const std::string& text) {
    return std::stoull(stripUnderscores(text), nullptr, 16);
}

std::filesystem::path locateRepoFile(const std::string& relative_path) {
    const std::array<std::filesystem::path, 4> candidates = {
        std::filesystem::current_path() / relative_path,
        std::filesystem::current_path() / ".." / relative_path,
        std::filesystem::current_path() / ".." / ".." / relative_path,
        std::filesystem::path("/home/chenxun/Documents/Project/Venturi") / relative_path
    };

    for (const auto& candidate : candidates) {
        std::error_code ec;
        if (std::filesystem::exists(candidate, ec)) {
            return std::filesystem::canonical(candidate, ec);
        }
    }

    return {};
}

} // namespace

FpgaQueueLiveValidator::FpgaQueueLiveValidator(uint16_t que_idx,
                                               std::string symbol_name,
                                               uint16_t stock_locate,
                                               std::string fixture_path)
    : m_que_idx(que_idx),
      m_symbol_name(std::move(symbol_name)),
      m_stock_locate(stock_locate),
      m_fixture_path(std::move(fixture_path)) {
}

bool FpgaQueueLiveValidator::loadExpectedEvents() {
    std::vector<uint8_t> frame_bytes;
    if (!_loadFixtureFrame(frame_bytes)) {
        return false;
    }

    if (!_parseExpectedEvents(frame_bytes)) {
        return false;
    }

    m_batch_limit = std::min<std::size_t>(kCurrentPayloadEventCount, m_expected_events.size());
    info("Queue %u (%s) loaded %zu expected records, validation batch limit=%zu",
         m_que_idx,
         m_symbol_name.c_str(),
         m_expected_events.size(),
         m_batch_limit);
    return true;
}

bool FpgaQueueLiveValidator::validateBatch(std::span<const FPGAEventDesc> batch) {
    if (batch.empty()) {
        return true;
    }

    _printBatch(batch);

    if (m_next_event_idx >= m_expected_events.size()) {
        warn("Queue %u (%s) received unexpected extra batch after all expected records were consumed",
             m_que_idx,
             m_symbol_name.c_str());
        return false;
    }

    const std::size_t remaining = m_expected_events.size() - m_next_event_idx;
    const std::size_t allowed_count = std::min({batch.size(), remaining, m_batch_limit});
    if (batch.size() > allowed_count) {
        warn("Queue %u (%s) received %zu records, but the current payload-aligned validation limit is %zu",
             m_que_idx,
             m_symbol_name.c_str(),
             batch.size(),
             allowed_count);
        return false;
    }

    for (std::size_t record_idx = 0; record_idx < allowed_count; ++record_idx) {
        if (!_compareRecord(m_next_event_idx, batch[record_idx])) {
            return false;
        }
        ++m_next_event_idx;
    }

    if (isComplete()) {
        success("Queue %u (%s) validated all %zu expected records",
                m_que_idx,
                m_symbol_name.c_str(),
                m_expected_events.size());
    }

    return true;
}

bool FpgaQueueLiveValidator::isComplete() const {
    return !m_expected_events.empty() && m_next_event_idx >= m_expected_events.size();
}

bool FpgaQueueLiveValidator::_loadFixtureFrame(std::vector<uint8_t>& frame_bytes) const {
    const std::filesystem::path file_path = locateRepoFile(m_fixture_path);
    if (file_path.empty()) {
        warn("Could not locate fixture file: %s", m_fixture_path.c_str());
        return false;
    }

    std::ifstream input(file_path);
    if (!input.is_open()) {
        warn("Failed to open fixture file: %s", file_path.string().c_str());
        return false;
    }

    frame_bytes.assign(0x200, 0);
    std::size_t highest_written = 0;
    std::string line;
    while (std::getline(input, line)) {
        if (line.find("set16") != std::string::npos) {
            const std::size_t off_pos = line.find("'h");
            const std::size_t off_end = line.find(',', off_pos);
            const std::size_t value_pos = line.find("128'h", off_end);
            const std::size_t value_end = line.find(')', value_pos);
            const std::size_t offset = static_cast<std::size_t>(parseHexU64(line.substr(off_pos + 2, off_end - off_pos - 2)));
            const std::string value_hex = stripUnderscores(line.substr(value_pos + 5, value_end - value_pos - 5));
            for (std::size_t byte_idx = 0; byte_idx < 16; ++byte_idx) {
                frame_bytes[offset + byte_idx] = static_cast<uint8_t>(parseHexU64(value_hex.substr(byte_idx * 2, 2)));
            }
            highest_written = std::max(highest_written, offset + 16);
        } else if (line.find("set2") != std::string::npos) {
            const std::size_t off_pos = line.find("'h");
            const std::size_t off_end = line.find(',', off_pos);
            const std::size_t value_pos = line.find("16'h", off_end);
            const std::size_t value_end = line.find(')', value_pos);
            const std::size_t offset = static_cast<std::size_t>(parseHexU64(line.substr(off_pos + 2, off_end - off_pos - 2)));
            const std::string value_hex = stripUnderscores(line.substr(value_pos + 4, value_end - value_pos - 4));
            for (std::size_t byte_idx = 0; byte_idx < 2; ++byte_idx) {
                frame_bytes[offset + byte_idx] = static_cast<uint8_t>(parseHexU64(value_hex.substr(byte_idx * 2, 2)));
            }
            highest_written = std::max(highest_written, offset + 2);
        } else if (line.find("frame_bytes") != std::string::npos) {
            const std::size_t off_pos = line.find("'h");
            const std::size_t off_end = line.find(']', off_pos);
            const std::size_t value_pos = line.find("8'h", off_end);
            const std::size_t value_end = line.find(';', value_pos);
            const std::size_t offset = static_cast<std::size_t>(parseHexU64(line.substr(off_pos + 2, off_end - off_pos - 2)));
            frame_bytes[offset] = static_cast<uint8_t>(parseHexU64(line.substr(value_pos + 3, value_end - value_pos - 3)));
            highest_written = std::max(highest_written, offset + 1);
        }
    }

    frame_bytes.resize(highest_written);
    return true;
}

bool FpgaQueueLiveValidator::_parseExpectedEvents(const std::vector<uint8_t>& frame_bytes) {
    if (frame_bytes.size() <= kItchMsgBaseOffset) {
        warn("Frame fixture for queue %u (%s) is too short",
             m_que_idx,
             m_symbol_name.c_str());
        return false;
    }

    SymbolModel model;
    model.symbol_name = m_symbol_name;
    model.stock_locate = m_stock_locate;

    const uint16_t msg_count = readBe16(frame_bytes, kItchMsgCountOffset);
    std::size_t msg_offset = kItchMsgBaseOffset;

    for (uint16_t msg_idx = 0; msg_idx < msg_count; ++msg_idx) {
        if (msg_offset + 2 >= frame_bytes.size()) {
            warn("Unexpected end of frame while parsing queue %u (%s) fixture",
                 m_que_idx,
                 m_symbol_name.c_str());
            return false;
        }

        const uint16_t msg_len = readBe16(frame_bytes, msg_offset);
        if (msg_offset + 2 + msg_len > frame_bytes.size()) {
            warn("Malformed ITCH message length %u in queue %u (%s) fixture",
                 msg_len,
                 m_que_idx,
                 m_symbol_name.c_str());
            return false;
        }

        if (!_parseMessage(frame_bytes, msg_offset, msg_len, model)) {
            return false;
        }

        msg_offset += 2 + msg_len;
    }

    m_expected_events = std::move(model.expected_events);
    return true;
}

bool FpgaQueueLiveValidator::_parseMessage(const std::vector<uint8_t>& frame_bytes,
                                           std::size_t msg_offset,
                                           uint16_t msg_len,
                                           SymbolModel& model) {
    const std::size_t msg_body = msg_offset + 2;
    const uint8_t msg_type = frame_bytes[msg_body];
    if (msg_len == 0) {
        warn("Encountered zero-length ITCH message");
        return false;
    }

    if (msg_body + msg_len > frame_bytes.size()) {
        warn("ITCH message overruns frame");
        return false;
    }

    const uint16_t stock_locate = readBe16(frame_bytes, msg_body + 1);
    if (stock_locate != model.stock_locate) {
        return true;
    }

    switch (msg_type) {
        case kTypeAdd:
        case kTypeAddMpid: {
            const uint64_t order_ref = readBe64(frame_bytes, msg_body + 11);
            const char side = static_cast<char>(frame_bytes[msg_body + 19]);
            const uint32_t shares = readBe32(frame_bytes, msg_body + 20);
            const uint32_t price = readBe32(frame_bytes, msg_body + 32);
            _applyBookUpdate(model, msg_type, order_ref, 0, side, shares, price);
            return true;
        }
        case kTypeCancel:
        case kTypeExec:
        case kTypeExecPrice: {
            const uint64_t order_ref = readBe64(frame_bytes, msg_body + 11);
            const uint32_t shares = readBe32(frame_bytes, msg_body + 19);
            _applyBookUpdate(model, msg_type, order_ref, 0, '\0', shares, 0);
            return true;
        }
        case kTypeDelete: {
            const uint64_t order_ref = readBe64(frame_bytes, msg_body + 11);
            _applyBookUpdate(model, msg_type, order_ref, 0, '\0', 0, 0);
            return true;
        }
        case kTypeReplace: {
            const uint64_t order_ref = readBe64(frame_bytes, msg_body + 11);
            const uint64_t new_order_ref = readBe64(frame_bytes, msg_body + 19);
            const uint32_t shares = readBe32(frame_bytes, msg_body + 27);
            const uint32_t price = readBe32(frame_bytes, msg_body + 31);
            _applyBookUpdate(model, msg_type, order_ref, new_order_ref, '\0', shares, price);
            return true;
        }
        default:
            return true;
    }
}

bool FpgaQueueLiveValidator::_compareRecord(std::size_t event_idx, const FPGAEventDesc& actual) const {
    if (event_idx >= m_expected_events.size()) {
        warn("Queue %u (%s) produced an unexpected event at index %zu",
             m_que_idx,
             m_symbol_name.c_str(),
             event_idx);
        return false;
    }

    const FPGAEventDesc& expected = m_expected_events[event_idx];
    const bool fields_match =
        actual.stock_locate == expected.stock_locate &&
        actual.ask_price == expected.ask_price &&
        actual.ask_shares == expected.ask_shares &&
        actual.bid_price == expected.bid_price &&
        actual.bid_shares == expected.bid_shares;

    if (!fields_match) {
        warn("Queue %u (%s) payload mismatch at record %zu",
             m_que_idx,
             m_symbol_name.c_str(),
             event_idx);
        warn("  actual  : locate=%04x ask=(%u,%u) bid=(%u,%u) frame_start_tk=%llu event_logic_latency_tk=%llu",
             actual.stock_locate,
             actual.ask_price,
             actual.ask_shares,
             actual.bid_price,
             actual.bid_shares,
             static_cast<unsigned long long>(actual.frame_start_tk),
             static_cast<unsigned long long>(actual.event_logic_latency_tk));
        warn("  expected: locate=%04x ask=(%u,%u) bid=(%u,%u)",
             expected.stock_locate,
             expected.ask_price,
             expected.ask_shares,
             expected.bid_price,
             expected.bid_shares);
        return false;
    }

    info("Queue %u (%s) validated record %zu: locate=%04x ask=(%u,%u) bid=(%u,%u)",
         m_que_idx,
         m_symbol_name.c_str(),
         event_idx,
         actual.stock_locate,
         actual.ask_price,
         actual.ask_shares,
         actual.bid_price,
         actual.bid_shares);
    return true;
}

void FpgaQueueLiveValidator::_applyBookUpdate(SymbolModel& model,
                                              uint8_t msg_type,
                                              uint64_t order_ref_num,
                                              uint64_t new_order_ref_num,
                                              char side,
                                              uint32_t shares,
                                              uint32_t price) {
    auto order_it = model.orders.find(order_ref_num);

    switch (msg_type) {
        case kTypeAdd:
        case kTypeAddMpid: {
            OrderState order {side, shares, price};
            model.orders[order_ref_num] = order;
            _accumulateLevel((side == 'B') ? model.bid_book : model.ask_book, price, shares);
            break;
        }
        case kTypeCancel:
        case kTypeExec:
        case kTypeExecPrice: {
            if (order_it == model.orders.end()) {
                break;
            }

            const uint32_t removed = std::min(order_it->second.shares, shares);
            _accumulateLevel((order_it->second.side == 'B') ? model.bid_book : model.ask_book,
                             order_it->second.price,
                             -static_cast<int64_t>(removed));
            order_it->second.shares -= removed;
            if (order_it->second.shares == 0) {
                model.orders.erase(order_it);
            }
            break;
        }
        case kTypeDelete: {
            if (order_it == model.orders.end()) {
                break;
            }

            _accumulateLevel((order_it->second.side == 'B') ? model.bid_book : model.ask_book,
                             order_it->second.price,
                             -static_cast<int64_t>(order_it->second.shares));
            model.orders.erase(order_it);
            break;
        }
        case kTypeReplace: {
            if (order_it == model.orders.end()) {
                break;
            }

            const char old_side = order_it->second.side;
            _accumulateLevel((old_side == 'B') ? model.bid_book : model.ask_book,
                             order_it->second.price,
                             -static_cast<int64_t>(order_it->second.shares));
            model.orders.erase(order_it);

            OrderState order {old_side, shares, price};
            model.orders[new_order_ref_num] = order;
            _accumulateLevel((old_side == 'B') ? model.bid_book : model.ask_book, price, shares);
            break;
        }
        default:
            break;
    }

    _emitEventIfChanged(model);
}

void FpgaQueueLiveValidator::_accumulateLevel(std::map<uint32_t, uint64_t>& side_book,
                                              uint32_t price,
                                              int64_t delta_shares) {
    uint64_t current = 0;
    auto level_it = side_book.find(price);
    if (level_it != side_book.end()) {
        current = level_it->second;
    }

    const int64_t next = static_cast<int64_t>(current) + delta_shares;
    if (next <= 0) {
        if (level_it != side_book.end()) {
            side_book.erase(level_it);
        }
        return;
    }

    side_book[price] = static_cast<uint64_t>(next);
}

void FpgaQueueLiveValidator::_emitEventIfChanged(SymbolModel& model) {
    FPGAEventDesc event {};

    if (!model.ask_book.empty()) {
        event.ask_price = model.ask_book.begin()->first;
        event.ask_shares = static_cast<uint32_t>(model.ask_book.begin()->second);
    }

    if (!model.bid_book.empty()) {
        event.bid_price = model.bid_book.rbegin()->first;
        event.bid_shares = static_cast<uint32_t>(model.bid_book.rbegin()->second);
    }

    event.stock_locate = model.stock_locate;

    if (!model.has_last_event ||
        event.ask_price != model.last_event.ask_price ||
        event.ask_shares != model.last_event.ask_shares ||
        event.bid_price != model.last_event.bid_price ||
        event.bid_shares != model.last_event.bid_shares) {
        model.expected_events.push_back(event);
        model.last_event = event;
        model.has_last_event = true;
    }
}

void FpgaQueueLiveValidator::_printBatch(std::span<const FPGAEventDesc> batch) const {
    info("Queue %u (%s) received batch of %zu records",
         m_que_idx,
         m_symbol_name.c_str(),
         batch.size());
    for (std::size_t record_idx = 0; record_idx < batch.size(); ++record_idx) {
        const FPGAEventDesc& event = batch[record_idx];
        info("  batch[%zu]: locate=%04x ask=(%u,%u) bid=(%u,%u) frame_start_tk=%llu event_logic_latency_tk=%llu",
             record_idx,
             event.stock_locate,
             event.ask_price,
             event.ask_shares,
             event.bid_price,
             event.bid_shares,
             static_cast<unsigned long long>(event.frame_start_tk),
             static_cast<unsigned long long>(event.event_logic_latency_tk));
    }
}
