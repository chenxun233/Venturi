# Latency Tracker Trace ID Correlation Design

## Summary

Tighten `LatencyTracker` correlation without changing the runtime logic or the
business/data wiring of the existing pipeline.

This change is intentionally narrow:

1. add a monotonic `trace_id` field to `TimeRecord`
2. assign `trace_id` only to the first-event trace flow of a frame
3. stop forcing `event_tag = 0` for non-first events
4. treat `trace_id != 0` as the first-event tracing marker
5. replace the two shared `std::unordered_map` containers inside
   `LatencyTracker` with per-path contiguous storage aligned with the existing
   latency queues

The key constraint is:

- no logic change
- no wire change

That means:

- do not redesign trading/order/sender behavior
- do not redesign polling or batching
- do not propagate new fields through order-intent / order-execution /
  outbound-wire structs
- do not replace `event_tag` as the normal event identity

`trace_id` is a latency-tracker-local tracing aid carried only in `TimeRecord`.
`event_tag` remains the correlation identity already flowing through the rest of
the system.

## Goals

- add `trace_id` to `TimeRecord`
- assign a monotonic first-event `trace_id` at frame entry
- use `trace_id != 0` to identify traced first-event flow
- preserve `event_tag` for all events, including non-first events
- replace `LatencyTracker`'s two shared `std::unordered_map`s with
  cache-friendlier preallocated contiguous storage aligned per path
- use power-of-two capacity where wrap/index masking matters
- keep latency stage semantics unchanged
- keep runtime logic and existing pipeline wiring unchanged

## Non-Goals

- no behavior change in strategy, executor, sender, or connection flow
- no change to polling pattern
- no change to order path structs just to carry `trace_id`
- no global replacement of `event_tag`
- no redesign of reporting / percentile summary
- no new latency stages
- no business-logic use of `trace_id`

## Current Problem

`LatencyTracker` currently relies on two shared node-based hash maps:

- `std::unordered_map<EventKey, PendingEventState> m_pending_records`
- `std::unordered_map<StageKey, LatencyStats> m_latency_stats`

Those maps hurt sustained tracking because they add:

1. hash computation
2. pointer chasing
3. poor cache locality
4. less predictable latency in the tracker thread

Under load, the tracker falls behind, its input queue fills, and records start
dropping before correlation completes.

## Selected Approach

Keep the existing event flow and event identity, but make the latency-tracking
path more explicit and cheaper.

The selected design is:

1. `TimeRecord` gets a `trace_id`
2. RX assigns a queue-local monotonic `trace_id` to first-event trace records
3. non-first events keep their existing `event_tag` instead of being zeroed
4. later code treats `trace_id != 0` as "this belongs to the traced first-event
   flow"
5. `LatencyTracker` keeps its existing `m_latency_queues`, drains them fairly,
   and replaces both shared internal `std::unordered_map`s with per-path
   preallocated contiguous storage aligned with those queues

This keeps the change boundary narrow:

- no wire/data-model redesign outside latency records
- no propagation of `trace_id` through normal order structs
- no change to how `event_tag` is used elsewhere

## Why This Approach

Two broad options were discussed:

1. bigger redesign: propagate a dense `trace_id` through the entire order path
2. narrow redesign: add `trace_id` only to `TimeRecord`, keep `event_tag` as-is,
   and replace the tracker's hash maps

The narrow redesign is selected because the user explicitly wants:

- no logic change
- no wire change

So this design improves the tracker without broad pipeline surgery.

## Trace ID Model

### Scope

`trace_id` exists only in `TimeRecord`.

It is not added to:

- `OrderIntent`
- `OrderExecution`
- `TxOutboundRecord`
- any network/business-facing struct

### Assignment Rule

Each queue owns a monotonic counter for first-event tracing.

When RX sees a traced first event:

- allocate the next queue-local `trace_id`
- write that `trace_id` into the emitted `TimeRecord`s belonging to that
  first-event trace flow

For non-first events:

- `trace_id` remains `0`
- `event_tag` remains populated and is no longer forced to `0`

### First-Event Detection Rule

The new rule is:

- `trace_id != 0` means first-event tracing flow
- `trace_id == 0` means not part of first-event tracing flow

This replaces using `event_tag == 0` as the first/non-first discriminator.

## Event Tag Rule

`event_tag` remains the normal event identity field.

Required change:

- stop making `event_tag` equal to `0` for non-first events

Reason:

- `event_tag` should remain available as the stable event identity across the
  existing pipeline
- first-event-ness is now represented by `trace_id`, not by zeroing `event_tag`

## LatencyTracker Container Replacement

`LatencyTracker` already has per-path ingress queues through
`m_latency_queues`, with one `SpscRingQueue<TimeRecord>` per path / record
buffer. That ingress shape stays.

The structural change is that pending correlation state and latency stats stop
being shared globally. They become per-path storage aligned with
`m_latency_queues`:

- `m_latency_queues[path_idx]` remains the ingress queue for that path
- pending correlation storage is split into one contiguous table per path
- latency stats storage is split into one direct-indexed table per path

`run()` must drain the existing ingress queues in a fair pattern, rather than
fully draining one path before servicing the next. Round-robin queue service is
the intended behavior so sustained traffic on one path cannot starve another
inside the tracker thread.

### Pending Event Correlation

Replace the shared `m_pending_records` with contiguous preallocated storage per
path.

