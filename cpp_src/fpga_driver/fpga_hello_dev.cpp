#include "fpga_hello_dev.h"
#include "../common/dma_memory_allocator.h"
#include "../common/log.h"
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>

FPGAHelloDev::FPGAHelloDev(std::string pci_addr) : BasicDev(pci_addr, 1) {
}

FPGAHelloDev::~FPGAHelloDev() {
}

bool FPGAHelloDev::initHardware() {
    info("Initializing FPGA hardware...");

    // Get VFIO file descriptor and map BAR0
    if (!_getFD()) {
        error("Failed to get VFIO device file descriptor");
        return false;
    }

    if (!_getBARAddr(0)) {
        error("Failed to map BAR addresses");
        return false;
    }

    // Verify we have BAR0 mapped
    if (m_basic_para.p_bar_addr[0] == nullptr) {
        error("BAR0 not mapped!");
        return false;
    }

    return true;
}

bool FPGAHelloDev::initializeInterrupt(const int interrupt_interval, const uint32_t timeout_ms) {
    return true;
}

bool FPGAHelloDev::wait4Link() {
    return true;
}

bool FPGAHelloDev::_enableDMA() {
    return true;
}

void FPGAHelloDev::_initStatus(DevStatus* stats) {
    if (stats == nullptr) return;
    memset(stats, 0, sizeof(DevStatus));
}

//-----------------------------------------------------------------------------
// Register Access Functions
//-----------------------------------------------------------------------------

void FPGAHelloDev::write_reg64(uint32_t offset, uint64_t value) {
    if (m_basic_para.p_bar_addr[0] == nullptr) {
        error("BAR0 not mapped!");
        return;
    }
    __asm__ volatile ("" ::: "memory")	;
    volatile uint64_t* reg = (volatile uint64_t*)(m_basic_para.p_bar_addr[0] + offset);
    *reg = value;
}

uint64_t FPGAHelloDev::read_reg64(uint32_t offset) {
    if (m_basic_para.p_bar_addr[0] == nullptr) {
        error("BAR0 not mapped!");
        return 0;
    }
     __asm__ volatile ("" ::: "memory")	;
    volatile uint64_t* reg = (volatile uint64_t*)(m_basic_para.p_bar_addr[0] + offset);
    return *reg;
}

void FPGAHelloDev::write_reg32(uint32_t offset, uint32_t value) {
    if (m_basic_para.p_bar_addr[0] == nullptr) {
        error("BAR0 not mapped!");
        return;
    }
    __asm__ volatile ("" ::: "memory")	;
    volatile uint32_t* reg = (volatile uint32_t*)(m_basic_para.p_bar_addr[0] + offset);
    *reg = value;
}

uint32_t FPGAHelloDev::read_reg32(uint32_t offset) {
    if (m_basic_para.p_bar_addr[0] == nullptr) {
        error("BAR0 not mapped!");
        return 0;
    }
    __asm__ volatile ("" ::: "memory")	;
    volatile uint32_t* reg = (volatile uint32_t*)(m_basic_para.p_bar_addr[0] + offset);
    return *reg;
}

//-----------------------------------------------------------------------------
// Test Functions
//-----------------------------------------------------------------------------



bool FPGAHelloDev::test_scratch_register() {
    info("--- Test 3: Scratch Register ---");

    const uint64_t test_values[] = {
        0x1111111111111111ULL,
        0xFFFFFFFFFFFFFFFFULL,
        0xAAAAAAAAAAAAAAAAULL,
        0x5555555555555555ULL,
        0x123456789ABCDEF0ULL
    };

    int passed = 0;
    int total = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < total; i++) {
        uint64_t write_val = test_values[i];
        write_reg64(REG_SCRATCH, write_val);
        uint64_t read_val = read_reg64(REG_SCRATCH);

        if (read_val == write_val) {
            info("  Write: 0x%016lX, Read: 0x%016lX [PASS]", write_val, read_val);
            passed++;
        } else {
            info("  Write: 0x%016lX, Read: 0x%016lX [FAIL]", write_val, read_val);
        }
    }

    info("  Scratch test: %d/%d passed", passed, total);
    return (passed == total);
}

void FPGAHelloDev::trigger_interrupt() {
    info("--- Test 4: Trigger MSI Interrupt ---");

    // Read status before
    uint64_t status_before = read_reg64(REG_STATUS);
    uint16_t count_before = (status_before >> 16) & 0xFFFF;

    // Trigger interrupt by writing to INT_CTRL
    write_reg32(REG_INT_CTRL, 0x1);

    // Small delay for interrupt to process
    usleep(1000);

    // Read status after
    uint64_t status_after = read_reg64(REG_STATUS);
    uint16_t count_after = (status_after >> 16) & 0xFFFF;

    info("  Interrupt count before: %u", count_before);
    info("  Interrupt count after:  %u", count_after);

    if (count_after > count_before) {
        info("  [PASS] Interrupt counter incremented!");
    } else {
        warn("  [WARN] Interrupt counter did not increment (MSI may not be enabled)");
    }
}

