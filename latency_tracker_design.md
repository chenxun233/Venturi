# LatencyTracker Design Notes

This file records the intended design direction for `LatencyTracker` before implementation changes.

## Agreed Simplifications

1. Remove `m_tracked_event_tables`

- `LatencyTracker` should not keep a second table that decides whether later stages may be emitted.
- The tracker should remain a passive latency-tracking module, not a gatekeeper for stage emission.
- `m_pending_tables` should remain the live in-progress tracking state.

2. Propagate `trace_id` end-to-end and use it as the only latency correlation key

- `trace_id` should be carried through the runtime path, similar to how `event_id` was handled before.
- `trace_id` becomes the primary and only latency correlation key after RX.
- `event_tag` may still remain in structs as metadata, but latency correlation should no longer depend on it.
- Downstream modules should preserve `trace_id` and use `trace_id != 0` to identify traced flows.

## Current Design Conclusions

### 1. Scope And Purpose

`LatencyTracker` remains a live runtime latency module for latency testing.

It is not intended to participate in pressure testing.

Its purpose is:
- collect stage timing records for traced flows
- compute full-path adjacent-stage latency
- produce a runtime summary/report for latency tests

It should not be treated as a production-grade pressure-test module.

### 2. Keep Printed Reporting

For the demo workflow, latency reporting should remain available at runtime and be printed at the end of a latency test.

The current `LatencyTracker -> LogPrinter` integration can remain for now.

There is no requirement to finish `Quill` / `spdlog` integration in this demo project, but the design should avoid blocking future replacement of `LogPrinter` with another sink.

### 3. Simplify Tracking Model

The current `LatencyTracker` design is more complicated than necessary.

The main simplification direction is:
- remove `m_tracked_event_tables`
- propagate `trace_id` end-to-end
- use `trace_id` as the only latency correlation key
- keep runtime tracking passive

`LatencyTracker` should not act as a gatekeeper deciding whether later stages may be emitted.

### 4. Stage Emission Policy

Stage emission should move toward a uniform rule:
- relevant trace stages are pushed immediately when they occur
- no stage should be pushed unless `trace_id != 0`

The current mixed model, where some records are pushed immediately and some are pushed later after a batch, makes runtime tracking harder to reason about.

The design goal is:
- one canonical push point per stage
- timestamps mean "this stage happened now"
- stage emission sites must use the same gating rule: `trace_id != 0`

### 5. Sampling Rule For Demo Simplicity

To simplify the live runtime tracker, tracing is restricted by two rules:
- only events with `is_first_event != 0` are eligible to start tracing
- per queue, only one traced event may be active at a time

This means:
- a first event may receive a non-zero `trace_id` only if that queue has no active traced event in flight
- if a queue already has an active traced event, later first events receive `trace_id = 0`
- only the event that receives a non-zero `trace_id` emits latency records through the full path
- untraced events do not participate in latency tracking

This is a deliberate sampling design for the demo latency test, not a general-purpose full-observability design.

### 6. Trace Start Rule

Trace start is not runtime-configurable by batch-local event index.

The rule is fixed:
- RX checks `is_first_event`
- if `is_first_event == 0`, the event is never traced
- if `is_first_event != 0` and the queue has no active traced event, RX allocates a new non-zero `trace_id`
- if `is_first_event != 0` but the queue already has an active traced event, RX assigns `trace_id = 0`

This remains valid even if a batch contains multiple first events:
- the first eligible one may start tracing
- the rest remain untraced until the active trace for that queue finishes

`trace_id` assignment rules:
- `trace_id` is monotonic
- it increases by 1 each time a new trace is actually started
- it is assigned only when a first event is accepted for tracing
- the value that increases and is assigned to `trace_id` is owned and stored inside `LatencyTracker`
- `trace_id` allocation must include wrap handling
- when the allocator wraps, it must skip `0` so that `trace_id == 0` remains the untraced sentinel value
- power-of-two wrap optimization applies to indexed containers or slot tables associated with tracing state, not to the scalar `trace_id` value itself

Decision mechanism:
- `LatencyTracker` owns the per-queue active-trace state and the monotonic `trace_id` allocator
- `FPGARxEngine` asks `LatencyTracker` whether the current first event may start a trace
- downstream modules do not make the tracing decision again; they only preserve the assigned `trace_id`

### 7. Effect On Runtime State

If only one first event may be active per queue at a time, the tracker design can be simplified significantly.

The design intent is to avoid:
- a large mixed pending-state bag
- split-brain control structures
- unnecessary per-trace complexity

The runtime model should be reduced to the minimum state needed for the chosen traced event flow and final report generation.

Additional runtime restriction:
- for each queue, `LatencyTracker` allows at most one active traced event at a time across batches
- a queue cannot start tracing a new first event while its previous traced event has not yet reached the terminal tracked stage

Per-queue active-trace mechanism:
- `LatencyTracker` owns per-queue active-trace state
- this state indicates whether the queue currently has an active traced event in flight
- if a queue already has an active traced event, RX must not assign a new non-zero `trace_id` to a later first event for that queue
- only when the current traced event reaches the terminal tracked stage and is finalized may the queue allocate a new `trace_id` for a later batch

Consequence:
- some first events may be skipped for tracing if the previous traced event for that queue is still in flight
- this is an intentional simplification for the demo design

### 7A. Queue-Scan Calculation Rule

Completed-sample calculation is a one-pass consume-and-calculate loop.

The rule is:
- `TxSender` reaches the terminal stage and triggers calculation with the current queue and `trace_id`
- `LatencyTracker` consumes records from the front of that queue
- records with different `trace_id` are discarded first
- once mismatched front records are removed, calculation starts immediately
- each popped record is checked against the next expected stage
- if the stage is correct, the corresponding latency step is calculated immediately
- if the stage is wrong, calculation stops immediately and the sample is dropped
- after such a failure, the following remaining records for that unfinished trace are also dropped
- only if the full stage sequence is consumed in order is one completed `LatencyLogRecord` emitted

This means:
- there is no separate "validate first, calculate later" pass
- validation and calculation happen in the same pop loop
- scanned records are removed from the queue in both success and failure cases

### 8. Latency Test vs Pressure Test

Two different tests are recognized:

- pressure test
  - focuses on throughput, backlog, drop, saturation, and stability
  - `LatencyTracker` is not intended to participate

- latency test
  - focuses on timing behavior for traced flows
  - `LatencyTracker` is active here
  - printed summary/report remains useful

This separation means `LatencyTracker` does not need to be optimized as if it were part of maximum-throughput pressure testing.

### 9. Latency Analysis Model

Latency analysis is separated conceptually from trace construction, but it does not need its own runtime thread in this demo design.

Design choice:
- `LatencyTracker` builds completed latency results for traced events
- completed latency results are stored for later exact analysis
- percentile and summary calculation is deferred until `Ctrl+C`

Percentile model:
- use exact calculation, not approximate streaming percentiles
- store all completed samples needed for each latency metric
- when the user presses `Ctrl+C`, compute summary statistics once
- summary statistics include:
  - min
  - max
  - p50
  - p90
  - p99

Threading consequence:
- no separate `LatencyAnalyzer` thread is required
- storing completed latency results during runtime in the same latency-processing thread is acceptable for this demo
- final statistics are calculated only at shutdown / stop time

Reporting consequence:
- `LogPrinter` prints the final report after the deferred exact summary calculation is completed
