#include "fpga_dev.h"
#include "../common/log.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

namespace {

constexpr uint16_t kAaplLocate = 0x000d;
constexpr uint16_t kHsbcLocate = 0x0ee8;
constexpr std::size_t kItchMsgCountOffset = 0x3c;
constexpr std::size_t kItchMsgBaseOffset = 0x3e;
constexpr uint8_t kTypeAdd = 0x41;      // A
constexpr uint8_t kTypeCancel = 0x58;   // X
constexpr uint8_t kTypeDelete = 0x44;   // D
constexpr uint8_t kTypeReplace = 0x55;  // U
constexpr uint8_t kTypeExec = 0x45;     // E
constexpr uint8_t kTypeAddMpid = 0x46;  // F
constexpr uint8_t kTypeExecPrice = 0x43;// C
constexpr std::chrono::seconds kPollTimeout(10);
constexpr uint32_t kConsPtrBatchSize = 16;

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

std::string bytes_to_hex(const uint8_t* bytes, std::size_t length) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (std::size_t idx = 0; idx < length; ++idx) {
        oss << std::setw(2) << static_cast<unsigned>(bytes[idx]);
    }
    return oss.str();
}

uint16_t read_le16(const uint8_t* bytes, std::size_t offset) {
    return static_cast<uint16_t>(static_cast<uint16_t>(bytes[offset]) |
                                 (static_cast<uint16_t>(bytes[offset + 1]) << 8));
}

uint32_t read_le32(const uint8_t* bytes, std::size_t offset) {
    return static_cast<uint32_t>(bytes[offset]) |
           (static_cast<uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<uint32_t>(bytes[offset + 3]) << 24);
}

