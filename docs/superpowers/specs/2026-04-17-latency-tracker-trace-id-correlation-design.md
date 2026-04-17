# Latency Tracker Trace ID Correlation Design

## Summary

Tighten `LatencyTracker` correlation without changing the runtime logic or the
business/data wiring of the existing pipeline.

This change is intentionally narrow:

1. add a monotonic `trace_id` field to `TimeRecord`
2. assign `trace_id` only to the first-event trace flow of a frame
3. stop forcing `event_tag = 0` for non-first events
4. treat `trace_id != 0` as the first-event tracing marker
5. replace the two `std::unordered_map` containers inside `LatencyTracker`

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
- replace `LatencyTracker`'s two `std::unordered_map`s with cache-friendlier
  preallocated contiguous storage
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

`LatencyTracker` currently relies on two node-based hash maps:

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
5. `LatencyTracker` replaces both internal `std::unordered_map`s with
   preallocated contiguous storage

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

### Pending Event Correlation

Replace `m_pending_records` with contiguous preallocated storage.

The tracker still correlates by existing event identity semantics, but it should
do so without `std::unordered_map`.

A practical shape is a vector-backed fixed table / open-addressing structure for
pending event state. The important requirement is:

- contiguous preallocated storage
- no node-based hash map
- explicit power-of-two capacity

Conceptually:

```cpp
struct PendingTraceSlot {
    bool occupied;
    uint16_t que_idx;
    uint64_t event_tag;
    uint32_t trace_id;
    PendingEventState state;
};
```

plus one or more vector-backed tables with power-of-two capacity.

### Stats Storage

Replace `m_latency_stats` with direct indexed storage.

There is no reason to hash `(queue, prev_stage, curr_stage)` because that key
space is small and bounded.

A direct indexed vector/array layout should be used instead.

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

### Strategy / Executor / Sender

No wire redesign is allowed.

So these modules should remain structurally the same:

- do not add `trace_id` to business/order structs
- do not redesign their interfaces

The only required behavioral adjustment is:

- stop using `event_tag == 0` as the first-event marker
- use `trace_id != 0` in `TimeRecord` where first-event tracing decisions are
  needed

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
2. pending-table insertion/reuse failure
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
  replace the two `std::unordered_map` members
- `cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp`
  replace hash-map-based correlation/stats storage with contiguous storage
- tests that assert latency-tracing identity behavior

## Testing Strategy

Required coverage:

1. `TimeRecord` carries `trace_id`
2. first-event records get non-zero `trace_id`
3. non-first events keep `event_tag`
4. first-event checks use `trace_id != 0`
5. tracker still emits the same stage-latency results as before
6. power-of-two capacity is enforced for the new tracker storage
7. tracker stats updates still behave correctly without `std::unordered_map`

## Success Criteria

This design is successful if:

1. no runtime logic or pipeline wiring is redesigned
2. `TimeRecord` has a monotonic first-event `trace_id`
3. non-first events no longer zero `event_tag`
4. first-event tracing checks use `trace_id != 0`
5. both `std::unordered_map`s in `LatencyTracker` are replaced
6. new tracker storage uses power-of-two-capacity-friendly indexing where wrap
   matters
7. existing latency semantics and reporting behavior remain intact

