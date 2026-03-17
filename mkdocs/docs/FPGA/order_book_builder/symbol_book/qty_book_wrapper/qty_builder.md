# qty_builder
`qty_builder` is the update engine inside [qty_book_wrapper](overview.md).
## Introduction
The implementation is in `verilog_src/order_book_related/dependencies/qty_builder.v`.
## Design Logic
The module receives quantity deltas, filters them by side, and updates one aggregated price level in the external [bram_dp](bram_dp.md).

It uses an internal [fifo](../fifo.md) because one update requires a BRAM read-modify-write sequence:

1. pop one matching quantity message
2. translate `price` to a compact price index
3. issue a BRAM read
4. wait for the synchronous read result
5. compute the new share total
6. issue the BRAM write

At the write phase it also checks whether the price level changed occupancy:

- `0 -> non-zero`
- `non-zero -> 0`

Only those occupancy changes are forwarded to [tree_builder](tree_builder.md).
## Interface
### Inputs
- `i_clk_156`: main clock.
- `i_rst`: active-high reset.
- `i_qty_msg`: `{bid_ask, price, is_add, d_shares, op_done}`.
- `i_bram_o_data`: current quantity read from BRAM port A.
### Outputs
- `o_price_idx`: price index whose occupancy changed.
- `o_price_change`: `NON_EMPTY`, `EMPTY`, or `IDLE`.
- `o_op_done`: completion flag aligned to the occupancy-change path.
- `o_bram_addr`: BRAM address for port A.
- `o_bram_op`: BRAM operation.
- `o_bram_i_data`: updated share total written back to BRAM.
## Side Filtering
The current side filter is purely based on the side field:

```verilog
wire ff_push = i_qty_side == BID_OR_ASK;
```
So each wrapper sees the same bus but only enqueues messages for its own side.
## Price Mapping
The helper `cal_qty_book_addr()` converts an absolute price into a compact level index:

```text
price_idx = (price - PRICE_BASE) >> 2
```

That means one stored level corresponds to a price step of `4` in the encoded price unit.
## Limits
- The FIFO push-ready signal is not consumed here, so overflow must be handled at system level.
- Out-of-range prices collapse to index `0`.
