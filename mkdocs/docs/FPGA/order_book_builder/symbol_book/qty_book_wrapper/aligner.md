# aligner
`aligner` is the final delay stage inside [qty_book_wrapper](overview.md).
## Introduction
The implementation is in `verilog_src/order_book_related/dependencies/aligner.v`.
## Design Logic
The best-price path and the BRAM best-share read do not arrive in the same cycle:

- `tree_builder` determines the best price index
- BRAM port B returns the shares for that level one cycle later

`aligner` delays:

- best price
- completion pulse

so they line up with the returned best-share value.
## Interface
### Inputs
- `i_best_price`
- `i_best_shares`
- `i_t_op_done`
### Outputs
- `o_best_price_aligned`
- `o_best_shares`
- `o_op_done_aligned`

There is no explicit valid bit in the current module.
