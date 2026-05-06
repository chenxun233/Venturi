#pragma once

#include "../../common/dma_memory_allocator.h"
#include "../../common/device.h"
#include "../common/shared_types.h"
#include <vector>
#include <array>
#include <cstddef>
#include <cstdint>
#include <linux/vfio.h>
#include <memory>
#include <string>



#define SLOT_SIZE_BYTES 32 // 32 bytes, 256 bits.

struct FpgaBasicPara {
    std::string pci_addr;
    uint16_t rx_que_num;
    uint16_t tx_que_num;
    uint8_t* bar0_addr;
};

struct FpgaVfioFd {
    int container_fd;
    int group_id;
    int group_fd;
    int device_fd;
};

struct QueueConfig {
    uint16_t    stock_locate {0};
    uint32_t    slot_num {0};
    uint64_t    price_base {0};
    uint32_t    slot_size_bytes {SLOT_SIZE_BYTES};
    DMABuffer   dma_memory;
};


class FPGADev {
public:
    FPGADev(std::string pci_addr);
    virtual ~FPGADev();

    bool    initHardware();
    bool    setSymbolLocate(uint16_t que_idx, uint16_t stock_locate) ;
    bool    setPriceBase(uint16_t que_idx, uint64_t price_base) ;    
    bool    setRxRingBuffers( uint32_t slot_num, uint32_t slot_size);
    // bool    setTxRingBuffers(uint16_t tx_que_num, uint32_t slot_num, uint32_t slot_size);
    virtual bool isValid() const { return m_is_valid; }
    virtual void readProdPtr(uint16_t que_idx, uint64_t& prod_ptr) const;
    virtual uint64_t readDropCount(uint16_t que_idx) const;
    virtual const uint8_t* pollDataRaw(uint16_t que_idx, uint64_t cons_ptr) const;
    virtual void writeConsPtr(uint16_t que_idx, uint64_t cons_ptr);



private:
    //offsets and strides for BAR0 registers
    static constexpr uint64_t BAR0_GLOBAL_REG_RESET_OFFSET                  = 0x00;
    static constexpr uint64_t BAR0_GLOBAL_REG_RX_SYMBOL_COUNT_OFFSET        = 0x14;
    static constexpr uint64_t BAR0_RX_QUEUE_BLOCK_BASE_OFFSET               = 0x40;
    static constexpr uint64_t BAR0_RX_QUEUE_BLOCK_STRIDE                    = 0x40;
    static constexpr uint64_t BAR0_RX_QUEUE_REG_IOVA_OFFSET                 = 0x00;
    static constexpr uint64_t BAR0_RX_QUEUE_REG_SLOT_COUNT_OFFSET           = 0x08;
    static constexpr uint64_t BAR0_RX_QUEUE_REG_ENABLE_OFFSET               = 0x10;
    static constexpr uint64_t BAR0_RX_QUEUE_REG_CONS_PTR_OFFSET             = 0x18;
    static constexpr uint64_t BAR0_RX_QUEUE_REG_PROD_PTR_OFFSET             = 0x20;
    static constexpr uint64_t BAR0_RX_QUEUE_REG_DROP_COUNT_OFFSET           = 0x28;
    static constexpr uint64_t BAR0_RX_QUEUE_REG_STATUS_OFFSET               = 0x30;
    static constexpr uint64_t BAR0_RX_QUEUE_REG_SYMBOL_LOC_OFFSET           = 0x34;
    static constexpr uint64_t BAR0_RX_QUEUE_REG_PRICE_BASE_OFFSET           = 0x38;
    
private:
    bool        _getFD();
    bool        _getGroupID();
    bool        _getContainerFD();
    bool        _getGroupFD();
    bool        _addGroup2Container();
    bool        _getDeviceFD();
    bool        _getBARAddr();
    bool        _initDMAMemoryAllocator();
    void        _writeReg64(uint32_t offset, uint64_t value);
    uint64_t    _readReg64(uint32_t offset) const;
    void        _writeReg32(uint32_t offset, uint32_t value);
    uint32_t    _readReg32(uint32_t offset) const;
    DMAMemoryAllocator& _getDMAAllocator();
    const DMAMemoryAllocator& _getDMAAllocator() const;
    uint32_t    _getRegAddr(uint16_t que_idx, uint32_t reg_offset) const;
    void        _readSymbolNum() ;


private:
    FpgaBasicPara m_basic_para {};
    FpgaVfioFd m_fds {-1, -1, -1, -1};
    std::unique_ptr<DMAMemoryAllocator> m_dma_allocator;
    std::vector<QueueConfig> m_rx_queues;
    bool m_hw_ready {false};
    bool m_is_valid {false};
};
