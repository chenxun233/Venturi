# bram_dp
This module is the dual-port storage used inside [qty_book_wrapper](overview.md). It stores the **live total shares at every tracked price level** and allows the wrapper to update one price level while reading the shares of the current best price level at the same time.

## Design Logic
This storage is used because [qty_book_wrapper](overview.md) needs **two memory accesses in parallel**:

1. port A is used by [qty_builder](qty_builder.md) to read the old quantity of the updated price level and then write back the new total shares
2. port B is used to read the shares stored at the current best price index selected by [tree_builder](tree_builder.md)

Using a dual-port BRAM keeps the full quantity book in one memory block and avoids duplicating storage just to support those two access patterns.

The tradeoff is the same as with ordinary FPGA BRAM: both reads and writes are **clocked operations**. A read result appears on `o_data_a` or `o_data_b` after the next rising edge, so [aligner](aligner.md) is needed to line up `best_valid` and `best_price` with the returned best-share data.
## Interface
### Inputs
- `i_clk`: BRAM clock.
- `i_rst`: clears the two output registers.
- `i_addr_a`: address for port A.
- `i_op_a`: operation for port A, where `2'b01` is read and `2'b10` is write.
- `i_data_a`: write data for port A.
- `i_addr_b`: address for port B.
- `i_op_b`: operation for port B, where `2'b01` is read and `2'b10` is write.
- `i_data_b`: write data for port B.
### Outputs
- `o_data_a`: registered read data from port A.
- `o_data_b`: registered read data from port B.
## Stored Format
Unlike the order-table BRAM in [book_builder/bram.md](../book_builder/bram.md), this memory does not store full order state. Each row stores only the **aggregated shares** of one price level.

## Read And Write Behavior
Each port can independently request `IDLE`, `READ`, or `WRITE`:

- `WRITE`: on the rising edge, the selected row is updated from the corresponding `i_data_*`
- `READ`: on the rising edge, the selected row is copied into `o_data_*`
- `IDLE`: the output for that port is driven to zero on the next clock if reset is not asserted

So the module behaves like a **synchronous dual-port BRAM** with two independently controlled ports.

## Reset Behavior
Two reset-related details matter:

1. The `initial` block fills the whole memory array with zeros.
2. Runtime `i_rst` only clears `o_data_a` and `o_data_b`; it does not walk through the entire memory and erase every row.

So if the design wants to clear one price level during normal operation, it must explicitly write zero into that row.

## Limits
- There is no byte enable or partial write path.
- The outputs are registered, so users of this module must account for read latency explicitly.
