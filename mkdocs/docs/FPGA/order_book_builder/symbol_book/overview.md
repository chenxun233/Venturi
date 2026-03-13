# symbol_book

This module is the key to realize multiple symbol order book building. Each `symbol_book` only manages one symbol. Inside, it contains:

1. one [book_builder](book_builder/overview.md)
      - It maintains a live order book inside. It also sends operations and data to [ask_wrapper_inst](qty_book_wrapper/overview.md) and [bid_wrapper_inst](qty_book_wrapper/overview.md) to guide them how to operate the price level book.
      - The `stock_locate` signal in the incoming message will be checked before it inputs to [book_builder](book_builder/overview.md). Unmatched messages will be discarded.

2. one [ask_wrapper_inst](qty_book_wrapper/overview.md)
      - It maintains a live price level book on ask side. It also has a tree structure, which constantly updates the best price now. It outputs the best ask and the corresponding shares to the fifo once there is an event.

3. one [bid_wrapper_inst](qty_book_wrapper/overview.md)
      - Bid version of above

4. one [event_fifo_inst](fifo.md)
      - As there can be multiple `symbol_book`s in one [order_book_builder](../overview.md), an [arbiter](../../arbiter.md) is used to schedule the output of `symbol_book`s. The output of `symbol_book` will be saved in to their [event_fifo_inst](fifo.md) first, [arbiter](../../arbiter.md) decides which `symbol_book` gets popped out.

## Hierarchy

symbol_book is a little be complex, below gives the inside structure for better understanding.

![symbol_book structure](../../../figures/FPGA/order_book_builder/hierarchy_symbol_book.png)