uint64_t read_le64(const uint8_t* bytes, std::size_t offset) {
    uint64_t value = 0;
    for (int byte_idx = 0; byte_idx < 8; ++byte_idx) {
        value |= (static_cast<uint64_t>(bytes[offset + byte_idx]) << (8 * byte_idx));
    }
    return value;
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

FPGADev::FPGADev(std::string pci_addr)
    : BasicDev(std::move(pci_addr), 1) {
    m_rx_queues[0].symbol_name = "AAPL";
    m_rx_queues[0].stock_locate = kAaplLocate;
    m_rx_queues[1].symbol_name = "HSBC";
    m_rx_queues[1].stock_locate = kHsbcLocate;
}

FPGADev::~FPGADev() = default;

bool FPGADev::initHardware() {
    info("Initializing FPGA RX validation hardware...");

    if (!_getFD()) {
        warn("Failed to get VFIO device file descriptor");
        return false;
    }

    if (!_getBARAddr(0)) {
        warn("Failed to map BAR0");
        return false;
    }

    if (m_basic_para.p_bar_addr[0] == nullptr) {
        warn("BAR0 not mapped");
        return false;
    }
    write_reg32(REG_RESET, 1);
    (void)read_reg32(REG_STATUS);
info("Queue0 drop count is %llu",
     static_cast<unsigned long long>(
         read_reg64(_queueRegOffset(0, REG_RX_QUE_DROP_OFFSET))));
info("Queue1 drop count is %llu",
     static_cast<unsigned long long>(
         read_reg64(_queueRegOffset(1, REG_RX_QUE_DROP_OFFSET))));

    m_hw_ready = true;
    return true;
}

bool FPGADev::setRxRingBuffers(uint16_t num_rx_queues, uint32_t num_pckt, uint32_t pckt_size) {
    if (!m_hw_ready && !initHardware()) {
        return false;
    }

    if (num_rx_queues != RX_QUEUE_COUNT) {
        warn("Current FPGA image expects exactly %u RX queues, got %u", RX_QUEUE_COUNT, num_rx_queues);
        return false;
    }

    if (num_pckt == 0) {
        warn("RX queue slot count must be non-zero");
        return false;
    }

    if (pckt_size != RX_RECORD_BYTES) {
        warn("Ignoring requested RX record size %u and using fixed %u-byte FPGA records", pckt_size, RX_RECORD_BYTES);
    }

    auto& allocator = DMAMemoryAllocator::getInstance();
    for (uint16_t que_idx = 0; que_idx < RX_QUEUE_COUNT; ++que_idx) {
        QueueRuntime& queue = m_rx_queues[que_idx];
        queue.slot_num = num_pckt;
        queue.slot_size_bytes = RX_RECORD_BYTES;
        queue.host_cons_ptr = 0;

        const std::size_t queue_bytes = static_cast<std::size_t>(queue.slot_num) * queue.slot_size_bytes;
        queue.dma_memory = allocator.allocDMAMemory(queue_bytes, m_fds.container_fd);
        if (queue.dma_memory.virt == nullptr || queue.dma_memory.iova == 0) {
            warn("Failed to allocate DMA memory for queue %u", que_idx);
            return false;
        }

        std::memset(queue.dma_memory.virt, 0, queue_bytes);

        write_reg64(_queueRegOffset(que_idx, REG_RX_IOVA_OFFSET), queue.dma_memory.iova);
        write_reg64(_queueRegOffset(que_idx, REG_RX_QUE_SLOT_NUM_OFFSET), queue.slot_num);
        write_reg64(_queueRegOffset(que_idx, REG_RX_QUE_CONS_PTR_OFFSET), 0);

        info("Configured RX queue %u (%s): IOVA=0x%016llx slots=%u slot_bytes=%u",
             que_idx,
             queue.symbol_name.c_str(),
             static_cast<unsigned long long>(queue.dma_memory.iova),
             queue.slot_num,
             queue.slot_size_bytes);
    }

    m_basic_para.num_rx_queues = num_rx_queues;
    return true;
}

bool FPGADev::setTxRingBuffers(uint16_t num_tx_queues, uint32_t num_pckt, uint32_t pckt_size) {
    (void)num_tx_queues;
    (void)num_pckt;
    (void)pckt_size;
    info("TX rings are not used by the current FPGA RX validation flow");
    return true;
}

bool FPGADev::_enableDMA() {
    return true;
}

void FPGADev::_initStatus(DevStatus* stats) {
    if (stats == nullptr) {
        return;
    }
    std::memset(stats, 0, sizeof(DevStatus));
}

void FPGADev::write_reg64(uint32_t offset, uint64_t value) {
    if (m_basic_para.p_bar_addr[0] == nullptr) {
        warn("BAR0 not mapped");
        return;
    }
    __asm__ volatile("" ::: "memory");
    volatile uint64_t* reg = reinterpret_cast<volatile uint64_t*>(m_basic_para.p_bar_addr[0] + offset);
    *reg = value;
}

uint64_t FPGADev::read_reg64(uint32_t offset) {
    if (m_basic_para.p_bar_addr[0] == nullptr) {
        warn("BAR0 not mapped");
        return 0;
    }
    __asm__ volatile("" ::: "memory");
    volatile uint64_t* reg = reinterpret_cast<volatile uint64_t*>(m_basic_para.p_bar_addr[0] + offset);
    return *reg;
}

void FPGADev::write_reg32(uint32_t offset, uint32_t value) {
    if (m_basic_para.p_bar_addr[0] == nullptr) {
        warn("BAR0 not mapped");
        return;
    }
    __asm__ volatile("" ::: "memory");
    volatile uint32_t* reg = reinterpret_cast<volatile uint32_t*>(m_basic_para.p_bar_addr[0] + offset);
    *reg = value;
}

uint32_t FPGADev::read_reg32(uint32_t offset) {
    if (m_basic_para.p_bar_addr[0] == nullptr) {
        warn("BAR0 not mapped");
        return 0;
    }
    __asm__ volatile("" ::: "memory");
    volatile uint32_t* reg = reinterpret_cast<volatile uint32_t*>(m_basic_para.p_bar_addr[0] + offset);
    return *reg;
}

bool FPGADev::test_register() {
    if (!m_hw_ready && !initHardware()) {
        return false;
    }

    const uint64_t module_id = read_reg64(REG_ID);
    const uint64_t status = read_reg64(REG_STATUS);
    info("rx_dma_config ID register: 0x%016llx", static_cast<unsigned long long>(module_id));
    info("rx_dma_config status register: 0x%016llx", static_cast<unsigned long long>(status));

    if (module_id != RX_DMA_CFG_ID) {
        warn("Unexpected module ID, expected 0x%016llx", static_cast<unsigned long long>(RX_DMA_CFG_ID));
        return false;
    }

    success("FPGA BAR0 register access works");
    return true;
}

bool FPGADev::trigger_interrupt() {
    warn("Interrupt test is not implemented for the current polling-based RX design");
    return false;
}

bool FPGADev::test_dma_write() {
    warn("Legacy DMA write smoke test is not implemented for the current RX validation flow");
    return false;
}

bool FPGADev::test_dma_roundtrip() {
    if (!m_hw_ready && !initHardware()) {
        return false;
    }

    if (!setRxRingBuffers(RX_QUEUE_COUNT, 128, RX_RECORD_BYTES)) {
        return false;
    }

    if (!_loadExpectedPayloads()) {
        return false;
    }

    if (!_runReplayEnvironmentChecks()) {
        warn("Replay environment checks reported issues; continue only if tcpreplay/interface setup is intentional");
    }

    info("Replay traffic from another terminal with:");
    info("  tcpreplay -i enp1s0f1 -t --loop=0 market_data/HSBC_AAPL.pcap");
    info("Starting one polling thread per FPGA RX queue...");

    std::atomic<bool> validation_ok(true);
    std::array<std::thread, RX_QUEUE_COUNT> threads;

    for (uint16_t que_idx = 0; que_idx < RX_QUEUE_COUNT; ++que_idx) {
        threads[que_idx] = std::thread([this, que_idx, &validation_ok]() {
            if (!_pollQueueAndValidate(que_idx)) {
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

uint32_t FPGADev::_queueRegOffset(uint16_t que_idx, uint32_t reg_offset) const {
    return REG_RX_QUE_BASE0 + static_cast<uint32_t>(que_idx) * REG_RX_QUE_STRIDE + reg_offset;
}

bool FPGADev::_loadExpectedPayloads() {
    struct FixtureSpec {
        uint16_t que_idx;
        const char* file_name;
    };

    const std::array<FixtureSpec, RX_QUEUE_COUNT> fixtures = {{
        {0, "market_data/AAPL_13_B_payload_frames_hex.txt"},
        {1, "market_data/HSBC_3816_S_payload_frames_hex.txt"}
    }};

    for (const auto& fixture : fixtures) {
        std::vector<uint8_t> frame_bytes;
        if (!_loadFixtureFrame(fixture.file_name, frame_bytes)) {
            return false;
        }

        m_rx_queues[fixture.que_idx].expected_events.clear();
        if (!_parseExpectedEvents(frame_bytes, m_rx_queues[fixture.que_idx])) {
            return false;
        }

        info("Loaded %zu expected events for queue %u (%s)",
             m_rx_queues[fixture.que_idx].expected_events.size(),
             fixture.que_idx,
             m_rx_queues[fixture.que_idx].symbol_name.c_str());
    }

    return true;
}

bool FPGADev::_loadFixtureFrame(const std::string& file_name, std::vector<uint8_t>& frame_bytes) {
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

bool FPGADev::_parseExpectedEvents(const std::vector<uint8_t>& frame_bytes, QueueRuntime& queue) {
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

        if (!_parseMessage(frame_bytes, msg_offset, msg_len, model)) {
            return false;
        }

        msg_offset += 2 + msg_len;
    }

    queue.expected_events = std::move(model.expected_events);
    return true;
}

bool FPGADev::_parseMessage(const std::vector<uint8_t>& frame_bytes, std::size_t msg_offset, uint16_t msg_len, SymbolModel& model) {
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
            _applyBookUpdate(model, msg_type, order_ref, 0, side, shares, price);
            return true;
        }
        case kTypeCancel:
        case kTypeExec:
        case kTypeExecPrice: {
            const uint64_t order_ref = read_be64(frame_bytes, msg_body + 11);
            const uint32_t shares = read_be32(frame_bytes, msg_body + 19);
            _applyBookUpdate(model, msg_type, order_ref, 0, '\0', shares, 0);
            return true;
        }
        case kTypeDelete: {
            const uint64_t order_ref = read_be64(frame_bytes, msg_body + 11);
            _applyBookUpdate(model, msg_type, order_ref, 0, '\0', 0, 0);
            return true;
        }
        case kTypeReplace: {
            const uint64_t order_ref = read_be64(frame_bytes, msg_body + 11);
            const uint64_t new_order_ref = read_be64(frame_bytes, msg_body + 19);
            const uint32_t shares = read_be32(frame_bytes, msg_body + 27);
            const uint32_t price = read_be32(frame_bytes, msg_body + 31);
            _applyBookUpdate(model, msg_type, order_ref, new_order_ref, '\0', shares, price);
            return true;
        }
        default:
            return true;
    }
}

void FPGADev::_applyBookUpdate(SymbolModel& model, uint8_t msg_type, uint64_t order_ref_num, uint64_t new_order_ref_num, char side, uint32_t shares, uint32_t price) {
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

void FPGADev::_accumulateLevel(std::map<uint32_t, uint64_t>& side_book, uint32_t price, int64_t delta_shares) {
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

void FPGADev::_emitEventIfChanged(SymbolModel& model) {
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

bool FPGADev::_runReplayEnvironmentChecks() {
    bool ok = true;
    ok &= run_command("which tcpreplay");
    ok &= run_command("getcap $(which tcpreplay)");
    ok &= run_command("ip link show enp1s0f1");
    return ok;
}

bool FPGADev::_pollQueueAndValidate(uint16_t que_idx) {
    const QueueRuntime* queue_ptr = _queueForIndex(que_idx);
    if (queue_ptr == nullptr) {
        warn("Invalid queue index %u", que_idx);
        return false;
    }

    QueueRuntime& queue = m_rx_queues[que_idx];
    if (queue.expected_events.empty()) {
        warn("Queue %u (%s) has no expected events to validate", que_idx, queue.symbol_name.c_str());
        return false;
    }

    uint64_t last_timestamp = 0;
    const auto deadline = std::chrono::steady_clock::now() + kPollTimeout;

    info("Queue %u (%s) waiting for %zu FPGA events",
         que_idx,
         queue.symbol_name.c_str(),
         queue.expected_events.size());

    while (queue.host_cons_ptr < queue.expected_events.size()) {
        const uint64_t prod_ptr = read_reg64(_queueRegOffset(que_idx, REG_RX_QUE_PROD_OFFSET));
        uint32_t batch_count = 0;
        while (queue.host_cons_ptr < prod_ptr && queue.host_cons_ptr < queue.expected_events.size()) {
            const std::size_t slot_index = static_cast<std::size_t>(queue.host_cons_ptr % queue.slot_num);
            const uint8_t* slot_bytes = static_cast<const uint8_t*>(queue.dma_memory.virt) +
                                        slot_index * queue.slot_size_bytes;
            const DecodedEvent actual = _decodeRecord(slot_bytes);

            if (!_compareEvent(que_idx, queue.host_cons_ptr, actual, last_timestamp)) {
                return false;
            }

            ++queue.host_cons_ptr;
            ++batch_count;
            if (batch_count >= kConsPtrBatchSize) {
                write_reg64(_queueRegOffset(que_idx, REG_RX_QUE_CONS_PTR_OFFSET), queue.host_cons_ptr);
                batch_count = 0;
            }
        }

        if (batch_count != 0) {
            write_reg64(_queueRegOffset(que_idx, REG_RX_QUE_CONS_PTR_OFFSET), queue.host_cons_ptr);
        }

        if (queue.host_cons_ptr >= queue.expected_events.size()) {
            break;
        }

        if (std::chrono::steady_clock::now() > deadline) {
            warn("Timed out waiting for queue %u (%s): consumed %llu / %zu events",
                 que_idx,
                 queue.symbol_name.c_str(),
                 static_cast<unsigned long long>(queue.host_cons_ptr),
                 queue.expected_events.size());
            return false;
        }

        std::this_thread::yield();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    const uint64_t final_prod_ptr = read_reg64(_queueRegOffset(que_idx, REG_RX_QUE_PROD_OFFSET));
    if (final_prod_ptr != queue.expected_events.size()) {
        warn("Queue %u (%s) produced %llu events, expected %zu",
             que_idx,
             queue.symbol_name.c_str(),
             static_cast<unsigned long long>(final_prod_ptr),
             queue.expected_events.size());
        return false;
    }

    const uint64_t drop_count = read_reg64(_queueRegOffset(que_idx, REG_RX_QUE_DROP_OFFSET));
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

FPGADev::DecodedEvent FPGADev::_decodeRecord(const uint8_t* slot_bytes) const {
    DecodedEvent event;
    event.stock_locate = read_le16(slot_bytes, 0);
    event.event_timestamp = read_le64(slot_bytes, 2);
    event.bid_shares = read_le32(slot_bytes, 10);
    event.bid_price = read_le32(slot_bytes, 14);
    event.ask_shares = read_le32(slot_bytes, 18);
    event.ask_price = read_le32(slot_bytes, 22);
    event.bid_valid = event.bid_shares != 0;
    event.ask_valid = event.ask_shares != 0;
    return event;
}

bool FPGADev::_compareEvent(uint16_t que_idx, uint64_t event_idx, const DecodedEvent& actual, uint64_t& last_timestamp) {
    const QueueRuntime* queue = _queueForIndex(que_idx);
    if (queue == nullptr || event_idx >= queue->expected_events.size()) {
        warn("Unexpected queue/event index during comparison: queue=%u event=%llu",
             que_idx,
             static_cast<unsigned long long>(event_idx));
        return false;
    }

    const ExpectedEvent& expected = queue->expected_events[event_idx];
    const bool fields_match =
        actual.stock_locate == expected.stock_locate &&
        actual.ask_valid == expected.ask_valid &&
        actual.ask_price == expected.ask_price &&
        actual.ask_shares == expected.ask_shares &&
        actual.bid_valid == expected.bid_valid &&
        actual.bid_price == expected.bid_price &&
        actual.bid_shares == expected.bid_shares;

    if (!fields_match) {
        const std::size_t slot_index = static_cast<std::size_t>(event_idx % queue->slot_num);
        const uint8_t* slot_bytes = static_cast<const uint8_t*>(queue->dma_memory.virt) +
                                    slot_index * queue->slot_size_bytes;
        warn("Queue %u (%s) payload mismatch at event %llu",
             que_idx,
             queue->symbol_name.c_str(),
             static_cast<unsigned long long>(event_idx));
        warn("  actual  : locate=%04x ask=(%u,%u,%u) bid=(%u,%u,%u) timestamp=%llu",
             actual.stock_locate,
             actual.ask_valid,
             actual.ask_price,
             actual.ask_shares,
             actual.bid_valid,
             actual.bid_price,
             actual.bid_shares,
             static_cast<unsigned long long>(actual.event_timestamp));
        warn("  expected: locate=%04x ask=(%u,%u,%u) bid=(%u,%u,%u)",
             expected.stock_locate,
             expected.ask_valid,
             expected.ask_price,
             expected.ask_shares,
             expected.bid_valid,
             expected.bid_price,
             expected.bid_shares);
        warn("  raw slot: %s", bytes_to_hex(slot_bytes, queue->slot_size_bytes).c_str());
        return false;
    }

    const bool all_zero_event =
        actual.stock_locate == 0 &&
        !actual.ask_valid &&
        actual.ask_price == 0 &&
        actual.ask_shares == 0 &&
        !actual.bid_valid &&
        actual.bid_price == 0 &&
        actual.bid_shares == 0 &&
        actual.event_timestamp == 0;

    if (!all_zero_event && event_idx != 0 && actual.event_timestamp <= last_timestamp) {
        warn("Queue %u (%s) timestamp did not increase at event %llu: prev=%llu cur=%llu",
             que_idx,
             queue->symbol_name.c_str(),
             static_cast<unsigned long long>(event_idx),
             static_cast<unsigned long long>(last_timestamp),
             static_cast<unsigned long long>(actual.event_timestamp));
        return false;
    }

    if (!all_zero_event) {
        last_timestamp = actual.event_timestamp;
    }
    info("Queue %u (%s) event %llu validated: locate=%04x ask=(%u,%u,%u) bid=(%u,%u,%u) timestamp=%llu",
         que_idx,
         queue->symbol_name.c_str(),
         static_cast<unsigned long long>(event_idx),
         actual.stock_locate,
         actual.ask_valid,
         actual.ask_price,
         actual.ask_shares,
         actual.bid_valid,
         actual.bid_price,
         actual.bid_shares,
         static_cast<unsigned long long>(actual.event_timestamp));
    return true;
}

const FPGADev::QueueRuntime* FPGADev::_queueForIndex(uint16_t que_idx) const {
    if (que_idx >= RX_QUEUE_COUNT) {
        return nullptr;
    }
    return &m_rx_queues[que_idx];
}
