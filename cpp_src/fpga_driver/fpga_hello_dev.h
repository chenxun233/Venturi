#pragma once
#include "../common/basic_dev.h"
#include <cstdint>

/**
 * Simple FPGA PCIe Hello World Device
 *
 * This is a minimal implementation of BasicDev for the FPGA hello world example.
 * It provides direct BAR0 register access without the full NIC complexity.
 *
 * Register Map (BAR0):
 *   0x00: Scratch Register (R/W)    - 64-bit scratch pad
 *   0x08: ID Register (RO)          - Returns 0xDEADBEEF_CAFEBABE
 *   0x10: Interrupt Control (W)     - Write to trigger MSI
 *   0x18: Status Register (RO)      - Bit 0: Link Up, [31:16]: Int count
 *   0x20: DMA Target Addr Low (W)   - Lower 32 bits of host memory IOVA
 *   0x28: DMA Target Addr High (W)  - Upper 32 bits of host memory IOVA
 *   0x30: DMA Control (W)           - Write 1 to trigger DMA write
 *   0x38: DMA Status (RO)           - Bit 0: Busy, Bit 1: Done
 */
class FPGAHelloDev : public BasicDev {
public:
    FPGAHelloDev(std::string pci_addr);
    ~FPGAHelloDev() override;

    // BasicDev interface - minimal implementations
    bool initHardware() override;
    bool initializeInterrupt(const int interrupt_interval, const uint32_t timeout_ms) override;
    bool enableDevQueues() override { return true; }  // No queues in hello world
    bool enableDevInterrupt() override { return true; }
    bool wait4Link() override;
    bool setRxRingBuffers(uint16_t num_rx_queues, uint32_t num_buf, uint32_t buf_size) override { return true; }
    bool setTxRingBuffers(uint16_t num_tx_queues, uint32_t num_buf, uint32_t buf_size) override { return true; }
    bool setPromisc(bool enable) override { return true; }
    bool sendOnQueue(uint8_t* p_data, size_t size, uint16_t queue_id) override { return false; }

    // FPGA-specific register access
    void write_reg64(uint32_t offset, uint64_t value);
    uint64_t read_reg64(uint32_t offset);
    void write_reg32(uint32_t offset, uint32_t value);
    uint32_t read_reg32(uint32_t offset);
    // Test functions
    bool test_scratch_register();
    void trigger_interrupt();

    // DMA functions
    bool test_dma_write();

    // Register offsets
    static constexpr uint32_t REG_SCRATCH      = 0x00;  // 
    static constexpr uint32_t REG_ID           = 0x04;  // 
    static constexpr uint32_t REG_INT_CTRL     = 0x08;  // 
    static constexpr uint32_t REG_STATUS       = 0x0C;  // 
    static constexpr uint32_t REG_DMA_ADDR_LO  = 0x10;  // 
    static constexpr uint32_t REG_DMA_ADDR_HI  = 0x14;  // 
    static constexpr uint32_t REG_DMA_CTRL     = 0x18;  // 
    static constexpr uint32_t REG_DMA_STATUS   = 0x1C;  // 

    static constexpr uint64_t EXPECTED_ID = 0xDEADBEEFCAFEBABEULL;

    // DMA test patterns (must match Verilog)
    static constexpr uint64_t DMA_PATTERN_0 = 0xDEADBEEFCAFEBABEULL;
    static constexpr uint64_t DMA_PATTERN_1 = 0x123456789ABCDEF0ULL;
    static constexpr uint64_t DMA_PATTERN_2 = 0xFEDCBA9876543210ULL;
    static constexpr uint64_t DMA_PATTERN_3 = 0xAAAAAAAA55555555ULL;

private:
    // _getFD() and _getBARAddr() are now inherited from BasicDev
    bool _enableDMA() override;
    void _initStatus(DevStatus* stats) override;
};
