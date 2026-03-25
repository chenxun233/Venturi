#include "runtime_payload_validator.h"

#include "../../common/log.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

constexpr uint16_t kAaplLocate = 0x000d;
constexpr uint16_t kHsbcLocate = 0x0ee8;
constexpr std::size_t kItchMsgCountOffset = 0x3c;
constexpr std::size_t kItchMsgBaseOffset = 0x3e;
constexpr uint8_t kTypeAdd = 0x41;
constexpr uint8_t kTypeCancel = 0x58;
constexpr uint8_t kTypeDelete = 0x44;
constexpr uint8_t kTypeReplace = 0x55;
constexpr uint8_t kTypeExec = 0x45;
constexpr uint8_t kTypeAddMpid = 0x46;
constexpr uint8_t kTypeExecPrice = 0x43;

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

RuntimePayloadValidator::RuntimePayloadValidator()
    : m_queues({
          QueueValidationState{"AAPL", kAaplLocate, {}, 0, false},
          QueueValidationState{"HSBC", kHsbcLocate, {}, 0, false},
      }) {
}

bool RuntimePayloadValidator::loadExpectedPayloads() {
    static const std::array<std::pair<uint16_t, const char*>, 2> fixtures = {{
        {0, "market_data/AAPL_13_B_payload_frames_hex.txt"},
        {1, "market_data/HSBC_3816_S_payload_frames_hex.txt"},
    }};

    std::lock_guard<std::mutex> lock(m_mutex);
    m_failed = false;
    m_failure_reason.clear();

    for (const auto& [que_idx, file_name] : fixtures) {
        std::vector<uint8_t> frame_bytes;
        if (!loadFixtureFrame(file_name, frame_bytes)) {
            setFailureLocked("failed to load expected payload fixture");
            return false;
        }

        QueueValidationState& queue = m_queues[que_idx];
        queue.expected_events.clear();
        queue.next_expected_idx = 0;
        queue.validated = false;

        if (!parseExpectedEvents(frame_bytes, queue)) {
            setFailureLocked("failed to parse expected payload fixture");
            return false;
        }

        info("Loaded %zu expected events for queue %u (%s)",
             queue.expected_events.size(),
             que_idx,
             queue.symbol_name.c_str());
    }

    return true;
}

bool RuntimePayloadValidator::validateBatch(uint16_t que_idx, const FPGAEventDesc* events, std::size_t count) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_failed) {
        return false;
    }

    if (que_idx >= m_queues.size()) {
        setFailureLocked("received validation batch for invalid queue");
        return false;
    }

    QueueValidationState& queue = m_queues[que_idx];
    if (queue.validated || count == 0) {
        return !m_failed;
    }

    for (std::size_t event_idx = 0; event_idx < count; ++event_idx) {
        if (queue.next_expected_idx >= queue.expected_events.size()) {
            queue.validated = true;
            break;
        }

        const FPGAEventDesc& actual = events[event_idx];
        if (queue.next_expected_idx == 0 &&
            actual.ask_price == 0 &&
            actual.ask_shares == 0 &&
            actual.bid_price == 0 &&
            actual.bid_shares == 0) {
            continue;
        }

        const ExpectedEvent& expected = queue.expected_events[queue.next_expected_idx];
        if (!compareEvent(que_idx, actual, expected)) {
            std::ostringstream reason;
            reason << "queue " << que_idx << " (" << queue.symbol_name
                   << ") payload mismatch at expected event " << queue.next_expected_idx;
            setFailureLocked(reason.str());
            return false;
        }

        ++queue.next_expected_idx;
        if (queue.next_expected_idx == queue.expected_events.size()) {
            queue.validated = true;
            success("Queue %u (%s) validated %zu expected events",
                    que_idx,
                    queue.symbol_name.c_str(),
                    queue.expected_events.size());
            break;
        }
    }

    return !m_failed;
}

bool RuntimePayloadValidator::isReadyForMeasurement() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_failed) {
        return false;
    }

    for (const QueueValidationState& queue : m_queues) {
        if (!queue.validated) {
            return false;
        }
    }

    return true;
}

bool RuntimePayloadValidator::hasFailed() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_failed;
}

