# Invariant TSC Latency Tracker Design

## Goal

Reduce host-side timestamp capture overhead in the latency-tracking path by replacing syscall-based host timestamp capture with invariant-TSC reads, while keeping latency logs expressed in nanoseconds.

The key boundary for this change is:

- hot path captures raw host ticks only
- `LatencyTracker` converts host-host deltas from ticks to nanoseconds with integer math

## Scope

This change covers only:

- [`time_utils.h`](/home/chenxun/Documents/Project/Venturi/cpp_src/FPGA_boost_demo/common/time_utils.h)
- [`latency_tracker.h`](/home/chenxun/Documents/Project/Venturi/cpp_src/FPGA_boost_demo/latency/latency_tracker.h)
- [`latency_tracker.cpp`](/home/chenxun/Documents/Project/Venturi/cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp)
- only the tests required by this timing-source change

This change does not cover:

- sender, strategy, executor, RX engine, or transport behavior
- log format changes
- stage schema changes
- CPU affinity changes
- FPGA timestamp handling changes
- unrelated timing code outside this latency-tracking path

If any unrelated file appears to need adjustment, that is out of scope for this change.

## Current Problem

Host-side latency stages currently use `clock_gettime(CLOCK_MONOTONIC_RAW)` through `readMonotonicRawNs()`.

That has two problems for fine-grained telemetry:

- the syscall or vDSO-backed clock read is materially more expensive than a TSC read
- for small host-side deltas, the timing-source overhead and jitter contaminate the measurement

On this machine, the platform characteristics are suitable for a TSC-backed fast path:

- `constant_tsc`
- `nonstop_tsc`
- `rdtscp`
- current Linux clocksource is `tsc`

## Approved Approach

Keep the existing `readMonotonicRawNs()` interface, but change its meaning within the latency-tracking path:

- it returns raw host TSC ticks for host-captured stages
- it no longer converts ticks to nanoseconds on the hot path

`LatencyTracker` becomes the only place that converts host-side captured deltas from ticks to nanoseconds.

This keeps the hot path cheap while preserving nanosecond output in logs and tests.

## Timestamp Domains

There are two distinct timestamp domains in the current latency flow:

1. FPGA-derived timestamps
   - `FRAME_START`
   - `DMA_EMIT`

2. Host-captured timestamps
   - `BATCH_START`
   - `BATCH_END`
   - `STRATEGY_START`
   - `TX_EXECUTION_ACCEPTED`
   - `TX_ENQUEUE`
   - `TX_SEND`

This change applies only to the host-captured domain.

The FPGA-derived path remains unchanged, including the existing `frame_start -> dma_emit_ns` computation.

## Hot-Path Capture Rules

All current host-side call sites that use `readMonotonicRawNs()` for latency tracking continue to call the same function, but the function now returns raw invariant-TSC ticks.

The hot path must not perform:

- tick-to-ns floating-point conversion
- tick-to-ns integer conversion
- extra calibration work per timestamp capture

The hot path should only do:

- one TSC read
- return the raw tick value

## TSC Read Method

Use an invariant-TSC read based on `RDTSCP`.

Requirements:

- use integer-only arithmetic for later conversion
- keep the read helper inline
- avoid changing call sites outside the existing `readMonotonicRawNs()` interface

`RDTSCP` is preferred over bare `RDTSC` because it provides a stronger ordering point for telemetry capture.

## Calibration And Conversion

One fixed host tick-to-nanosecond scale is required.

The implementation should:

1. initialize a fixed TSC frequency once
2. store that calibration in a form usable by integer math
3. let `LatencyTracker` convert host-host tick deltas to nanoseconds

Preferred calibration source:

- a fixed kernel-exposed TSC frequency if available

Fallback:

- one-time calibration against `CLOCK_MONOTONIC_RAW`

After initialization, no hot-path call site may perform calibration again.

## LatencyTracker Responsibilities

`LatencyTracker` should treat host-captured timestamps as raw host ticks and convert only when deriving a host-host latency value.

Examples:

- `batch_duration_ns`
- `batch_end_to_strategy_start_ns`
- `strategy_start_to_tx_execution_accepted_ns`
- `tx_execution_accepted_to_tx_enqueue_ns`
- `tx_enqueue_to_tx_send_ns`

The conversion should happen at the point where the tracker currently derives each signed delta.

The tracker should keep emitting latency logs in nanoseconds exactly as it does today.

## Mixed-Domain Handling

No mixed-domain conversion logic should be introduced beyond what already exists today.

Specifically:

- `FRAME_START` and `DMA_EMIT` stay FPGA-derived
- host-side stages stay host-derived
- `frame_start -> dma_emit_ns` remains unchanged
- only host-host intervals are converted from TSC ticks to nanoseconds in `LatencyTracker`

This avoids broadening the change into a larger timestamp-schema redesign.

## Fallback Behavior

If invariant-TSC initialization cannot be completed safely, the implementation may fall back to the existing `CLOCK_MONOTONIC_RAW` path behind the same interface.

That fallback must:

- preserve current behavior
- require no call-site changes
- stay contained within the timing utility layer

## Testing

Focused tests should confirm:

1. host-side latency records still produce nanosecond outputs
2. `LatencyTracker` converts host-host tick deltas correctly with integer math
3. existing latency log expectations remain semantically unchanged
4. no unrelated stage flow or log schema changes were introduced

Tests should stay narrowly scoped to this timing-source change.

## Stop Condition

This change is complete when:

1. host-side latency-tracking timestamps are captured as raw TSC ticks on the hot path
2. `LatencyTracker` performs the tick-to-nanosecond conversion for host-host deltas
3. existing latency logs still print nanosecond values
4. no unrelated code beyond the approved scope has been modified
