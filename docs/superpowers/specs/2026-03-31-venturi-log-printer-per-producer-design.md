# Venturi Log Printer Per-Producer Buffer Design

Date: 2026-03-31

## Goal

Refine logging in `FPGA_boost_demo` so that:

- `LatencyLogPrinter` no longer uses a shared push mutex
- each producer writes into its own `TraceBuffer<AsyncLogRecord>`
- the log printer thread remains the only consumer
- the push API follows the explicit producer-index pattern already used by `LatencyTracker`
- snapshot logging is produced only by `FPGARxEngine`
- the redundant snapshot cache and snapshot re-push logic in `Venturi.cpp` is removed

## Scope

### In Scope

- Refactor `LatencyLogPrinter` to own one trace buffer per producer
- Change log-printer push APIs to accept `producer_idx`
- Update current logging call sites in `LatencyTracker`, `Executor`, `TxEngine`, and `FPGARxEngine`
- Remove snapshot logging state and periodic snapshot push logic from `Venturi.cpp`
- Keep all changes inside `cpp_src/FPGA_boost_demo`

### Out of Scope

- Redesigning log record formats
- Changing the textual output format of existing log lines
- Introducing a new queue type unrelated to `TraceBuffer`
- Moving log printing into any thread other than the log-printer worker thread
- Reworking unrelated regression or sync logic in `Venturi.cpp`

## Existing Problem

`LatencyLogPrinter` currently uses one shared ring buffer protected by `m_push_mutex`. That means:

- all producers serialize on one mutex
- the ownership model is hidden inside the logger instead of being explicit at call sites
- the current design does not match the explicit single-producer/single-consumer pattern already used elsewhere in the codebase

At the same time, snapshot logging currently comes from two places:

- `FPGARxEngine::pollBatchSync(...)` pushes a snapshot when a new capture happens
- `Venturi.cpp` stores a shared cached snapshot and periodically re-pushes it to the log printer

That second path is redundant for the intended design. The user wants snapshots to be logged only when they are captured by the RX path.

## Architecture

### `LatencyLogPrinter`

`LatencyLogPrinter` becomes structurally similar to `LatencyTracker`.

Instead of:

- one shared `std::vector<AsyncLogRecord>`
- one shared atomic `head`
- one shared atomic `tail`
- one shared push mutex

it will own:

- `std::vector<std::unique_ptr<TraceBuffer<AsyncLogRecord>>> m_buffers`
- one buffer per producer
- a single consumer thread that drains all producer buffers in round-robin order

The push-side interface becomes explicit:

- `pushLatency(uint16_t producer_idx, const LatencyLogRecord& record)`
- `pushSnapshot(uint16_t producer_idx, const FpgaSyncSnapshot& snapshot)`
- `pushExecution(uint16_t producer_idx, const ExecutionLogRecord& record)`
- `pushTxEvent(uint16_t producer_idx, const TxLogRecord& record)`

Each push method wraps the typed record into `AsyncLogRecord` and writes it into exactly one producer-owned `TraceBuffer`.

### Producer/Consumer Contract

Each trace buffer must remain SPSC:

- exactly one producer thread writes a given buffer
- only the log-printer worker thread reads from that buffer

This keeps the threading model honest and avoids locking on the hot push path.

### Drain Loop

The existing log-printer worker thread remains responsible for:

- waiting for work
- draining available records
- formatting and printing each record

The worker thread will iterate over all producer buffers round-robin so no single producer permanently dominates the drain order.

If the current condition-variable wakeup is kept, it must be compatible with the per-producer buffers. The wakeup mechanism is allowed to remain separate from the per-producer data buffers because it does not protect record ownership; it only reduces idle spinning.

## Producer Mapping

For the current `Venturi.cpp` topology, the producer layout is fixed and explicit.

Recommended mapping:

- `0..kQueueCount-1`: latency records pushed from `LatencyTracker::run()`
- `kQueueCount`: executor-thread execution logs
- `kQueueCount + 1`: tx-thread TX event logs
- `kQueueCount + 2`: RX snapshot logs pushed from `FPGARxEngine`

With the current application configuration where `kQueueCount == 2`, that means:

- `0`: latency producer for queue 0
- `1`: latency producer for queue 1
- `2`: executor producer
- `3`: TX producer
- `4`: RX snapshot producer

This mapping must be defined clearly in code instead of inferred implicitly.

## Snapshot Ownership Change

Snapshot logging is owned only by the RX path.

### Keep

- `FPGARxEngine::pollBatchSync(...)` continues to:
  - capture `FpgaSyncSnapshot`
  - update regression state when configured
  - push the snapshot to `LatencyLogPrinter`

### Remove From `Venturi.cpp`

Delete the following snapshot-related behavior from `Venturi.cpp`:

- `sync_snapshot`
- `has_latest_snapshot`
- the mutex-protected cached snapshot read/write path
- the periodic `latency_log_printer.pushSnapshot(snapshot_to_print)` path in the control thread

The control thread will no longer re-publish snapshots to the logger.

### Behavioral Consequence

After this change, snapshot logs are emitted only when `get_time` triggers a fresh capture in the RX engine.

This is the intended behavior. The previous print interval in `Venturi.cpp` should not be preserved.

## Call-Site Changes

### `LatencyTracker`

`LatencyTracker` already routes records by queue index. It will push latency logs with that same index:

- `pushLatency(record.que_idx, ...)`

This preserves the existing per-queue producer ownership pattern.

### `Executor`

`Executor` runs on its own thread and will use the dedicated executor producer index when pushing execution logs.

### `TxEngine`

`TxEngine` runs on the TX thread and will use the dedicated TX producer index when pushing TX events.

### `FPGARxEngine`

`FPGARxEngine` will use the dedicated snapshot producer index when pushing snapshots during `pollBatchSync(...)`.

## Error Handling And Overflow

Each producer buffer keeps the existing `TraceBuffer` behavior:

- writes may fail or overwrite based on the chosen `TraceBuffer` API
- failure accounting should remain explicit

`LatencyLogPrinter` will preserve its current dropped-record accounting semantics as closely as possible. If the implementation changes how drops are counted because each producer now owns a separate buffer, the behavior must remain deterministic and easy to reason about.

## Testing

The implementation will add or update tests to cover:

- pushing latency, snapshot, execution, and TX records through explicit producer indices
- successful drain of records from multiple producer buffers
- correct preservation of all four record kinds after the refactor
- snapshot logging still occurring through `FPGARxEngine`
- removal of the old `Venturi.cpp` snapshot re-publish path

The most important regression to prevent is silent loss of records due to an incorrect producer mapping or a non-SPSC use of a buffer.

## Risks

### Producer Index Drift

If producer indices are scattered as unnamed constants, future changes may route multiple threads into the same buffer by mistake.

Mitigation:

- define indices centrally and name them clearly

### Partial Snapshot Refactor

If `Venturi.cpp` keeps any of the cached snapshot path while `FPGARxEngine` also pushes snapshots directly, duplicate snapshot logging will remain.

Mitigation:

- remove the full cached snapshot logging path from `Venturi.cpp`

### Drain Fairness

If the printer always drains one producer fully before checking others, a noisy producer can delay less active producers.

Mitigation:

- use round-robin buffer visitation

## Result

The final design makes the logger consistent with the rest of the codebase's threading model:

- explicit producer ownership
- SPSC trace buffers per producer
- one consumer thread for formatting and printing
- no shared push mutex
- snapshots emitted only by the RX capture path
