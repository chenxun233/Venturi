# Common Infrastructure

This directory contains the shared infrastructure used by the host-side runtime in the Venturi project.

## Overview

The common layer provides hardware-agnostic abstractions and utilities reused across the host-side runtime, especially the FPGA demo path.

## Files

### Core Abstractions

#### `basic_dev.h` / `basic_dev.cpp`
Abstract base class defining the device driver interface.

**Key classes:**
- `BasicDev` - Abstract device interface
- `basic_para_type` - Device parameters (PCI address, BAR addresses, etc.)
- `VfioFd` - VFIO file descriptor collection
- `DevStatus` - Device statistics structure

`BasicDev` carries the common VFIO and device-management pieces needed by the current host runtime.

**Pure virtual methods** (must be implemented by derived classes):
```cpp
virtual bool initHardware() = 0;
virtual bool setRxRingBuffers(...) = 0;
virtual bool setTxRingBuffers(...) = 0;
```

**Utility methods:**
- `_monotonic_time()` - High-resolution timestamp
- `_print_stats_diff()` - Statistics comparison

#### `device.h`
Shared register-access helpers and device metadata structures used by the current host runtime.

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

In particular, `BasicDev` is intended to stop at:
- VFIO/PCI discovery and BAR mapping
- generic device counts and statistics
- common timing helpers

and not own subsystem-specific runtime policy.

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
    MyDevice(std::string pci_addr) : BasicDev(pci_addr) {
        // Initialize VFIO
        _getFD();
        _getBARAddr();
    }

    // Implement required methods
    bool initHardware() override {
        // Device-specific initialization
        return true;
    }

    bool setRxRingBuffers(uint16_t qcount, uint32_t num_buf, uint32_t buf_size) override {
        return true;
    }

    bool setTxRingBuffers(uint16_t qcount, uint32_t num_buf, uint32_t buf_size) override {
        return true;
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

See `FPGADev` for the active implementation example in this repository.

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

- [C++ Runtime README](../README.md)
- [Main Project README](../README.md)
- [VFIO Kernel Documentation](https://www.kernel.org/doc/html/latest/driver-api/vfio.html)

## Known Issues

- Memory pool currently only supports fixed-size buffers

## Future Work

- [ ] Add generic descriptor ring abstraction
- [ ] Support variable-size buffers in memory pool
- [ ] Add memory pool statistics
- [ ] Implement lock-free memory pool
- [ ] Add unit tests for all common components