bool RuntimePayloadValidator::loadFixtureFrame(const std::string& file_name,
                                               std::vector<uint8_t>& frame_bytes) const {
    const std::filesystem::path file_path = locateRepoFile(file_name);
    if (file_path.empty()) {
        warn("Could not locate fixture file: %s", file_name.c_str());
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
            const std::size_t offset =
                static_cast<std::size_t>(parseHexU64(line.substr(off_pos + 2, off_end - off_pos - 2)));
            const std::string value_hex =
                stripUnderscores(line.substr(value_pos + 5, value_end - value_pos - 5));
            for (std::size_t byte_idx = 0; byte_idx < 16; ++byte_idx) {
                frame_bytes[offset + byte_idx] =
                    static_cast<uint8_t>(parseHexU64(value_hex.substr(byte_idx * 2, 2)));
            }
            highest_written = std::max(highest_written, offset + 16);
        } else if (line.find("set2") != std::string::npos) {
            const std::size_t off_pos = line.find("'h");
            const std::size_t off_end = line.find(',', off_pos);
            const std::size_t value_pos = line.find("16'h", off_end);
            const std::size_t value_end = line.find(')', value_pos);
            const std::size_t offset =
                static_cast<std::size_t>(parseHexU64(line.substr(off_pos + 2, off_end - off_pos - 2)));
            const std::string value_hex =
                stripUnderscores(line.substr(value_pos + 4, value_end - value_pos - 4));
            for (std::size_t byte_idx = 0; byte_idx < 2; ++byte_idx) {
                frame_bytes[offset + byte_idx] =
                    static_cast<uint8_t>(parseHexU64(value_hex.substr(byte_idx * 2, 2)));
            }
            highest_written = std::max(highest_written, offset + 2);
        } else if (line.find("frame_bytes") != std::string::npos) {
            const std::size_t off_pos = line.find("'h");
            const std::size_t off_end = line.find(']', off_pos);
            const std::size_t value_pos = line.find("8'h", off_end);
            const std::size_t value_end = line.find(';', value_pos);
            const std::size_t offset =
                static_cast<std::size_t>(parseHexU64(line.substr(off_pos + 2, off_end - off_pos - 2)));
            frame_bytes[offset] =
                static_cast<uint8_t>(parseHexU64(line.substr(value_pos + 3, value_end - value_pos - 3)));
            highest_written = std::max(highest_written, offset + 1);
        }
    }

    frame_bytes.resize(highest_written);
    return true;
}

bool RuntimePayloadValidator::parseExpectedEvents(const std::vector<uint8_t>& frame_bytes,
                                                  QueueValidationState& queue) const {
    if (frame_bytes.size() <= kItchMsgBaseOffset) {
        warn("Frame fixture for %s is too short", queue.symbol_name.c_str());
        return false;
    }

    SymbolModel model;
    model.symbol_name = queue.symbol_name;
    model.stock_locate = queue.stock_locate;

    const uint16_t msg_count = readBe16(frame_bytes, kItchMsgCountOffset);
    std::size_t msg_offset = kItchMsgBaseOffset;

    for (uint16_t msg_idx = 0; msg_idx < msg_count; ++msg_idx) {
        if (msg_offset + 2 >= frame_bytes.size()) {
            warn("Unexpected end of frame while parsing %s fixture", queue.symbol_name.c_str());
            return false;
        }

        const uint16_t msg_len = readBe16(frame_bytes, msg_offset);
        if (msg_offset + 2 + msg_len > frame_bytes.size()) {
            warn("Malformed ITCH message length %u in %s fixture", msg_len, queue.symbol_name.c_str());
            return false;
        }

        if (!parseMessage(frame_bytes, msg_offset, msg_len, model)) {
            return false;
        }

        msg_offset += 2 + msg_len;
    }

    queue.expected_events = std::move(model.expected_events);
    return true;
}

bool RuntimePayloadValidator::parseMessage(const std::vector<uint8_t>& frame_bytes,
                                           std::size_t msg_offset,
                                           uint16_t msg_len,
                                           SymbolModel& model) const {
    const std::size_t msg_body = msg_offset + 2;
    const uint8_t msg_type = frame_bytes[msg_body];
    if (msg_len == 0 || msg_body + msg_len > frame_bytes.size()) {
        warn("Malformed ITCH message in %s fixture", model.symbol_name.c_str());
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
            applyBookUpdate(model, msg_type, order_ref, 0, side, shares, price);
            return true;
        }
        case kTypeCancel:
        case kTypeExec:
        case kTypeExecPrice: {
            const uint64_t order_ref = readBe64(frame_bytes, msg_body + 11);
            const uint32_t shares = readBe32(frame_bytes, msg_body + 19);
            applyBookUpdate(model, msg_type, order_ref, 0, '\0', shares, 0);
            return true;
        }
        case kTypeDelete: {
            const uint64_t order_ref = readBe64(frame_bytes, msg_body + 11);
            applyBookUpdate(model, msg_type, order_ref, 0, '\0', 0, 0);
            return true;
        }
        case kTypeReplace: {
            const uint64_t order_ref = readBe64(frame_bytes, msg_body + 11);
            const uint64_t new_order_ref = readBe64(frame_bytes, msg_body + 19);
            const uint32_t shares = readBe32(frame_bytes, msg_body + 27);
            const uint32_t price = readBe32(frame_bytes, msg_body + 31);
            applyBookUpdate(model, msg_type, order_ref, new_order_ref, '\0', shares, price);
            return true;
        }
        default:
            return true;
    }
}

