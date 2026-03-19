/**
 * test_fpga_hello_v2.cpp - FPGA RX host validation entrypoint
 *
 * This version uses the shared VFIO infrastructure and exercises the current
 * FPGA RX BAR0 contract:
 *   0x00: REG_RESET
 *   0x04: REG_ID
 *   0x0C: REG_SYNC_ENABLE
 *   0x40+: per-queue RX configuration and counters
 *
 * Usage: sudo ./test_fpga_hello_v2 <pci_address> <test_num>
 *   1: BAR0 register and REG_SYNC_ENABLE test
 *   2: interrupt placeholder
 *   3: legacy DMA smoke-test placeholder
 *   4: replay validator for live tcpreplay traffic
 */

#include "fpga_dev.h"
#include "fpga_replay_validator.h"
#include "../../common/log.h"
#include <cstdio>
#include <cstdlib>
#include <memory>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pci_address> <test_num>\n", argv[0]);
        fprintf(stderr, "  e.g.: %s 0000:05:00.0 1 (1-4)\n", argv[0]);
        return 1;
    }

    const char* pci_addr = argv[1];
    const char* test_num = argv[2];

    printf("=== FPGA RX Host Validation Test ===\n");
    printf("Using shared VFIO infrastructure\n");
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
        fprintf(stderr,     "Failed to create FPGA device object\n");
        fprintf(stderr,     "Make sure the device is bound to vfio-pci:\n");
        fprintf(stderr,     "sudo modprobe vfio-pci ids=10ee:8038\n");
        fprintf(stderr,     "echo %s | sudo tee /sys/bus/pci/drivers/vfio-pci/bind\n", pci_addr);
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
            dev->test_register();
            break;
        case 2:
            dev->trigger_interrupt();
            break;
        case 3:
            dev->test_dma_write();
            break;
        case 4:
        {
            FpgaReplayValidator validator(*dev);
            validator.run();
            break;
        }
        default:
            printf("Unknown test: %d (valid: 1=register+sync, 2=interrupt placeholder, 3=DMA placeholder, 4=replay validator)\n",
                   test_num_int);
            break;
    }
}
