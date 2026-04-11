# Stage-Local First-Event Timestamp Outputs Design

## Goal

Move first-event latency timestamp capture into the modules where each stage boundary actually occurs, while keeping `Venturi.cpp` as the explicit coordinator that pushes `TimeRecord`s.

The new design should:

- stop capturing decode, strategy, executor, `TX_ENQUEUE`, and `TX_SEND` timestamps in `Venturi.cpp`
- let each stage capture its own timestamp at the closest practical point
- keep timestamp data attached to the produced item when one call can emit multiple items
- apply only to the first-event latency-tracked path
- make only minimal structural modifications to existing APIs and payload flow
- preserve current behavior when callers do not request timestamps

## Current Problems

- `Venturi.cpp` currently calls `readSystemTimeNs(...)` after work has already happened in lower layers
- decode timing is especially late because `pollDecodedBatchSync()` returns only after decoding the full batch
- strategy, executor, enqueue, and send timing are also sampled outside the modules that perform the work
- this folds unrelated top-layer latency into stage measurements

## Chosen Direction

Keep the explicit top-level pipeline in `Venturi.cpp`, but move stage-local timing into the payloads and wrappers produced by the modules.

The minimal-change rule is:

- use a small wrapper only where one call can emit multiple payloads and a separate scalar timestamp would lose attribution
- otherwise attach first-event timing metadata directly to the existing payload that already crosses the stage boundary

This yields:

- RX batch output becomes a small wrapper around `FPGAEventDesc` so each decoded event can carry its own decode-time metadata
- `OrderIntent` carries strategy and executor timing only for tracked first events
- `TxOutboundRecord` carries enqueue and send timing only for tracked first events

`Venturi.cpp` remains responsible for:

- orchestrating the pipeline
- deciding whether to push a latency record
- pushing `TimeRecord`s into `LatencyTracker`

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
- affected tests for these APIs

### Files Out Of Scope

- changing the latency stage model itself
- recording per-item timestamps for non-first-event traffic
- moving `pushRecord()` calls out of `Venturi.cpp`
- redesigning regression or latency aggregation logic
- redesigning the way that modules are wired in currently

## API Pattern

The common pattern is per-item timing metadata rather than per-call scalar outputs.

Use:

- a small wrapper where one API call can emit multiple payloads
- inline metadata fields on existing payloads where one payload already crosses the stage boundary

Do not add wrapper types everywhere. The point is to preserve attribution with the least change needed.

## RxEngine Boundary

`FPGARxEngine` is the one place where a separate scalar output is not sufficient, because one poll call can return multiple decoded events.

Replace raw batch output with a small decoded-event wrapper, for example:

- `DecodedEvent { FPGAEventDesc event; uint64_t decode_time_ns; }`

Behavior:

- decode records as today
- when a decoded record has `is_first_event`, capture the host time immediately after `decodeRawRecord(...)`
- store that value in the wrapper for that record only
- non-first-event wrappers keep `decode_time_ns == 0`

This preserves correct attribution across multi-event batches with minimal redesign.

## Strategy Boundary

`BasicStrategy::evaluateEvent(...)` and `DummyStrategy::evaluateEvent(...)` should stamp the output `OrderIntent` directly.

Behavior:

- if the strategy rejects the event, return `false` and leave timing metadata at `0`
- if the strategy accepts a tracked first event, capture the timestamp immediately before returning `true`
- store it inside the produced `OrderIntent`

This keeps the timestamp attached to the intent that moves into the next stage without adding a separate side channel.

## Executor Boundary

The executor stage boundary for latency purposes is when the intent is accepted into the next queue.

`Executor::acceptIntent(...)` should stamp the accepted `OrderIntent` metadata that is already travelling through the queue.

Behavior:

- when the push into the executor buffer succeeds for a tracked first-event intent, capture the timestamp immediately after the successful accept
- store it on the intent before it is queued
- on failure, do not invent timing metadata

This keeps the stage meaning aligned with the queue acceptance point rather than the later polling point in the executor thread.

## TX Enqueue Boundary

The `TX_ENQUEUE` stage should correspond to the moment a tracked order becomes ready outbound inside `TxTranslator`.

This design makes that boundary explicit:

- `TX_ENQUEUE` means the tracked order has entered the ready-to-send outbound queue, not merely the translator’s internal intent buffer

Therefore the timestamp should be produced in the code path that builds and queues the ready outbound record, and stored on the produced `TxOutboundRecord`.

`TxTranslator::acceptIntent(...)` remains an input-buffer push only. The enqueue timing belongs on the outbound record created later, not on the accepted input call.

## TX Send Boundary

`TxEngine::sendOutboundRecord(...)` should stamp the `TxOutboundRecord` it is sending when that record belongs to the tracked first-event path.

Behavior:

- if the payload send succeeds for a tracked outbound record, capture the timestamp immediately after the successful socket send path returns
- store it on the `TxOutboundRecord`
- if send fails, do not report a send timestamp

This keeps `TX_SEND` close to the actual outbound transport boundary.

## Venturi Coordination

`Venturi.cpp` should stop calling `readSystemTimeNs(...)` for these stages:

- `DECODE`
- `STRATEGY`
- `EXECUTOR`
- `TX_ENQUEUE`
- `TX_SEND`

Instead:

- consume stage-local timing metadata attached to decoded events, intents, and outbound records
- push `TimeRecord`s only when the corresponding timing metadata is nonzero

For `FRAME_START` and `DMA_EMIT`:

- keep `time_captured` as the raw FPGA tick already carried by the event data
- do not convert them in `Venturi.cpp`
- keep host-side timestamp fields at `0` before conversion
- let `LatencyTracker` continue doing FPGA-to-host conversion internally for those stages

`Venturi.cpp` remains the only place that assembles the ordered latency records for an event.

## Timestamp Contract

For consistency, all new timing metadata fields should follow one explicit contract.

Recommended contract:

- timing metadata fields default to `0`
- if the tracked first-event stage boundary is crossed successfully, the producing module overwrites the field with the captured time
- callers treat `0` as “no host timestamp produced”

This avoids stale values and keeps non-first-event items lightweight.

## Error Handling

- Missing timing metadata must never cause the stage operation itself to fail.
- Failed stage operations must not publish fake timestamps.
- Timing metadata is observational data only; payload success/failure remains the source of truth.

## Test Strategy

Update tests around the affected APIs to verify:

- existing payload flow is unchanged for non-first-event traffic
- tracked first-event items carry nonzero timing metadata at the right stage boundaries
- rejected or failed paths leave timing metadata at `0`
- `Venturi`-level latency record creation consumes module-provided timing metadata rather than local captures
- FPGA-tick-based stages still rely on `LatencyTracker` conversion instead of top-level host capture

The tests do not need to validate exact wall-clock values. They only need to validate:

- nonzero timestamp when expected
- zero timestamp when not expected
- no regression in payload flow

## Risks

- the RX wrapper adds one targeted batch-type change, which is the minimum needed to keep per-event attribution correct
- adding timing metadata into `OrderIntent` and `TxOutboundRecord` slightly broadens those payloads, so the fields must stay clearly documented as first-event latency metadata
- `TX_ENQUEUE` still depends on a clear ready-outbound boundary definition in translator code comments

## Verification

After implementation:

- build the affected test targets
- run the updated unit tests for RxEngine, strategy, executor, translator, and regression-adjacent latency behavior as applicable
- run any focused `ctest` selection covering the touched modules

## Non-Goals

- per-item timestamps for all events
- changing latency record structure
- removing the explicit orchestration role of `Venturi.cpp`
- broad wrapper types for every stage
