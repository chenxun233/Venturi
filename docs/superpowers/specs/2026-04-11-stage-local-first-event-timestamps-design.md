# Stage-Local First-Event Timestamp Outputs Design

## Goal

Move first-event latency timestamp capture into the modules where each stage boundary actually occurs, while keeping `Venturi.cpp` as the explicit coordinator that pushes `TimeRecord`s.

The new design should:

- stop capturing decode, strategy, executor, `TX_ENQUEUE`, and `TX_SEND` timestamps in `Venturi.cpp`
- let each stage capture its own timestamp at the closest practical point
- expose those timestamps through optional output parameters on existing APIs
- apply only to the first-event latency-tracked path
- preserve current behavior when callers do not request timestamps

## Current Problems

- `Venturi.cpp` currently calls `readSystemTimeNs(...)` after work has already happened in lower layers
- decode timing is especially late because `pollDecodedBatchSync()` returns only after decoding the full batch
- strategy, executor, enqueue, and send timing are also sampled outside the modules that perform the work
- this folds unrelated top-layer latency into stage measurements

## Chosen Direction

Keep the explicit top-level pipeline in `Venturi.cpp`, but extend the relevant stage APIs with optional output parameters for first-event timestamps.

Each module captures a timestamp internally at the real stage boundary and writes it to the caller-provided output only when the current item belongs to the first-event tracked path.

`Venturi.cpp` remains responsible for:

- orchestrating the pipeline
- deciding whether the item is latency-tracked
- pushing `TimeRecord`s into `LatencyTracker`

## Scope

### Files In Scope

- `cpp_src/FPGA_boost_demo/app/Venturi.cpp`
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

## API Pattern

The common pattern is:

- keep the existing payload result
- add an optional `uint64_t*` output parameter for the stage-local timestamp
- let callers pass `nullptr` when they do not need the timestamp

This design intentionally does not introduce a new wrapper type for each stage. The chosen tradeoff is lower churn and minimal surface-area change, even if the signatures become slightly longer.

## RxEngine Boundary

`FPGARxEngine::pollDecodedBatchSync()` and the shared implementation path should gain an optional output parameter:

- `uint64_t* first_event_decode_time_ns`

Behavior:

- initialize the output to `0` when provided
- decode records as today
- when the first decoded record in the batch with `is_first_event` is found, capture the host time immediately after `decodeRawRecord(...)`
- store that value once and leave later first-event records in the same batch ignored for this output

This is the closest practical decode boundary without redesigning the batch return format.

## Strategy Boundary

`BasicStrategy::evaluateEvent(...)` and `DummyStrategy::evaluateEvent(...)` should gain an optional output parameter:

- `uint64_t* strategy_time_ns`

Behavior:

- if the strategy rejects the event, return `false` and leave the timestamp unset or zeroed according to the chosen contract
- if the strategy accepts the event and the caller requested timing for a first-event path, capture the timestamp immediately before returning `true`

The strategy module should not decide whether an event is globally latency-tracked. `Venturi.cpp` still decides when to pass a non-null output pointer.

## Executor Boundary

The executor stage boundary for latency purposes is when the intent is accepted into the next queue.

`Executor::acceptIntent(...)` should gain an optional output parameter:

- `uint64_t* executor_time_ns`

Behavior:

- when the push into the executor buffer succeeds and the caller requested timing, capture the timestamp immediately after the successful accept
- on failure, do not stamp a time

This keeps the stage meaning aligned with the queue acceptance point rather than the later polling point in the executor thread.

## TX Enqueue Boundary

The `TX_ENQUEUE` stage should correspond to the moment a tracked order becomes ready outbound inside `TxTranslator`.

This design makes that boundary explicit:

- `TX_ENQUEUE` means the tracked order has entered the ready-to-send outbound queue, not merely the translator’s internal intent buffer

Therefore the timestamp should be produced in the code path that builds and queues the ready outbound record, not in `TxTranslator::acceptIntent(...)` if that API remains only an input-buffer push.

The implementation should expose an optional timestamp output on the `TxTranslator` API boundary that `Venturi.cpp` uses to advance accepted intents into ready outbound state, and the code comments should state clearly that the timestamp is taken when the record becomes ready outbound.

## TX Send Boundary

`TxEngine::sendOutboundRecord(...)` should gain an optional output parameter:

- `uint64_t* tx_send_time_ns`

Behavior:

- if the payload send succeeds and the caller requested timing, capture the timestamp immediately after the successful socket send path returns
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

- decide whether the current item is the first-event tracked path
- pass a non-null timestamp output pointer only for that tracked path
- push `TimeRecord`s using the timestamps returned by the modules

`Venturi.cpp` remains the only place that assembles the ordered latency records for an event.

## Timestamp Contract

For consistency, all new optional timestamp outputs should follow one explicit contract.

Recommended contract:

- when a non-null output pointer is provided, the callee sets `*out = 0` on entry
- if the tracked stage boundary is crossed successfully, the callee overwrites `*out` with the captured time
- callers treat `0` as “no timestamp produced”

This avoids leaving stale caller values behind.

## Error Handling

- Passing `nullptr` must preserve existing behavior exactly.
- A missing timestamp must never cause the stage operation itself to fail.
- Failed stage operations must not publish a fake timestamp.
- The timestamp outputs are observational data only; payload success/failure remains the source of truth.

## Test Strategy

Update tests around the affected APIs to verify:

- existing behavior is unchanged when callers do not request timestamps
- first-event tracked calls populate the timestamp output on successful stage transitions
- rejected or failed paths leave the timestamp at `0`
- `Venturi`-level latency record creation consumes module-provided timestamps rather than local captures

The tests do not need to validate exact wall-clock values. They only need to validate:

- nonzero timestamp when expected
- zero timestamp when not expected
- no regression in payload flow

## Risks

- output-parameter signatures are lower churn, but they can become awkward if more metadata is added later
- `TX_ENQUEUE` needs a clear boundary definition or the timestamp can still be semantically misleading
- if multiple first-event records appear in one batch, the RxEngine output must document that it reports only the first decoded one

## Verification

After implementation:

- build the affected test targets
- run the updated unit tests for RxEngine, strategy, executor, translator, and regression-adjacent latency behavior as applicable
- run any focused `ctest` selection covering the touched modules

## Non-Goals

- per-item timestamps for all events
- changing latency record structure
- removing the explicit orchestration role of `Venturi.cpp`
