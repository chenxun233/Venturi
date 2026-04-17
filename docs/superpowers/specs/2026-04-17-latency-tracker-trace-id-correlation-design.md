# Latency Tracker Trace ID Correlation Design

## Summary

Replace `LatencyTracker`'s node-based `std::unordered_map` correlation path with
a cache-friendly direct-indexed design based on a dense per-queue `trace_id`.

The dense `trace_id` exists only for first-event latency tracing. It is assigned
when the RX path sees the first traced event in a frame, then propagated through
the existing order and sender pipeline so that every latency stage belonging to
that traced event can be correlated by direct table lookup instead of hashing.

`event_tag` remains in the system and continues to be used for normal event
identity, business flow, and human-readable logs. This design does not replace
`event_tag` globally. It adds `trace_id` only to make latency tracking cheaper
and more predictable.

The pending-event storage inside `LatencyTracker` should use power-of-two
capacity so index wrap is done with masking instead of modulo.

## Goals

- remove `std::unordered_map` from `LatencyTracker`'s hot correlation path
- use a dense per-queue `trace_id` for traced first-event latency correlation
- keep `event_tag` for normal event identity and logging
- use preallocated contiguous storage with power-of-two capacity
- make wrap and slot lookup fast with bit masking
- preserve existing latency stage semantics
- preserve existing polling pattern and sender/runtime behavior
- preserve existing final latency summary output contract

## Non-Goals

- no changes to which latency stages are measured
- no changes to percentile reporting shape
- no changes to trading logic or order generation logic
- no changes to sender polling or batching pattern
- no attempt in this design to sample latency instead of tracing every first
  event
- no redesign of the logging/reporting subsystem beyond carrying the new
  correlation field where needed

## Current Problem

`LatencyTracker` currently correlates multi-stage latency records by
`(que_idx, event_tag)` using:

- `std::unordered_map<EventKey, PendingEventState> m_pending_records`
- `std::unordered_map<StageKey, LatencyStats> m_latency_stats`

That creates several performance problems:

1. node-based hash map storage has poor cache locality
2. lookups and inserts require hashing and pointer chasing
3. the pending-event path performs this work for every traced stage
4. under offered load, the tracker falls behind and the input queue fills
5. once the queue fills, new records are dropped before they can be correlated

This was observed indirectly through latency summary sample counts such as:

- `24 = 1024 - 1000`
- `924 = 1024 - 100`

Those counts matched input queue capacity minus warmup, showing that the
measurement path was bottlenecking before the replay workload was exhausted.

## Selected Approach

Introduce a dense per-queue `trace_id` and use it as the correlation key only
for traced first-event latency flow.

The high-level model is:

1. RX assigns a per-queue dense `trace_id` when it emits a traced first event
2. that `trace_id` is carried through the order/sender path together with the
   existing `event_tag`
3. every `TimeRecord` for traced first-event latency stages carries this
   `trace_id`
4. `LatencyTracker` uses `trace_id` plus queue to index directly into a
   preallocated per-queue pending table
5. the pending table uses power-of-two capacity and mask-based wrap
6. final latency output still prints `event_tag` for readability

This keeps external latency semantics intact while removing the hash-map
correlation cost from the hot path.

## Why `trace_id` Instead Of Hashing `event_tag`

Two viable directions were considered:

1. keep `event_tag` and replace `std::unordered_map` with a vector-backed manual
   hash table
2. introduce a dense propagated `trace_id` and use direct indexing

The dense `trace_id` approach was selected because:

- it removes hash and probing work from the pending-event lookup path
- it gives the most predictable cost
- it has the best cache locality
- it fits HFT-style latency tracking better than generic hashmap-style lookup

The main trade-off is that `trace_id` must be propagated through the existing
pipeline. That is a larger change than a vector-backed manual hash table, but it
produces the better runtime design.

## Trace ID Model

### Scope

`trace_id` is only for tracing the first event in a frame.

This means:

- only first-event traced records receive a valid `trace_id`
- non-traced or non-first events do not rely on `trace_id`
- `event_tag` remains the general-purpose event identity field everywhere

### Assignment

Each queue owns a local dense counter:

- queue 0 has its own counter
- queue 1 has its own counter
- counters advance independently

When RX decodes a traced first event, it assigns the next queue-local
`trace_id`.

### Propagation

The assigned `trace_id` is propagated through the same path that already carries
`event_tag` for traced first events:

- `FPGAEventDesc`
- `OrderIntent`
- `OrderExecution`
- `TxOutboundRecord`
- `TimeRecord`
- `LatencyLogRecord` if needed for diagnostics or future inspection

The correlation identity for the tracker becomes:

- queue-local `trace_id` for slot lookup
- `event_tag` retained as metadata

## Pending Table Design

### Storage Shape

Replace `m_pending_records` with one pending table per queue using contiguous
preallocated storage, conceptually:

```cpp
struct PendingTraceSlot {
    bool occupied;
    uint32_t trace_id;
    uint64_t event_tag;
    PendingEventState state;
};
```

and then:

```cpp
std::vector<std::vector<PendingTraceSlot>> m_pending_trace_tables;
```

### Capacity

Each queue's pending table capacity must be a power of two.

Required property:

