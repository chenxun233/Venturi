# fifo
`fifo` is the small synchronous FIFO used in several order-book helper blocks.
## Introduction
The implementation is in `verilog_src/order_book_related/dependencies/fifo.v`.
## Design Logic
This FIFO stores one word per entry in a simple single-clock queue.

It exposes:

- push request plus `o_push_ready`
- pop request
- `o_not_empty`
- registered `o_data`
- one-cycle `o_valid` pulse when a pop returns data

The current implementation is used as a short decoupling queue inside [book_builder](book_builder/overview.md) and [qty_builder](qty_book_wrapper/qty_builder.md).
## Timing Behavior
The important behavior is on the pop side:

- `i_do_pop` requests one pop
- on that clock edge, the FIFO copies `mem[rd_ptr]` into `o_data`
- `o_valid` is asserted for that cycle after the register update

So `o_data` is meaningful when `o_valid = 1`, not merely when `i_do_pop = 1`.
## Limits
- `o_push_ready` is not checked everywhere the FIFO is instantiated, so overflow handling is still a system-level concern.
- The reset is synchronous to `i_clk` in this module.
