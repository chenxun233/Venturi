# TX Send-Enter Latency Split Design

## Goal

Replace the current aggregate sender-tail latency line:

- `tx_enqueue -> tx_send_ns`

with two narrower latency lines:

- `tx_enqueue -> tx_send_enter_ns`
- `tx_send_enter -> tx_send_ns`

The goal is to separate:

- wait before the sender begins the actual send loop
- time spent inside the send loop and syscall path

## Scope

This change covers only:

- the latency stage schema needed to introduce `TX_SEND_ENTER`
- sender-side emission of `TX_SEND_ENTER`
- `LatencyTracker` decoding for the split sender tail
- `LatencyLogRecord` fields related to this split
- printed latency output for the split sender tail
- focused tests required by this split

This change does not cover:

- any latency boundary before `TX_ENQUEUE`
- CPU affinity changes
- transport behavior changes
- sender queue behavior changes
- socket behavior changes
- unrelated telemetry cleanup

If an unrelated file appears to need modification, that is out of scope.

## Approved Stage Flow

The retained sender-local tail becomes:

1. `TX_EXECUTION_ACCEPTED`
2. `TX_ENQUEUE`
3. `TX_SEND_ENTER`
4. `TX_SEND`

No other latency stages should be changed by this design.

## `TX_SEND_ENTER` Placement

`TX_SEND_ENTER` should be emitted inside [`tx_sender.cpp`](/home/chenxun/Documents/Project/Venturi/cpp_src/FPGA_boost_demo/tx_engine/tx_sender.cpp), inside `_sendPayload(...)`, after the early validity checks and immediately before the send loop:

```cpp
if (record.payload_length == 0 ||
    record.payload_length > record.payload.size() ||
    m_send_fd < 0) {
    return false;
}
```

The new stage should be stamped after those guards and before:

```cpp
while (offset < static_cast<std::size_t>(record.payload_length)) {
```

This makes the split mean:

- `tx_enqueue -> tx_send_enter_ns`
  time from ready-queue insertion until `_sendPayload()` is about to attempt the send loop

- `tx_send_enter -> tx_send_ns`
  time spent inside the send loop, including the actual `send()` syscall path and any partial-write looping before the existing `TX_SEND` stamp

## Emission Rules

### `TX_ENQUEUE`

Keep `TX_ENQUEUE` emission exactly where it is emitted today.

### `TX_SEND_ENTER`

Emit `TX_SEND_ENTER` only when the record:

- has passed the early `_sendPayload(...)` guards
- has a non-zero `event_tag`
- has a non-null latency tracker

### `TX_SEND`

Keep `TX_SEND` exactly where it is emitted today.

## Latency Record Changes

`LatencyLogRecord` should remove:

- `tx_enqueue_to_tx_send_ns`

and replace it with:

- `tx_enqueue_to_tx_send_enter_ns`
- `tx_send_enter_to_tx_send_ns`

No other existing latency fields should be changed.

## Tracker State And Decoding

`LatencyTracker` should:

- store the `TX_SEND_ENTER` host tick
- validate the sender tail as:
  - `TX_EXECUTION_ACCEPTED -> TX_ENQUEUE`
  - `TX_ENQUEUE -> TX_SEND_ENTER`
  - `TX_SEND_ENTER -> TX_SEND`

The tracker should derive:

- `tx_enqueue_to_tx_send_enter_ns`
- `tx_send_enter_to_tx_send_ns`

using the same host tick-to-nanosecond conversion path already used for other host-host intervals.

The tracker should stop deriving the old aggregate:

- `tx_enqueue_to_tx_send_ns`

No other tracker stage relationships should be changed.

## Validation Rules

After this change, the sender tail validation should be:

- `TX_EXECUTION_ACCEPTED -> TX_ENQUEUE`
- `TX_ENQUEUE -> TX_SEND_ENTER`
- `TX_SEND_ENTER -> TX_SEND`

Drop accounting should be updated only for the old aggregate edge being replaced by the two new edges.

No earlier stage validation should be changed.

## Printing

The printed latency block should replace:

- `tx_enqueue -> tx_send_ns`

with exactly:

- `tx_enqueue -> tx_send_enter_ns`
- `tx_send_enter -> tx_send_ns`

All other printed latency lines should remain unchanged.

## Testing

Focused tests should confirm:

1. `TxSender` emits `TX_SEND_ENTER` only after the `_sendPayload(...)` guards succeed and before the send loop begins.
2. `LatencyTracker` accepts the sender tail:
   - `TX_EXECUTION_ACCEPTED -> TX_ENQUEUE -> TX_SEND_ENTER -> TX_SEND`
3. `LatencyTracker` computes:
   - `tx_enqueue_to_tx_send_enter_ns`
   - `tx_send_enter_to_tx_send_ns`
4. `LatencyLogRecord` no longer contains `tx_enqueue_to_tx_send_ns`.
5. printed latency output contains:
   - `tx_enqueue -> tx_send_enter_ns`
   - `tx_send_enter -> tx_send_ns`
6. printed latency output no longer contains:
   - `tx_enqueue -> tx_send_ns`

## Stop Condition

This change is complete when:

1. the old aggregate sender-tail line `tx_enqueue -> tx_send_ns` has been fully replaced
2. `TX_SEND_ENTER` is emitted at the approved boundary in `_sendPayload(...)`
3. `LatencyTracker` decodes and logs the two split sender-tail latencies
4. no unrelated telemetry behavior outside this split has been modified
