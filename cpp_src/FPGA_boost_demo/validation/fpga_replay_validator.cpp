#include "fpga_replay_validator.h"
#include "../../common/log.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <thread>

namespace {

constexpr uint16_t    kAaplLocate = 0x000d;
constexpr uint16_t    kHsbcLocate = 0x0ee8;
constexpr std::size_t kItchMsgCountOffset = 0x3c;
constexpr std::size_t kItchMsgBaseOffset = 0x3e;
constexpr uint8_t kTypeAdd = 0x41;
constexpr uint8_t kTypeCancel = 0x58;
constexpr uint8_t kTypeDelete = 0x44;
constexpr uint8_t kTypeReplace = 0x55;
constexpr uint8_t kTypeExec = 0x45;
constexpr uint8_t kTypeAddMpid = 0x46;
constexpr uint8_t kTypeExecPrice = 0x43;
constexpr std::chrono::seconds kPollTimeout(10);

uint16_t read_be16(const std::vector<uint8_t>& bytes, std::size_t offset) {
    return static_cast<uint16_t>((static_cast<uint16_t>(bytes[offset]) << 8) |
                                 static_cast<uint16_t>(bytes[offset + 1]));
}

uint32_t read_be32(const std::vector<uint8_t>& bytes, std::size_t offset) {
    return (static_cast<uint32_t>(bytes[offset]) << 24) |
           (static_cast<uint32_t>(bytes[offset + 1]) << 16) |
           (static_cast<uint32_t>(bytes[offset + 2]) << 8) |
           static_cast<uint32_t>(bytes[offset + 3]);
}

uint64_t read_be64(const std::vector<uint8_t>& bytes, std::size_t offset) {
    uint64_t value = 0;
    for (int byte_idx = 0; byte_idx < 8; ++byte_idx) {
        value = (value << 8) | static_cast<uint64_t>(bytes[offset + byte_idx]);
    }
    return value;
}

std::string strip_underscores(std::string text) {
    text.erase(std::remove(text.begin(), text.end(), '_'), text.end());
    return text;
}

uint64_t parse_hex_u64(const std::string& text) {
    return std::stoull(strip_underscores(text), nullptr, 16);
}

bool run_command(const std::string& command) {
    info("Replay check: %s", command.c_str());
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        warn("Failed to execute command: %s", command.c_str());
        return false;
    }

    char line[256];
    bool saw_output = false;
    while (fgets(line, sizeof(line), pipe) != nullptr) {
        saw_output = true;
        std::string text(line);
        while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
            text.pop_back();
        }
        info("  %s", text.c_str());
    }

    const int status = pclose(pipe);
    if (status != 0) {
        warn("Command exited with status %d: %s", status, command.c_str());
        return false;
    }

    if (!saw_output) {
        info("  <no output>");
    }
    return true;
}

