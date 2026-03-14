#include "fpga_dev.h"
#include "../common/dma_memory_allocator.h"
#include "../common/log.h"
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

FPGADev::FPGADev(std::string pci_addr) : 
BasicDev(pci_addr, 1) 
{}

FPGADev::~FPGADev() {}

bool FPGADev::initHardware() {
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

bool FPGADev::initializeInterrupt(const int interrupt_interval,
                                  const uint32_t timeout_ms) {
  return true;
}

bool FPGADev::wait4Link() { return true; }

bool FPGADev::_enableDMA() { return true; }

void FPGADev::_initStatus(DevStatus *stats) {
  if (stats == nullptr)
    return;
  memset(stats, 0, sizeof(DevStatus));
}

//-----------------------------------------------------------------------------
// Register Access Functions
//-----------------------------------------------------------------------------

void FPGADev::write_reg64(uint32_t offset, uint64_t value) {
  if (m_basic_para.p_bar_addr[0] == nullptr) {
    warn("BAR0 not mapped!");
    printf("BAR0 is, %p\n", m_basic_para.p_bar_addr[0]);
    return;
  }
  __asm__ volatile("" ::: "memory");
  volatile uint64_t *reg =
      (volatile uint64_t *)(m_basic_para.p_bar_addr[0] + offset);
  *reg = value;
}

uint64_t FPGADev::read_reg64(uint32_t offset) {
  if (m_basic_para.p_bar_addr[0] == nullptr) {
    error("BAR0 not mapped!");
    return 0;
  }
  __asm__ volatile("" ::: "memory");
  volatile uint64_t *reg =
      (volatile uint64_t *)(m_basic_para.p_bar_addr[0] + offset);
  return *reg;
}

void FPGADev::write_reg32(uint32_t offset, uint32_t value) {
  if (m_basic_para.p_bar_addr[0] == nullptr) {
    error("BAR0 not mapped!");
    return;
  }
  __asm__ volatile("" ::: "memory");
  volatile uint32_t *reg =
      (volatile uint32_t *)(m_basic_para.p_bar_addr[0] + offset);
  *reg = value;
}

uint32_t FPGADev::read_reg32(uint32_t offset) {
  if (m_basic_para.p_bar_addr[0] == nullptr) {
    error("BAR0 not mapped!");
    return 0;
  }
  __asm__ volatile("" ::: "memory");
  volatile uint32_t *reg =
      (volatile uint32_t *)(m_basic_para.p_bar_addr[0] + offset);
  return *reg;
}

//-----------------------------------------------------------------------------
// Test Functions
//-----------------------------------------------------------------------------

bool FPGADev::test_register() {
  info("--- Test 1: Scratch Register ---");

  const uint64_t test_values[] = {0x000001005E000001ULL, 0x00000000E9010203ULL,
                                  0x00000000000004D2ULL}; // mac, ip, port

  write_reg64(REG_MAC, test_values[0]);
  write_reg64(REG_IP, test_values[1]);
  write_reg64(REG_PORT, test_values[2]);
  write_reg64(REG_ETH_FIRE, 0x1); // Trigger "fire" to capture values into scratch reg
  return true;
}

void FPGADev::trigger_interrupt() {
  info("--- Test 4: Trigger MSI Interrupt ---");

  // // Read status before
  // uint64_t status_before = read_reg64(REG_STATUS);
  // uint16_t count_before = (status_before >> 16) & 0xFFFF;

  // // Trigger interrupt by writing to INT_CTRL
  // write_reg32(REG_INT_CTRL, 0x1);

  // // Small delay for interrupt to process
  // usleep(1000);

  // // Read status after
  // uint64_t status_after = read_reg64(REG_STATUS);
  // uint16_t count_after = (status_after >> 16) & 0xFFFF;

  // info("  Interrupt count before: %u", count_before);
  // info("  Interrupt count after:  %u", count_after);

  // if (count_after > count_before) {
  //   info("  [PASS] Interrupt counter incremented!");
  // } else {
  //   warn("  [WARN] Interrupt counter did not increment (MSI may not be "
  //        "enabled)");
  // }
}

//-----------------------------------------------------------------------------
// DMA Test Functions
//-----------------------------------------------------------------------------

bool FPGADev::test_dma_write() {
  info("--- Test: DMA Write from FPGA to Host ---");

  // Get DMA memory allocator
  DMAMemoryAllocator &allocator = DMAMemoryAllocator::getInstance();

  // =========================================================================
  // Test 1: Small DMA (4 DWords = 16 bytes) - fits in ONE beat
  // =========================================================================
  info("Test 1: Small DMA transfer (4 DWords, 1 beat)");

  // Allocate DMA buffer for small transfer
  DMAMemoryPair small_buf = allocator.allocDMAMemory(4096, m_fds.container_fd);
  if (small_buf.virt == nullptr) {
    error("Failed to allocate small DMA buffer");
    return false;
  }

  // Clear buffer and set sentinel values
  volatile uint64_t *small_data = (volatile uint64_t *)small_buf.virt;
  small_data[0] = 0xFFFFFFFFFFFFFFFFULL; // Will be overwritten
  small_data[1] = 0xFFFFFFFFFFFFFFFFULL; // Will be overwritten

  // Expected data (must match user_logic.v TEST_SMALL_DATA)
  const uint64_t expected_small[2] = {
      0xDEADBEEFCAFEBABEULL, // DW[1:0]
      0x123456789ABCDEF0ULL  // DW[3:2]
  };

  // Program DMA target address (64-bit write)
  write_reg64(REG_DMA_ADDR, small_buf.iova);

  info("  DMA target IOVA: 0x%016lX", small_buf.iova);

  // Trigger DMA (write 0x01 to DMA_CTRL for small transfer)
  write_reg32(REG_DMA_CTRL, 0x01);

  // Wait for DMA completion (poll DMA_STATUS)
  int timeout = 1000;
  while (timeout-- > 0) {
    uint32_t status = read_reg32(REG_DMA_STATUS);
    if (status & 0x2) { // Done bit
      break;
    }
    usleep(100);
  }

  if (timeout <= 0) {
    warn("  Small DMA timeout!");
    return false;
  }

  // Memory barrier before reading
  __asm__ volatile("mfence" ::: "memory");
  // Verify received data
  bool small_pass = true;
  for (int i = 0; i < 2; i++) {
    if (small_data[i] != expected_small[i]) {
      warn("  Small DMA mismatch at QW[%d]: got 0x%016lX, expected 0x%016lX", i,
           small_data[i], expected_small[i]);
      small_pass = false;
    }
  }

  if (small_pass) {
    info("  Small DMA [PASS] - Data verified:");
    info("    QW[0]: 0x%016lX", small_data[0]);
    info("    QW[1]: 0x%016lX", small_data[1]);
  }

  // =========================================================================
  // Test 2: Large DMA (12 DWords = 48 bytes) - requires THREE beats
  // Beat 1: descriptor(4DW) + data[3:0]
  // Beat 2: data[7:4] (saved) + data[11:8] (new lower half only used)
  // Beat 3: data[11:8] upper half (one_more_cycle)
  // =========================================================================
  info("Test 2: Large DMA transfer (12 DWords, 3 beats)");

  // Allocate DMA buffer for large transfer
  DMAMemoryPair large_buf = allocator.allocDMAMemory(4096, m_fds.container_fd);
  if (large_buf.virt == nullptr) {
    error("Failed to allocate large DMA buffer");
    return false;
  }

  // Clear buffer
  volatile uint64_t *large_data = (volatile uint64_t *)large_buf.virt;
  for (int i = 0; i < 6; i++) {
    large_data[i] = 0xFFFFFFFFFFFFFFFFULL;
  }

  // Expected data (must match user_logic.v TEST_LARGE_DATA)
  // 12 DWords = 6 QWords
  const uint64_t expected_large[6] = {
      0xAAAAAAAABBBBBBBBULL, // DW[1:0]
      0xCCCCCCCCDDDDDDDDULL, // DW[3:2]
      0xEEEEEEEEFFFFFFFFULL, // DW[5:4]
      0x1111111122222222ULL, // DW[7:6]
      0x3333333344444444ULL, // DW[9:8]
      0x5555555566666666ULL  // DW[11:10]
  };

  // Program DMA target address (64-bit write)
  write_reg64(REG_DMA_ADDR, large_buf.iova);

  info("  DMA target IOVA: 0x%016lX", large_buf.iova);

  // Trigger DMA (write 0x02 to DMA_CTRL for large transfer)
  write_reg32(REG_DMA_CTRL, 0x02);

  // Wait for DMA completion
  timeout = 1000;
  while (timeout-- > 0) {
    uint32_t status = read_reg32(REG_DMA_STATUS);
    if (status & 0x2) { // Done bit
      break;
    }
    usleep(100);
  }

  if (timeout <= 0) {
    warn("  Large DMA timeout!");
    return false;
  }

  // Memory barrier before reading
  __asm__ volatile("mfence" ::: "memory");

  // Verify received data
  bool large_pass = true;
  for (int i = 0; i < 6; i++) {
    if (large_data[i] != expected_large[i]) {
      warn("  Large DMA mismatch at QW[%d]: got 0x%016lX, expected 0x%016lX", i,
           large_data[i], expected_large[i]);
      large_pass = false;
    }
  }

  if (large_pass) {
    info("  Large DMA [PASS] - Data verified:");
    for (int i = 0; i < 6; i++) {
      info("    QW[%d]: 0x%016lX", i, large_data[i]);
    }
  }

  // =========================================================================
  // Summary
  // =========================================================================
  info("--- DMA Test Summary ---");
  info("  Small DMA (1 beat):  %s", small_pass ? "PASS" : "FAIL");
  info("  Large DMA (3 beats): %s", large_pass ? "PASS" : "FAIL");

  return small_pass && large_pass;
}

//-----------------------------------------------------------------------------
// Round-Trip DMA Test: Host -> FPGA (RC) -> FPGA -> Host (RQ)
//-----------------------------------------------------------------------------
bool FPGADev::test_dma_roundtrip() {
  info("--- Test: DMA Round-Trip (Host -> FPGA -> Host) ---");

  DMAMemoryAllocator &allocator = DMAMemoryAllocator::getInstance();

  struct RoundTripCase {
    const char *name;
    uint32_t ctrl;
    int qwords;
    uint64_t seed;
  };

  const RoundTripCase cases[] = {
      {"Small round-trip (4 DWords)", 0x01, 2, 0x1122334455667788ULL},
      {"Large round-trip (12 DWords)", 0x02, 6, 0x0001000200030004ULL},
      {"Long round-trip (20 DWords)", 0x03, 10, 0xABCDEF0000000000ULL},
  };

  bool results[3] = {false, false, false};

  auto fill_pattern = [](volatile uint64_t *buf, int qwords, uint64_t seed) {
    for (int i = 0; i < qwords; ++i) {
      buf[i] = seed + 0x0101010101010101ULL * static_cast<uint64_t>(i);
    }
  };

  for (size_t case_idx = 0; case_idx < 3; ++case_idx) {
    const RoundTripCase &test_case = cases[case_idx];
    info("Test %zu: %s", case_idx + 1, test_case.name);

    DMAMemoryPair src = allocator.allocDMAMemory(4096, m_fds.container_fd);
    if (src.virt == nullptr) {
      error("Failed to allocate source buffer for %s", test_case.name);
      return false;
    }

    DMAMemoryPair dst = allocator.allocDMAMemory(4096, m_fds.container_fd);
    if (dst.virt == nullptr) {
      error("Failed to allocate destination buffer for %s", test_case.name);
      return false;
    }

    volatile uint64_t *src_data = (volatile uint64_t *)src.virt;
    volatile uint64_t *dst_data = (volatile uint64_t *)dst.virt;
    fill_pattern(src_data, test_case.qwords, test_case.seed);
    for (int i = 0; i < test_case.qwords; ++i) {
      dst_data[i] = 0xFFFFFFFFFFFFFFFFULL;
    }

    __asm__ volatile("mfence" ::: "memory");

    info("  Source IOVA:      0x%016lX", src.iova);
    info("  Destination IOVA: 0x%016lX", dst.iova);

    write_reg64(REG_RT_SRC_ADDR, src.iova);
    write_reg64(REG_RT_DST_ADDR, dst.iova);
    write_reg32(REG_RT_CTRL, test_case.ctrl);

    int timeout = 2000;
    bool done = false;
    while (timeout-- > 0) {
      uint32_t status = read_reg32(REG_RT_STATUS);
      if (status & 0x4) {
        error("  %s reported FPGA-side error status 0x%08X", test_case.name,
              status);
        break;
      }
      if (status & 0x2) {
        done = true;
        break;
      }
      usleep(100);
    }

    write_reg32(REG_RT_CTRL, 0x00);

    if (!done) {
      if (timeout <= 0) {
        warn("  %s timeout!", test_case.name);
      }
      continue;
    }

    __asm__ volatile("mfence" ::: "memory");

    bool pass = true;
    for (int i = 0; i < test_case.qwords; ++i) {
      if (dst_data[i] != src_data[i]) {
        warn("  %s mismatch at QW[%d]: got 0x%016lX, expected 0x%016lX",
             test_case.name, i, dst_data[i], src_data[i]);
        pass = false;
      }
    }

    if (pass) {
      info("  %s [PASS] - Data verified:", test_case.name);
      for (int i = 0; i < test_case.qwords; ++i) {
        info("    Dst[%d]: 0x%016lX", i, dst_data[i]);
      }
    }

    results[case_idx] = pass;
  }

  info("--- Round-Trip Test Summary ---");
  info("  Small RT (4 DW):   %s", results[0] ? "PASS" : "FAIL");
  info("  Large RT (12 DW):  %s", results[1] ? "PASS" : "FAIL");
  info("  Long RT (20 DW):   %s", results[2] ? "PASS" : "FAIL");

  return results[0] && results[1] && results[2];
}