The tracker still correlates by existing event identity semantics, but it should
do so without `std::unordered_map`.

A practical shape is a vector-backed fixed table / open-addressing structure for
pending event state, with one table per path. The important requirements are:

- contiguous preallocated storage
- no node-based hash map
- explicit constructor-provided power-of-two capacity
- per-path isolation aligned with `m_latency_queues`
- oldest-live-slot reuse when a path-local pending table is full

Conceptually:

```cpp
struct PendingTraceSlot {
    bool occupied;
    uint64_t event_tag;
    uint32_t trace_id;
    PendingEventState state;
};
```

plus a vector of per-path tables with power-of-two capacity.

Each path-local pending table uses open addressing with linear probing for
lookup. When a new `FRAME_START` arrives for a path and that path's table is
full, the tracker reuses the oldest live entry in that same path. The reuse must
be explicit and counted in tracker telemetry rather than happening silently.

### Stats Storage

Replace the shared `m_latency_stats` with direct indexed storage per path.

There is no reason to hash `(queue, prev_stage, curr_stage)` because that key
space is small and bounded.

A direct indexed vector/array layout should be used instead, with one stats
table per path aligned with `m_latency_queues`.

## Power-Of-Two Capacity Rule

Any tracker table that wraps by index must use power-of-two capacity.

Required property:

- `capacity != 0`
- `(capacity & (capacity - 1)) == 0`

Wrap should use masking rather than modulo whenever possible, for example:

```cpp
idx = value & (capacity - 1);
```

This is explicitly chosen because the user wants fast wrap behavior.

This applies to each path-local pending table and to any wrap-based auxiliary
structure used for explicit oldest-entry reuse.

## Data Flow Changes

### RX Engine

RX is the only place that assigns `trace_id`.

When RX emits first-event latency records such as:

- `FRAME_START`
- `DMA_EMIT`
- `BATCH_START`
- later `BATCH_END`

it should attach the queue-local monotonic `trace_id`.

For non-first events:

- `trace_id` stays `0`
- `event_tag` stays populated
- the record stays on the normal event-tag path instead of being identified by
  `event_tag == 0`

### Strategy / Executor / Sender

No wire redesign is allowed.

So these modules should remain structurally the same:

- do not add `trace_id` to business/order structs
- do not redesign their interfaces

The only required behavioral adjustment is:

- stop using `event_tag == 0` as the first-event marker
- use `trace_id != 0` in `TimeRecord` where first-event tracing decisions and
  path selection are needed

Where a module already emits a `TimeRecord`, it may populate `trace_id` only if
that value is already available without changing the surrounding wire model.
Otherwise the design stays with `event_tag` as existing metadata and does not
force broader struct plumbing.

## Correlation Semantics

This design does **not** globally replace `event_tag`.

Instead:

- `event_tag` remains the event identity used by the existing path
- `trace_id` becomes the explicit first-event tracing marker in `TimeRecord`

This is an important distinction:

- `trace_id` answers "is this record part of the traced first-event flow?"
- `event_tag` answers "which event is this?"

## Error Handling And Drop Semantics

This design should also make drop/failure conditions more explicit.

At minimum, the tracker should have explicit accounting for:

1. input queue full
2. pending-table eviction / reuse due to full capacity
3. missing/invalid pending state during stage processing
4. stage ordering violation

These conditions must not silently disappear behind the old container behavior.

## File-Level Impact

Expected files impacted:

- `cpp_src/FPGA_boost_demo/common/shared_types.h`
  add `trace_id` to `TimeRecord`
- `cpp_src/FPGA_boost_demo/rx_engine/fpga_rx_engine.cpp`
  assign queue-local monotonic `trace_id` for first-event trace records
- places that currently zero `event_tag` for non-first events
  stop doing that
- `cpp_src/FPGA_boost_demo/latency/latency_tracker.h`
  replace the two shared `std::unordered_map` members with per-path contiguous
  storage aligned with `m_latency_queues`
- `cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp`
  replace shared hash-map-based correlation/stats storage with path-local
  contiguous storage and fair queue draining
- tests that assert latency-tracing identity behavior

## Testing Strategy

Required coverage:

1. `TimeRecord` carries `trace_id`
2. first-event records get non-zero `trace_id`
3. non-first events keep `event_tag`
4. first-event checks use `trace_id != 0`
5. tracker drains the existing latency queues in a fair pattern
6. pending correlation and stats storage are path-local and aligned with
   `m_latency_queues`
7. tracker still emits the same stage-latency results as before
8. power-of-two capacity is enforced for each path-local pending table
9. oldest-live-slot reuse is exercised and counted explicitly
10. tracker stats updates still behave correctly without `std::unordered_map`

## Success Criteria

This design is successful if:

1. no runtime logic or pipeline wiring is redesigned
2. `TimeRecord` has a monotonic first-event `trace_id`
3. non-first events no longer zero `event_tag`
4. first-event tracing checks use `trace_id != 0`
5. the existing `m_latency_queues` remain the ingress shape and `run()` drains
   them fairly
6. both shared `std::unordered_map`s in `LatencyTracker` are replaced by
   per-path contiguous storage aligned with `m_latency_queues`
7. new tracker storage uses power-of-two-capacity-friendly indexing where wrap
   matters
8. path-local pending tables reuse the oldest live entry when full and account
   for that reuse explicitly
9. existing latency semantics and reporting behavior remain intact