void RuntimePayloadValidator::applyBookUpdate(SymbolModel& model,
                                              uint8_t msg_type,
                                              uint64_t order_ref_num,
                                              uint64_t new_order_ref_num,
                                              char side,
                                              uint32_t shares,
                                              uint32_t price) const {
    auto order_it = model.orders.find(order_ref_num);

    switch (msg_type) {
        case kTypeAdd:
        case kTypeAddMpid: {
            const OrderState order {side, shares, price};
            model.orders[order_ref_num] = order;
            accumulateLevel((side == 'B') ? model.bid_book : model.ask_book, price, shares);
            break;
        }
        case kTypeCancel:
        case kTypeExec:
        case kTypeExecPrice: {
            if (order_it == model.orders.end()) {
                break;
            }

            const uint32_t removed = std::min(order_it->second.shares, shares);
            accumulateLevel((order_it->second.side == 'B') ? model.bid_book : model.ask_book,
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

            accumulateLevel((order_it->second.side == 'B') ? model.bid_book : model.ask_book,
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
            accumulateLevel((old_side == 'B') ? model.bid_book : model.ask_book,
                            order_it->second.price,
                            -static_cast<int64_t>(order_it->second.shares));
            model.orders.erase(order_it);

            const OrderState order {old_side, shares, price};
            model.orders[new_order_ref_num] = order;
            accumulateLevel((old_side == 'B') ? model.bid_book : model.ask_book, price, shares);
            break;
        }
        default:
            break;
    }

    emitEventIfChanged(model);
}

void RuntimePayloadValidator::accumulateLevel(std::map<uint32_t, uint64_t>& side_book,
                                              uint32_t price,
                                              int64_t delta_shares) const {
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

void RuntimePayloadValidator::emitEventIfChanged(SymbolModel& model) const {
    ExpectedEvent event;

    if (!model.ask_book.empty()) {
        event.ask_valid = true;
        event.ask_price = model.ask_book.begin()->first;
        event.ask_shares = static_cast<uint32_t>(model.ask_book.begin()->second);
    }

    if (!model.bid_book.empty()) {
        event.bid_valid = true;
        event.bid_price = model.bid_book.rbegin()->first;
        event.bid_shares = static_cast<uint32_t>(model.bid_book.rbegin()->second);
    }

    event.stock_locate = model.stock_locate;

    if (!model.has_last_event ||
        event.ask_valid != model.last_event.ask_valid ||
        event.ask_price != model.last_event.ask_price ||
        event.ask_shares != model.last_event.ask_shares ||
        event.bid_valid != model.last_event.bid_valid ||
        event.bid_price != model.last_event.bid_price ||
        event.bid_shares != model.last_event.bid_shares) {
        model.expected_events.push_back(event);
        model.last_event = event;
        model.has_last_event = true;
    }
}

bool RuntimePayloadValidator::compareEvent(uint16_t que_idx,
                                           const FPGAEventDesc& actual,
                                           const ExpectedEvent& expected) const {
    const bool fields_match =
        actual.stock_locate == expected.stock_locate &&
        actual.ask_price == expected.ask_price &&
        actual.ask_shares == expected.ask_shares &&
        actual.bid_price == expected.bid_price &&
        actual.bid_shares == expected.bid_shares;

    if (fields_match) {
        return true;
    }

    warn("Queue %u payload mismatch", que_idx);
    warn("  actual  : locate=%04x ask=(%u,%u) bid=(%u,%u) frame_start_tk=%llu event_tk=%llu",
         actual.stock_locate,
         actual.ask_price,
         actual.ask_shares,
         actual.bid_price,
         actual.bid_shares,
         static_cast<unsigned long long>(actual.frame_start_tk),
         static_cast<unsigned long long>(actual.event_tk));
    warn("  expected: locate=%04x ask=(%u,%u) bid=(%u,%u)",
         expected.stock_locate,
         expected.ask_price,
         expected.ask_shares,
         expected.bid_price,
         expected.bid_shares);
    return false;
}

void RuntimePayloadValidator::setFailureLocked(const std::string& reason) {
    if (m_failed) {
        return;
    }

    m_failed = true;
    m_failure_reason = reason;
    error("Runtime payload validation failed: %s", m_failure_reason.c_str());
}
