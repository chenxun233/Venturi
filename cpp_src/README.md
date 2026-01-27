# Venturi - High-Performance NIC Drivers

This directory contains modular, high-performance userspace drivers for network interface cards using VFIO (Virtual Function I/O).

## Directory Structure

```
cpp_src/
├── common/              # Shared infrastructure for all drivers
│   ├── basic_dev.*      # Abstract device base class
│   ├── basic_ring_buffer.*  # Generic ring buffer implementation
│   ├── dma_memory_allocator.*  # DMA memory management
│   ├── memory_pool.*    # Packet buffer pool
│   ├── log.h            # Logging utilities
│   └── device.h         # Device abstraction layer
│
├── intel_driver/        # Intel 82599 10GbE NIC driver
│   ├── vfio_dev.*       # Intel-specific VFIO implementation
│   ├── ixgbe_ring_buffer.*  # Intel descriptor ring buffers
│   ├── ixgbe_type.h     # Intel 82599 register definitions
│   ├── factory.*        # Device factory for creating instances
│   ├── test_app_loopsend.cpp   # Loop send performance test
│   └── test_app_pcap.cpp       # Packet capture application
│
└── fpga_driver/         # FPGA-based NIC driver
    ├── fpga_hello_dev.*         # FPGA device class
    ├── test_fpga_hello.cpp      # Standalone test (no dependencies)
    ├── test_fpga_hello_v2.cpp   # Infrastructure-based test
    └── README_FPGA_HELLO.md     # FPGA driver documentation
```

## Architecture

### Three-Layer Design

1. **Common Layer** (`common/`)
   - Provides abstract interfaces (`BasicDev`)
   - Implements shared functionality (DMA, memory pools, ring buffers)
   - Hardware-agnostic VFIO utilities
   - Used by all drivers

2. **Driver Layer** (`intel_driver/`, `fpga_driver/`)
   - Hardware-specific implementations
   - Inherits from `BasicDev` abstract class
   - Implements device-specific register access
   - Provides vendor-specific optimizations

3. **Application Layer** (test applications)
   - Uses driver APIs to send/receive packets
   - Performance testing utilities
   - Example applications for reference

### Inheritance Hierarchy

```
BasicDev (common/basic_dev.h)
    │
    ├── Intel82599Dev (intel_driver/vfio_dev.h)
    │       └── Full-featured 10GbE NIC driver
    │           - TX/RX ring buffers
    │           - MSI-X interrupts
    │           - DMA engine
    │           - Flow control
    │
    └── FPGAHelloDev (fpga_driver/fpga_hello_dev.h)
            └── Simple PCIe register interface
                - Basic register read/write
                - MSI interrupts
                - Foundation for FPGA NIC
```

## Building

### Quick Build

```bash
cd /home/chenxun/Documents/Project/Venturi
./scripts/build_tests.sh
```

### Manual Build

```bash
cd cpp_src
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

### Build Targets

| Target | Description | Driver |
|--------|-------------|--------|
| `test_app_loopsend` | High-speed packet generator | Intel 82599 |
| `test_app_pcap` | Packet capture to pcap file | Intel 82599 |
| `test_fpga_hello` | Standalone FPGA test | FPGA (standalone) |
| `test_fpga_hello_v2` | Infrastructure-based FPGA test | FPGA (common infra) |

## Usage Examples

### Intel 82599 NIC

```bash
# Bind device to vfio-pci
sudo modprobe vfio-pci
echo "8086 10fb" | sudo tee /sys/bus/pci/drivers/vfio-pci/new_id

# Run loop send test
sudo ./test_app_loopsend 0000:01:00.0

# Capture packets
sudo ./test_app_pcap 0000:01:00.0 1000 output.pcap
```

### FPGA Device

```bash
# Bind device to vfio-pci
sudo modprobe vfio-pci
echo "10ee 8038" | sudo tee /sys/bus/pci/drivers/vfio-pci/new_id

# Run standalone test
sudo ./test_fpga_hello 0000:03:00.0

