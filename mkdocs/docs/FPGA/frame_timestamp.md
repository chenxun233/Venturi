# frame_timestamp
`frame_timestamp` is the local timestamp source used by the exported order-book event payload.
## Introduction
The implementation is in `verilog_src/frame_timestamp.v`.
## Design Logic
The module increments a free-running `tick_counter` on every clock cycle.

When `i_event` is high, it copies the current counter value into `o_event_timestamp`.

In the current [symbol_book](order_book_builder/symbol_book/overview.md) integration, `i_event` is tied high, so the module effectively publishes the current cycle counter every cycle. The event path then captures the timestamp that is present when the symbol snapshot is emitted.
## Interface
### Inputs
- `i_clk`
- `i_rst`
- `i_event`
### Outputs
- `o_event_timestamp`
