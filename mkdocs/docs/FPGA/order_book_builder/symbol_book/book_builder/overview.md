# book_builder
This module is the per-symbol order-table stage inside [symbol_book](../overview.md). It accepts parser events from [order_book_parser](../../../order_book_parser.md), stores the current state of each live order, and emits **compact quantity-delta messages** for the [bid-side and ask-side quantity books](../qty_book_wrapper/overview.md) at the same time.

## Structure

Below gives the inside schematic of book builder:

![book_builder_schematic](../../../../figures/FPGA/order_book_builder/hierarchy_book_builder.png)

## Design Logic

1. [fifo](../fifo.md) is necessary, as the operation cannot be done in one cycle.
2. It maintains an [order_book_inst](bram.md) inside, which is a [bram](../book_builder/bram.md) saving the live order books. It is a database for the building of the price level books.
3. When the incoming messages are being parsed, the state machine also translates the operations to [qty_builder](../qty_book_wrapper/qty_builder.md) inside [qty_book_wrapper](../qty_book_wrapper/overview.md) to update the best bid/ask.

## Interface
### Inputs
- `i_clk_156` is the main clock for the FIFO, state machine, and BRAM access.
- `i_rst` clears the order table control state and the current quantity-delta output registers.
- `i_stock_valid` gates events so only the configured symbol enters this `book_builder`.
- `i_parser_msg` is the packed parser payload. In this design it carries `{msg_valid, seq_num, msg_type, stock_locate, order_ref_num, new_order_ref_num, side, shares, price}`.
### Outputs
- `o_qty_msg` is a `131-bit` packed quantity update `{bid_ask[1:0], price[31:0], is_add, d_shares[31:0], seq_num[63:0]}`.

## Order Table
Each table entry stores:

- `valid`: whether the slot currently holds an order
- `side`: `BID` (`2'b01`) or `ASK` (`2'b10`)
- `shares`: current resting shares for that order
- `price`: current resting price

The address is not the raw `order_ref_num`. Instead, `cal_order_book_addr()` XOR-folds the 64-bit order reference into `BOOK_LEVEL_BIT` bits:
```verilog
for (i = 0; i < BOOK_LEVEL_BIT; i = i + 1) begin
    for (j = 0; j < 64; j = j + BOOK_LEVEL_BIT) begin
        if (i + j < 64) begin
            cal_order_book_addr[i] = cal_order_book_addr[i] ^ order_ref_num[i+j];
        end
    end
end
```
This keeps the table compact, but it also means **different order references can collide** and map to the same BRAM slot.

The function can be modified if the entropy of the higher bits is low.
## State Machine
The update path uses four states because the BRAM has synchronous read latency.

- `IDLE`: handle `A/F` immediately with one write, or start a read for `D/X/E/C/U`.
- `FIRST_CYCLE`: keep the read request active while waiting for the stored order to appear on `book_o_data`.
- `SECOND_CYCLE`: consume the stored order and perform delete, reduce, or the first phase of replace.
- `THIRD_CYCLE`: finish the second write of a `U` replace event.


In practice, add-order messages finish in one cycle, delete/reduce messages take three cycles, and replace messages take four cycles because they perform a remove followed by an add.
## Message Handling
### Add path
- `TYPE_A` and `TYPE_F` write `{1'b1, msg_side, msg_shares, msg_price}` into the order table at `cal_order_book_addr(msg_order_ref_num)`.
- The same cycle, `o_qty_msg` reports an **add** event with `qty_is_add = 1'b1`.

### Remove or reduce path
- `TYPE_D` reads the old order, clears the BRAM entry, and emits a **remove** event for the full resting shares.
- `TYPE_X`, `TYPE_E`, and `TYPE_C` read the old order, subtract `msg_shares`, rewrite the order entry, and emit a **remove** event for `msg_shares`.
- If the order is missing, or if `book_o_shares < msg_shares`, the module falls back to an idle output update.

### Replace path
- `TYPE_U` first reads and removes the old order referenced by `msg_order_ref_num`.
- One cycle later it writes a new entry under `msg_new_order_ref`.
- The new entry reuses the **old side** from `book_o_side`, together with the **new shares** and **new price** from the parser message.
- As a result, one `U` message generates two quantity updates on consecutive cycles: one remove, then one add.
## Quantity Output
The quantity stream is the only public output of this block, and it is exactly what the downstream quantity books need:

- `qty_bid_ask` selects bid or ask.
- `qty_price` selects the price level to update.
- `qty_is_add` distinguishes add from remove.
- `qty_d_shares` is the delta applied at that level.
- `qty_seq_num` carries the parser sequence number so downstream best-price snapshots can keep event ordering.

When no valid update is being emitted, `qty_bid_ask` falls back to `IDLE` (`2'b00`).
## Design Limits
- `msg_fifo_inst.o_push_ready` is not checked, so parser events can be dropped if the internal FIFO becomes full.
- `cal_order_book_addr()` is a **hash**, not a collision-free order-reference mapping.
- The default parameter `PARSER_MSG_BIT = 1+64+8+16+64+64+2+32+32+48` still includes an extra `+48`, but the implemented field slices only use bits up to `i_parser_msg[282]`.


