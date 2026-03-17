# Welcome to Venturi
## Introduction
Venturi is an FPGA plus Linux project centered on **receiving Nasdaq ITCH 5.0 market data**, **building order-book state in hardware**, and **exporting FPGA-generated events to host memory through PCIe DMA**.

The documentation below reflects the **current FPGA-side implementation** in this repository. The active receive path is already documented module by module, from Ethernet ingress to host-memory writes.
## Current FPGA Side
The present FPGA design is split into two main clock domains:

1. **Receive path at 156.25 MHz**: Ethernet data is converted to XGMII, framed into an AXI-style stream, parsed as ITCH messages, and reduced into order-book events.
2. **PCIe path at 250 MHz**: those events cross into the PCIe user clock domain, where software-programmed RX queues are serviced by a DMA write engine.

At a high level, the pipeline is:

`pcs_pma_wrapper -> MAC_layer_rx -> order_book_parser -> order_book_builder -> frame_timestamp -> async_fifo -> rx_dma_stage -> pcie_wrapper`
## FPGA Data Path
- [FPGA overview](FPGA/overview.md): top-level summary of the active FPGA hierarchy.
- [pcs_pma_wrapper](FPGA/pcs_pma_wrapper.md): converts the Ethernet PHY-side interface into the XGMII stream used by the receive logic.
- [MAC_layer_rx](FPGA/MAC_layer_rx.md): strips preamble/SFD and converts XGMII RX traffic into a simpler streaming interface.
- [order_book_parser](FPGA/order_book_parser.md): walks the Ethernet/IP/UDP headers and decodes supported ITCH 5.0 message types.
- [order_book_builder](FPGA/order_book_builder/overview.md): updates per-symbol order-book state and produces event payloads for downstream export.
- [frame_timestamp](FPGA/frame_timestamp.md): provides the event timestamp carried with parsed data.
- [async_fifo](FPGA/async_fifo.md): crosses event payloads from the receive clock domain into the PCIe user clock domain.
- [rx_dma_config](FPGA/rx_dma_config.md): exposes the RX queue programming and status registers to host software over MMIO.
- [rx_dma_stage](FPGA/rx_dma_stage/overview.md): selects a non-empty event queue, checks host ring availability, and issues fixed-size PCIe DMA writes.
- [pcie_wrapper](FPGA/pcie_wrapper/overview.md): connects the user logic to the Xilinx PCIe Gen3 IP and separates MMIO traffic from requester traffic.
## Current Focus Areas
The documented FPGA-side behavior today is mainly the **RX and host-export path**:

- Ethernet receive and frame adaptation
- ITCH message parsing
- order-book state update
- clock-domain crossing into PCIe logic
- MMIO-based queue programming
- fixed-size RX DMA writes into host memory

This means the existing docs are strongest on the path from **incoming wire data** to **host-visible DMA records**.
## Documentation Map
- FPGA pages: start from [FPGA overview](FPGA/overview.md) for the hardware hierarchy.
- DMA and queueing: use [rx_dma_config](FPGA/rx_dma_config.md), [rx_dma_stage](FPGA/rx_dma_stage/overview.md), and [arbiter](FPGA/rx_dma_stage/arbiter.md).
- PCIe internals: use [pcie_wrapper overview](FPGA/pcie_wrapper/overview.md), then drill into [CQ_parser](FPGA/pcie_wrapper/CQ_parser.md), [CC_formatter](FPGA/pcie_wrapper/CC_formatter.md), [RQ_formatter](FPGA/pcie_wrapper/RQ_formatter/overview.md), and [RC_parser](FPGA/pcie_wrapper/RC_parser/overview.md).
