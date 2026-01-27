# DMA Write: FPGA Writes to Host Memory

How the FPGA initiates writes to host memory using PCIe DMA.

---

## Overview

```
┌─────────────────┐         PCIe            ┌─────────────────┐
│     FPGA        │ ══════════════════════► │   Host Memory   │
│                 │     DMA Write           │                 │
│  RQ Interface   │     (MWr64 TLP)         │   DMA Buffer    │
└─────────────────┘                         └─────────────────┘
```

**Key Difference from Register Access:**
- Register access: Host → FPGA (CQ/CC channels)
- DMA: FPGA → Host memory (RQ/RC channels)

---

## Data Flow

### Step 1: Host Allocates DMA Buffer

```cpp
// Host allocates buffer with IOMMU mapping
DMAMemoryPair dma_mem = allocator.allocDMAMemory(64, container_fd);

// dma_mem.virt = CPU-accessible virtual address
// dma_mem.iova = FPGA-accessible IO virtual address (via IOMMU)
```

### Step 2: Host Tells FPGA Where to Write

```cpp
// Write IOVA to FPGA registers
write_reg32(REG_DMA_ADDR_LO, iova & 0xFFFFFFFF);  // 0x20
write_reg32(REG_DMA_ADDR_HI, iova >> 32);          // 0x28
```

### Step 3: Host Triggers DMA

```cpp
write_reg32(REG_DMA_CTRL, 0x1);  // 0x30 - Start DMA
```

### Step 4: FPGA Builds RQ Descriptor

```verilog
// RQ (Requester Request) - FPGA initiates memory write
s_axis_rq_tdata[1:0]     <= 2'b00;              // Address Type
s_axis_rq_tdata[63:2]    <= dma_target_addr[63:2]; // Host IOVA
s_axis_rq_tdata[74:64]   <= 16;                 // DWORD Count = 16 (64 bytes)
s_axis_rq_tdata[78:75]   <= 4'b0001;            // MWr64 (Memory Write 64-bit)
s_axis_rq_tdata[95:80]   <= requester_id;       // Our Bus:Dev:Func
s_axis_rq_tdata[103:96]  <= tag;                // Transaction ID
s_axis_rq_tdata[255:128] <= write_data;         // Payload (first 4 DWORDs)
s_axis_rq_tvalid <= 1'b1;
```

### Step 5: PCIe Core Generates TLP

```
Memory Write TLP (Posted - No completion needed):
  Header DW0: 0x60000010  // Fmt=011, Type=00000, Length=16
  Header DW1: 0x0600FF00  // Requester ID, Tag, BE
  Header DW2-3: IOVA address
  Data DW0-15: Payload (64 bytes)
```

### Step 6: TLP Travels to Host

```
FPGA PCIe Core → PCIe Lanes → Root Complex → IOMMU → Host Memory
```

### Step 7: Host Reads Buffer

```cpp
volatile uint64_t* buf = (volatile uint64_t*)dma_mem.virt;
uint64_t value = buf[0];  // Read data written by FPGA
```

---

## Register Map (DMA Extension)

| Offset | Name | R/W | Description |
|--------|------|-----|-------------|
| 0x20 | DMA_ADDR_LO | W | Lower 32 bits of host IOVA |
| 0x28 | DMA_ADDR_HI | W | Upper 32 bits of host IOVA |
| 0x30 | DMA_CTRL | W | Write 1 to trigger DMA |
| 0x38 | DMA_STATUS | R | Bit 0: Busy, Bit 1: Done |

---

## RQ Descriptor Format (256 bits)

| Bits | Field | Value for DMA Write |
|------|-------|---------------------|
| [1:0] | Address Type | 00 (untranslated) |
| [63:2] | Address | Host IOVA (from registers) |
| [74:64] | DWORD Count | 16 (64 bytes) |
| [78:75] | Request Type | 0001 (MWr64) |
| [95:80] | Requester ID | Our Bus:Dev:Func |
| [103:96] | Tag | Transaction ID |
| [255:128] | Write Data | First 4 DWORDs of payload |

**Note:** For writes > 128 bits (4 DWORDs), additional beats carry remaining data.

