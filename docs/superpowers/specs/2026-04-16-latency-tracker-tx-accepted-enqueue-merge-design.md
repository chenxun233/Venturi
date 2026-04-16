# Latency Tracker TX Accepted-To-Enqueue Merge Design

## Goal

Simplify the sender-local telemetry so the latency chain no longer records or decodes the intermediate TX stages between `TX_EXECUTION_ACCEPTED` and `TX_ENQUEUE`.

The only behavioral change in scope is to merge:

- `tx_execution_accepted -> tx_execution_dequeue_ns`
- `tx_execution_dequeue -> tx_order_frame_built_ns`
- `tx_order_frame_built -> tx_pending_recorded_ns`
- `tx_pending_recorded -> tx_enqueue_ns`

into:

- `tx_execution_accepted -> tx_enqueue_ns`

## Scope

This change covers only:

- sender-local latency stage emission between `TX_EXECUTION_ACCEPTED` and `TX_ENQUEUE`
- `LatencyTracker` pending-state fields and stage decoding for those removed boundaries
- `LatencyLogRecord` fields related to those removed boundaries
- focused tests that assert the merged latency behavior

This change does not cover:

- any latency boundary before `TX_EXECUTION_ACCEPTED`
- `TX_ENQUEUE -> TX_SEND`
- logging format beyond replacing the removed fields with the merged field
- CPU affinity, runtime topology, sender behavior, or queue behavior
- any unrelated telemetry cleanup

## Approved Stage Flow

The retained tracked flow becomes:

1. `FRAME_START`
2. `DMA_EMIT`
3. `BATCH_START`
4. `BATCH_END`
5. `STRATEGY_START`
6. `TX_EXECUTION_ACCEPTED`
7. `TX_ENQUEUE`
8. `TX_SEND`

The following sender-local stages are removed from this telemetry flow:

- `TX_EXECUTION_DEQUEUE`
- `TX_ORDER_FRAME_BUILT`
- `TX_PENDING_RECORDED`

## Emission Changes

### `TxSender::acceptExecution(...)`

Keep emitting `TX_EXECUTION_ACCEPTED` exactly where it is emitted today.

### Sender-local intermediate push sites

Delete the latency emission for:

- `TX_EXECUTION_DEQUEUE`
- `TX_ORDER_FRAME_BUILT`
- `TX_PENDING_RECORDED`

### `TX_ENQUEUE` and `TX_SEND`

Keep emitting:

- `TX_ENQUEUE`
- `TX_SEND`

exactly where they are emitted today.

## Tracker State And Decoding

`LatencyTracker` should no longer store or decode any timestamp or derived latency for:

- `TX_EXECUTION_DEQUEUE`
- `TX_ORDER_FRAME_BUILT`
- `TX_PENDING_RECORDED`

The sender-local pending state should keep only the timestamps needed to compute:

- `strategy_start_to_tx_execution_accepted_ns`
- `tx_execution_accepted_to_tx_enqueue_ns`
- `tx_enqueue_to_tx_send_ns`

The tracker should derive `tx_execution_accepted_to_tx_enqueue_ns` directly from the `TX_EXECUTION_ACCEPTED` timestamp and the `TX_ENQUEUE` timestamp.

## Validation Rules

Pending-event validation should be updated only for the removed sender-local boundaries.

The required flow after `STRATEGY_START` becomes:

- `STRATEGY_START -> TX_EXECUTION_ACCEPTED`
- `TX_EXECUTION_ACCEPTED -> TX_ENQUEUE`
- `TX_ENQUEUE -> TX_SEND`

The tracker should stop maintaining drop accounting for the removed in-between sender-local edges.

No other validation edges should be changed.

## Latency Log Record

`LatencyLogRecord` should remove:

- `tx_execution_accepted_to_tx_execution_dequeue_ns`
- `tx_execution_dequeue_to_tx_order_frame_built_ns`
- `tx_order_frame_built_to_tx_pending_recorded_ns`
- `tx_pending_recorded_to_tx_enqueue_ns`

and replace them with:

- `tx_execution_accepted_to_tx_enqueue_ns`

No other `LatencyLogRecord` field should be changed.

## Printing

The printed latency block should replace the four removed sender-local lines with exactly one line:

- `tx_execution_accepted -> tx_enqueue_ns`

All other printed latency lines should remain as they are today.

## Testing

Focused tests should confirm:

1. `TxSender` no longer emits `TX_EXECUTION_DEQUEUE`, `TX_ORDER_FRAME_BUILT`, or `TX_PENDING_RECORDED`.
2. `LatencyTracker` accepts `TX_EXECUTION_ACCEPTED -> TX_ENQUEUE -> TX_SEND` as the sender-local tail.
3. `LatencyTracker` computes `tx_execution_accepted_to_tx_enqueue_ns` correctly.
4. `LatencyLogRecord` no longer contains the removed sender-local fields.
5. printed latency output contains `tx_execution_accepted -> tx_enqueue_ns`.
6. printed latency output no longer contains the removed in-between sender-local lines.

## Stop Condition

This change is complete when:

1. the only retained sender-local tracked stages are `TX_EXECUTION_ACCEPTED`, `TX_ENQUEUE`, and `TX_SEND`
2. `LatencyTracker` no longer decodes or stores the removed in-between sender-local stages
3. the printed/logged latency record exposes `tx_execution_accepted -> tx_enqueue_ns`
4. no telemetry behavior outside this merge has been modified