std::filesystem::path locate_repo_file(const std::string& relative_path) {
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

FpgaReplayValidator::FpgaReplayValidator(FPGADev& device)
    : m_device(device),
      m_adapter(device),
      m_queues({
          QueueValidationState{"AAPL", kAaplLocate, {}},
          QueueValidationState{"HSBC", kHsbcLocate, {}},
      }) {
}

bool FpgaReplayValidator::run() {
    if (!m_device.initHardware()) {
        warn("Hardware initialization failed or link is down");
        return false;
    }

    if (!validateSyncEnable()) {
        return false;
    }

    if (!m_device.setRxRingBuffers(FPGADev::kRxQueueCount, 128, FPGADev::kRxRecordBytes)) {
        return false;
    }

    if (!loadExpectedPayloads()) {
        return false;
    }

    if (!runReplayEnvironmentChecks()) {
        warn("Replay environment checks reported issues; continue only if tcpreplay/interface setup is intentional");
    }

    info("Replay traffic from another terminal with:");
    info("  tcpreplay -i enp1s0f1 -t --loop=0 market_data/HSBC_AAPL.pcap");
    info("Starting one polling thread per FPGA RX queue through the adapter...");

    std::atomic<bool> validation_ok(true);
    std::array<std::thread, FPGADev::kRxQueueCount> threads;

    for (uint16_t que_idx = 0; que_idx < FPGADev::kRxQueueCount; ++que_idx) {
        threads[que_idx] = std::thread([this, que_idx, &validation_ok]() {
            if (!pollQueueAndValidate(que_idx)) {
                validation_ok.store(false, std::memory_order_relaxed);
            }
        });
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    if (!validation_ok.load(std::memory_order_relaxed)) {
        warn("RX payload validation failed");
        return false;
    }

    success("FPGA RX payload validation passed for both queues");
    return true;
}

bool FpgaReplayValidator::loadExpectedPayloads() {
    struct FixtureSpec {
        uint16_t que_idx;
        const char* file_name;
    };

    const std::array<FixtureSpec, FPGADev::kRxQueueCount> fixtures = {{
        {0, "market_data/AAPL_13_B_payload_frames_hex.txt"},
        {1, "market_data/HSBC_3816_S_payload_frames_hex.txt"}
    }};

    for (const auto& fixture : fixtures) {
        std::vector<uint8_t> frame_bytes;
        if (!loadFixtureFrame(fixture.file_name, frame_bytes)) {
            return false;
        }

        m_queues[fixture.que_idx].expected_events.clear();
        if (!parseExpectedEvents(frame_bytes, m_queues[fixture.que_idx])) {
            return false;
        }

        info("Loaded %zu expected events for queue %u (%s)",
             m_queues[fixture.que_idx].expected_events.size(),
             fixture.que_idx,
             m_queues[fixture.que_idx].symbol_name.c_str());
    }

    return true;
}

bool FpgaReplayValidator::loadFixtureFrame(const std::string& file_name, std::vector<uint8_t>& frame_bytes) {
    const std::filesystem::path file_path = locate_repo_file(file_name);
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
            const std::size_t offset = static_cast<std::size_t>(parse_hex_u64(line.substr(off_pos + 2, off_end - off_pos - 2)));
            const std::string value_hex = strip_underscores(line.substr(value_pos + 5, value_end - value_pos - 5));
            for (std::size_t byte_idx = 0; byte_idx < 16; ++byte_idx) {
                frame_bytes[offset + byte_idx] = static_cast<uint8_t>(parse_hex_u64(value_hex.substr(byte_idx * 2, 2)));
            }
            highest_written = std::max(highest_written, offset + 16);
        } else if (line.find("set2") != std::string::npos) {
            const std::size_t off_pos = line.find("'h");
            const std::size_t off_end = line.find(',', off_pos);
            const std::size_t value_pos = line.find("16'h", off_end);
            const std::size_t value_end = line.find(')', value_pos);
            const std::size_t offset = static_cast<std::size_t>(parse_hex_u64(line.substr(off_pos + 2, off_end - off_pos - 2)));
            const std::string value_hex = strip_underscores(line.substr(value_pos + 4, value_end - value_pos - 4));
            for (std::size_t byte_idx = 0; byte_idx < 2; ++byte_idx) {
                frame_bytes[offset + byte_idx] = static_cast<uint8_t>(parse_hex_u64(value_hex.substr(byte_idx * 2, 2)));
            }
            highest_written = std::max(highest_written, offset + 2);
        } else if (line.find("frame_bytes") != std::string::npos) {
            const std::size_t off_pos = line.find("'h");
            const std::size_t off_end = line.find(']', off_pos);
            const std::size_t value_pos = line.find("8'h", off_end);
            const std::size_t value_end = line.find(';', value_pos);
            const std::size_t offset = static_cast<std::size_t>(parse_hex_u64(line.substr(off_pos + 2, off_end - off_pos - 2)));
            frame_bytes[offset] = static_cast<uint8_t>(parse_hex_u64(line.substr(value_pos + 3, value_end - value_pos - 3)));
            highest_written = std::max(highest_written, offset + 1);
        }
    }

    frame_bytes.resize(highest_written);
    return true;
}

