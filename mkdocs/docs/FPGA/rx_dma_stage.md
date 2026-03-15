# rx_dma_stage

`rx_dma_stage` is the PCIe-side RX DMA engine used to move fixed-format order-book events from the FPGA into host memory.

## Introduction

The implementation is in `verilog_src/DMA_related/rx_dma_stage.v`.

This module sits in the `250 MHz` PCIe clock domain. It does not parse market data itself. Instead, it consumes already-built per-symbol events after they cross from the `156.25 MHz` receive clock domain through `async_fifo`.

At the top level, the RX export path is organized around two active downstream stages:

1. `async_fifo`: one per symbol, used for clock-domain crossing from RX clock to PCIe clock
2. `rx_dma_stage`: PCIe-side scheduling and DMA write generation

## What It Does

`rx_dma_stage` is responsible for:

- observing which per-symbol CDC FIFOs are non-empty
- arbitrating between rings when more than one ring has data ready
- dropping events when a host ring is disabled or full
- packing each event into a fixed `64-byte` host record
- computing the destination address from the programmed ring base, size, and producer pointer
- issuing PCIe `RQ` write requests into host memory

The module uses one shared DMA writer for all RX rings. If more than one ring has work pending, arbitration happens inside the DMA engine rather than ahead of the per-symbol `async_fifo` sources.

## Host Ring Model

The host-visible RX ring is a fixed-stride event ring, not a descriptor ring.

For each ring, software programs:

- `base_addr`
- `ring_size`
- `ctrl`
- `cons_ptr`

The FPGA reports:

- `prod_ptr`
- `drop_count`
- `status`

The write address for each event is:

```text
write_addr = base_addr + (prod_ptr % ring_size) * 64
```

This works because the current order-book event payload is fixed-format and can be zero-padded into a single `64-byte` host record.

## Dataflow

```text
order_book_builder per-symbol event
  -> async_fifo
  -> rx_dma_stage
     -> ring arbitration
     -> 64B record formatting
     -> RQ write
     -> host memory ring
```

## Overflow Behavior

If the host ring is full, `rx_dma_stage` does not overwrite unread data.

Instead it:

1. pops the pending event from the CDC FIFO
2. increments the ring's `drop_count`
3. skips the DMA write

This keeps the RX pipeline moving while preserving host-side ring correctness.


## Why It Is Not A Descriptor Engine

This design does not fetch host-provided descriptors for each event. That would be necessary for a general NIC RX path with variable packet buffers, but the current project only exports fixed-size order-book events.

So `rx_dma_stage` is a direct `order_book_builder`-event-to-host-ring DMA engine, not a regular NIC-style descriptor fetch path.
