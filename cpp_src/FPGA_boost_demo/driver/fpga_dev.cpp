#include "fpga_dev.h"
#include "../../common/log.h"

#include <cstring>

namespace {

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

uint64_t read_le48(const uint8_t* bytes, std::size_t offset) {
    uint64_t value = 0;
    for (int byte_idx = 0; byte_idx < 6; ++byte_idx) {
        value |= (static_cast<uint64_t>(bytes[offset + byte_idx]) << (8 * byte_idx));
    }
    return value;
}

} // namespace

FPGADev::FPGADev(std::string pci_addr)
    : BasicDev(std::move(pci_addr), 1) {
    m_rx_queues[0].symbol_name = "AAPL";
    m_rx_queues[0].stock_locate = 0x000d;
    m_rx_queues[1].symbol_name = "HSBC";
    m_rx_queues[1].stock_locate = 0x0ee8;
}

FPGADev::~FPGADev() = default;

bool FPGADev::initHardware() {
    if (m_hw_ready && m_basic_para.p_bar_addr[0] != nullptr) {
        return true;
    }

    info("Initializing FPGA RX hardware...");

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
    (void)read_reg32(REG_SYNC_ENABLE);
    m_hw_ready = true;
    return true;
}

bool FPGADev::setRxRingBuffers(uint16_t num_rx_queues, uint32_t num_pckt, uint32_t pckt_size) {
    if (!m_hw_ready && !initHardware()) {
        return false;
    }

    if (num_rx_queues != kRxQueueCount) {
        warn("Current FPGA image expects exactly %u RX queues, got %u", kRxQueueCount, num_rx_queues);
        return false;
    }

    if (num_pckt == 0) {
        warn("RX queue slot count must be non-zero");
        return false;
    }

    if (pckt_size != kRxRecordBytes) {
        warn("Ignoring requested RX record size %u and using fixed %u-byte FPGA records", pckt_size, kRxRecordBytes);
    }

    auto& allocator = DMAMemoryAllocator::getInstance();
    for (uint16_t que_idx = 0; que_idx < kRxQueueCount; ++que_idx) {
        QueueRuntime& queue = m_rx_queues[que_idx];
        queue.slot_num = num_pckt;
        queue.slot_size_bytes = kRxRecordBytes;
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
    info("TX rings are not used by the current FPGA RX flow");
    return true;
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

uint16_t FPGADev::rxQueueCount() const {
    return kRxQueueCount;
}

uint64_t FPGADev::rxQueueProdPtr(uint16_t que_idx) {
    if (_queueForIndex(que_idx) == nullptr) {
        warn("Invalid queue index %u", que_idx);
        return 0;
    }
    return read_reg64(_queueRegOffset(que_idx, REG_RX_QUE_PROD_OFFSET));
}

uint64_t FPGADev::rxQueueDropCount(uint16_t que_idx) {
    if (_queueForIndex(que_idx) == nullptr) {
        warn("Invalid queue index %u", que_idx);
        return 0;
    }
    return read_reg64(_queueRegOffset(que_idx, REG_RX_QUE_DROP_OFFSET));
}

bool FPGADev::setSyncEnable(bool enable) {
    write_reg64(REG_SYNC_ENABLE, enable ? 1ULL : 0ULL);
    bool readback = false;
    if (!readSyncEnable(readback)) {
        return false;
    }
    return readback == enable;
}

bool FPGADev::readSyncEnable(bool& enabled) {
    const uint64_t value = read_reg64(REG_SYNC_ENABLE);
    enabled = (value & 0x1ULL) != 0;
    return true;
}

bool FPGADev::pollDecodedRecord(uint16_t que_idx, FPGAEventDesc& out) {
    return pollDecodedRecords(que_idx, &out, 1) == 1;
}

std::size_t FPGADev::pollDecodedRecords(uint16_t que_idx, FPGAEventDesc* out, std::size_t max_records) {
    if (out == nullptr || max_records == 0) {
        return 0;
    }

    QueueRuntime* queue = nullptr;
    if (que_idx >= kRxQueueCount) {
        warn("Invalid queue index %u", que_idx);
        return 0;
    }
    queue = &m_rx_queues[que_idx];

    if (queue->dma_memory.virt == nullptr || queue->slot_num == 0) {
        warn("Queue %u is not configured", que_idx);
        return 0;
    }

    uint64_t prod_ptr = 0;
    if (!_queueAvailable(que_idx, prod_ptr)) {
        return 0;
    }

    std::size_t record_count = 0;
    while (record_count < max_records && queue->host_cons_ptr < prod_ptr) {
        const std::size_t slot_index = static_cast<std::size_t>(queue->host_cons_ptr % queue->slot_num);
        const uint8_t* slot_bytes = static_cast<const uint8_t*>(queue->dma_memory.virt) +
                                    slot_index * queue->slot_size_bytes;

        out[record_count] = _decodeSlotBytes(que_idx, slot_bytes);

        ++queue->host_cons_ptr;
        ++record_count;
    }

    if (record_count != 0) {
        write_reg64(_queueRegOffset(que_idx, REG_RX_QUE_CONS_PTR_OFFSET), queue->host_cons_ptr);
    }

    return record_count;
}

bool FPGADev::pollRawRecord(uint16_t que_idx, FpgaRawRxRecord& out) {
    return pollRawRecords(que_idx, &out, 1) == 1;
}

std::size_t FPGADev::pollRawRecords(uint16_t que_idx, FpgaRawRxRecord* out, std::size_t max_records) {
    if (out == nullptr || max_records == 0) {
        return 0;
    }

    QueueRuntime* queue = nullptr;
    if (que_idx >= kRxQueueCount) {
        warn("Invalid queue index %u", que_idx);
        return 0;
    }
    queue = &m_rx_queues[que_idx];

    if (queue->dma_memory.virt == nullptr || queue->slot_num == 0) {
        warn("Queue %u is not configured", que_idx);
        return 0;
    }

    uint64_t prod_ptr = 0;
    if (!_queueAvailable(que_idx, prod_ptr)) {
        return 0;
    }

    std::size_t record_count = 0;
    while (record_count < max_records && queue->host_cons_ptr < prod_ptr) {
        const std::size_t slot_index = static_cast<std::size_t>(queue->host_cons_ptr % queue->slot_num);
        const uint8_t* slot_bytes = static_cast<const uint8_t*>(queue->dma_memory.virt) +
                                    slot_index * queue->slot_size_bytes;

        out[record_count].queue_id = que_idx;
        out[record_count].sequence = queue->host_cons_ptr;
        std::memcpy(out[record_count].bytes.data(), slot_bytes, out[record_count].bytes.size());

        ++queue->host_cons_ptr;
        ++record_count;
    }

    if (record_count != 0) {
        write_reg64(_queueRegOffset(que_idx, REG_RX_QUE_CONS_PTR_OFFSET), queue->host_cons_ptr);
    }

    return record_count;
}

FPGAEventDesc FPGADev::decodeRawRecord(const FpgaRawRxRecord& record) const {
    return _decodeSlotBytes(record.queue_id, record.bytes.data());
}

FPGAEventDesc FPGADev::_decodeSlotBytes(uint16_t que_idx, const uint8_t* slot_bytes) const {
    FPGAEventDesc event;
    event.queue_id = que_idx;
    event.stock_locate = read_le16(slot_bytes, 0);
    event.frame_latency = read_le48(slot_bytes, 2);
    event.event_latency = read_le48(slot_bytes, 8);
    event.bid_shares = read_le32(slot_bytes, 14);
    event.bid_price = read_le32(slot_bytes, 18);
    event.ask_shares = read_le32(slot_bytes, 22);
    event.ask_price = read_le32(slot_bytes, 26);
    return event;
}

bool FPGADev::test_register() {
    if (!m_hw_ready && !initHardware()) {
        return false;
    }

    const uint64_t module_id = read_reg64(REG_ID);
    bool sync_enable = false;
    if (!readSyncEnable(sync_enable)) {
        warn("Failed to read REG_SYNC_ENABLE");
        return false;
    }
    info("rx_dma_config ID register: 0x%016llx", static_cast<unsigned long long>(module_id));
    info("rx_dma_config sync enable register: %u", sync_enable ? 1U : 0U);

    if (module_id != RX_DMA_CFG_ID) {
        warn("Unexpected module ID, expected 0x%016llx", static_cast<unsigned long long>(RX_DMA_CFG_ID));
        return false;
    }

    if (!setSyncEnable(true)) {
        warn("Failed to set REG_SYNC_ENABLE to 1");
        return false;
    }

    if (!setSyncEnable(false)) {
        warn("Failed to set REG_SYNC_ENABLE back to 0");
        return false;
    }

    success("FPGA BAR0 ID and REG_SYNC_ENABLE access work");
    return true;
}

bool FPGADev::trigger_interrupt() {
    warn("Interrupt test is not implemented for the current polling-based RX design");
    return false;
}

bool FPGADev::test_dma_write() {
    warn("Legacy DMA write smoke test is not implemented for the current RX flow");
    return false;
}

void FPGADev::_initStatus(DevStatus* stats) {
    if (stats == nullptr) {
        return;
    }
    std::memset(stats, 0, sizeof(DevStatus));
}

uint32_t FPGADev::_queueRegOffset(uint16_t que_idx, uint32_t reg_offset) const {
    return REG_RX_QUE_BASE0 + static_cast<uint32_t>(que_idx) * REG_RX_QUE_STRIDE + reg_offset;
}

bool FPGADev::_queueAvailable(uint16_t que_idx, uint64_t& prod_ptr) const {
    const QueueRuntime* queue = _queueForIndex(que_idx);
    if (queue == nullptr) {
        return false;
    }

    prod_ptr = const_cast<FPGADev*>(this)->rxQueueProdPtr(que_idx);
    return queue->host_cons_ptr < prod_ptr;
}

const FPGADev::QueueRuntime* FPGADev::_queueForIndex(uint16_t que_idx) const {
    if (que_idx >= kRxQueueCount) {
        return nullptr;
    }
    return &m_rx_queues[que_idx];
}
