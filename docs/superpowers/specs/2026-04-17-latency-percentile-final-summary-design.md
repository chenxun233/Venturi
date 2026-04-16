# Latency Percentile Final Summary Design

## Summary

Replace per-event latency printing with one final latency summary per queue,
printed only at shutdown.

Each queue summary should contain one line per latency stage currently present
in `LatencyLogRecord`, and each line should report:

- sample count
- minimum
- p50
- p99
- maximum

Warmup should be record-based, not time-based. Each queue ignores its first
configured number of latency records, and only post-warmup samples participate
in percentile aggregation.

This change is about reporting only. It must not change latency stage
generation, latency record semantics, sender behavior, or queueing behavior.

## Goals

- stop printing one latency block per event
- print one final latency summary per queue
- keep one line per existing latency field/stage
- report `count / min / p50 / p99 / max` for each line
- use record-count-based warmup per queue
- print only post-warmup results
- print the final summary only when the runtime shuts down cleanly
- keep `LatencyLogRecord` production unchanged

## Non-Goals

- no new latency stages
- no rename of existing latency fields
- no changes to `LatencyTracker` stage semantics
- no changes to `TxSender`, `Executor`, or RX hot-path behavior
- no periodic percentile printing
- no rolling window statistics
- no time-based warmup

## Current Problem

`LogPrinter` currently prints one full `LatencyNs ...` block for every
`LatencyLogRecord`.

That creates two problems for percentile measurement:

1. stdout is flooded with per-event records instead of final benchmark results
2. there is no built-in p50/p99 reporting, so the user has to manually inspect
   or post-process the output

For the current measurement workflow, the desired output is not event-by-event
visibility but one final benchmark summary.

## Selected Approach

Aggregate latency samples inside `LogPrinter`.

`LatencyTracker` should keep pushing the same `LatencyLogRecord` values it
produces today. `LogPrinter` should stop printing each record immediately and
instead:

1. receive each `LatencyLogRecord`
2. apply per-queue warmup filtering
3. append post-warmup values into per-queue/per-stage sample storage
4. print one final summary block per queue during `LogPrinter::stop()`

This keeps the change local to the reporting component and avoids changing the
producer-side latency contract.

## Rejected Approaches

### 1. Aggregate inside `LatencyTracker`

Rejected because it would blur the responsibility boundary. `LatencyTracker`
produces latency records; `LogPrinter` owns output. Since the requested change
is about output format, `LogPrinter` is the correct integration point.

### 2. Post-process stdout externally

Rejected because the user explicitly wants the runtime to print final results
directly instead of printing each latency record.

### 3. Print periodic summaries

Rejected because it introduces extra behavioral choices:

- print interval
- cumulative vs reset behavior
- partial-run interpretation

The requested output is final result only.

### 4. Time-based warmup

Rejected because the user chose record-based ignore semantics. Warmup should
track traffic volume, not wall-clock duration.

## Reporting Shape

For each queue, print one final latency summary block.

Within each queue block, print one line for each latency field currently carried
by `LatencyLogRecord`:

- `frame_start -> dma_emit_ns`
- `batch_duration_ns`
- `batch_end -> strategy_start_ns`
- `strategy_start -> tx_execution_accepted_ns`
- `tx_execution_accepted -> tx_enqueue_ns`
- `tx_enqueue -> tx_send_enter_ns`
- `tx_send_enter -> tx_send_syscall_enter_ns`
- `tx_send_syscall_enter -> tx_send_ns`

Each line should report:

- `count`
- `min`
- `p50`
- `p99`
- `max`

Backlog and send-loop counters should not be folded into this percentile report
for now. This design is limited to the latency duration fields already printed
as latency lines.

## Warmup Model

Warmup is per queue and record-based.

Required behavior:

1. each queue has a configured warmup record count
2. the first `N` `LatencyLogRecord`s for that queue are ignored for percentile
   aggregation
3. once warmup is exhausted for a queue, subsequent latency values for that
   queue are recorded
4. queues warm up independently

This means queue 0 and queue 1 may start collecting “real” samples at different
record counts if their traffic rates differ.

If a queue never receives any post-warmup samples, the final summary must print
a clear “insufficient post-warmup samples” message for that queue rather than
fake percentiles.

## Shutdown / Print Timing

`LogPrinter` should print the latency percentile summary only during clean
shutdown.

The intended lifecycle is:

1. runtime runs normally
2. `LogPrinter` aggregates latency samples during execution
3. runtime receives a shutdown trigger such as `SIGINT`
4. threads exit
5. `main()` calls `log_printer.stop()`
6. `LogPrinter::stop()` joins the worker and prints final queue summaries

No latency summary should be printed periodically during runtime.

## Data Storage

`LogPrinter` needs per-queue/per-stage sample storage.

A practical representation is one vector of samples for each queue and each
latency field, for example conceptually:

```cpp
struct LatencyFieldSamples {
    std::vector<int64_t> samples;
};
```

and then one per queue per field.

This design explicitly favors measurement simplicity over strict bounded-memory
behavior, because benchmark runs are finite and the requested output is final
percentiles. If storage pressure later becomes a problem, that can be handled
in a separate design.

## Percentile Semantics

Use exact percentile calculation from the collected post-warmup samples.

Recommended behavior:

- sort the sample vector at final print time
- compute:
  - `min` from first element
  - `max` from last element
  - `p50` from the median index
  - `p99` from the 99th percentile index

The exact indexing rule must be implemented consistently across all stage lines.
For this design, consistency matters more than matching any external statistics
package exactly.

## `LogPrinter` Changes

Files:

- `cpp_src/FPGA_boost_demo/latency/log_printer.h`
- `cpp_src/FPGA_boost_demo/latency/log_printer.cpp`

Required changes:

- add configuration for per-queue warmup record count
- add per-queue counters tracking how many latency records have been ignored for
  warmup
- add per-queue/per-stage sample storage
- stop printing each latency record in `_printLatencyRecord(...)`
- add final summary printing on `stop()`

`pushLatencyLog(...)` should remain the producer-facing latency API.

## `Venturi` Changes

File:

- `cpp_src/FPGA_boost_demo/app/Venturi.cpp`

Required changes:

- add a clean shutdown path so `log_printer.stop()` can actually be reached
- configure `LogPrinter` warmup count
- make the runtime exit worker loops on `SIGINT` / `Ctrl+C`

This is the minimal runtime change needed to support final-only reporting.

## Testing

Focused tests should confirm:

1. `LogPrinter` no longer prints one `LatencyNs ...` block per pushed latency
   record
2. `LogPrinter` prints one final summary per queue on shutdown
3. each summary line includes:
   - `count`
   - `min`
   - `p50`
   - `p99`
   - `max`
4. warmup ignores the first configured number of latency records per queue
5. only post-warmup samples contribute to summary statistics
6. queues with no post-warmup samples print an explicit insufficient-sample
   message

## Stop Condition

This change is complete when:

1. per-event latency printing is removed from runtime output
2. `LogPrinter` aggregates latency samples per queue and per stage
3. warmup is record-based and applied independently per queue
4. final output prints one summary per queue
5. each latency stage line prints `count / min / p50 / p99 / max`
6. the summary prints only at clean shutdown
7. latency record generation and stage semantics remain unchanged
