# FPGA PCIe Hello World Test Applications

This directory contains two versions of the FPGA PCIe hello world test application.

## Version 1: test_fpga_hello (Standalone)

**File**: `test_fpga_hello.cpp`

### Description
A completely standalone test application with no dependencies on the existing driver infrastructure. All VFIO setup code is included directly in the file.

### Features
- Self-contained - no external dependencies beyond Linux headers
- Good for learning VFIO API
- Minimal and easy to understand
- ~290 lines of code

### Usage
```bash
# Build
cd build
cmake ..
make test_fpga_hello

# Run (requires root or CAP_SYS_RAWIO)
sudo ./test_fpga_hello 0000:03:00.0
```

### Pros
- Easy to understand for beginners
- No dependencies - can be used as a template
- Direct VFIO API usage is visible

### Cons
- Duplicates VFIO setup code
- No code reuse with main driver
- Limited error handling

---

## Version 2: test_fpga_hello_v2 (Infrastructure-based)

**Files**:
- `test_fpga_hello_v2.cpp` - Main test application
- `fpga_dev.h` - FPGA device class header
- `fpga_dev.cpp` - FPGA device class implementation

### Description
Uses the existing VFIO infrastructure from the Intel NIC driver. Demonstrates proper object-oriented design and code reuse.

### Features
- Leverages existing `BasicDev` abstract class
- Proper OOP design with inheritance
- Better error handling via `log.h` functions
- Code reuse - VFIO setup shared with Intel driver
- Easy to extend for future features

### Usage
```bash
# Build
cd build
cmake ..
make test_fpga_hello_v2

# Run (requires root or CAP_SYS_RAWIO)
sudo ./test_fpga_hello_v2 0000:03:00.0
```

### Pros
- Clean separation of concerns
- Reuses battle-tested VFIO code
- Easier to extend (e.g., add DMA, interrupts)
- Consistent with main driver architecture
- Better logging and error reporting

### Cons
- Requires understanding of `BasicDev` interface
- More files to maintain
- Slightly more complex build

---

## Architecture Comparison

### Version 1 (Standalone)
```
test_fpga_hello.cpp
    |
    +-- Direct VFIO API calls
    +-- Manual BAR mapping
    +-- Inline register access
```

### Version 2 (Infrastructure)
```
test_fpga_hello_v2.cpp
    |
    +-- FPGADev (fpga_dev.cpp/h)
            |
            +-- Inherits from BasicDev (basic_dev.h)
            +-- Uses VFIO infrastructure
            +-- Proper abstraction layers
```

---

## Register Map (Current FPGA RX Design)

The infrastructure-based test tracks the current FPGA RX BAR0 contract:

| Offset | Name | Access | Description |
|--------|------|--------|-------------|
| 0x00 | REG_RESET | WO | Reset RX queue configuration state |
| 0x04 | REG_ID | RO | Returns `0x4d5f52585f434647` |
| 0x0C | REG_SYNC_ENABLE | R/W | Enable sync/calibration behavior on `prod_ptr` reads |
| 0x40 + n*0x40 | RX queue window | R/W/RO | Queue base, slot count, enable, consumer pointer, producer pointer, drop count, status |

---

## Test Sequence

The current infrastructure-based test executable provides:

1. **Register + Sync Test**: Read `REG_ID`, read/write/readback `REG_SYNC_ENABLE`
2. **Interrupt Placeholder**: Reserved for future interrupt testing
3. **DMA Placeholder**: Reserved for legacy smoke testing
4. **Replay Validator**: Poll both FPGA RX queues and compare events against the fixture-derived expected book updates while you drive traffic with `tcpreplay`

---

## Which Version to Use?

### Use Version 1 if:
- You're learning VFIO API
- You need a minimal standalone example
- You want a template for other simple PCIe devices
- You don't need advanced features

### Use Version 2 if:
- You're extending the codebase with more FPGA features
- You want proper software architecture
- You plan to add DMA or interrupts later
- You want consistency with the main driver

---

## Building Both Versions

```bash
# From project root
cd build
cmake ../cpp_src
make

# This will build:
# - test_fpga_hello (standalone version)
# - test_fpga_hello_v2 (infrastructure version)
# - test_app_loopsend (Intel NIC test)
# - test_app_pcap (Intel NIC packet capture)
```

---

## Prerequisites

Before running either version:

1. **Load VFIO modules**
   ```bash
   sudo modprobe vfio-pci
   ```

2. **Bind FPGA device to vfio-pci**
   ```bash
   # Automatic binding (recommended)
   echo "10ee 8038" | sudo tee /sys/bus/pci/drivers/vfio-pci/new_id

   # OR manual binding
   echo 0000:03:00.0 | sudo tee /sys/bus/pci/drivers/vfio-pci/bind
   ```

3. **Verify binding**
   ```bash
   lspci -nnk -d 10ee:8038
   # Should show: Kernel driver in use: vfio-pci
   ```

4. **Check IOMMU**
   ```bash
   dmesg | grep -i iommu
   # Should show IOMMU enabled
   ```

---

## Troubleshooting

### "Failed to get IOMMU group"
- Device not bound to vfio-pci
- IOMMU not enabled in BIOS/GRUB

### "Group not viable"
- Other devices in same IOMMU group not bound to vfio-pci
- Use `lspci -v` to check IOMMU grouping

### "BAR0 size is 0"
- FPGA not programmed with bitstream
- PCIe link not trained properly

### "ID Register mismatch"
- Wrong bitstream loaded
- PCIe communication issue
- Check FPGA programming

### "REG_SYNC_ENABLE readback failed"
- Host-side code and current bitstream disagree on BAR0 offset `0x0C`
- The FPGA may not be programmed with the current RX image
- Check that the device is reachable through VFIO and BAR0 reads/writes work

---

## Future Extensions

Version 2 (`FPGADev`) can be easily extended to add:

- **Ring Buffers**: Add TX/RX queues like Intel driver
- **Interrupts**: Implement `initializeInterrupt()`
- **Ethernet MAC**: Add packet send/receive functions

The infrastructure is already in place via the `BasicDev` interface!

---

## References

- VFIO Documentation: `Documentation/driver-api/vfio.rst` in Linux kernel
- PCIe Base Spec: PCI-SIG specifications
- Xilinx UltraScale+ PCIe: PG156 (Product Guide)
- Intel 82599 Datasheet: For reference driver implementation

---

## License

This code follows the same license as the main Venturi project.
