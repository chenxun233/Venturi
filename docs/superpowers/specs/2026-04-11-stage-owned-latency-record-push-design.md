# Stage-Owned Latency Record Push Design

## Goal

Move `latency_tracker.pushRecord(...)` into the stage modules themselves so each stage emits its own latency record exactly where that stage boundary happens, while keeping one shared `LatencyTracker` instance owned and wired by `Venturi.cpp`.

The new design should:

- keep the explicit runtime pipeline in `Venturi.cpp`
- keep a single shared `LatencyTracker` instance
- let each stage attach that tracker through `attachLatenyTracker(...)`
- remove stage timestamp transport through payload metadata
- remove `DecodedEvent` and return plain `FPGAEventDesc` from RX again
- keep `FRAME_START` and `DMA_EMIT` as FPGA-tick records converted later by `LatencyTracker`
- preserve first-event-only latency tracking semantics

## Current Problems

- `Venturi.cpp` still owns all `latency_tracker.pushRecord(...)` calls, so stage-local timing ownership is split between modules and the top-level coordinator
- the previous metadata-based design reduced top-layer timestamp capture, but it still requires timestamps to be carried through payloads and emitted later
- RX had to wrap `FPGAEventDesc` inside `DecodedEvent` only to move decode-time data back up to `Venturi.cpp`
- `OrderIntent` and `TxOutboundRecord` were expanded with latency-only metadata fields that are not part of their core business payload

## Chosen Direction

Keep one explicit pipeline in `Venturi.cpp`, but move latency record emission into the stages themselves.

The rule becomes:

- the stage that knows a boundary happened is also the stage that calls `LatencyTracker::pushRecord(...)`

This means:

- `Venturi.cpp` constructs one `LatencyTracker` and attaches it into each stage
- stages do not return timestamps upward as metadata
- `Venturi.cpp` stops pushing stage records itself
- payloads keep only routing and business data, not latency timestamps

## Scope

### Files In Scope

- `cpp_src/FPGA_boost_demo/app/Venturi.cpp`
- `cpp_src/FPGA_boost_demo/common/shared_types.h`
- `cpp_src/FPGA_boost_demo/rx_engine/fpga_rx_engine.h`
- `cpp_src/FPGA_boost_demo/rx_engine/fpga_rx_engine.cpp`
- `cpp_src/FPGA_boost_demo/strategy/basic_strategy.h`
- `cpp_src/FPGA_boost_demo/strategy/dummy_strategy.h`
- `cpp_src/FPGA_boost_demo/strategy/dummy_strategy.cpp`
- `cpp_src/FPGA_boost_demo/tx_engine/executor.h`
- `cpp_src/FPGA_boost_demo/tx_engine/executor.cpp`
- `cpp_src/FPGA_boost_demo/tx_engine/tx_translator.h`
- `cpp_src/FPGA_boost_demo/tx_engine/tx_translator.cpp`
- `cpp_src/FPGA_boost_demo/tx_engine/tx_engine.h`
- `cpp_src/FPGA_boost_demo/tx_engine/tx_engine.cpp`
- tests covering RX, strategy, executor, translator, TX engine, and latency tracker behavior

### Files Out Of Scope

- redesigning `LatencyTracker` stage ordering or aggregation logic
- changing the single-tracker runtime ownership model
- broad pipeline restructuring beyond moving record emission to stages
- tracking latency for non-first-event traffic

## API Direction

Each stage that can emit latency records gets:

- `void attachLatenyTracker(LatencyTracker* latency_tracker);`

The name is intentionally spelled `attachLatenyTracker(...)` to match the requested API.

Contract:

- attaching `nullptr` is allowed
- if no tracker is attached, the stage behaves normally and simply skips latency emission
- stage success/failure behavior must not depend on latency tracking

## RX Engine

`FPGARxEngine` should stop emitting `DecodedEvent` and go back to emitting plain `FPGAEventDesc` batches.

Responsibilities:

- decode raw records into `FPGAEventDesc`
- when `event.is_first_event != 0`, immediately push:
  - `FRAME_START` with `frame_start_tk`
  - `DMA_EMIT` with `event_tk`
  - `DECODE` with the captured host monotonic timestamp taken immediately after decode

