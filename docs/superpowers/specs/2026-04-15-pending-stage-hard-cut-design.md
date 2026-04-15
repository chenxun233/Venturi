# Pending Stage Hard Cut Design

## Summary

Remove the intermediate pending latency stages `PENDING_CAPACITY_HANDLED` and
`PENDING_TAG_RECORDED` from the shared latency contract.

After this change, the pending-side latency path becomes:

- `ORDER_FRAME_BUILT`
- `PENDING_RECORDED`
- `TX_ENQUEUE`
- `TX_SEND`

`order_frame_built_to_pending_recorded_ns` remains, but it is no longer an
aggregate over hidden substages. It becomes the direct measured delta from the
`ORDER_FRAME_BUILT` push to the `PENDING_RECORDED` push.

## Goals

- remove the two obsolete intermediate pending stages entirely
- make `TxSender` push only the two timestamps needed for the pending-recorded
  interval
- simplify `LatencyTracker` so it decodes only real emitted pending boundaries
- simplify `LatencyLogRecord` and stdout latency output to match the new
  contract
- keep historical specs unchanged and document this change in a new spec

## Non-Goals

- no compatibility layer for removed stdout latency keys
- no retention of dead enum values or dead log fields
- no redesign of unrelated stage boundaries outside the pending section
- no change to historical design docs that introduced the removed stages

## Motivation

The direct-index pending table changed the internal pending insertion path so
the old substage breakdown is no longer a useful contract. Keeping
`PENDING_CAPACITY_HANDLED` and `PENDING_TAG_RECORDED` would preserve internal
implementation details that are no longer meaningful and would require
`LatencyTracker` and `LogPrinter` to keep decoding and emitting latency fields
that the runtime no longer needs.

The correct hard cut is to remove those stages everywhere and keep only the
direct latency from order-frame completion to pending-slot recording.

## Selected Approach

Use a full hard cut:

1. remove `PENDING_CAPACITY_HANDLED` and `PENDING_TAG_RECORDED` from the shared
   `stage` enum
2. stop pushing those stages from `TxSender`
3. remove their state, handlers, and log fields from `LatencyTracker`
4. keep only `order_frame_built_to_pending_recorded_ns` for the pending-record
   latency segment
5. treat the stdout latency format change as intentional and breaking for any
   downstream parser that still expects the removed keys

This keeps the runtime contract, type schema, and printed output aligned.

## Runtime Contract

### Pending-Side Stage Order

The required pending-adjacent stage order becomes:

1. `ORDER_FRAME_BUILT`
2. `PENDING_RECORDED`
3. `TX_ENQUEUE`
4. `TX_SEND`

### Pending-Side Latency Meaning

`order_frame_built_to_pending_recorded_ns` now means:

- host timestamp captured at `PENDING_RECORDED`
- minus host timestamp captured at `ORDER_FRAME_BUILT`

That is the only pending-recording latency interval emitted for this region.

### Emission Rules

`TxSender` should:

- keep pushing `ORDER_FRAME_BUILT` when the outbound order frame is fully built
- stop pushing `PENDING_CAPACITY_HANDLED`
- stop pushing `PENDING_TAG_RECORDED`
- keep pushing `PENDING_RECORDED` after the pending slot write succeeds
- keep pushing `TX_ENQUEUE` and `TX_SEND` unchanged

This gives `LatencyTracker` only the two time points required to compute
`order_frame_built_to_pending_recorded_ns`.

## Code Changes

### Shared Types

File:

- `cpp_src/FPGA_boost_demo/common/shared_types.h`

Changes:

- remove `PENDING_CAPACITY_HANDLED` from `stage`
- remove `PENDING_TAG_RECORDED` from `stage`
- remove these fields from `LatencyLogRecord`:
  - `order_frame_built_to_pending_capacity_handled_ns`
  - `pending_capacity_handled_to_pending_tag_recorded_ns`
  - `pending_tag_recorded_to_pending_recorded_ns`
- keep `order_frame_built_to_pending_recorded_ns`

### TxSender

File:

- `cpp_src/FPGA_boost_demo/tx_engine/tx_sender.cpp`

Changes:

- delete `push_pending_stage(stage::PENDING_CAPACITY_HANDLED);`
- delete `push_pending_stage(stage::PENDING_TAG_RECORDED);`
- keep `push_pending_stage(stage::PENDING_RECORDED);`

