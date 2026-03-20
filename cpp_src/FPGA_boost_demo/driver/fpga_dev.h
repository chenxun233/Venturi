#pragma once

#include "../../common/basic_dev.h"
#include "../../common/dma_memory_allocator.h"
#include <vector>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>


#define SLOT_SIZE_BYTES 32

class FPGARxDataAdaptor;
class FpgaReplayValidator;

class FPGADev : public BasicDev {
public:

    FPGADev(std::string pci_addr);
    ~FPGADev() override;

    bool    initHardware() override;
    bool    setSymbolLocate(uint16_t que_idx, uint16_t stock_locate) ;
    bool    setPriceBase(uint16_t que_idx, uint64_t price_base) ;    
    bool    setRxRingBuffers(uint16_t rx_que_num, uint32_t slot_num, uint32_t slot_size) override;
    bool    setTxRingBuffers(uint16_t tx_que_num, uint32_t slot_num, uint32_t slot_size) override;
    bool    validateRxQueue() const;
    void    setSync(bool enable);
    std::size_t pollRawRecords(uint16_t que_idx, uint64_t cons_ptr, const uint8_t** out, std::size_t max_records) const;
    std::size_t pollRawRecordsSync(uint16_t que_idx, uint64_t cons_ptr, const uint8_t** out, uint64_t& FPGA_tick, std::size_t max_records) const;


private:

    static constexpr uint64_t REG_RX_IOVA_OFFSET            = 0x00;
    static constexpr uint64_t REG_RESET                     = 0x00;
    static constexpr uint64_t REG_ID                        = 0x04;
    static constexpr uint64_t REG_RX_SYMBOL_NUM             = 0x14;
    static constexpr uint64_t REG_SYNC_ENABLE               = 0x0C;
    static constexpr uint64_t REG_RX_QUE_IOVA               = 0x40;
    static constexpr uint64_t REG_RX_QUE_STRIDE             = 0x40;
    static constexpr uint64_t REG_RX_QUE_SLOT_NUM_OFFSET    = 0x08;
    static constexpr uint64_t REG_RX_QUE_ENABLE_OFFSET      = 0x10;
    static constexpr uint64_t REG_RX_QUE_CONS_PTR_OFFSET    = 0x18;
    static constexpr uint64_t REG_RX_QUE_PROD_OFFSET        = 0x20;
    static constexpr uint64_t REG_RX_QUE_DROP_OFFSET        = 0x28;
    static constexpr uint64_t REG_RX_QUE_STAT_OFFSET        = 0x30;
    static constexpr uint64_t REG_RX_QUE_SYMBOL_LOC_OFFSET  = 0x34;
    static constexpr uint64_t REG_RX_QUE_PRICE_BASE_OFFSET  = 0x38;
    static constexpr uint64_t RX_DMA_CFG_ID                 = 0x4d5f52585f434647ULL;

    struct QueueConfig {
        uint16_t    stock_locate {0};
        uint32_t    slot_num {0};
        uint64_t    price_base {0};
        uint32_t    slot_size_bytes {SLOT_SIZE_BYTES};
        DMABuffer   dma_memory;
    };


    void        _initStatus(DevStatus* stats) override;
    uint32_t    _getRegAddr(uint16_t que_idx, uint32_t reg_offset) const;
    void        _readSymbolNum() ;
    

private:
    friend class FPGARxDataAdaptor;
    friend class FpgaReplayValidator;
    // Checked validation/setup path for callers that need to establish queue invariants.
    
    uint64_t    _readProdPtr(uint16_t que_idx) const;
    void        _writeConsPtr(uint16_t que_idx, uint64_t cons_ptr);
    uint64_t    _readDropCount(uint16_t que_idx) const;
    void        _readSyncEnable(bool& enabled);
    void        _readFPGATickAndProdPtr(uint16_t que_idx, uint64_t& prod_ptr, uint64_t& fpga_tick) const;




private:
    std::vector<QueueConfig> m_rx_queues;
    bool m_sync_enable {false};
    bool m_hw_ready {false};
};