Because RX now emits its own records directly, the extra `DecodedEvent` wrapper is no longer needed.

## Strategy

`BasicStrategy::evaluateEvent(...)` and `DummyStrategy::evaluateEvent(...)` should accept the queue index and, on successful first-event intent creation, push `STRATEGY` directly.

Responsibilities:

- decide whether the event produces an `OrderIntent`
- keep setting `OrderIntent::event_ts` and `OrderIntent::que_idx` for tracked intents
- if the accepted event is first-in-frame and a tracker is attached, capture host time and push `STRATEGY` before returning `true`
- on rejection, emit nothing

`OrderIntent` should no longer carry `captured_time_ns`.

## Executor

`Executor::acceptIntent(...)` should emit `EXECUTOR` when a tracked intent is successfully queued.

Responsibilities:

- preserve queueing semantics
- if the intent is tracked and the push succeeds, capture host time and push `EXECUTOR`
- if the queue push fails, emit nothing

This keeps the executor boundary aligned with successful handoff into the executor-owned queue.

## TX Translator

`TxTranslator` should emit `TX_ENQUEUE` when a tracked order enters the ready-to-send outbound queue.

Responsibilities:

- keep translating accepted intents into `TxOutboundRecord`
- when a tracked outbound record is placed into the ready queue, capture host time and push `TX_ENQUEUE`
- if the record is still blocked waiting for session establishment, emit nothing until it actually becomes ready

`TxOutboundRecord` should no longer carry `captured_time_ns`.

## TX Engine

`TxEngine::sendOutboundRecord(...)` should emit `TX_SEND` immediately after a successful send of a tracked outbound record.

Responsibilities:

- preserve send success/failure semantics
- on successful send of a tracked record, capture host time and push `TX_SEND`
- on send failure, emit nothing and keep existing disconnect handling

## Venturi Coordination

`Venturi.cpp` remains the explicit coordinator for:

- stage construction
- attaching the single `LatencyTracker`
- wiring the pipeline order
- snapshot acceptance and regression servicing

`Venturi.cpp` should stop calling `latency_tracker.pushRecord(...)` for:

- `FRAME_START`
- `DMA_EMIT`
- `DECODE`
- `STRATEGY`
- `EXECUTOR`
- `TX_ENQUEUE`
- `TX_SEND`

It should still:

- create the one `LatencyTracker`
- attach the tracker into all stages during startup
- keep the same explicit handoff order between RX, strategy, executor, translator, and TX engine

## Error Handling

- stages must never emit fake records on failure paths
- `attachLatenyTracker(nullptr)` must be safe
- latency emission failures must not break business logic
- first-event detection remains the source of truth for whether a record should be emitted

## Testing

Tests should shift from timestamp-carrying payload assertions to direct record-emission assertions.

Required coverage:

- RX pushes `FRAME_START`, `DMA_EMIT`, and `DECODE` only for first events
- strategy pushes `STRATEGY` only on accepted first-event paths
- executor pushes `EXECUTOR` only on successful tracked queue accepts
- translator pushes `TX_ENQUEUE` only when tracked outbound records become ready
- TX engine pushes `TX_SEND` only after successful tracked sends
- `Venturi.cpp` no longer pushes stage records directly
- `LatencyTracker` still assembles the stage chain correctly from stage-emitted records
- `DecodedEvent` and payload `captured_time_ns` fields are removed without breaking tracked event identity

## Risks

- direct dependency on `LatencyTracker` inside stages increases coupling relative to the metadata design
- the requested `attachLatenyTracker(...)` spelling becomes public API and should be used consistently if adopted
- moving `FRAME_START` and `DMA_EMIT` push responsibility into RX makes that module responsible for both raw decode and early latency emission, so tests there need to stay tight
- if tracked-event identity fields on `OrderIntent` or `TxOutboundRecord` drift, later-stage record correlation will break even though timestamps are emitted locally

## Non-Goals

- adding a generic latency sink abstraction
- introducing multiple tracker instances
- removing the explicit pipeline from `Venturi.cpp`
- tracking latency for every event instead of first-event traffic only
