# bram
This module is the single-port storage used by [book_builder](overview.md) as `order_book_inst`. It keeps the **current live order state** for one symbol, so later cancel, execute, delete, or replace messages can find the old side, shares, and price.
## Introduction
In `book_builder`, it is instantiated with `ADDR_WIDTH = BOOK_LEVEL_BIT` and `DATA_WIDTH = 67`, so each entry stores `{valid, side, shares, price}` for one hashed order reference.
## Design Logic
The module is intentionally small:

BRAM is a dedicated on-chip memory block provided by the FPGA fabric, separate from ordinary flip-flop registers and LUTs. It is used here because the order table is naturally a **memory array**: with `BOOK_LEVEL_BIT = 12`, the design needs `4096` (can adjust) addressable rows, and each row stores `67` bits of live-order state. Implementing that table with BRAM is much more area-efficient than building the same storage from registers.

The tradeoff is that read and write are **clocked operations**. In this module, a read result appears on `o_data` after the next rising edge, so [book_builder](overview.md) uses the `FIRST_CYCLE` and `SECOND_CYCLE` states before it consumes `book_o_data`.

## Interface
### Inputs
- `i_clk`: BRAM clock.
- `i_rst`: clears only the output register `o_data`.
- `i_addr`: row address.
- `i_op`: operation selector, where `2'b01` is read and `2'b10` is write.
- `i_data`: row payload to write.
### Outputs
- `o_data`: registered read data.
## Stored Format
Inside [book_builder](overview.md), the row width is `67` bits:
```verilog
{valid, side[1:0], shares[31:0], price[31:0]}
```
Example:
```verilog
{1'b1, 2'b01, 32'd100, 32'd200}
```
This means the slot is **valid**, the order is on the **bid** side, the resting quantity is `100`, and the price is `200`.
## Read And Write Behavior
- `WRITE`: on the rising edge, `bram[i_addr] <= i_data`.
- `READ`: on the rising edge, `o_data <= bram[i_addr]`.
- `IDLE`: memory and output keep their previous values unless reset changes `o_data`.

This is a **single-port BRAM**, so one cycle performs one logical operation chosen by `i_op`.
## Reset Behavior
Two reset-related details matter:

1. The `initial` block fills the memory array with zeros. This helps simulation and also gives FPGA tools a zero-initialized BRAM image when supported.
2. Runtime `i_rst` does **not** sweep through the whole memory and clear all rows. It only sets `o_data` to zero.

Because of that, [book_builder](overview.md) removes an order by explicitly writing an all-zero row back into the selected slot.
## Addressing
This BRAM does not know anything about `order_ref_num` directly. [book_builder](overview.md) computes the address first with `cal_order_book_addr()` and passes the folded result into `i_addr`.

That means the BRAM itself is just a storage array; the **hashing and collision risk** live outside this module.
## Synthesis Note
The array is declared with:
```verilog
(* ram_style = "block" *) reg [DATA_WIDTH-1:0] bram [0:(1<<ADDR_WIDTH)-1];
```
The attribute asks the synthesis tool to map this storage to **block RAM** instead of distributed LUT RAM when possible.
## Limits
- There is no byte enable or partial update path; every write replaces the full row.
- There is no explicit valid-ready handshake on the output; the caller must already understand the one-cycle read latency.
- The module does not detect address collisions. If two order references hash to the same `i_addr`, the later write overwrites the earlier row.
