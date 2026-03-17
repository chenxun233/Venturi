# rx_dma_stage
`rx_dma_stage` is the PCIe-side RX DMA engine that writes fixed-size order-book events into host memory.
## Introduction
The implementation is in `verilog_src/DMA_related/rx_dma_stage.v`.
## Design Logic
This module mainly performs DMA write. It pops out packets from one of the [CDC fifos](../async_fifo.md) (done by an [arbiter module](arbiter.md) inside), send it to [RQ_formatter](../pcie_wrapper/RQ_formatter/overview.md) in the PCIe wrapper. Address calculation is important. This has already been explained in [rx_dma_config](../rx_dma_config.md).
```text
write_addr = base_addr + slot_index * 32
```
## Drop Behavior
The module drops an event when the selected queue is disabled or full. This behavior can also be found in [rx_dma_config](../rx_dma_config.md).
In that case it:
1. pops the event from the CDC FIFO
2. increments `drop_count`
3. skips the DMA write
This means the receive pipeline keeps moving even when the host ring cannot accept more records.
## Reset Behavior
`rx_dma_stage` also accepts `i_reg_reset` from [rx_dma_config](../rx_dma_config.md). That synchronous reset path clears the queue producer state and drop counters independently of the global PCIe reset.
## Limits
- There is no backpressure toward the upstream event source. Disabled or full queues cause drops.
- The engine writes fixed-size records only. It is not a descriptor-fetch RX path.
