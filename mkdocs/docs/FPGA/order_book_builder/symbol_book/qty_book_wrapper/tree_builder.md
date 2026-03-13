# tree_builder
This note documents the module implemented in `verilog_src/order_book_related/dependencies/tree_builder.v`. The current HDL module name is `builder`, and its job is to track which price levels are active and return the **best valid price index** for one side of the quantity book.
## Introduction
This block is used inside [qty_book_wrapper](overview.md) after [qty_builder](qty_builder.md) updates one price level. It does not calculate share totals. It only tracks whether each price index is **empty or non-empty**, then reduces that information into one best index.
## Design Logic
The wrapper splits the problem into two parts:

1. [qty_builder](qty_builder.md) updates the **shares** stored at one price level and reports whether that level changed occupancy
2. this module updates the **best price index** based on those occupancy changes

That split is useful because best-price selection does not require the exact share count of every level. It only requires knowing which price indices are currently valid.

The internal structure is a reduction tree:

- the bottom level stores one valid bit per price index
- intermediate nodes propagate whether any child subtree is valid
- the same intermediate nodes propagate the winning index for that subtree

The winner rule depends on `BID_OR_ASK`:

- bid-style selection chooses the **higher** valid index
- ask-style selection chooses the **lower** valid index
## Interface
### Inputs
- `i_clk`: main clock.
- `i_rst`: active-high reset.
- `i_price_idx`: price index whose occupancy changed.
- `i_price_change`: `NON_EMPTY`, `EMPTY`, or `IDLE`.
### Outputs
- `o_best_valid`: high when at least one tracked price level is non-empty.
- `o_best_price_idx`: best valid price index for this side.
## Occupancy Input
The input event comes from [qty_builder](qty_builder.md):

- `NON_EMPTY`: the price level changed from `0 -> non-zero`
- `EMPTY`: the price level changed from `non-zero -> 0`
- `IDLE`: no occupancy change

On each clock edge, if the change is not `IDLE`, the corresponding bottom-level valid bit is updated:
```verilog
btm_valid[i_price_idx] <= (i_price_change == NON_EMPTY);
```
## Tree Structure
The current HDL uses these names:

- `btm_valid`: leaf-level valid bits
- `mid_to_btm_valid` and `mid_to_btm_idx`: combinational reduction from the bottom half up to `MID_LEVEL`
- `mid_valid` and `mid_idx`: registered midpoint results
- `top_to_mid_valid` and `top_to_mid_idx`: combinational reduction from the midpoint up to the root

So the tree is split into three sections:

1. bottom valid storage in `btm_valid`
2. a registered middle stage in `mid_valid` and `mid_idx`
3. a top reduction stage that drives `o_best_valid` and `o_best_price_idx`

This registered midpoint shortens the longest combinational path compared with a fully flat reduction tree.
## Winner Rule
At every node, the tree chooses between the left child and right child:

- if only one child is valid, that child wins
- if both are valid, the rule depends on the side

For bid-style selection:

- the **right** child wins because it corresponds to a higher price index

For ask-style selection:

- the **left** child wins because it corresponds to a lower price index

Example:

- valid levels = `{3, 8, 12}`
- bid-style tree returns `12`
- ask-style tree returns `3`
## Output Meaning
`o_best_valid` tells downstream logic whether any price level is currently active.

`o_best_price_idx` is only meaningful when `o_best_valid = 1`. [qty_book_wrapper](overview.md) converts that index back into a price and uses [bram_dp](bram_dp.md) port B to read the shares stored at that best level.
## Latency
This block is not purely combinational from input update to final result. It includes the registered midpoint:

- one clock updates `btm_valid` and captures the mid-level results
- the root output is then reduced from those registered midpoint values

That is one reason [qty_book_wrapper](overview.md) is pipelined.
## Complexity
With `QTY_PRICE_LVL_BIT = N`, the tree covers `2^N` price levels.

Example:

- if `QTY_PRICE_LVL_BIT = 10`, the tree tracks `1024` price indices
## Limits
- The module tracks only empty/non-empty state, not the actual share quantity.
- It assumes the occupancy transitions from [qty_builder](qty_builder.md) are correct.
- The module name in the current HDL is `builder`, while this document path is still `tree_builder.md`.
- Timing and latency still scale with `QTY_PRICE_LVL_BIT`, even though the midpoint is registered.
