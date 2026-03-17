# order_book_builder
`order_book_builder` receives parsed ITCH messages from [order_book_parser](../order_book_parser.md) and builds one independent top-of-book pipeline per tracked symbol.
## Introduction
The implementation is in `verilog_src/order_book_related/order_book_builder.v`.
## Design Logic
The module fans one parser event out to all configured [symbol_book](symbol_book/overview.md) instances.

The hierarchy can be found below:

![order_book_builder](../../figures/FPGA/order_book_builder/hierarachy_order_book_builder.png)

## Interface
### Inputs
- `i_clk_156`: main order-book clock.
- `i_rst`: active-high reset.
- `i_msg_valid`: parser output valid.
- `i_msg_type`: ITCH message type.
- `i_stock_locate`: parser stock locate.
- `i_order_ref_num`: original order reference.
- `i_new_order_ref_num`: replacement order reference for `U`.
- `i_buy_sell`: ASCII side field for add-style messages.
- `i_shares`: message quantity.
- `i_price`: message price.
### Outputs
- `o_event_valid`: one bit per configured symbol. A bit pulses when that `symbol_book` finishes an update that changes top-of-book.
- `o_event_payload`: concatenated per-symbol payload bus.
## Payload Layout
Each symbol payload is `208` bits wide:

```text
{ask_price[31:0], ask_shares[31:0], bid_price[31:0], bid_shares[31:0], event_timestamp[63:0], stock_locate[15:0]}
```

This is the same payload that later crosses the CDC FIFO and is zero-padded into the `32-byte` host record used by [rx_dma_stage](..//rx_dma_stage/overview.md).
## Per-Symbol Behavior
The current implementation instantiates two `symbol_book`s:

- AAPL with `STOCK_LOCATE = 16'h000d`
- HSBC with `STOCK_LOCATE = 16'h0ee8`

That list is a top-level integration choice, not a limit of the module interface.
## Limits
- The event output is top-of-book only. Full depth is not exported.
- The symbol count and stock-locate mapping are compile-time parameters in the current top-level integration.