//-----------------------------------------------------------------------------
// DMA Test Functions
//-----------------------------------------------------------------------------

bool FPGAHelloDev::test_dma_write() {
    info("--- Test 5: DMA Write with Status Write-Back ---");

    // Allocate DMA buffer: 64 bytes data + 8 bytes completion status
    constexpr size_t DMA_DATA_SIZE = 64;
    constexpr size_t STATUS_OFFSET = 64;    // Completion status at offset 0x40
    constexpr size_t DMA_BUF_SIZE = 72;     // Total: data + status
    constexpr uint32_t DMA_DONE_MAGIC = 0x444F4E45;  // "DONE" in ASCII

    DMAMemoryAllocator& allocator = DMAMemoryAllocator::getInstance();
    DMAMemoryPair dma_mem = allocator.allocDMAMemory(DMA_BUF_SIZE, m_fds.container_fd);

    if (dma_mem.virt == nullptr) {
        error("  Failed to allocate DMA memory!");
        return false;
    }

    info("  DMA buffer allocated:");
    info("    Virtual addr:  %p", dma_mem.virt);
    info("    IOVA:          0x%016lX", dma_mem.iova);
    info("    Size:          %zu bytes", dma_mem.size);

    // Clear entire buffer (data + status)
    memset(dma_mem.virt, 0, DMA_BUF_SIZE);

    // Pointer to completion status in local RAM
    volatile uint32_t* status_ptr = (volatile uint32_t*)((uint8_t*)dma_mem.virt + STATUS_OFFSET);

    // Write target address to FPGA
    uint64_t iova = dma_mem.iova;
    write_reg32(REG_DMA_ADDR_LO, (uint32_t)(iova & 0xFFFFFFFF));
    write_reg32(REG_DMA_ADDR_HI, (uint32_t)(iova >> 32));

    // Trigger DMA write
    write_reg32(REG_DMA_CTRL, 0x1);

    // Poll LOCAL RAM for completion (fast ~50-100ns per read)
    // Instead of polling PCIe MMIO register (slow ~500ns-2us per read)
    int timeout_us = 1000000;  // 1 second
    int elapsed_us = 0;
    int poll_count = 0;

    while (elapsed_us < timeout_us) {
        // Read completion status from local RAM (very fast!)
        uint32_t magic = *status_ptr;
        poll_count++;

        if (magic == DMA_DONE_MAGIC) {
            uint32_t seq = *(status_ptr + 1);
            info("  DMA completed! (polled %d times)", poll_count);
            info("    Status magic: 0x%08X", magic);
            info("    Sequence:     %u", seq);
            break;
        }

        // Tight polling loop - no sleep for lowest latency
        // For production, consider adding a small delay or using pause instruction
        __asm__ volatile("pause" ::: "memory");
        elapsed_us++;  // Approximate, actual time depends on loop iterations
    }

    if (elapsed_us >= timeout_us) {
        error("  DMA timeout! Status word: 0x%08X", *status_ptr);
        return false;
    }

    // // Verify data written by FPGA
    // info("  Verifying DMA data...");
    // volatile uint64_t* buf = (volatile uint64_t*)dma_mem.virt;

    // // Expected pattern from Verilog:
    // // DW0-1: 0xDEADBEEF_CAFEBABE
    // // DW2-3: 0x12345678_9ABCDEF0
    // // DW4-5: 0xFEDCBA98_76543210
    // // DW6-7: 0xAAAAAAAA_55555555
    // // Then repeats...

    // const uint64_t expected[] = {
    //     DMA_PATTERN_0,  // bytes 0-7
    //     DMA_PATTERN_1,  // bytes 8-15
    //     DMA_PATTERN_2,  // bytes 16-23
    //     DMA_PATTERN_3,  // bytes 24-31
    //     DMA_PATTERN_0,  // bytes 32-39 (repeat)
    //     DMA_PATTERN_1,  // bytes 40-47
    //     DMA_PATTERN_2,  // bytes 48-55
    //     DMA_PATTERN_3   // bytes 56-63
    // };

    // int passed = 0;
    // int total = sizeof(expected) / sizeof(expected[0]);

    // for (int i = 0; i < total; i++) {
    //     uint64_t actual = buf[i];
    //     if (actual == expected[i]) {
    //         info("    [%d] 0x%016lX == 0x%016lX [PASS]", i, actual, expected[i]);
    //         passed++;
    //     } else {
    //         error("    [%d] 0x%016lX != 0x%016lX [FAIL]", i, actual, expected[i]);
    //     }
    // }

    // info("  DMA verification: %d/%d passed", passed, total);

    // if (passed == total) {
    //     info("  [PASS] DMA write test passed!");
    //     return true;
    // } else {
    //     error("  [FAIL] DMA write test failed!");
    //     return false;
    // }
    return true;
}
