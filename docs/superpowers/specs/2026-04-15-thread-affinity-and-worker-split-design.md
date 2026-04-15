# Thread Affinity And Worker Split Design

## Summary

Split the current mixed top-level runtime loop into dedicated worker threads and
pin each worker to an explicit CPU:

- `rx_thread0` -> CPU 0
- `rx_thread1` -> CPU 1
- `latency_thread` -> CPU 2
- `LogPrinter` worker thread -> CPU 3
- `regression_thread` -> CPU 4

The application should also keep its main thread off CPUs 0, 1, 2, and 3.

This change is about process-local thread topology and affinity management. It
does not claim to fully isolate CPUs from the rest of the operating system by
itself; that remains an external system configuration requirement.

## Goals

- split `LatencyTracker` execution into its own dedicated thread
- keep `LogPrinter` as its own worker thread and pin it explicitly
- split `FPGARegression` execution into its own dedicated thread
- keep one dedicated RX thread per queue and pin each one explicitly
- keep the main thread off CPUs 0, 1, 2, and 3
- document that CPUs 0, 1, 2, and 3 are intended to be isolated from the
  system

## Non-Goals

- no redesign of RX decoding, strategy, executor, transport, or latency logic
- no attempt to enforce whole-system CPU isolation purely from application code
- no automatic IRQ steering or boot-parameter configuration
- no requirement that CPU 4 be isolated

## Current Problem

`Venturi.cpp` currently runs regression triggering and latency draining inside a
single `control_thread`. The RX workers are already split by queue, but
affinity is not managed explicitly, and `LogPrinter` owns an internal thread
with no configurable CPU assignment.

This creates three problems:

- `LatencyTracker` does not have a dedicated execution context
- `FPGARegression` does not have a dedicated execution context
- the runtime has no explicit CPU placement contract for worker threads

## Selected Approach

Use one dedicated thread per runtime responsibility boundary and pin each
thread at its own entry point.

That means:

1. keep two RX threads, one per queue
2. replace the current `control_thread` with:
   - `latency_thread`
   - `regression_thread`
3. extend `LogPrinter` so its internal worker thread can pin itself to CPU 3
4. add explicit affinity helpers in `Venturi.cpp`
5. pin the main thread away from CPUs 0-3

This keeps the topology easy to read and avoids parent-thread pinning races.

## Thread Topology

### RX Thread 0

Pinned to CPU 0.

Responsibilities:

- poll `rx_engine0`
- submit snapshots when requested
- evaluate `strategy0`
- drain `executor0`
- drive `tx_connection0`
- drive `tx_sender0`

### RX Thread 1

Pinned to CPU 1.

Responsibilities:

- poll `rx_engine1`
- submit snapshots when requested
- evaluate `strategy1`
- drain `executor1`
- drive `tx_connection1`
- drive `tx_sender1`

### Latency Thread

Pinned to CPU 2.

Responsibilities:

- repeatedly call `latency_tracker.run()`
- sleep/back off when no records were processed

`LatencyTracker` remains a shared object constructed in `main`, but its
execution context becomes the dedicated `latency_thread`.

### Log Printer Thread

Pinned to CPU 3.

Responsibilities:

- run the existing `LogPrinter` internal worker loop
- drain latency, execution, and tx log queues
- print results

`LogPrinter` should keep owning its worker thread internally. The only required
extension is an affinity configuration hook so that thread can pin itself after
startup.

### Regression Thread

Pinned to CPU 4.

Responsibilities:

- repeatedly call `FPGARegression::run(...)`
- trigger snapshot capture requests for RX workers

`FPGARegression` should no longer share a loop with latency processing.

### Main Thread

The main thread is not assigned a dedicated single CPU, but it must not remain
eligible to run on CPUs 0, 1, 2, or 3. Pin it to an allowed CPU set that
excludes those CPUs.

## Affinity Contract

### In-Process Guarantees

Application code will enforce:

- CPU 0 only for `rx_thread0`
- CPU 1 only for `rx_thread1`
- CPU 2 only for `latency_thread`
- CPU 3 only for `LogPrinter` worker thread
- CPU 4 only for `regression_thread`
- main thread excluded from CPUs 0-3

