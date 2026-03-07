# Venturi

Venturi is a project focused on building an FPGA-based high-frequency trading data path with an order parser on the RX side and a dummy host-based TX path.

The project covers both host and FPGA development.

## Current Status

- FPGA-side PCIe connection has been achieved.
- FPGA-side order book building has been implemented.
- The FPGA RX path is still under development and not yet finished.
- The TX path is not finished yet; DPDK is being considered for host-side transmission.
- The project also includes a user-space driver for the `ixgbe` NIC, implemented in C++.

## Repository Layout

The following describes the top-level folders in this repository that are not ignored by `.gitignore`.

- `cpp_src/`: C++ host-side code, including the user-space `ixgbe` NIC driver, shared VFIO/DMA infrastructure, FPGA driver experiments, and CMake build files.
- `documents/`: Reference PDFs and specifications for the protocol and FPGA/NIC-related IP blocks.
- `figures/`: Design diagrams and images used to explain architecture and implementation details.
- `market_data/`: Sample market data captures and packet payload files, mainly in `.pcap` and text form, used for parser and order book testing.
- `scripts/`: Helper shell scripts for PCIe reset, hugepage setup, and VFIO setup.
- `verilog_backup/`: Older or backup RTL modules kept for reference during development.
- `verilog_src/`: Active FPGA RTL sources, including the top-level design, PCIe-related logic, MAC/PCS-PMA blocks, control-plane logic, and order book builder/parser modules.
- `verilog_tb/`: Verilog testbenches for MAC, PCIe gearboxes, and order book parser/builder verification.
