# tree_builder
Its job is to track which price levels are active and return the **best valid price index** for one side of the quantity book. This is one of the final outputs from [order_book_builder](../../../order_book_builder/overview.md)
## Introduction
This block is used inside [qty_book_wrapper](overview.md) after [qty_builder](qty_builder.md) updates one price level. It does not calculate share totals. It only tracks whether each price index is **empty or non-empty**, then reduces that information into one best price index.

## Design Logic
[qty_book_wrapper](../qty_book_wrapper/overview.md) splits the problem into two parts:

1. [qty_builder](qty_builder.md) updates the **shares** stored at one price level in the [double-port bram](../qty_book_wrapper/bram_dp.md) and reports whether that level changed occupancy.
2. this module updates the **best price index** based on those occupancy changes
3. The corresponding shares then be read from [double-port bram](../qty_book_wrapper/bram_dp.md). This is the tree_builder side operation.

That split is useful because best-price selection does not require the exact share count of every level. It only requires knowing which price indices are currently valid.

The internal structure is a reduction tree (refer to [endrias_Project_Final_Report.pdf](https://web.mit.edu/6.111/volume2/www/f2019/projects/endrias_Project_Final_Report.pdf)):

I have redrawn it to a four-level structure for better explanation, as give below:

![tree_structure](../../../../figures/FPGA/order_book_builder/tree%20structure.png)

The bottom registers are one-on-one to all the valid price index. Thus, the left is always less than right. Once a price index moves from empty to non-empty or vice versa, the bottom registers will also change accordingly. this is in 
```verilog
if (i_price_change != IDLE) begin
    btm_valid[i_price_idx] <= (i_price_change == NON_EMPTY);
end
```
The layer uppon bottom will automatically update values in each node, 
The winner rule depends on `BID_OR_ASK`:

- bid-style selection chooses the **higher** valid index
- ask-style selection chooses the **lower** valid index
  
This can be noticed in the following block.
```verilog
if (BID_OR_ASK == 2'b01) begin
   // For bid tree, if both children are valid, take the right one (higher price).
   assign mid_to_btm_idx[level][node] =
       btm_valid[RIGHT_CHILD] ? RIGHT_LEAF_IDX :
       btm_valid[LEFT_CHILD]  ? LEFT_LEAF_IDX  :
                                       {QTY_PRICE_LVL_BIT{1'b0}};
end else if (BID_OR_ASK == 2'b10) begin
   // For ask tree, if both children are valid, take the left one (lower price).
   assign mid_to_btm_idx[level][node] =
       btm_valid[LEFT_CHILD]  ? LEFT_LEAF_IDX  :
       btm_valid[RIGHT_CHILD] ? RIGHT_LEAF_IDX :
                                       {QTY_PRICE_LVL_BIT{1'b0}};
end
```

This update will propagate.

Instead of using an all-register tree, which only updates one level per cycle, here the combinational and sequential logics are combined to construct the tree. Only the `bottom`, `mid` and the `top` nodes are registers while other nodes are all wires. this makes the update faster.

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


## Output Meaning
`o_best_valid` tells downstream logic whether any price level is currently active.

`o_best_price_idx` is only meaningful when `o_best_valid = 1`. [qty_book_wrapper](overview.md) converts that index back into a price and uses [bram_dp](bram_dp.md) port B to read the shares stored at that best level.

## Complexity
With `QTY_PRICE_LVL_BIT = N`, the tree covers `2^N` price levels.

## Limits

