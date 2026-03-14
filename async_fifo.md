# async_fifo

This note documents the CDC strategy used in [verilog_src/async_fifo.v](/home/chenxun/Documents/Project/Venturi/verilog_src/async_fifo.v).

## Binary pointers vs Gray-code pointers

The FIFO keeps both binary and Gray-code versions of the read and write pointers.

- Binary pointers are used locally inside each clock domain.
- Gray-code pointers are used only for crossing clock domains.

Binary pointers are still needed because they are convenient for:

- incrementing the read and write positions
- indexing the memory array
- generating the next local pointer state

Gray-code pointers are needed because the opposite side must know where the local pointer is when generating `full` and `empty`.

## When Gray code is needed

If neither clock domain needs to know the other side's pointer, and some external guarantee prevents overflow and underflow, then each side can operate with its own local binary pointer and no cross-domain pointer transfer is needed.

That is the narrow case where Gray code is not required.

Once either side needs visibility of the opposite-side pointer, the design is doing clock-domain crossing. In an async FIFO, this is required for status generation:

- the write side needs the read pointer to decide `full`
- the read side needs the write pointer to decide `empty`

## Why not synchronize binary pointers directly

A binary counter can change multiple bits in one increment. For example, `4'b0111` to `4'b1000` flips four bits at once.

If that multi-bit value is sampled asynchronously by the other clock domain, the destination can observe a mixed intermediate value and make a wrong `full` or `empty` decision.

Gray code avoids that problem because only one bit changes per increment:

```verilog
gray = (bin >> 1) ^ bin;
```

That does not remove metastability completely, but it makes cross-domain sampling of a multi-bit pointer practical and robust.

## What the synchronizer is doing

The pointer is not sampled only when it increments. Instead, the Gray-coded pointer is continuously passed into the opposite clock domain through a two-flop synchronizer.

In this FIFO:

- `rd_ptr_gray` is synchronized into the write clock domain
- `wr_ptr_gray` is synchronized into the read clock domain

The two cascaded registers do not "prove" the signal is stable. Their role is to reduce the probability that metastability propagates into the domain that uses the synchronized pointer.

## How `full` and `empty` are generated

The local side still computes its next pointer in binary, converts it to Gray code, and compares it against the synchronized Gray pointer from the opposite side.

- `o_rd_empty` is based on whether the next read pointer matches the synchronized write pointer
- `o_wr_full` is based on whether the next write pointer reaches the Gray-code full condition derived from the synchronized read pointer

So the correct mental model is:

1. Use binary pointers for local arithmetic and RAM addressing.
2. Convert the local pointer to Gray code.
3. Synchronize that Gray pointer into the opposite clock domain with two flops.
4. Use the synchronized Gray pointers to compute `full` and `empty`.

## Practical summary

- No cross-domain visibility needed: local binary pointers may be enough if overflow/underflow is prevented elsewhere.
- Cross-domain visibility needed: synchronize pointers across domains.
- For async FIFO status logic, synchronize Gray-coded pointers, not raw binary pointers.
