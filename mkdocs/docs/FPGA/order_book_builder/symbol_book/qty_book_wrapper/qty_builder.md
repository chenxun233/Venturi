# qty_builder
This module is the update engine inside [qty_book_wrapper](overview.md). It receives quantity-delta messages for both sides, keeps only the messages for its configured side, updates the total shares stored at the affected price level, and tells [tree_builder](tree_builder.md) when that price level changes between empty and non-empty.

## Design Logic

This module itself does not contain a bram, it controls a [double-port bram](../../symbol_book/qty_book_wrapper/bram_dp.md) outside it, which is shared with [tree_builder](tree_builder.md). Thus, the output of this module is the address, operation and data for the bram. Also, it tells [tree_builder](tree_builder.md) every time when the shares of a price becomes zero or from zero becomes non-empty. This signal triggers [tree_builder](tree_builder.md) to find the new best price.

Because the operation cannot be done in one cycle or in pipeline, there is a [fifo](../fifo.md) inside.

If the fifo is not empty and the state machine is in `IDLE`, it pops out the data and latch it for operation (the latch may be improved in the future design as the output of the fifo itself is already latched if there is no new pop signal). Then, it starts the state machine for live price level update and message sending to [tree_builder](tree_builder.md).

The operation is a fixed routine. First, read the data from the [double-port bram](../../symbol_book/qty_book_wrapper/bram_dp.md), the address is the latched price index. You do not know whether there is already a share in the live price book, so you cannot skip this step. The second is to wait for the read to be finished. The third is write back the new shares of that price. The new shares have already been prepared once the read is finished. The calculation is easy, just add or minus from the read-out data with code give below:

```verilog
qty_new = is_add ? (qty_cur + d_shares)
                 : ((qty_cur > d_shares) ? (qty_cur - d_shares) : 0)
```

At the same time, there is a combinational logic watching if the state machine is in the `SECOND_CYCLE`, which is the *write cycle*. Because at this step, we have the current shares and the new shares for the price. This is directly used to check whether the shares become to zero or from zero to non-empty. We send out this event signal to [tree_builder](tree_builder.md) to find the new best price.

## Interface
### Inputs
- `i_clk_156`: main clock.
- `i_rst`: active-high reset.
- `i_qty_msg`: packed quantity update. In the wrapper integration, the format is `{bid_ask, price, is_add, d_shares, seq_num}`.
- `i_bram_o_data`: current quantity read back from BRAM port A.
### Outputs
- `o_tree_price_idx`: price index whose occupancy changed.
- `o_tree_price_change`: `NON_EMPTY`, `EMPTY`, or `IDLE`.
- `o_seq_num`: sequence number latched from the current quantity message.
- `o_bram_addr`: BRAM address for the current price level.
- `o_bram_op`: BRAM operation, where `READ` is `2'b01` and `WRITE` is `2'b10`.
- `o_bram_i_data`: new total shares written back to BRAM.

## Message Filtering
The wrapper sends the same `i_qty_msg` stream to both bid-side and ask-side instances. `qty_builder` accepts only the messages whose side matches `BID_OR_ASK`:
```verilog
wire ff_push = i_qty_side == BID_OR_ASK;
```
So one instance updates only bid levels, and the other updates only ask levels.
## Price Mapping
The module does not use the raw price as a BRAM address. It converts `price` into a compact index with `cal_qty_book_addr()`:
```verilog
price_idx = (price - PRICE_BASE) >> 2
```
This means adjacent tracked price levels are spaced by `4` in the encoded price unit.

Example with `PRICE_BASE = 100`:
```verilog
price = 112
price_idx = (112 - 100) >> 2 = 3
```
If the computed offset is negative or beyond the configured depth, the function returns index `0`.

## Tree Update Pulse
`qty_builder` does not notify [tree_builder](tree_builder.md) on every quantity change. It only notifies the tree when the price level changes occupancy:

- `0 -> non-zero`: emit `NON_EMPTY`
- `non-zero -> 0`: emit `EMPTY`
- otherwise: emit `IDLE`

This is enough because the tree only needs to know whether a price level should participate in best-price selection.

## BRAM Interface
This module drives only one side of [bram_dp](bram_dp.md):

- `o_bram_addr`: selected price index
- `o_bram_op`: `READ` during the first phase, then `WRITE`
- `o_bram_i_data`: new total shares during the write phase

The stored BRAM value is just the aggregated shares for that price level, not the raw order data.

## Limits

- Prices below `PRICE_BASE` or outside the tracked depth collapse to index `0`.
- The FIFO push-ready signal is not checked here, so sustained overflow would need system-level handling.

