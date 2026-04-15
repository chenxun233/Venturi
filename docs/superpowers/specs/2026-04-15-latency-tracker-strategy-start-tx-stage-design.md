# Latency Tracker Strategy-Start And TX Stage Design

## Goal

Refine `LatencyTracker` so the tracked event flow captures the strategy-entry boundary explicitly, treats the sender-local path as `TX_*` stages, removes overlapping aggregate latency, and prints one latency per line with aligned values.

## Scope

This change covers:

- stage definitions used by latency tracking for first-event records
- latency record emission points in strategy and sender code
- `LatencyTracker` pending-state fields and derived-latency computation
- `LatencyLogRecord` schema
- human-facing latency log formatting
- focused tests that assert the revised telemetry

This change does not cover:

- order-generation logic
- executor queue behavior
- sender networking behavior
- any runtime threading or scheduling changes

## Approved Stage Flow

The tracked flow for first-event latency becomes:

1. `FRAME_START`
2. `DMA_EMIT`
3. `BATCH_START`
4. `BATCH_END`
5. `STRATEGY_START`
6. `TX_EXECUTION_ACCEPTED`
7. `TX_EXECUTION_DEQUEUE`
8. `TX_ORDER_FRAME_BUILT`
9. `TX_PENDING_RECORDED`
10. `TX_ENQUEUE`
11. `TX_SEND`

The previous `EXECUTOR` stage is removed from this event flow.

## Emission Points

### `DummyStrategy::evaluateEvent(...)`

Push a latency record tagged `STRATEGY_START` immediately before:

```cpp
const OrderIntentAction action = _readAction(event);
```

Remove the existing post-action latency push that currently emits `stage::STRATEGY`.

This means the strategy stage measures when the strategy begins evaluating the event, not when the intent has already been produced.

### `Executor::acceptIntent(...)`

Remove the current latency push tagged `stage::EXECUTOR`.

The executor still forwards the `OrderExecution` as before; it simply stops contributing a tracked stage in this first-event latency chain.

### `TxSender::acceptExecution(...)`

After a successful enqueue into the sender-local execution buffer, push a latency record tagged `TX_EXECUTION_ACCEPTED`.

### Existing Sender-Local Push Sites

Keep the current sender-local push locations but rename their stage tags:

- `EXECUTION_DEQUEUE` -> `TX_EXECUTION_DEQUEUE`
- `ORDER_FRAME_BUILT` -> `TX_ORDER_FRAME_BUILT`
- `PENDING_RECORDED` -> `TX_PENDING_RECORDED`

`TX_ENQUEUE` and `TX_SEND` remain as the sender-local enqueue and send boundaries.

## Tracker State And Derived Latencies

`LatencyTracker` should store timestamps and derived latencies only for the approved flow.

Derived latency fields become:

- `frame_start_to_dma_emit_ns`
- `batch_duration_ns`
- `batch_end_to_strategy_start_ns`
- `strategy_start_to_tx_execution_accepted_ns`
- `tx_execution_accepted_to_tx_execution_dequeue_ns`
- `tx_execution_dequeue_to_tx_order_frame_built_ns`
- `tx_order_frame_built_to_tx_pending_recorded_ns`
- `tx_pending_recorded_to_tx_enqueue_ns`
- `tx_enqueue_to_tx_send_ns`

The following tracked field and aggregate must be deleted:

- `executor_to_tx_enqueue_ns`

Because `EXECUTOR` is removed from this flow, `LatencyTracker` should also stop maintaining any `EXECUTOR`-based pending-state timestamps or stage-to-stage derived latencies for this event chain.

## Validation Rules

Pending-event validation should continue to reject missing or out-of-order stages, but the required boundaries are updated to the renamed flow:

- `FRAME_START -> DMA_EMIT`
- `DMA_EMIT -> BATCH_START`
- `BATCH_START -> BATCH_END`
- `BATCH_END -> STRATEGY_START`
- `STRATEGY_START -> TX_EXECUTION_ACCEPTED`
- `TX_EXECUTION_ACCEPTED -> TX_EXECUTION_DEQUEUE`
- `TX_EXECUTION_DEQUEUE -> TX_ORDER_FRAME_BUILT`
- `TX_ORDER_FRAME_BUILT -> TX_PENDING_RECORDED`
- `TX_PENDING_RECORDED -> TX_ENQUEUE`
- `TX_ENQUEUE -> TX_SEND`

If any required predecessor is missing or any timestamp is out of order, the pending event should be dropped exactly as the tracker already does for invalid chains.

## Latency Log Record

`LatencyLogRecord` should expose only the revised fields listed above.

It should no longer contain:

- `executor_to_tx_enqueue_ns`

The record should stay focused on one latency value per actual adjacent boundary in the approved flow.

## Printing Format

The latency log output should print one logical event record as a short block:

- first line contains the event identity header
- each following line contains exactly one latency metric
- stage-pair labels use ` -> ` instead of `_to_`
- latency values are aligned using tabs

Example shape:

```text
LatencyNs queue=1 event_tag=767284787
frame_start -> dma_emit_ns			185
batch_duration_ns				183
batch_end -> strategy_start_ns		119
strategy_start -> tx_execution_accepted_ns	154
tx_execution_accepted -> tx_execution_dequeue_ns	339
tx_execution_dequeue -> tx_order_frame_built_ns	130
tx_order_frame_built -> tx_pending_recorded_ns	177
tx_pending_recorded -> tx_enqueue_ns	265
tx_enqueue -> tx_send_ns			9712
```

The exact tab count can be chosen to keep the values visually aligned in the current logger implementation, but values must not share a single long line anymore.

## Testing

Focused tests should be updated or added to confirm:

1. `DummyStrategy` emits `STRATEGY_START` before `_readAction(event)` and no longer emits the old post-action strategy stage.
2. `Executor::acceptIntent(...)` no longer emits a latency stage for this event flow.
3. `TxSender` emits `TX_EXECUTION_ACCEPTED`, `TX_EXECUTION_DEQUEUE`, `TX_ORDER_FRAME_BUILT`, and `TX_PENDING_RECORDED` as expected.
4. `LatencyTracker` computes the renamed adjacent latencies correctly.
5. `LatencyLogRecord` no longer contains the overlapping `executor_to_tx_enqueue_ns` field.
6. human-facing latency output prints one metric per line, uses ` -> ` labels, and aligns values with tabs.

## Stop Condition

This design is complete when:

1. the first-event latency flow starts at `STRATEGY_START`
2. sender-local boundaries use the approved `TX_*` stage names
3. `EXECUTOR` is removed from this tracked event flow
4. `executor_to_tx_enqueue_ns` is removed from tracker state, log record schema, and printed output
5. latency output prints one aligned value per line using ` -> ` stage labels
6. focused tests cover the revised telemetry behavior