### System-Level Isolation

The application cannot fully guarantee whole-system exclusivity for CPUs 0-3.
That requires external configuration such as:

- boot-time isolated CPU settings
- keeping unrelated processes off CPUs 0-3
- moving IRQs away from CPUs 0-3

This design assumes CPUs 0-3 are intended to be isolated, but code changes are
limited to process-local affinity management.

## Code Changes

### Venturi

File:

- `cpp_src/FPGA_boost_demo/app/Venturi.cpp`

Add:

- a helper to pin the current thread to a single CPU
- a helper to pin the current thread to an allowed CPU mask
- constants for:
  - RX0 CPU = 0
  - RX1 CPU = 1
  - Latency CPU = 2
  - LogPrinter CPU = 3
  - Regression CPU = 4

Replace the current `control_thread` with:

- `regression_thread`
- `latency_thread`

Pin:

- main thread away from CPUs 0-3
- `regression_thread` to CPU 4
- `rx_thread0` to CPU 0
- `rx_thread1` to CPU 1
- `latency_thread` to CPU 2

Preserve the existing RX-stage logic as much as possible. The thread split
should not become an excuse for unrelated behavioral changes.

### LogPrinter

Files:

- `cpp_src/FPGA_boost_demo/latency/log_printer.h`
- `cpp_src/FPGA_boost_demo/latency/log_printer.cpp`

Add:

- a small API for optional worker CPU assignment, for example a setter or a
  constructor field

Behavior:

- when configured, the worker thread pins itself after startup and before
  entering the main drain loop
- when not configured, behavior stays unchanged

This keeps tests and other callers simple while allowing `Venturi.cpp` to opt
into pinning.

## Snapshot Request Flow

The current snapshot flow can stay logically the same:

1. regression thread requests capture
2. per-queue atomic flags are set
3. RX threads observe their flag and capture a snapshot during polling
4. RX threads call `FPGARegression::tryAcceptSnapshot(...)`

The change is only who drives the request loop:

- before: mixed `control_thread`
- after: dedicated `regression_thread`

No queue-crossing ownership redesign is required if the current atomics are
kept.

## Error Handling

If thread affinity setup fails for a dedicated worker:

- fail fast during startup
- log or print a useful error if practical
- do not continue in a partially pinned configuration silently

If main-thread affinity exclusion fails:

- fail fast during startup as well

The purpose of this change is deterministic placement. Silent fallback would
hide the failure of the feature.

## Testing

### Build And API Validation

Verify that:

- `Venturi` builds with the new thread topology
- `LogPrinter` tests still build and pass after the new affinity configuration
  API is added
- `Regression` tests still build and pass if the signaling type or call pattern
  changes

### Runtime Validation

At minimum, the implementation should make it easy to verify affinity at
runtime, for example via:

- a debugger
- `/proc/<pid>/task/<tid>/status`
- `taskset -pc <tid>`

The code does not need to add a full telemetry subsystem for this change.

### Targeted Test Scope

Primary verification targets:

- `Venturi` build
- `log_printer_test`
- `regression_test`

Additional runtime tests may be manual because CPU affinity behavior is partly
environment dependent.

## Risks

- startup now has more thread-affinity failure points
- pinning the main thread to an allowed mask may behave differently on systems
  with fewer than 5 CPUs
- if CPUs 0-3 are not actually isolated at the OS level, system noise can still
  land there despite correct process-local affinity
- moving regression to its own thread slightly changes the timing of snapshot
  requests

## Acceptance Criteria

- `Venturi.cpp` no longer uses one mixed `control_thread` for both regression
  and latency work
- `LatencyTracker` is driven by its own dedicated thread on CPU 2
- `LogPrinter` worker thread is pinned to CPU 3
- `FPGARegression` is driven by its own dedicated thread on CPU 4
- RX queue 0 runs on CPU 0
- RX queue 1 runs on CPU 1
- main thread is excluded from CPUs 0-3
- code documents that CPUs 0-3 are intended to be isolated from the rest of the
  system
