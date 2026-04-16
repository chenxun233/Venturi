# Log Printer Registration-Based Producer Design

Date: 2026-04-14

## Goal

Remove the shared producer-side mutex from `LogPrinter` while preserving:

- one consumer thread responsible for formatting and printing
- best-effort non-blocking enqueue behavior on producer paths
- support for multiple producer threads
- use of `SpscRingQueue<AsyncLogRecord>` on the hot path

## Why This Change Is Needed

`LogPrinter` currently accepts records from multiple threads through one shared internal ring buffer guarded by `m_push_mutex`.

That means:

- all producers serialize on one mutex
- ownership of the write path is hidden inside `LogPrinter`
- the queue primitive already used elsewhere in the codebase, `SpscRingQueue`, cannot be used directly with the current shared-buffer shape

`LatencyTracker` already uses the correct structural pattern for this problem:

- one SPSC queue per producer
- one consumer that drains all queues

The new `LogPrinter` design should follow the same threading model, but without forcing every call site to pass a hard-coded producer index.

## Chosen Approach

Chosen approach:

- `LogPrinter` owns one `SpscRingQueue<AsyncLogRecord>` per registered producer
- startup wiring registers producers and receives producer-specific handles
- producers push through their own handle instead of calling a shared `LogPrinter::pushXxx(...)`
- `LogPrinter` remains the sole consumer and drains all producer queues in round-robin order
- registration is startup-only for this change; dynamic unregister is out of scope

Rejected alternatives:

- one shared `SpscRingQueue<AsyncLogRecord>`
  - rejected because the runtime has multiple producers, so that would violate the SPSC contract
- fixed `producer_idx` push APIs
  - rejected because the user selected the registration-based design instead of explicit index plumbing
- fully dynamic register/unregister while the printer thread is active
  - rejected because it would require extra synchronization around the registry and would weaken the goal of removing shared enqueue-side coordination

## Architecture

### `LogPrinter`

`LogPrinter` becomes a single consumer over a stable set of producer-owned queues.

It should own:

- a collection of producer slots
- one `SpscRingQueue<AsyncLogRecord>` per slot
- aggregate or per-slot drop accounting
- the background consumer thread

It should no longer own:

- one shared producer buffer
- `m_push_mutex`

The queue set is finalized before the worker thread begins consuming records.

### `LogProducer`

Introduce a lightweight producer handle type associated with exactly one internal queue.

The handle should expose typed methods equivalent to the current producer API, for example:

- `pushLatencyLog(...)`
- `pushRegressionStatus(...)`
- `pushExecution(...)`
- `pushTxEvent(...)`

Each method wraps the typed payload into `AsyncLogRecord` and pushes into that handle's dedicated SPSC queue.

The handle is the ownership boundary:

- exactly one producer thread owns and writes through a given handle
- only `LogPrinter` reads from the corresponding queue

This makes the SPSC contract explicit instead of implicit.

## Registration Model

Registration happens during startup wiring before logging begins.

Recommended API shape:

- `LogPrinter::registerProducer(...) -> LogProducer`

The registration call allocates a new producer slot and returns a handle bound to that slot.

The handle may be:

- move-only
- copy-disabled
- cheap to store inside existing components as a member or attached dependency

This change does not support:

- registering new producers after `LogPrinter::start()`
- unregistering producers while `LogPrinter` is running
- reassigning one queue to multiple writers

If a component does not have a registered handle, it should not emit logs.

## Expected Wiring

The current runtime already has long-lived producer contexts that map naturally to dedicated handles.

Expected producers include:

- the latency path that currently emits completed latency records from `LatencyTracker::run()`
- each executor-side execution logging path
- each `TxConnection` path that emits connection events
- each `TxSender` path that emits TX events
- any one-off startup path that emits regression status, if that path must remain asynchronous

The exact number of handles is determined during startup assembly in `Venturi.cpp`.

This design intentionally binds logging ownership to producer context, not to record kind.

## Data Flow

1. Startup code constructs `LogPrinter`.
2. Startup code registers one producer handle per real producer context.
3. Startup code passes each handle to the owning component before worker threads begin.
4. Producers package typed records and push to their own queue through their handle.
5. `LogPrinter::_run()` drains all producer queues round-robin and prints records.
6. `LogPrinter::stop()` stops the background wait loop and drains remaining queued records before returning.

Round-robin visitation should remain explicit so a busy producer does not permanently starve quieter ones.

## Error Handling

Producer push remains best-effort:

- if a producer queue is full, the push returns `false`
- dropping a record must not block the producer
- drop accounting must remain visible and deterministic

Registration after the printer has started should fail explicitly.

Using one handle from multiple producer threads is invalid. The design relies on startup ownership discipline rather than runtime locking to enforce the SPSC contract.

## Scope

Files expected to change during implementation:

- `cpp_src/FPGA_boost_demo/latency/log_printer.h`
- `cpp_src/FPGA_boost_demo/latency/log_printer.cpp`
- logging call sites that currently store `LogPrinter*`
- startup wiring in `cpp_src/FPGA_boost_demo/app/Venturi.cpp`
- tests covering logger enqueue and drain behavior

Files intentionally not redesigned in this change:

- `cpp_src/FPGA_boost_demo/common/spsc_ring_queue.h`
- latency computation logic in `LatencyTracker`
- log text formatting beyond any mechanical call-site adjustments

## Testing

Implementation verification should cover:

1. distinct registrations return independent producer handles
2. records pushed through multiple handles are drained correctly by one consumer
3. queue-full behavior returns `false` and updates drop accounting
4. `stop()` drains remaining records before exit
5. existing textual log formatting still matches expectations
6. startup wiring does not leave any active logging path without a producer handle

## Risks

### Hidden Multi-Writer Misuse

If two threads accidentally share one producer handle, the SPSC queue contract is violated.

Mitigation:

- make handles move-only
- register handles in startup code where thread ownership is explicit
- avoid shared global accessors for producers

### Registry Drift

If some call sites keep `LogPrinter*` while others move to producer handles, the design becomes inconsistent.

Mitigation:

- convert all asynchronous producer call sites in one pass
- keep `LogPrinter` focused on registration, lifecycle, draining, and formatting only

### Shutdown Loss

If `stop()` does not drain every producer queue after the worker loop exits, buffered records may be lost.

Mitigation:

- require a final synchronous drain across all registered queues before returning from `stop()`

## Relationship To Earlier Spec

This design supersedes the producer-identification portion of:

- `docs/superpowers/specs/2026-03-31-venturi-log-printer-per-producer-design.md`

That earlier design chose explicit `producer_idx` push APIs.

This design keeps the same per-producer SPSC queue architecture but replaces explicit index plumbing with registration-based producer handles.

## Stop Condition

This design is complete when:

1. `LogPrinter` no longer uses a shared producer-side mutex
2. every asynchronous producer pushes through its own registered handle
3. each producer handle owns exactly one `SpscRingQueue<AsyncLogRecord>`
4. the printer thread remains the single consumer
5. startup registration and shutdown draining rules are explicit and testable
