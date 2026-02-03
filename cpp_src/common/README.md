# Common Infrastructure

This directory contains the shared infrastructure used by all device drivers in the Venturi project.

## Overview

The common layer provides hardware-agnostic abstractions and utilities that are reused across different device drivers (Intel NIC, FPGA NIC, etc.). This promotes code reuse and maintains consistency across drivers.

## Files

### Core Abstractions

#### `basic_dev.h` / `basic_dev.cpp`
Abstract base class defining the device driver interface.

**Key classes:**
- `BasicDev` - Abstract device interface
- `basic_para_type` - Device parameters (PCI address, BAR addresses, etc.)
- `VfioFd` - VFIO file descriptor collection
- `DevStatus` - Device statistics structure
- `InterruptQueue` - Interrupt queue management

**Pure virtual methods** (must be implemented by derived classes):
```cpp
virtual bool initHardware() = 0;
virtual bool initializeInterrupt(...) = 0;
virtual bool enableDevQueues() = 0;
virtual bool enableDevInterrupt() = 0;
virtual bool wait4Link() = 0;
virtual bool setRxRingBuffers(...) = 0;
virtual bool setTxRingBuffers(...) = 0;
virtual bool setPromisc(bool enable) = 0;
virtual bool sendOnQueue(...) = 0;
```

**Utility methods:**
- `_monotonic_time()` - High-resolution timestamp
- `_print_stats_diff()` - Statistics comparison

#### `device.h`
Device-specific constants and structures (Intel 82599-specific, may need refactoring for true hardware independence).

### Memory Management

#### `memory_pool.h` / `memory_pool.cpp`
DMA-capable packet buffer pool management.

**Key class:**
- `DMAMemoryPool` - Pool of DMA-mapped packet buffers

**Features:**
- Huge page allocation for performance
- IOMMU mapping for DMA safety
- Zero-copy buffer management
- Efficient allocation/deallocation

#### `dma_memory_allocator.h` / `dma_memory_allocator.cpp`
Low-level DMA memory allocator.

**Key class:**
- `DMAMemoryAllocator` - Allocates and maps DMA memory

**Features:**
- Huge page support (2MB/1GB pages)
- IOMMU/VFIO mapping
- Physical address translation
- NUMA-aware allocation

### Ring Buffers

#### `basic_ring_buffer.h` / `basic_ring_buffer.cpp`
Generic circular buffer implementation.

**Key classes:**
- `BasicRxRingBuffer` - Generic RX descriptor ring
- `BasicTxRingBuffer` - Generic TX descriptor ring

**Features:**
- Lock-free design for single producer/consumer
- Batch operations for efficiency
- Descriptor wrapping logic
- Hardware-agnostic interface

### Utilities

#### `log.h`
Logging macros for consistent debug output.

**Macros:**
- `info(fmt, ...)` - Informational messages
- `warn(fmt, ...)` - Warning messages
- `debug(fmt, ...)` - Debug-level messages

**Features:**
- Color-coded output
- File/line/function information
- printf-style formatting
- Compile-time enable/disable

## Design Principles

### 1. Hardware Abstraction
The common layer provides interfaces that are independent of specific hardware. Device-specific details are pushed to driver implementations.

### 2. RAII (Resource Acquisition Is Initialization)
Resources are acquired in constructors and released in destructors. This ensures proper cleanup even in error paths.

### 3. Zero-Copy
DMA buffers are mapped directly to userspace. No data copying occurs between kernel and userspace.

### 4. Performance
- Huge pages reduce TLB pressure
- Batch operations minimize overhead
- Lock-free algorithms where possible
- Cache-friendly data structures

### 5. Type Safety
Strong typing via classes and structs. Minimal use of void* and raw pointers.

## Usage Example

```cpp
#include "basic_dev.h"
#include "memory_pool.h"
#include "dma_memory_allocator.h"

// Derive from BasicDev
class MyDevice : public BasicDev {
public:
    MyDevice(std::string pci_addr) : BasicDev(pci_addr, 0) {
        // Initialize VFIO
        _getFD();
        _getBARAddr(0);
    }

    // Implement required methods
    bool initHardware() override {
        // Device-specific initialization
        return true;
    }

    bool sendOnQueue(uint8_t* data, size_t size, uint16_t qid) override {
        // Device-specific TX logic
        return true;
    }

    // ... implement other virtual methods

private:
    bool _getFD() override {
        // VFIO setup
        return _getGroupID() && _getContainerFD() && ...;
    }
};
```

## VFIO Integration

The common layer provides patterns for VFIO integration:

1. **IOMMU Group Discovery**: Find device's IOMMU group via sysfs
2. **Container Setup**: Open /dev/vfio/vfio and configure IOMMU
3. **Group Management**: Add group to container
4. **Device Access**: Get device FD for BAR mapping
5. **BAR Mapping**: Memory-map device registers
6. **DMA Setup**: Configure IOMMU for DMA access

See derived classes (`Intel82599Dev`, `FPGADev`) for implementation examples.

## Thread Safety

- `BasicDev` is **not thread-safe** by default
- Ring buffers are **single-producer/single-consumer**
- Memory pools are **thread-safe** with internal synchronization
- Applications should use separate queues per thread

## Performance Considerations

### Huge Pages
Use huge pages for DMA buffers:
```bash
# Reserve 1GB huge pages
echo 512 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
```

### NUMA Awareness
Allocate memory on the same NUMA node as the device:
```bash
# Check device NUMA node
cat /sys/bus/pci/devices/0000:03:00.0/numa_node
```

### CPU Affinity
Pin threads to cores on the same NUMA node:
```bash
taskset -c 0-3 ./test_app
```

## Extending the Common Layer

When adding new shared functionality:

1. **Keep it hardware-agnostic**: Don't add device-specific code
2. **Document interfaces**: Clear contracts for implementers
3. **Add tests**: Unit tests for new functionality
4. **Maintain compatibility**: Don't break existing drivers
5. **Performance first**: Common code is on the hot path

## Related Documentation

- [Intel Driver README](../intel_driver/README.md)
- [FPGA Driver README](../fpga_driver/README_FPGA_HELLO.md)
- [Main Project README](../README.md)
- [VFIO Kernel Documentation](https://www.kernel.org/doc/html/latest/driver-api/vfio.html)

## Known Issues

- `device.h` contains Intel-specific definitions (should be refactored)
- Some ring buffer logic assumes Intel descriptor format
- Memory pool currently only supports fixed-size buffers

## Future Work

- [ ] Refactor `device.h` to be truly hardware-agnostic
- [ ] Add generic descriptor ring abstraction
- [ ] Support variable-size buffers in memory pool
- [ ] Add memory pool statistics
- [ ] Implement lock-free memory pool
- [ ] Add unit tests for all common components
