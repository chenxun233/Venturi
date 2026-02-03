/**
 * test_fpga_hello_v2.cpp - FPGA PCIe Hello World Test (using existing infrastructure)
 *
 * This version uses the existing VFIO infrastructure from the Intel NIC driver
 * to simplify device access and demonstrate code reuse.
 *
 * Register Map (BAR0):
 *   0x00: Scratch Register (R/W) - 64-bit scratch pad
 *   0x08: ID Register (RO) - Returns 0xDEADBEEF_CAFEBABE
 *   0x10: Interrupt Control (W) - Write to trigger MSI
 *   0x18: Status Register (RO) - Bit 0: Link Up, [31:16]: Int count
 *
 * Usage: sudo ./test_fpga_hello_v2 <pci_address>
 *   e.g.: sudo ./test_fpga_hello_v2 0000:03:00.0
 */

#include "fpga_dev.h"
#include "../common/log.h"
#include <cstdio>
#include <cstdlib>
#include <memory>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <test_num>\n", argv[0]);
        fprintf(stderr, "  e.g.: %s 1 (1-4)\n", argv[0]);
        return 1;
    }

    const char* pci_addr = "0000:06:00.0";
    const char* test_num = argv[1];

    printf("=== FPGA PCIe Hello World Test (v2) ===\n");
    printf("Using existing VFIO infrastructure\n");
    printf("PCI Address: %s\n\n", pci_addr);

    // Create device object - this handles all VFIO setup
    std::unique_ptr<FPGADev> dev;

    try {
        info("Creating FPGA device object...");
        dev = std::make_unique<FPGADev>(std::string(pci_addr));
    } catch (const std::exception& e) {
        fprintf(stderr, "Failed to create device: %s\n", e.what());
        return 1;
    }

    if (!dev) {
        fprintf(stderr, "Failed to create FPGA device object\n");
        fprintf(stderr, "Make sure the device is bound to vfio-pci:\n");
        fprintf(stderr, "  sudo modprobe vfio-pci ids=10ee:8038\n");
        fprintf(stderr, "  echo %s | sudo tee /sys/bus/pci/drivers/vfio-pci/bind\n", pci_addr);
        return 1;
    }

    // Initialize hardware
    info("Initializing hardware...");
    if (!dev->initHardware()) {
        warn("Hardware initialization failed or link is down");
        // Continue anyway - some tests might still work
    }

    printf("\n");

    int test_num_int = std::atoi(test_num);
    switch (test_num_int) {
        case 1:
            dev->test_scratch_register();
            break;
        case 2:
            dev->trigger_interrupt();
            break;
        case 3:
            dev->test_dma_write();
            break;
        case 4:
            dev->test_dma_roundtrip();
            break;
        default:
            printf("Unknown test: %d (valid: 1-4)\n", test_num_int);
            break;
    }



}
