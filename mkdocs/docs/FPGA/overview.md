# FPGA Overview
This section documents the active FPGA-side data path in Venturi.
## Introduction
It performs the parsing and building of order books from the incoming UDP frames, where NASDAQ ITCH 5.0 protocol is used. It outputs the best ask/bid prices as well as the corresponding shares per symbol.

Below gives the hierarchy of the FPGA side design:

![FPGA_hierarchy](../figures/FPGA/top%20level/overview.png)

## Main Modules
- [PCS_PMA_wrapper](pcs_pma_wrapper.md): converts the UDP frames to xgmii interface. (this one I copied from the official design example.)
- [MAC_layer_rx](MAC_layer_rx.md): converts XGMII RX data into a simpler streaming interface.
- [order_book_parser](order_book_parser.md): decodes ITCH messages from the received payload stream.
- [order_book_builder](order_book_builder/overview.md): tracks top-of-book state per symbol.
- [async_fifo](async_fifo.md): crosses event payloads from the RX clock into the PCIe clock.
- [frame_timestamp](frame_timestamp.md): generates the exported event timestamp (Actually there should be one stamp at the mac layer).
- [rx_dma_config](rx_dma_config.md): MMIO register block for queue programming.
- [rx_dma_stage](rx_dma_stage/overview.md): DMA write engine for fixed-size host records.
- [pcie_wrapper](pcie_wrapper/overview.md): PCIe integration shell and AXI-Stream channel wiring.
