#pragma once

#include "../../common/basic_dev.h"
#include "../../common/dma_memory_allocator.h"
#include "../adapter/fpga_rx_types.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

class FPGADev : public BasicDev {
public:
    static constexpr uint16_t kRxQueueCount = 2;
    static constexpr uint32_t kRxRecordBytes = static_cast<uint32_t>(FpgaRawRxRecord::kRecordBytes);

    FPGADev(std::string pci_addr);
    ~FPGADev() override;

    bool initHardware() override;
    bool setRxRingBuffers(uint16_t num_rx_queues, uint32_t num_buf, uint32_t buf_size) override;
    bool setTxRingBuffers(uint16_t num_tx_queues, uint32_t num_buf, uint32_t buf_size) override;

    void write_reg64(uint32_t offset, uint64_t value);
    uint64_t read_reg64(uint32_t offset);
    void write_reg32(uint32_t offset, uint32_t value);
    uint32_t read_reg32(uint32_t offset);
    uint16_t rxQueueCount() const;
    uint64_t rxQueueProdPtr(uint16_t que_idx);
    uint64_t rxQueueDropCount(uint16_t que_idx);
    bool setSyncEnable(bool enable);
    bool readSyncEnable(bool& enabled);
    bool pollDecodedRecord(uint16_t que_idx, FPGAEventDesc& out);
    std::size_t pollDecodedRecords(uint16_t que_idx, FPGAEventDesc* out, std::size_t max_records);
    bool pollRawRecord(uint16_t que_idx, FpgaRawRxRecord& out);
    std::size_t pollRawRecords(uint16_t que_idx, FpgaRawRxRecord* out, std::size_t max_records);
    FPGAEventDesc decodeRawRecord(const FpgaRawRxRecord& record) const;

    bool test_register();
    bool trigger_interrupt();
    bool test_dma_write();

private:

    static constexpr uint32_t REG_RX_IOVA_OFFSET            = 0x00;
    static constexpr uint32_t REG_RESET                     = 0x00;
    static constexpr uint32_t REG_ID                        = 0x04;
    static constexpr uint32_t REG_SYNC_ENABLE               = 0x0C;
    static constexpr uint32_t REG_RX_QUE_BASE0              = 0x40;
    static constexpr uint32_t REG_RX_QUE_STRIDE             = 0x40;
    static constexpr uint32_t REG_RX_QUE_SLOT_NUM_OFFSET    = 0x08;
    static constexpr uint32_t REG_RX_QUE_ENABLE_OFFSET      = 0x10;
    static constexpr uint32_t REG_RX_QUE_CONS_PTR_OFFSET    = 0x18;
    static constexpr uint32_t REG_RX_QUE_PROD_OFFSET        = 0x20;
    static constexpr uint32_t REG_RX_QUE_DROP_OFFSET        = 0x28;
    static constexpr uint32_t REG_RX_QUE_STAT_OFFSET        = 0x30;
    static constexpr uint64_t RX_DMA_CFG_ID                 = 0x4d5f52585f434647ULL;

    struct QueueRuntime {
        std::string symbol_name;
        uint16_t stock_locate {0};
        uint32_t slot_num {0};
        uint32_t slot_size_bytes {kRxRecordBytes};
        uint64_t host_cons_ptr {0};
        DMAMemoryPair dma_memory {nullptr, 0, 0};
    };


    void _initStatus(DevStatus* stats) override;
    FPGAEventDesc _decodeSlotBytes(uint16_t que_idx, const uint8_t* slot_bytes) const;

    uint32_t _queueRegOffset(uint16_t que_idx, uint32_t reg_offset) const;
    bool _queueAvailable(uint16_t que_idx, uint64_t& prod_ptr) const;
    const QueueRuntime* _queueForIndex(uint16_t que_idx) const;

private:
    std::array<QueueRuntime, kRxQueueCount> m_rx_queues {};
    bool m_hw_ready {false};
};
