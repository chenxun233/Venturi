# arbiter
This module selects which [symbol_book](order_book_builder/symbol_book/overview.md) is allowed to output its next top-of-book event. It applies a **round-robin policy** across all symbol FIFOs so one active symbol does not permanently block the others.
## Introduction
The implementation is in `verilog_src/order_book_related/dependencies/arbiter.v`. It is instantiated in [order_book_builder](order_book_builder/overview.md) as `event_fifo_arbiter_inst`.

Each `symbol_book` has its own event FIFO. The arbiter looks at all of them in parallel and chooses one source to pop in the current cycle.
## Design Logic
The arbiter solves a scheduling problem:

1. many `symbol_book` instances can have pending events at the same time
2. only one payload should be forwarded to the shared output in one cycle
3. the selected source should rotate fairly over time

To do that, the module keeps a round-robin pointer `rr_pointer`. Each cycle, it scans the `i_src_not_empty` bitmap starting from that pointer and picks the first non-empty source it finds. After choosing one source, it advances the round-robin pointer to the next index after the chosen one.

This gives a simple fairness rule: after one source is served, the next search starts from the following source instead of always starting from source `0`.
## Interface
### Inputs
- `i_clk_156`: main clock.
- `i_rst`: active-high reset.
- `i_src_not_empty`: one bit per source FIFO, indicating that the source has at least one queued event.
- `i_src_valid`: one bit per source FIFO, indicating that the source is returning valid pop data.
- `i_src_payload`: concatenated payload bus from all sources.
### Outputs
- `o_src_pop`: one-hot pop request back to the selected source.
- `o_valid`: valid flag for the selected output payload.
- `o_payload`: payload from the selected source.
## Round-Robin Scan Example
The combinational scan uses:

- `rr_pointer`: where the search starts
- `scan_idx`: current scanned source index
- `pointer`: winning source index for this cycle
- `found_nonempty`: whether the scan already found one eligible source

The helper function `wrap_idx()` implements circular indexing so the search can move past the last source and wrap back to `0`.

Example with `SYMBOL_NUM = 4` and `rr_pointer = 2`:

- scan order is `2 -> 3 -> 0 -> 1`
- if source `3` is the first non-empty entry in that order, it wins
- the next `rr_pointer` becomes `0`
## Pop Generation
The output `o_src_pop` is generated as a one-hot vector:

```verilog
assign o_src_pop[src_idx] = found_nonempty && (pointer == src_idx);
```

So only the chosen source is asked to pop in that cycle.
## Payload Selection
One subtle part of the design is that `o_valid` and `o_payload` are indexed by `latch_pointer`, not by the current combinational `pointer`:

```verilog
assign o_valid   = i_src_valid[latch_pointer];
assign o_payload = i_src_payload[latch_pointer*PAYLOAD_W +: PAYLOAD_W];
```

This is needed because the source FIFO payload and valid bit are observed after the selected pop request path. The module first decides which source to pop, then latches that source index into `latch_pointer`, and uses the latched index to select the returned payload.

So:

1. combinational logic chooses which FIFO to pop now
2. sequential logic stores that choice in `latch_pointer`
3. output muxing uses `latch_pointer` to forward the selected payload
## Pointer Update
On each cycle where a non-empty source is found:

- `latch_pointer <= pointer`
- `rr_pointer <= pointer + 1`, with wraparound

If the selected source is the last index, `rr_pointer` wraps back to zero.

If no source is non-empty, the pointers hold their previous values.
## Reset Behavior
On reset, both pointers are cleared:

- `rr_pointer = 0`
- `latch_pointer = 0`

So after reset, the first scan starts from source `0`.
## How It Is Used In order_book_builder
In [order_book_builder](order_book_builder/overview.md), each `symbol_book` exports:

- `o_not_empty`
- `o_valid`
- `o_payload`

These are packed into:

- `arb_src_not_empty`
- `arb_src_valid`
- `arb_src_payload`

The arbiter then decides which symbol FIFO to pop and forwards that symbol's top-of-book snapshot onto the shared `o_payload` output of `order_book_builder`.
## Limits
