# First-Event Batch Boundary Latency Design

## Summary

Replace the current regression-based FPGA-to-host latency conversion in the Venturi RX path with a simpler first-event-only latency model. The new model keeps latency tracking scoped to records where `is_first_event != 0`, removes runtime dependence on `FPGARegression`, uses only `pollDecodedBatch()`, and reports three timings:

1. `frame_start_to_dma_emit_ns`
2. `batch_start_ns`
3. `batch_end_ns`

`frame_start_to_dma_emit_ns` is derived directly from FPGA ticks using a fixed conversion of `6.4 ns/tick`. `batch_start_ns` is captured in `pollDecodedBatchImpl()`, and `batch_end_ns` is captured in `pollDecodedBatch()` using a lightweight post-pass over the decoded batch.

## Goals

- Remove runtime use of regression from the current RX latency pipeline.
- Remove regression-driven control flow from `Venturi.cpp`, including capture requests, sync initialization, and the regression thread.
- Stop using `pollDecodedBatchSync()` in the app path.
- Keep the existing first-event-only latency ownership model.
- Make the remaining latency numbers explicit and easy to reason about.
- Preserve the rest of the RX, strategy, executor, and TX behavior.

## Non-Goals

- Delete `FPGARegression` from the repository.
- Track latency for non-first events.
- Rework the broader strategy, executor, or TX stage timing model in this change.
- Infer or report a trusted `dma_emit_to_decode_ns` value.

## Existing Problem

The current `dma_emit_to_decode_ns` path mixes FPGA-domain timestamps with host-domain timestamps through regression. In practice, the regression anchor and sync snapshot midpoint noise make the converted values unstable enough that the metric is not useful for tuning.

The current implementation already treats latency as first-event-only:

- RX-side latency records are only pushed when `out[record_count].is_first_event != 0`.
- Strategy-side latency records are only pushed for first events with a non-zero `event_tag`.

That first-event-only contract should remain unchanged.

## Proposed Model

### 1. FPGA-derived metric

Keep `frame_start_to_dma_emit_ns`, but compute it in `LatencyTracker` from the FPGA record fields:

- start tick: `frame_start_tk`
- end tick: `event_tk`
- conversion: `(event_tk - frame_start_tk) * 64 / 10`

The hot path should push raw first-event timing inputs only. No fixed-tick arithmetic should be done in the RX hot path.

This remains a per-first-event metric.

### 2. Host-side batch boundary metrics

Add two host timestamps for the tracked first event in a decoded batch:

- `batch_start_ns`
- `batch_end_ns`

Definitions:

- `batch_start_ns` is captured inside `pollDecodedBatchImpl()` when the first tracked event in the current batch begins decode handling.
- `batch_end_ns` is captured inside `pollDecodedBatch()` after `pollDecodedBatchImpl()` returns and is attached to the same tracked event.

If a batch contains no tracked first event, no batch-boundary latency records are emitted for that batch.

### 3. Removed metric

Stop reporting `dma_emit_to_decode_ns` as a runtime latency metric.

Once regression is removed from the runtime path, there is no trustworthy mixed-domain conversion for that interval. The tracker and log output should no longer treat it as a valid number.

## Data Flow

### RX engine

Inside `pollDecodedBatchImpl()`:

- Detect first-event records using the existing `is_first_event != 0` condition.
- For the first tracked event in the batch:
  - push the raw timing inputs needed to derive `frame_start_to_dma_emit_ns`
  - capture and push `batch_start_ns`
- Preserve the current decode loop behavior for all records.

`pollDecodedBatchSync()` is not part of the active runtime design after this change. The RX path should use `pollDecodedBatch()` only.

### RX thread

`pollDecodedBatch()` is responsible for `batch_end_ns` capture:

- after `pollDecodedBatchImpl()` returns, do a lightweight post-pass over the decoded records
- scan until the first `is_first_event != 0` record is found
- capture `batch_end_ns` once and push it against that event's `event_tag`
- break immediately after the first match

This keeps `batch_end_ns` outside `pollDecodedBatchImpl()` but inside `pollDecodedBatch()`, and bounds the extra work to a single short scan with early exit.

### Latency tracker

`LatencyTracker` should be simplified to reflect the new model:

- remove runtime dependency on `FPGARegression`
- remove `FRAME_START` and `DMA_EMIT` stage handling that exists only to support regression conversion
- remove `dma_emit_to_decode_ns` state, stats updates, and drop handling
- keep first-event keyed pending-state tracking
- add handling for:
  - FPGA-derived `frame_start_to_dma_emit_ns`
  - host `batch_start_ns`
  - host `batch_end_ns`

The tracker should only compute metrics that are explicitly valid under the new model.

## API and Type Changes

The shared latency types should be adjusted so they describe the remaining valid fields only.

Expected changes:

- add explicit record support for `batch_start_ns`
- add explicit record support for `batch_end_ns`
- remove or stop emitting `dma_emit_to_decode_ns`
- keep log records aligned with the new metric set

If the current `stage` enum is no longer a good fit for these boundaries, it may be reduced or renamed as part of the cleanup, as long as behavior stays first-event-only and the write path remains clear.

## Runtime Wiring

In `Venturi.cpp`:

- stop attaching regression to `LatencyTracker`
- remove `CapSignal capture_signal {}`
- remove `std::atomic<bool> rx0_capture_request {false}`
- remove regression-driven snapshot capture for latency purposes
- remove the regression worker thread entirely
- remove `FPGA_regression.initSync(...)`
- comment out `device.setSync(kSyncEnabled);`
- use `pollDecodedBatch()` in both RX threads
- keep the rest of the RX thread flow unchanged

`FPGARegression` remains in the repository but is no longer part of the active latency pipeline for this app.

## Logging

Per-event latency logs should report:

- `frame_start_to_dma_emit_ns`
- `batch_start_ns`
- `batch_end_ns`
- any downstream fields that remain valid after the tracker cleanup

The log output should not print removed metrics as if they are meaningful.

## Error Handling

- If a batch has no first event, do nothing.
- If the batch-start owner cannot be matched at batch end, drop only that batch-end record.
- Latency tracking remains best-effort and must not change trading behavior.

## Testing

Update or add tests for:

- first-event-only batch start capture
- first-event-only batch end capture
- no latency output for batches without a first event
- fixed-tick conversion for `frame_start_to_dma_emit_ns`
- `Venturi.cpp` no longer references regression-driven snapshot control
- RX flow uses `pollDecodedBatch()` rather than `pollDecodedBatchSync()`
- removal or absence of `dma_emit_to_decode_ns` in logs and tracker outputs

## Risks

- The batch-start definition must stay precise. It is the start of decode handling for the tracked first event, not the start of polling the queue.
- Batches with multiple first-event records need one explicit ownership rule. This design assigns the batch-boundary timestamps to the first tracked event encountered in the batch.
- Existing tests and log consumers may assume the old metric names and will need coordinated updates.

## Implementation Notes

- Use integer math for the FPGA tick conversion.
- Keep the hot path simple and avoid floating-point arithmetic.
- Do not delete regression code in this change; only remove its runtime wiring from the current latency path.
- `frame_start_to_dma_emit_ns` is calculated in `LatencyTracker` from the decoded event fields:
  - `frame_start_to_dma_emit_ns = (event_tk - frame_start_tk) * 64 / 10`
- `batch_end_ns` capture should minimize added influence:
  - scan decoded records only until the first `is_first_event != 0`
  - take one timestamp
  - push one latency record
  - break immediately after the first match
