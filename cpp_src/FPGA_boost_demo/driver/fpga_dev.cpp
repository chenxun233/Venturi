#include "fpga_dev.h"
#include "../../common/log.h"

#include <cstring>
#include <ctime>
    // m_rx_queues.resize(m_basic_para.rx_que_num);
    // m_rx_queues[0].symbol_name = "AAPL";
    // m_rx_queues[0].stock_locate = 0x000d;
    // m_rx_queues[1].symbol_name = "HSBC";
    // m_rx_queues[1].stock_locate = 0x0ee8;

FPGADev::FPGADev(std::string pci_addr)
    : BasicDev(std::move(pci_addr)) {

}

FPGADev::~FPGADev() = default;

bool FPGADev::initHardware() {
    if (m_hw_ready && m_basic_para.bar0_addr != nullptr) {
        return true;
    }

    info("Initializing FPGA RX hardware...");

    if (!_getFD()) {
        warn("Failed to get VFIO device file descriptor");
        return false;
    }
    if (!_initDMAMemoryAllocator()) {
        warn("Failed to initialize DMA memory allocator");
        return false;
    }
    if (!_getBARAddr()) {
        warn("Failed to map BAR0");
        return false;
    }
    if (m_basic_para.bar0_addr == nullptr) {
        warn("BAR0 not mapped");
        return false;
    }
    _writeReg32(REG_RESET, 1);
    (void)_readReg32(REG_SYNC_ENABLE);
    _readSymbolNum();
    m_hw_ready = true;
    return true;
}

bool FPGADev::setRxRingBuffers(uint16_t rx_que_num, uint32_t slot_num, uint32_t slot_size) {
    if (!m_hw_ready && !initHardware()) {
        error("FPGA hardware is not initialized");
        return false;
    }
    (void)rx_que_num;
    (void)slot_size;
    if (slot_num == 0) {
        error("RX queue slot count must be non-zero");
        return false;
    }
    if (slot_num == 0 || (slot_num & (slot_num - 1)) != 0) {
        error("RX queue slot count %u is not a power of 2, which may cause issues with the current FPGA design", slot_num);
        return false;
    }
    auto& allocator = _getDMAAllocator();
    for (uint16_t que_idx = 0; que_idx < m_basic_para.rx_que_num; ++que_idx) {
        QueueConfig& queue = m_rx_queues[que_idx];
        queue.slot_num = slot_num;

        const std::size_t queue_bytes = static_cast<std::size_t>(queue.slot_num) * queue.slot_size_bytes;
        queue.dma_memory = allocator.allocate(queue_bytes);
        if (!queue.dma_memory.valid()) {
            warn("Failed to allocate DMA memory for queue %u", que_idx);
            return false;
        }
        std::memset(queue.dma_memory.virt(), 0, queue_bytes);
        _writeReg64(_getRegAddr(que_idx, REG_RX_IOVA_OFFSET), queue.dma_memory.iova());
        _writeReg64(_getRegAddr(que_idx, REG_RX_QUE_SLOT_NUM_OFFSET), queue.slot_num);
        _writeReg64(_getRegAddr(que_idx, REG_RX_QUE_CONS_PTR_OFFSET), 0);

        info("Configured RX queue %u: IOVA=0x%016llx slots_num=%u slot_bytes=%u",
             que_idx,
             static_cast<unsigned long long>(queue.dma_memory.iova()),
             queue.slot_num,
             queue.slot_size_bytes);
    }
    return true;
}

bool FPGADev::setTxRingBuffers(uint16_t tx_que_num, uint32_t slot_num, uint32_t slot_size) {
    (void)tx_que_num;
    (void)slot_num;
    (void)slot_size;
    info("TX rings are not used by the current FPGA RX flow");
    return true;
}



bool  FPGADev::setSymbolLocate(uint16_t que_idx, uint16_t stock_locate) {
    if (!m_hw_ready) {
        warn("FPGA hardware is not initialized");
        return false;
    }
    if (que_idx < m_rx_queues.size()) {
        m_rx_queues[que_idx].stock_locate = stock_locate;
        _writeReg64(_getRegAddr(que_idx, REG_RX_QUE_SYMBOL_LOC_OFFSET), stock_locate);
        return true;
    }
    warn("Queue index %u more than available %u in setSymbolLocate", que_idx, static_cast<uint16_t>(m_rx_queues.size()));
    return false;
}