# Run infrastructure-based test
sudo ./test_fpga_hello_v2 0000:03:00.0
```

## Key Features

### Common Infrastructure

- **VFIO-based**: Secure userspace device access without kernel modules
- **Zero-copy**: DMA directly to/from userspace buffers
- **IOMMU protection**: Hardware-enforced memory isolation
- **Huge pages**: Reduced TLB misses for high throughput
- **NUMA-aware**: Memory allocation respects NUMA topology

### Intel 82599 Driver

- **Line rate**: 10 Gbps full duplex
- **Multi-queue**: RSS (Receive Side Scaling) support
- **Interrupts**: MSI-X with dynamic moderation
- **Offloads**: Checksum, TSO (planned)
- **Flow control**: IEEE 802.3x PAUSE frames

### FPGA Driver

- **Modular design**: Easy to extend for full NIC features
- **Register interface**: Direct BAR access with memory barriers
- **Interrupt support**: MSI interrupt generation
- **Extensible**: Foundation for adding DMA, queues, Ethernet MAC

## Dependencies

- **Linux kernel**: 4.0+ (for VFIO support)
- **CMake**: 3.16+
- **Compiler**: GCC/Clang with C++20 support
- **IOMMU**: Hardware IOMMU required (VT-d on Intel, AMD-Vi on AMD)

## Development

### Adding a New Driver

1. Create new directory: `cpp_src/<vendor>_driver/`
2. Inherit from `BasicDev` in `common/basic_dev.h`
3. Implement required virtual methods:
   - `initHardware()`
   - `enableDevQueues()`
   - `sendOnQueue()` / receive methods
   - VFIO setup (`_getFD()`, `_getBARAddr()`, etc.)
4. Add build target to CMakeLists.txt
5. Write test applications

### Code Style

- Follow existing conventions in `common/` and `intel_driver/`
- Use RAII for resource management
- Prefer composition over inheritance where appropriate
- Document hardware register access with datash references
- Add inline comments for non-obvious hardware quirks

## Performance

### Intel 82599 Benchmarks

- **TX throughput**: ~14.88 Mpps (line rate for 64-byte packets)
- **RX throughput**: ~14.88 Mpps (line rate)
- **Latency**: <1 µs (measured with hardware timestamping)

### Optimization Techniques

- Batch processing (64 packets per batch)
- Prefetching descriptor rings
- Huge pages for DMA buffers
- CPU affinity for interrupt handling
- Lock-free ring buffer design

## Troubleshooting

### "Failed to open /dev/vfio/vfio"

```bash
sudo modprobe vfio-pci
```

### "IOMMU group not found"

Enable IOMMU in BIOS and add to kernel command line:
```
intel_iommu=on iommu=pt
```

### "Group not viable"

All devices in IOMMU group must be bound to vfio-pci:
```bash
# Find group members
ls /sys/bus/pci/devices/0000:03:00.0/iommu_group/devices/

# Bind all to vfio-pci
for dev in /sys/bus/pci/devices/0000:03:00.0/iommu_group/devices/*; do
    echo vfio-pci | sudo tee $dev/driver_override
    echo $(basename $dev) | sudo tee /sys/bus/pci/drivers/vfio-pci/bind
done
```

## References

- [VFIO Documentation](https://www.kernel.org/doc/html/latest/driver-api/vfio.html)
- [Intel 82599 Datasheet](https://www.intel.com/content/www/us/en/embedded/products/networking/82599-10-gbe-controller-datasheet.html)
- [Xilinx UltraScale+ PCIe](https://www.xilinx.com/support/documentation/ip_documentation/pcie3_ultrascale/v4_4/pg156-ultrascale-pcie-gen3.pdf)
- [DPDK Documentation](https://doc.dpdk.org/) (for reference, though we don't use DPDK)

## License

This project follows the Venturi project license.

## Contributing

Contributions welcome! Please:
1. Follow the existing code style
2. Add tests for new features
3. Update documentation
4. Test on real hardware before submitting

## Contact

For questions or issues, please open a GitHub issue.
