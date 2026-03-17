# tree_builder
`tree_builder` tracks which price levels are currently non-empty and returns the best price index for one side of the book.
## Introduction
The implementation is in `verilog_src/order_book_related/dependencies/tree_builder.v`.
## Design Logic
The module stores one valid bit per tracked price level and reduces that occupancy information into one best index.

The update input comes from [qty_builder](qty_builder.md):

- `NON_EMPTY`: level changed from empty to non-empty
- `EMPTY`: level changed from non-empty to empty
- `IDLE`: no occupancy change

The reduction rule depends on `BID_OR_ASK`:

- bid side chooses the higher valid index
- ask side chooses the lower valid index

The implementation uses a mixed combinational/sequential tree. The lower half of the tree is reduced combinationally, one mid-level stage is registered, and the upper half is reduced combinationally again. That shortens the update path compared with an all-register tree.

Below gives a schematic of the tree structure, refer to [HFT Accelerator](https://web.mit.edu/6.111/volume2/www/f2019/projects/endrias_Project_Final_Report.pdf)

![tree structure](../../../../figures/FPGA/order_book_builder/tree%20structure.png)

## Interface
### Inputs
- `i_clk`
- `i_rst`
- `i_price_idx`
- `i_price_change`
- `i_op_done`
### Outputs
- `o_best_price_idx`
- `o_op_done`

There is no separate `o_best_valid` signal in the current RTL. When no level is active, the selected index collapses to `0`.
## Output Use
[qty_book_wrapper](overview.md) converts `o_best_price_idx` back into an absolute price and uses BRAM port B to fetch the shares stored at that best level.