- `capacity != 0`
- `(capacity & (capacity - 1)) == 0`

### Indexing

Use mask-based wrap:

```cpp
slot_idx = trace_id & (capacity - 1);
```

This is the chosen wrap mechanism because it is faster and more predictable than
modulo.

### Collision / Reuse Rule

Because `trace_id` is dense and queue-local, the tracker can use direct indexed
reuse as long as the number of concurrently outstanding traced events per queue
does not exceed table capacity.

If a new traced event maps to an occupied slot that still belongs to an older
unfinished `trace_id`, the tracker must treat that as overflow / overwrite risk
and count a drop rather than silently corrupting correlation.

This design therefore requires:

- a large enough pending-table capacity for the maximum outstanding traced
  first-event window
- explicit drop accounting when a slot cannot be safely reused

## Stats Storage Design

Replace `m_latency_stats` with direct indexed storage as well.

There is no need to hash `(queue, prev_stage, curr_stage)` because the stage
space is small and bounded.

A practical layout is:

```cpp
std::vector<std::array<std::array<LatencyStats, kStageCount>, kStageCount>> m_latency_stats;
```

or an equivalent flattened vector indexed by:

- queue
- previous stage
- current stage

This makes stats updates constant-time without heap-based hash-map lookup.

## Data Flow Changes

### RX Engine

RX becomes the owner of initial `trace_id` assignment for traced first events.

When RX identifies a first event that should be latency-traced:

1. allocate the next queue-local `trace_id`
2. store it into the decoded event object
3. use it in the emitted `TimeRecord`s for:
   - `FRAME_START`
   - `DMA_EMIT`
   - `BATCH_START`
   - later `BATCH_END` record associated with that event

### Strategy

When strategy emits `STRATEGY_START`, it should carry forward the event's
existing `trace_id` for traced first events.

### Executor

`OrderIntent` and `OrderExecution` must carry `trace_id` in addition to
`event_tag`.

### Sender

`TxOutboundRecord` must also carry `trace_id` so sender-side latency stages can
continue the same traced event:

- `TX_EXECUTION_ACCEPTED`
- `TX_ENQUEUE`
- `TX_SEND_ENTER`
- `TX_SEND_SYSCALL_ENTER`
- `TX_SEND`

### Latency Tracker

`TimeRecord` must include `trace_id`.

`LatencyTracker` should:

1. use `trace_id` to select the per-queue pending slot
2. validate that the slot's stored `trace_id` matches the incoming record
3. update the pending state directly
4. emit final `LatencyLogRecord` with the original `event_tag` preserved

## Correlation Semantics

The correlation rule becomes:

- `trace_id` is the tracker lookup key
- `event_tag` is descriptive metadata

This means the tracker should no longer search by `event_tag`.

However, to prevent silent corruption, the pending slot should still retain the
current `event_tag` and `trace_id` so debug assertions or defensive checks can
confirm that the propagated identity remains coherent.

## Error Handling And Drop Semantics

This design must make tracker failure modes explicit.

Required drop conditions include:

1. input latency queue full
2. pending table slot collision with an unfinished older trace
3. stage arrival for an unknown or invalid `trace_id`
4. stage ordering violation within a pending slot

All such conditions should increment drop counters in direct indexed stats
storage instead of failing silently.

This is important because the existing queue-capacity bottleneck was exposed
only indirectly through suspicious summary counts.

## File-Level Impact

Expected files impacted:

- `cpp_src/FPGA_boost_demo/common/shared_types.h`
  add `trace_id` to latency-carrying records and payload structs
- `cpp_src/FPGA_boost_demo/rx_engine/fpga_rx_engine.cpp`
  assign per-queue `trace_id`
- `cpp_src/FPGA_boost_demo/strategy/dummy_strategy.cpp`
  propagate `trace_id`
- `cpp_src/FPGA_boost_demo/tx_engine/executor.*`
  propagate `trace_id`
- `cpp_src/FPGA_boost_demo/tx_engine/tx_sender.cpp`
  propagate `trace_id`
- `cpp_src/FPGA_boost_demo/latency/latency_tracker.h`
  replace hash-map members with direct indexed storage
- `cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp`
  replace hash-based correlation logic with direct indexed correlation
- tests that construct or assert latency-carrying structs

## Testing Strategy

Testing should prove both correctness and the new correlation model.

Required coverage:

1. tracker still emits the same stage latencies for a valid traced event
2. traced first-event records correlate correctly using `trace_id`
3. `event_tag` remains preserved in final emitted latency logs
4. per-queue `trace_id` assignment is independent
5. power-of-two pending-table capacity is enforced
6. slot reuse after a completed trace works correctly
7. unfinished-slot collision increments drops instead of corrupting state
8. existing percentile/final-summary behavior remains compatible

## Success Criteria

This design is successful if:

1. `LatencyTracker` no longer uses `std::unordered_map` for pending-event or
   stats storage
2. traced first-event correlation is driven by propagated queue-local `trace_id`
3. `event_tag` remains available in emitted latency logs
4. pending-table indexing uses power-of-two capacity with mask-based wrap
5. latency stage semantics remain unchanged
6. existing runtime behavior outside correlation storage is unchanged
7. post-change measurement can sustain materially more traced samples before the
   tracker becomes the bottleneck

