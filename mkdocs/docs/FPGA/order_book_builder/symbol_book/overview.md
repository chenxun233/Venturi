# symbol_book

This module is the key to realize multiple symbol order book building. Each `symbol_book` only manages one symbol. Inside, it contains:

1. one [book_builder](../symbol_book/book_builder.md)
2. one [ask_wrapper_inst](../symbol_book/qty_book_wrapper/overview.md)
3. one [bid_wrapper_inst](../symbol_book/qty_book_wrapper/overview.md)
4. one [event_fifo_inst](../symbol_book/fifo.md)

[book_builder](../symbol_book/book_builder.md) is for the building of the order book itself, while [ask_wrapper_inst](../symbol_book/qty_book_wrapper/overview.md) and [bid_wrapper_inst](../symbol_book/qty_book_wrapper/overview.md) are instance of [qty_book_wrapper](../symbol_book/qty_book_wrapper/overview.md), which maintains a bid/ask level book and a tree structure. [event_fifo_inst](../symbol_book/fifo.md) presents here because there can be multiple `symbol_book`s in one [order_book_builder](../../order_book_builder/overview.md), an [arbiter](../../arbiter.md) is used to schedule the output of `symbol_book`s. The output of `symbol_book` will be saved in to their [event_fifo_inst](../symbol_book/fifo.md) first, [arbiter](../../arbiter.md) decides which `symbol_book` gets popped out.