bool FPGADev::setPriceBase(uint16_t que_idx, uint64_t price_base) {
    if (!m_hw_ready) {
        warn("FPGA hardware is not initialized");
        return false;
    }
    if (que_idx < m_rx_queues.size()) {
        m_rx_queues[que_idx].price_base = price_base;
        _writeReg64(_getRegAddr(que_idx, REG_RX_QUE_PRICE_BASE_OFFSET), price_base);
        return true;
    }
    warn("Queue index %u more than available %u in setPriceBase", que_idx, static_cast<uint16_t>(m_rx_queues.size()));
    return false;
}

bool FPGADev::validateRxAll() {
    if (!m_hw_ready || m_basic_para.bar0_addr == nullptr) {
        warn("FPGA hardware is not initialized");
        return false;
    }
    const uint64_t symbol_num   = _readReg64(REG_RX_SYMBOL_NUM);
    const bool reg_sync_enabled =_readReg64(REG_SYNC_ENABLE) & 0x1ULL;
    if (reg_sync_enabled != m_sync_enable) {
        warn("Device sync enable mismatch: expected %s but read %s",
            m_sync_enable ? "enabled" : "disabled",
            reg_sync_enabled ? "enabled" : "disabled");
        return false;
    }
    if (symbol_num != m_basic_para.rx_que_num || symbol_num != m_rx_queues.size()) {
        warn("Device symbol number mismatch: expected %llu but read %llu",
                static_cast<unsigned long long>(m_basic_para.rx_que_num),
                static_cast<unsigned long long>(symbol_num));
        return false;
    }
    for (auto que_idx = 0; que_idx < m_basic_para.rx_que_num; ++que_idx) {
        const uint64_t iova         = _readReg64(_getRegAddr(que_idx, REG_RX_IOVA_OFFSET));
        const uint64_t slot_num     = _readReg64(_getRegAddr(que_idx, REG_RX_QUE_SLOT_NUM_OFFSET));
        const uint64_t cons_ptr     = _readReg64(_getRegAddr(que_idx, REG_RX_QUE_CONS_PTR_OFFSET));
        const uint64_t prod_ptr     = _readReg64(_getRegAddr(que_idx, REG_RX_QUE_PROD_OFFSET));
        const uint64_t drop_count   = _readReg64(_getRegAddr(que_idx, REG_RX_QUE_DROP_OFFSET));
        const uint64_t symbol_loc   = _readReg64(_getRegAddr(que_idx, REG_RX_QUE_SYMBOL_LOC_OFFSET));
        const uint64_t price_base   = _readReg64(_getRegAddr(que_idx, REG_RX_QUE_PRICE_BASE_OFFSET));
        if (iova != m_rx_queues[que_idx].dma_memory.iova()) {
            warn("Queue %u IOVA mismatch: expected 0x%016llx but read 0x%016llx",
                 que_idx,
                 static_cast<unsigned long long>(m_rx_queues[que_idx].dma_memory.iova()),
                 static_cast<unsigned long long>(iova));
            return false;
        }
        if (slot_num != m_rx_queues[que_idx].slot_num) {
            warn("Queue %u slot_num mismatch: expected %llu but read %llu",
                 que_idx,
                 static_cast<unsigned long long>(m_rx_queues[que_idx].slot_num),
                 static_cast<unsigned long long>(slot_num));
            return false;
        }
        if (cons_ptr != 0) {
            warn("Queue %u cons_ptr expected to be 0 but read %llu", que_idx, static_cast<unsigned long long>(cons_ptr));
            return false;
        }
        if (prod_ptr != 0) {
            warn("Queue %u prod_ptr expected to be 0 but read %llu", que_idx, static_cast<unsigned long long>(prod_ptr));
            return false;
        }
        if (drop_count != 0) {
            warn("Queue %u drop_count expected to be 0 but read %llu", que_idx, static_cast<unsigned long long>(drop_count));
            return false;
        }
        if (symbol_loc != m_rx_queues[que_idx].stock_locate) {
            warn("Queue %u stock_locate mismatch: expected 0x%04x but read 0x%04llx",
                 que_idx,
                 m_rx_queues[que_idx].stock_locate,
                 static_cast<unsigned long long>(symbol_loc));
            return false;
        }
        if (price_base != m_rx_queues[que_idx].price_base) {
            warn("Queue %u price_base mismatch: expected %llu but read %llu",
                 que_idx,
                 static_cast<unsigned long long>(m_rx_queues[que_idx].price_base),
                 static_cast<unsigned long long>(price_base));
            return false;
        }
    }
    m_is_valid = true;
    return true;
}