Behavior:

- the pending insertion path still captures the final pending boundary
- the intermediate bookkeeping boundaries are no longer externally visible

### LatencyTracker State

File:

- `cpp_src/FPGA_boost_demo/latency/latency_tracker.h`

Changes inside `PendingEventState`:

- remove `pending_capacity_handled_host_ns`
- remove `pending_tag_recorded_host_ns`
- remove `order_frame_built_to_pending_capacity_handled_ns`
- remove `pending_capacity_handled_to_pending_tag_recorded_ns`
- remove `pending_tag_recorded_to_pending_recorded_ns`
- remove `has_pending_capacity_handled`
- remove `has_pending_tag_recorded`

Also remove the declarations for:

- `_handlePendingCapacityHandled(...)`
- `_handlePendingTagRecorded(...)`

### LatencyTracker Decode Logic

File:

- `cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp`

Changes:

- remove any dispatch branch for the two deleted stages
- delete `_handlePendingCapacityHandled(...)`
- delete `_handlePendingTagRecorded(...)`
- change `_handlePendingRecorded(...)` to require `has_order_frame_built`
- compute `order_frame_built_to_pending_recorded_ns` directly from
  `pending_recorded_host_ns - order_frame_built_host_ns`
- keep `_handleTxEnqueue(...)` dependent on `has_pending_recorded`
- keep drop accounting for:
  - `ORDER_FRAME_BUILT -> PENDING_RECORDED`
  - `PENDING_RECORDED -> TX_ENQUEUE`
  - `TX_ENQUEUE -> TX_SEND`

### Latency Log Output

File:

- `cpp_src/FPGA_boost_demo/latency/log_printer.cpp`

Changes:

- remove these printed keys:
  - `order_frame_built_to_pending_capacity_handled_ns`
  - `pending_capacity_handled_to_pending_tag_recorded_ns`
  - `pending_tag_recorded_to_pending_recorded_ns`
- keep printing `order_frame_built_to_pending_recorded_ns`

This is an intentional breaking stdout format change.

## Error Handling

The ordering rules stay strict.

If `PENDING_RECORDED` arrives before `ORDER_FRAME_BUILT`:

- increment the drop counter for `ORDER_FRAME_BUILT -> PENDING_RECORDED`
- discard the pending event state

If `TX_ENQUEUE` arrives before `PENDING_RECORDED`:

- increment the drop counter for `PENDING_RECORDED -> TX_ENQUEUE`
- discard the pending event state

There is no runtime fallback for the removed stage values. Because the enum
entries are deleted, any remaining producer or decoder references should fail at
compile time.

## Testing

### Update Existing Tests

Update:

- `cpp_src/FPGA_boost_demo/tests/latency_tracker_test.cpp`
- `cpp_src/FPGA_boost_demo/tests/log_printer_test.cpp`
- any affected `tx_sender` tests that reference the old pending stages or old
  latency log schema

### Required Assertions

Tests should verify:

- only `ORDER_FRAME_BUILT`, `PENDING_RECORDED`, `TX_ENQUEUE`, and `TX_SEND` are
  needed in the pending section
- `order_frame_built_to_pending_recorded_ns` is still emitted
- the three removed pending substage keys are not emitted
- `LatencyTracker` drop behavior now uses
  `ORDER_FRAME_BUILT -> PENDING_RECORDED` directly
- the project still builds cleanly after the enum removal

### Targeted Verification

Run targeted unit tests for:

- `latency_tracker_test`
- `log_printer_test`
- relevant `tx_sender` tests

## Risks

- downstream parsers that grep for the removed stdout keys will need updating
- any forgotten references to the removed enum values will fail the build until
  cleaned up
- tests that were asserting the old multi-stage pending breakdown must be
  rewritten to the new contract instead of partially preserved

## Acceptance Criteria

- `PENDING_CAPACITY_HANDLED` and `PENDING_TAG_RECORDED` do not exist in shared
  stage definitions
- `TxSender` no longer pushes those stages
- `LatencyTracker` no longer stores, decodes, or logs those stages
- `LatencyLogRecord` no longer carries the removed fields
- stdout latency output prints only
  `order_frame_built_to_pending_recorded_ns` for the pending-record interval
- updated unit tests enforce the new hard-cut contract
