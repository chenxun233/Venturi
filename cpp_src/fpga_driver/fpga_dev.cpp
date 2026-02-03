#include "fpga_dev.h"
#include "../common/dma_memory_allocator.h"
#include "../common/log.h"
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

FPGADev::FPGADev(std::string pci_addr) : BasicDev(pci_addr, 1) {}

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
    error("BAR0 not mapped!");
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

bool FPGADev::test_scratch_register() {
  info("--- Test 3: Scratch Register ---");

  const uint64_t test_values[] = {0x1111111111111111ULL, 0xFFFFFFFFFFFFFFFFULL,
                                  0xAAAAAAAAAAAAAAAAULL, 0x5555555555555555ULL,
                                  0x123456789ABCDEF0ULL};

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

void FPGADev::trigger_interrupt() {
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
    warn("  [WARN] Interrupt counter did not increment (MSI may not be "
         "enabled)");
  }
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

  // =========================================================================
  // Test 1: Small Round-Trip (4 DWords = 16 bytes)
  // =========================================================================
  info("Test 1: Small round-trip (4 DWords)");

  // Allocate source buffer and fill with test data
  DMAMemoryPair src_small = allocator.allocDMAMemory(4096, m_fds.container_fd);
  if (src_small.virt == nullptr) {
    error("Failed to allocate small source buffer");
    return false;
  }

  // Allocate destination buffer and clear it
  DMAMemoryPair dst_small = allocator.allocDMAMemory(4096, m_fds.container_fd);
  if (dst_small.virt == nullptr) {
    error("Failed to allocate small destination buffer");
    return false;
  }

  // Fill source with test pattern
  volatile uint64_t *src_small_data = (volatile uint64_t *)src_small.virt;
  src_small_data[0] = 0x1122334455667788ULL;
  src_small_data[1] = 0xAABBCCDDEEFF0011ULL;

  // Clear destination
  volatile uint64_t *dst_small_data = (volatile uint64_t *)dst_small.virt;
  dst_small_data[0] = 0xFFFFFFFFFFFFFFFFULL;
  dst_small_data[1] = 0xFFFFFFFFFFFFFFFFULL;

  // Memory barrier after writing source data
  __asm__ volatile("" ::: "memory");

  info("  Source IOVA:      0x%016lX", src_small.iova);
  info("  Destination IOVA: 0x%016lX", dst_small.iova);
  info("  Source data[0]:   0x%016lX", src_small_data[0]);
  info("  Source data[1]:   0x%016lX", src_small_data[1]);

  // Program round-trip addresses
  write_reg64(REG_RT_SRC_ADDR, src_small.iova);
  write_reg64(REG_RT_DST_ADDR, dst_small.iova);

  // Trigger small round-trip (bit 0 = small)
  write_reg32(REG_RT_CTRL, 0x01);

  // Wait for completion
  int timeout = 1000;
  while (timeout-- > 0) {
    uint32_t status = read_reg32(REG_RT_STATUS);
    if (status & 0x2) {   // Done bit
      if (status & 0x4) { // Error bit
        info("  Small round-trip error!");
        return false;
      }
      break;
    }
    usleep(100);
  }

  if (timeout <= 0) {
    warn("  Small round-trip timeout!");
    return false;
  }

  // Memory barrier before reading destination
  __asm__ volatile("mfence" ::: "memory");

  // Verify data
  bool small_pass = true;
  for (int i = 0; i < 2; i++) {
    if (dst_small_data[i] != src_small_data[i]) {
      warn("  Small RT mismatch at QW[%d]: got 0x%016lX, expected 0x%016lX", i,
           dst_small_data[i], src_small_data[i]);
      small_pass = false;
    }
  }

  if (small_pass) {
    info("  Small round-trip [PASS] - Data verified:");
    info("    Dst[0]: 0x%016lX", dst_small_data[0]);
    info("    Dst[1]: 0x%016lX", dst_small_data[1]);
  }

  write_reg32(REG_RT_CTRL, 0x00); // reset
  // =========================================================================
  // Test 2: Large Round-Trip (12 DWords = 48 bytes)
  // =========================================================================
  info("Test 2: Large round-trip (12 DWords)");

  // Allocate source buffer
  DMAMemoryPair src_large = allocator.allocDMAMemory(4096, m_fds.container_fd);
  if (src_large.virt == nullptr) {
    error("Failed to allocate large source buffer");
    return false;
  }

  // Allocate destination buffer
  DMAMemoryPair dst_large = allocator.allocDMAMemory(4096, m_fds.container_fd);
  if (dst_large.virt == nullptr) {
    error("Failed to allocate large destination buffer");
    return false;
  }

  // Fill source with test pattern (12 DW = 6 QW)
  volatile uint64_t *src_large_data = (volatile uint64_t *)src_large.virt;
  src_large_data[0] = 0x0001000200030004ULL;
  src_large_data[1] = 0x0005000600070008ULL;
  src_large_data[2] = 0x0009000A000B000CULL;
  src_large_data[3] = 0x000D000E000F0010ULL;
  src_large_data[4] = 0x0011001200130014ULL;
  src_large_data[5] = 0x0015001600170018ULL;

  // Clear destination
  volatile uint64_t *dst_large_data = (volatile uint64_t *)dst_large.virt;
  for (int i = 0; i < 6; i++) {
    dst_large_data[i] = 0xFFFFFFFFFFFFFFFFULL;
  }

  // Memory barrier
  __asm__ volatile("mfence" ::: "memory");

  info("  Source IOVA:      0x%016lX", src_large.iova);
  info("  Destination IOVA: 0x%016lX", dst_large.iova);

  // Program round-trip addresses
  write_reg64(REG_RT_SRC_ADDR, src_large.iova);
  write_reg64(REG_RT_DST_ADDR, dst_large.iova);

  // Trigger large round-trip (bit 1 = large)
  write_reg32(REG_RT_CTRL, 0x02);

  // Wait for completion
  timeout = 1000;
  while (timeout-- > 0) {
    uint32_t status = read_reg32(REG_RT_STATUS);
    if (status & 0x2) {   // Done bit
      if (status & 0x4) { // Error bit
        error("  Large round-trip error!");
        return false;
      }
      break;
    }
    usleep(100);
  }

  if (timeout <= 0) {
    warn("  Large round-trip timeout!");
    return false;
  }

  // Memory barrier
  __asm__ volatile("mfence" ::: "memory");

  // Verify data
  bool large_pass = true;
  for (int i = 0; i < 6; i++) {
    if (dst_large_data[i] != src_large_data[i]) {
      warn("  Large RT mismatch at QW[%d]: got 0x%016lX, expected 0x%016lX", i,
           dst_large_data[i], src_large_data[i]);
      large_pass = false;
    }
  }

  if (large_pass) {
    info("  Large round-trip [PASS] - Data verified:");
    for (int i = 0; i < 6; i++) {
      info("    Dst[%d]: 0x%016lX", i, dst_large_data[i]);
    }
  }

  // =========================================================================
  // Summary
  // =========================================================================
  info("--- Round-Trip Test Summary ---");
  info("  Small RT (4 DW):  %s", small_pass ? "PASS" : "FAIL");
  info("  Large RT (12 DW): %s", large_pass ? "PASS" : "FAIL");

  return large_pass;
}