void FPGADev::_readSymbolNum() {
    m_basic_para.rx_que_num = static_cast<uint8_t>(_readReg64(REG_RX_SYMBOL_NUM));
    m_rx_queues.resize(m_basic_para.rx_que_num);
    info("Device reports %llu symbols", static_cast<unsigned long long>(m_basic_para.rx_que_num));
    return;
}

void FPGADev::_readProdPtr(uint16_t que_idx, uint64_t& prod_ptr) const {
    prod_ptr = _readReg64(_getRegAddr(que_idx, REG_RX_QUE_PROD_OFFSET));
}

void FPGADev::_writeConsPtr(uint16_t que_idx, uint64_t cons_ptr) {
    _writeReg64(_getRegAddr(que_idx, REG_RX_QUE_CONS_PTR_OFFSET), cons_ptr);
}

uint64_t FPGADev::_readDropCount(uint16_t que_idx) const {
    return _readReg64(_getRegAddr(que_idx, REG_RX_QUE_DROP_OFFSET));
}

void FPGADev::setSync(bool enable) {
    _writeReg64(REG_SYNC_ENABLE, enable ? 1ULL : 0ULL);
    m_sync_enable = enable;
}

void FPGADev::_readSyncEnable(bool& enabled) {
    const uint64_t value = _readReg64(REG_SYNC_ENABLE);
    enabled = (value & 0x1ULL) != 0;
}




void FPGADev::_readProdPtrAndTick(uint16_t que_idx,
                                  uint64_t& prod_ptr,
                                  uint64_t& fpga_tick,
                                  uint64_t& host_time_ns,
                                  uint64_t& interval,
                                bool get_time) const {
    timespec ts_before {};
    timespec ts_after {};

    if (get_time) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts_before);
        _readReg128(_getRegAddr(que_idx, REG_RX_QUE_PROD_OFFSET), prod_ptr, fpga_tick);
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts_after);
        const uint64_t before_ns =
            static_cast<uint64_t>(ts_before.tv_sec) * 1000000000ULL +
            static_cast<uint64_t>(ts_before.tv_nsec);
        const uint64_t after_ns =
            static_cast<uint64_t>(ts_after.tv_sec) * 1000000000ULL +
            static_cast<uint64_t>(ts_after.tv_nsec);

        host_time_ns = (before_ns + after_ns) / 2ULL;
        interval = after_ns - before_ns;
    } else {
        _readProdPtr(que_idx, prod_ptr);
    }

}



const uint8_t* FPGADev::_pollOneRaw(uint16_t que_idx,
                                    uint64_t cons_ptr) const {

        const uint8_t* dma_base = static_cast<const uint8_t*>(m_rx_queues[que_idx].dma_memory.virt());
        const std::size_t slot_index = static_cast<std::size_t>(cons_ptr & (m_rx_queues[que_idx].slot_num - 1));
        return dma_base + slot_index * m_rx_queues[que_idx].slot_size_bytes;
}



void FPGADev::_initStatus(DevStatus* stats) {
    if (stats == nullptr) {
        return;
    }
    std::memset(stats, 0, sizeof(DevStatus));
}


uint32_t FPGADev::_getRegAddr(uint16_t que_idx, uint32_t reg_offset) const {
    return REG_RX_QUE_IOVA + static_cast<uint32_t>(que_idx) * REG_RX_QUE_STRIDE + reg_offset;
}