bool FpgaReplayValidator::parseExpectedEvents(const std::vector<uint8_t>& frame_bytes, QueueValidationState& queue) {
    if (frame_bytes.size() <= kItchMsgBaseOffset) {
        warn("Frame fixture for %s is too short", queue.symbol_name.c_str());
        return false;
    }

    SymbolModel model;
    model.symbol_name = queue.symbol_name;
    model.stock_locate = queue.stock_locate;

    const uint16_t msg_count = read_be16(frame_bytes, kItchMsgCountOffset);
    std::size_t msg_offset = kItchMsgBaseOffset;

    for (uint16_t msg_idx = 0; msg_idx < msg_count; ++msg_idx) {
        if (msg_offset + 2 >= frame_bytes.size()) {
            warn("Unexpected end of frame while parsing %s fixture", queue.symbol_name.c_str());
            return false;
        }

        const uint16_t msg_len = read_be16(frame_bytes, msg_offset);
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

bool FpgaReplayValidator::parseMessage(const std::vector<uint8_t>& frame_bytes, std::size_t msg_offset, uint16_t msg_len, SymbolModel& model) {
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

    const uint16_t stock_locate = read_be16(frame_bytes, msg_body + 1);
    if (stock_locate != model.stock_locate) {
        return true;
    }

    switch (msg_type) {
        case kTypeAdd:
        case kTypeAddMpid: {
            const uint64_t order_ref = read_be64(frame_bytes, msg_body + 11);
            const char side = static_cast<char>(frame_bytes[msg_body + 19]);
            const uint32_t shares = read_be32(frame_bytes, msg_body + 20);
            const uint32_t price = read_be32(frame_bytes, msg_body + 32);
            applyBookUpdate(model, msg_type, order_ref, 0, side, shares, price);
            return true;
        }
        case kTypeCancel:
        case kTypeExec:
        case kTypeExecPrice: {
            const uint64_t order_ref = read_be64(frame_bytes, msg_body + 11);
            const uint32_t shares = read_be32(frame_bytes, msg_body + 19);
            applyBookUpdate(model, msg_type, order_ref, 0, '\0', shares, 0);
            return true;
        }
        case kTypeDelete: {
            const uint64_t order_ref = read_be64(frame_bytes, msg_body + 11);
            applyBookUpdate(model, msg_type, order_ref, 0, '\0', 0, 0);
            return true;
        }
        case kTypeReplace: {
            const uint64_t order_ref = read_be64(frame_bytes, msg_body + 11);
            const uint64_t new_order_ref = read_be64(frame_bytes, msg_body + 19);
            const uint32_t shares = read_be32(frame_bytes, msg_body + 27);
            const uint32_t price = read_be32(frame_bytes, msg_body + 31);
            applyBookUpdate(model, msg_type, order_ref, new_order_ref, '\0', shares, price);
            return true;
        }
        default:
            return true;
    }
}

void FpgaReplayValidator::applyBookUpdate(SymbolModel& model, uint8_t msg_type, uint64_t order_ref_num, uint64_t new_order_ref_num, char side, uint32_t shares, uint32_t price) {
    auto order_it = model.orders.find(order_ref_num);

    switch (msg_type) {
        case kTypeAdd:
        case kTypeAddMpid: {
            OrderState order {side, shares, price};
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

            OrderState order {old_side, shares, price};
            model.orders[new_order_ref_num] = order;
            accumulateLevel((old_side == 'B') ? model.bid_book : model.ask_book, price, shares);
            break;
        }
        default:
            break;
    }

    emitEventIfChanged(model);
}

void FpgaReplayValidator::accumulateLevel(std::map<uint32_t, uint64_t>& side_book, uint32_t price, int64_t delta_shares) {
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

void FpgaReplayValidator::emitEventIfChanged(SymbolModel& model) {
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

bool FpgaReplayValidator::runReplayEnvironmentChecks() {
    bool ok = true;
    ok &= run_command("which tcpreplay");
    ok &= run_command("getcap $(which tcpreplay)");
    ok &= run_command("ip link show enp1s0f1");
    return ok;
}

bool FpgaReplayValidator::validateSyncEnable() {
    bool initial_value = false;
    if (!m_device.readSyncEnable(initial_value)) {
        warn("Failed to read REG_SYNC_ENABLE before replay validation");
        return false;
    }

    info("REG_SYNC_ENABLE initial state: %u", initial_value ? 1U : 0U);

    if (!m_device.setSyncEnable(true)) {
        warn("Failed to enable REG_SYNC_ENABLE during replay validation");
        return false;
    }

    if (!m_device.setSyncEnable(initial_value)) {
        warn("Failed to restore REG_SYNC_ENABLE to %u during replay validation", initial_value ? 1U : 0U);
        return false;
    }

    return true;
}

bool FpgaReplayValidator::pollQueueAndValidate(uint16_t que_idx) {
    if (que_idx >= m_queues.size()) {
        warn("Invalid queue index %u", que_idx);
        return false;
    }

    QueueValidationState& queue = m_queues[que_idx];
    if (queue.expected_events.empty()) {
        warn("Queue %u (%s) has no expected events to validate", que_idx, queue.symbol_name.c_str());
        return false;
    }

    uint64_t event_idx = 0;
    const auto deadline = std::chrono::steady_clock::now() + kPollTimeout;

    info("Queue %u (%s) waiting for %zu FPGA events",
         que_idx,
         queue.symbol_name.c_str(),
         queue.expected_events.size());

    while (event_idx < queue.expected_events.size()) {
        FPGAEventDesc actual;
        if (!m_adapter.pollOne(que_idx, actual)) {
            if (std::chrono::steady_clock::now() > deadline) {
                warn("Timed out waiting for queue %u (%s): consumed %llu / %zu events",
                     que_idx,
                     queue.symbol_name.c_str(),
                     static_cast<unsigned long long>(event_idx),
                     queue.expected_events.size());
                return false;
            }
            std::this_thread::yield();
            continue;
        }

        if (!compareEvent(que_idx, event_idx, actual)) {
            return false;
        }

        ++event_idx;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    const uint64_t final_prod_ptr = m_device.rxQueueProdPtr(que_idx);
    if (final_prod_ptr != queue.expected_events.size()) {
        warn("Queue %u (%s) produced %llu events, expected %zu",
             que_idx,
             queue.symbol_name.c_str(),
             static_cast<unsigned long long>(final_prod_ptr),
             queue.expected_events.size());
        return false;
    }

    const uint64_t drop_count = m_device.rxQueueDropCount(que_idx);
    if (drop_count != 0) {
        warn("Queue %u (%s) reported drop_count=%llu",
             que_idx,
             queue.symbol_name.c_str(),
             static_cast<unsigned long long>(drop_count));
        return false;
    }

    success("Queue %u (%s) validated %zu events",
            que_idx,
            queue.symbol_name.c_str(),
            queue.expected_events.size());
    return true;
}

bool FpgaReplayValidator::compareEvent(uint16_t que_idx, uint64_t event_idx, const FPGAEventDesc& actual) {
    if (que_idx >= m_queues.size() || event_idx >= m_queues[que_idx].expected_events.size()) {
        warn("Unexpected queue/event index during comparison: queue=%u event=%llu",
             que_idx,
             static_cast<unsigned long long>(event_idx));
        return false;
    }

    const ExpectedEvent& expected = m_queues[que_idx].expected_events[event_idx];
    const bool fields_match =
        actual.stock_locate == expected.stock_locate &&
        actual.ask_price == expected.ask_price &&
        actual.ask_shares == expected.ask_shares &&
        actual.bid_price == expected.bid_price &&
        actual.bid_shares == expected.bid_shares;

    if (!fields_match) {
        warn("Queue %u (%s) payload mismatch at event %llu",
             que_idx,
             m_queues[que_idx].symbol_name.c_str(),
             static_cast<unsigned long long>(event_idx));
        warn("  actual  : locate=%04x ask=(%u,%u) bid=(%u,%u) event_latency=%llu frame_latency=%llu",
             actual.stock_locate,
             actual.ask_price,
             actual.ask_shares,
             actual.bid_price,
             actual.bid_shares,
             static_cast<unsigned long long>(actual.event_latency),
             static_cast<unsigned long long>(actual.frame_latency));
        warn("  expected: locate=%04x ask=(%u,%u) bid=(%u,%u)",
             expected.stock_locate,
             expected.ask_price,
             expected.ask_shares,
             expected.bid_price,
             expected.bid_shares);
        return false;
    }

    info("Queue %u (%s) event %llu validated: locate=%04x ask=(%u,%u) bid=(%u,%u) event_latency=%llu frame_latency=%llu",
         que_idx,
         m_queues[que_idx].symbol_name.c_str(),
         static_cast<unsigned long long>(event_idx),
         actual.stock_locate,
         actual.ask_price,
         actual.ask_shares,
         actual.bid_price,
         actual.bid_shares,
         static_cast<unsigned long long>(actual.event_latency),
         static_cast<unsigned long long>(actual.frame_latency));
    return true;
}
