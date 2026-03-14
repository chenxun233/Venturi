# async_fifo
This module is an **asynchronous FIFO** used to pass event payloads between two clock domains. In this project, it transfers `order_book_builder` events from the receive clock domain (xgmii clock domain, which is 156.25 MHz) into the PCIe/control-plane clock domain (250 MHz).
## Introduction
In `verilog_src/Top.v`, it is instantiated as `event_cdc_fifo_inst`.

Its write side (156.25 MHz) runs on `i_wr_clk`, and its read side (250 MHz) runs on `i_rd_clk`. 
## Design Logic

The FIFO uses a shared memory array: data is written in the `156.25 MHz` clock domain and read in the `250 MHz` clock domain. The main challenge is not the memory itself, but the control logic required to determine when the FIFO is full on the write side and empty on the read side. That requires safe clock-domain crossing and Gray-coded pointer transfer.

The diagram below shows the internal structure of this `async_fifo`:

![async_fifo detail](../figures/FPGA/top%20level/async_fifo.png)

At the center of the design is a memory array accessed from two clock domains. Solid blocks represent registers, while dashed blocks represent combinational signals.

On the write side, running at `156.25 MHz`, the control logic must determine whether the FIFO is full so that further writes can be blocked. This `full` decision depends on the read pointer, which is generated in the read clock domain at `250 MHz`. To transfer that information safely across the clock-domain boundary, the read pointer is first encoded in Gray code, then passed into the write domain through a two-stage synchronizer. After synchronization, the write-side logic compares the next write pointer against the synchronized read pointer to determine whether the FIFO is full.

Similarly, on the read side, the write pointer is converted to Gray code before being transferred into the read clock domain and synchronized there for empty detection.

Gray code is used because only one bit changes between adjacent pointer values. This reduces the risk of sampling multiple bit transitions during clock-domain crossing and therefore makes the pointer transfer more robust.

## Interface
### Inputs
- `i_wr_clk`: write clock.
- `i_wr_rst`: write-domain reset.
- `i_wr_en`: write request.
- `i_wr_data`: write payload.
- `i_rd_clk`: read clock.
- `i_rd_rst`: read-domain reset.
- `i_rd_en`: read request.
### Outputs
- `o_wr_full`: write-side full flag.
- `o_rd_empty`: read-side empty flag.
- `o_rd_valid`: read-data valid pulse.
- `o_rd_data`: read payload.

