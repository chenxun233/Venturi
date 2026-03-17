# arbiter
`arbiter` is the small round-robin queue selector used on the FPGA side.
## Introduction
The active implementation is in `verilog_src/DMA_related/arbiter.v`.
## Design Logic
The module accepts one request bit per queue in `i_req` and chooses one winning queue index in round-robin order.

The current requester is remembered in `rr_pointer`. Each combinational evaluation scans the request vector starting from that pointer and picks the first asserted request.

Below illustrates how does the round-robin behave:

![abiter](../../figures/FPGA/top%20level/arbiter.png)

1. At time 1, it starts from `rr_pointer`. `rr_adder` adds `0`, `1`, ..., `SYMBOL_NUM` to `rr_pointer` each time. This helps to check whether the [CDC_fifo](../async_fifo.md) having data to be popped. If it does, it locked the current index to `selected_int`, also assign `o_que_idx` to that value.
2. At time 2, once `o_que_idx` is asserted, a packet will be popped out from the corresponding [CDC_fifo](../async_fifo.md). But the following is out of this module's scope.
3. At time 3, `rr_pointer` will start after `o_que_idx`, a new loop will automatically start again, until the next valid [CDC_fifo](../async_fifo.md)

## Interface
### Inputs
- `i_clk`
- `i_rst`
- `i_req`: one request bit per queue
- `i_accept`: handshake that commits the current selection
### Outputs
- `o_valid`: high when at least one request is present
- `o_que_idx`: selected queue index
## Current Use
In the active RTL, this arbiter is used by [rx_dma_stage](overview.md) to choose which per-symbol CDC FIFO should be serviced next.