---

## Multi-Beat Transfer

For 64-byte write (16 DWORDs):

```
Beat 1: [Header 128b] + [Data DW0-3 128b] = 256b, tkeep=0xFF, tlast=0
Beat 2: [Data DW4-11 256b]                = 256b, tkeep=0xFF, tlast=0
Beat 3: [Data DW12-15 128b] + [Padding]   = 256b, tkeep=0x0F, tlast=1
```

---

## IOMMU: Why IOVA, Not Physical Address?

```
Without IOMMU (Dangerous!):
  FPGA can write to ANY physical address
  → Security vulnerability (DMA attack)

With IOMMU:
  FPGA writes to IOVA (IO Virtual Address)
  IOMMU translates IOVA → Physical address
  Only allowed mappings work → Safe
```

```cpp
// VFIO sets up IOMMU mapping
struct vfio_iommu_type1_dma_map dma_map = {
    .vaddr = (uint64_t)virtual_address,  // CPU's view
    .iova  = iova,                        // FPGA's view
    .size  = size,
    .flags = VFIO_DMA_MAP_FLAG_READ | VFIO_DMA_MAP_FLAG_WRITE
};
ioctl(container_fd, VFIO_IOMMU_MAP_DMA, &dma_map);
```

---

## Posted vs Non-Posted

| Type | Example | Completion? | Latency |
|------|---------|-------------|---------|
| Posted | Memory Write | No | ~100ns |
| Non-Posted | Memory Read | Yes (RC) | ~500ns |

**DMA Write is Posted:** FPGA sends data and moves on. No acknowledgment from host.

---

## Complete Signal Flow

```
┌─────────────────────────────────────────────────────────────┐
│ Host CPU                                                     │
│   1. Allocate DMA buffer (mmap + VFIO_IOMMU_MAP_DMA)        │
│   2. Write IOVA to FPGA registers (via BAR0)                │
│   3. Write 1 to DMA_CTRL register                           │
│   4. (Wait or do other work)                                │
│   5. Read DMA buffer to get data                            │
└─────────────────────────────────────────────────────────────┘
        │ (2,3) CQ/CC         │ (5) CPU Read
        ▼                     ▼
┌─────────────────────────────────────────────────────────────┐
│ Host Memory                                                  │
│   DMA Buffer @ IOVA 0x10000                                 │
│   ┌────────────────────────────────────────┐                │
│   │ 0xDEADBEEF_CAFEBABE  (written by FPGA) │                │
│   │ 0x12345678_9ABCDEF0                    │                │
│   │ ...                                    │                │
│   └────────────────────────────────────────┘                │
└─────────────────────────────────────────────────────────────┘
        ▲ (4) DMA Write TLP
        │
┌─────────────────────────────────────────────────────────────┐
│ FPGA                                                         │
│   Register Interface:                                        │
│     - Receives CQ (IOVA address, trigger)                   │
│     - Stores in dma_addr_lo, dma_addr_hi                    │
│     - Pulses dma_trigger                                    │
│                                                             │
│   DMA Engine:                                               │
│     - Builds RQ descriptor with IOVA + data                 │
│     - Sends to PCIe core                                    │
│     - Sets dma_done when complete                           │
└─────────────────────────────────────────────────────────────┘
```

---

## Files

**Verilog:**
- [pcie_dma_write.v](../verilog_src/pcie_dma_write.v) - DMA write state machine
- [pcie_register_interface_dma.v](../verilog_src/pcie_register_interface_dma.v) - Integrated register + DMA

**C++:**
- [fpga_hello_dev.cpp](../cpp_src/fpga_driver/fpga_hello_dev.cpp) - `test_dma_write()` function
- [dma_memory_allocator.cpp](../cpp_src/common/dma_memory_allocator.cpp) - IOMMU buffer allocation

---

## Next Steps

1. **DMA Read:** FPGA reads from host memory (fetch descriptors)
2. **Descriptor Rings:** Host posts TX/RX descriptors for NIC
3. **Scatter-Gather:** Multiple buffers per packet
4. **Corundum Integration:** Use production queue management
