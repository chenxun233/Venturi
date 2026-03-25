#pragma once

#include "../../common/basic_dev.h"
#include "../../common/dma_memory_allocator.h"
#include "basic_rx_source.h"
#include <vector>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include "log.h"

#define SLOT_SIZE_BYTES 32 // 32 bytes, 256 bits.

class FPGARxDecoder;
class FpgaReplayValidator;

class FPGADev : public BasicDev, public BasicRxSource {
public:
    FPGADev(std::string pci_addr);
    ~FPGADev() override;

    bool    initHardware() override;
    bool    setSymbolLocate(uint16_t que_idx, uint16_t stock_locate) ;
    bool    setPriceBase(uint16_t que_idx, uint64_t price_base) ;    
    bool    setRxRingBuffers(uint16_t rx_que_num, uint32_t slot_num, uint32_t slot_size) override;
    bool    setTxRingBuffers(uint16_t tx_que_num, uint32_t slot_num, uint32_t slot_size) override;
    void    setSync(bool enable);
    bool    validateRxAll();
    bool    isValid() const override { return m_is_valid; }



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
    
private:
    friend class FPGARxDecoder;
    friend class FpgaReplayValidator;
    // This should only be called by friend classes (adaptors)
    // this function works with __readProdPtr() or __readProdPtrAndTick()

    const uint8_t*  _pollOneRaw(uint16_t que_idx, uint64_t cons_ptr) const override;
    void            _writeConsPtr(uint16_t que_idx, uint64_t cons_ptr) override;
private:
    void        _initStatus(DevStatus* stats) override;
    uint32_t    _getRegAddr(uint16_t que_idx, uint32_t reg_offset) const;
    void        _readSymbolNum() ;
    void        _readProdPtr(uint16_t que_idx, uint64_t& prod_ptr) const;
    // get system time should be put before and after this function  to do sync.
    void        _readProdPtrAndTime(uint16_t que_idx, 
                                    uint64_t& prod_ptr, 
                                    uint64_t& fpga_tick,
                                    uint64_t& host_time_ns, 
                                    uint64_t& interval,
                                    bool get_time ) const;
    // use after processing bactch of records, to update the cons_ptr on FPGA side.
    uint64_t    _readDropCount(uint16_t que_idx) const;
    void        _readSyncEnable(bool& enabled);


private:
    std::vector<QueueConfig> m_rx_queues;
    bool m_sync_enable {false};
    bool m_hw_ready {false};
    bool m_is_valid {false};
};
