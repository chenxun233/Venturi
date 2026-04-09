# Venturi Explicit Client Pipeline Design

Date: 2026-04-09

## Goal

Refactor the current client-side runtime so `Venturi.cpp` becomes the explicit composition root and pipeline wiring map for:

- instance creation
- thread allocation
- cross-thread message flow
- latency stage recording

The design also moves sync bootstrap out of `Venturi.cpp`, moves regression reporting responsibility out of inline `printf` calls, renames `LatencyLogPrinter` to `LogPrinter`, and completes the client-side latency path beyond decode.

## Scope

### In Scope

- Make `Venturi.cpp` explicitly show module wiring, thread ownership, and data flow
- Remove downstream module ownership by pointer or reference from the main client pipeline
- Move initial sync convergence from `Venturi.cpp` into `SyncHandler`
- Move regression reporting data ownership into `Regression`
- Move regression-status printing into `LogPrinter`
- Rename `LatencyLogPrinter` to `LogPrinter`, including file names and call sites
- Extend latency tracking to the full client-side stage chain
- Make cross-thread communication explicit and message-based

### Out of Scope

- Measuring server-side latency after the order leaves the client
- Hiding the pipeline inside a new coordinator class
- Changing the high-level product behavior of strategy, executor, translator, or transport
- Generalizing this into a reusable framework

## Chosen Approach

Use an explicit stage-driven runtime owned by `Venturi.cpp`.

`Venturi.cpp` stays as the only top-level wiring point. It creates components, allocates threads, and explicitly transfers messages from one stage to the next. Modules stop owning hidden downstream references for the main client pipeline. Instead, each module exposes processing methods plus output and input boundaries that `Venturi` wires together directly.

This is the best fit because it preserves explicitness at the application entry point while improving both latency observability and future modification safety.

## Why This Pattern Fits

This pattern is a good fit for this codebase if the implementation keeps hot-path boundaries thin.

It is good for future extension because:

- pipeline structure is visible in one place
- cross-thread boundaries become explicit message contracts
- modules can evolve without hidden call chains into downstream modules
- new measured stages can be inserted without burying more logic into opaque `run()` methods

It is good for low latency because:

- every queue hop and stage transition can be measured explicitly
- hot-path work is easier to inspect for copies, queueing, and blocking
- thread handoff cost is visible rather than inferred

The condition is that the message boundary APIs must remain fixed-capacity and allocation-free on the hot path. The design should not introduce heavy wrapper objects, dynamic dispatch, or formatting work in stage methods.

## Architectural Rules

The following rules define the design.

### Venturi As Composition Root Only

`Venturi.cpp` is responsible for:

- creating module instances
- attaching only stable non-pipeline dependencies
- allocating and joining threads
- explicitly calling stage methods in each thread
- explicitly moving messages between stages
- explicitly pushing latency records at stage boundaries
- handling top-level startup failure and shutdown

`Venturi.cpp` must not own:

- sync convergence logic
- inline regression-status formatting
- hidden pipeline logic buried in opaque `run()` methods
- hidden next-module wiring through attached downstream pointers

### Message-Based Boundaries

The main client pipeline must use explicit message passing.

A stage boundary works like this:

1. module A processes input and exposes output
2. `Venturi.cpp` reads the output
3. `Venturi.cpp` pushes the corresponding latency record
4. `Venturi.cpp` passes the message into module B through an explicit accept method
5. if the boundary crosses threads, that accept method is backed by `TraceBuffer`

No main-pipeline module should directly own the next module via pointer or reference for forwarding work across the pipeline.

### One Explicit Runtime Map

A reader should be able to open `Venturi.cpp` and understand:

- what runs on each thread
- where messages are created
- where messages cross threads
- where latency records are pushed
- what order the client-side stages run in

That readability is a primary design requirement, not an incidental benefit.

## Ownership Split

### SyncHandler

`SyncHandler` owns initial sync convergence.

It should expose a method that:

- polls `FPGADev::readSyncTimestamp(...)`
- filters snapshots by the accepted interval threshold
- updates `Regression`
- stops when regression becomes frozen
- reports success or failure to the caller

`SyncHandler` continues to own periodic capture triggering through `run(...)`.

### Regression

`Regression` owns regression model state and reporting-facing status data.

It must expose a method that returns the current regression status needed for logging, including whether parameters exist and the converted slope value needed by logs.

`Regression` must not print directly.

### LogPrinter

`LatencyLogPrinter` is renamed to `LogPrinter`.

`LogPrinter` owns asynchronous logging output for:

- latency logs
- sync snapshots
- regression status
- execution logs
- TX logs

The rename applies to class name, file names, include paths, and call sites.

### LatencyTracker

`LatencyTracker` owns only:

- event matching
- stage-latency calculation
- drop counting
- per-stage stats
- pushing completed latency records toward `LogPrinter`

`LatencyTracker` must not be owned directly by feature modules for hot-path push calls. `Venturi.cpp` owns when records are pushed.

### Feature Modules

The main client-side modules must expose explicit stage methods rather than hiding the pipeline behind one broad `run()` entry point.

Relevant modules include:

- `FPGARxEngine`
- strategy path
- `Executor`
- `TxTranslator`
- `TxEngine`

These modules must process input, expose output, and accept input. They must not own main-pipeline forwarding into the next module.

## Client-Side Latency Chain

The complete client-side latency chain is:

1. `FRAME_START`
2. `DMA_EMIT`
3. `DECODE`
4. `STRATEGY`
5. `EXECUTOR`
6. `TX_ENQUEUE`
7. `TX_SEND`

The following are explicitly out of scope for this latency chain:

- exchange ACK
- exchange reject
- exchange fill
- inbound handling of server responses

Those include server-side time and are not part of the current client-only latency goal.

## Latency Recording Rules

Latency recording must be explicit in `Venturi.cpp`.

Rule:

- stage methods expose the point where a stage boundary is crossed and provide the event identity needed for correlation
- the corresponding worker thread captures the timestamp itself at that boundary by calling a shared timestamp helper function defined in `Venturi.cpp`
- that same worker thread immediately pushes the resulting `TimeRecord` into `LatencyTracker`
- that same worker thread then performs the data handoff from the previous stage to the next stage

This applies both to in-thread stage transitions and to cross-thread handoffs.

For a same-thread boundary:

- the worker thread calls the shared timestamp helper from `Venturi.cpp`
- the worker thread pushes the resulting `TimeRecord` into `LatencyTracker`
- the worker thread continues to the next stage in the same thread

No extra handoff abstraction is required for same-thread timestamp capture.

For a cross-thread boundary:

- the sending-side thread captures the sending-side timestamp, pushes the corresponding record, and then enqueues the message toward the next stage
- the receiving-side thread captures the receiving-side timestamp when the next stage accepts or consumes the message and then pushes the next record

This makes queueing and handoff latency visible as part of the end-to-end client pipeline.

The required explicit boundary sequence is:

1. get timestamp in the corresponding worker thread through the shared helper in `Venturi.cpp`
2. push the `TimeRecord` into `LatencyTracker` from that same thread
3. push or hand off the data from the previous stage to the next stage from that same thread

Timestamp capture is therefore part of explicit runtime wiring and must not be hidden inside feature modules or moved to a different thread than the stage boundary being measured.

## Module API Direction

The design requires very explicit stage APIs.

### FPGADev

`FPGADev` should expose the hardware-facing RX read boundary explicitly rather than only serving as a hidden dependency behind higher layers.

Its explicit responsibility is:

- poll raw RX availability
- expose raw record access
- expose sync snapshot reads
- expose consumption or acknowledgement boundaries back to hardware

`FPGADev` remains hardware-facing and must not own decode logic, engine logic, latency tracking, or downstream pipeline wiring.

The explicitness goal at this layer is not “one method per register.” It is one method per meaningful hardware-stage boundary so that `Venturi.cpp` can understand where raw data enters the client pipeline.

### FPGARxDecoder

`FPGARxDecoder` should expose translation-stage methods explicitly between `FPGADev` and `FPGARxEngine`.

Its explicit responsibility is:

- accept raw records from the RX source
- decode raw bytes into `FPGAEventDesc`
- expose first-event information and event identity needed for latency correlation
- expose sync-aware decode boundaries when sync snapshots are sampled

`FPGARxDecoder` must stay translation-only. It must not own engine orchestration, latency pushes, regression updates, downstream module wiring, or the RX batch loop.

### FPGARxEngine

`FPGARxEngine` should act as the RX-subpipeline composition root.

It explicitly wires `FPGADev` and `FPGARxDecoder` together inside the RX stage family in the same style that `Venturi.cpp` uses for the major application pipeline.

That means `FPGARxEngine` owns the local RX batch loop and explicitly coordinates:

- raw record access from `FPGADev`
- raw-to-event translation through `FPGARxDecoder`
- sync-aware polling variants
- decoded batch exposure upward to the rest of the client pipeline

`FPGARxEngine` is therefore the top of the RX subpipeline, but not the top of the whole application.

It must not own:

- latency tracking
- regression side effects
- log printing
- downstream business stages

Those remain explicit at the higher `Venturi.cpp` layer.

Expose methods that let `Venturi.cpp` observe and wire:

- frame start capture
- DMA emit boundary
- decode completion

The engine should provide enough event identity data for `Venturi` to push `FRAME_START`, `DMA_EMIT`, and `DECODE` records explicitly. The timestamps for those records should be captured by the corresponding RX worker thread through the shared timestamp helper in `Venturi.cpp` rather than being supplied by the engine.

This means explicitness starts at the hardware read boundary, but the local loop remains owned by the RX-stage composition root:

`FPGADev -> FPGARxDecoder -> FPGARxEngine -> strategy`

That makes the RX side readable as a staged client pipeline rather than a single opaque “poll and decode” operation, without pushing the RX batch loop into `Venturi.cpp`.

### Strategy Path

Expose a strategy-processing method that returns or emits explicit strategy output. `Venturi.cpp` pushes the `STRATEGY` record at that point.

### Executor

Expose executor-stage methods that:

- accept strategy-side messages explicitly
- process one executor step explicitly
- expose executor output explicitly

`Venturi.cpp` pushes the `EXECUTOR` record at the executor-stage boundary.

### TxTranslator

Expose explicit TX-enqueue boundaries rather than hiding them behind attached downstream references. `Venturi.cpp` pushes the `TX_ENQUEUE` record when the translator-side output is accepted into the TX path.

### TxEngine

Expose explicit send-stage boundaries so `Venturi.cpp` can push `TX_SEND` when a payload is actually handed to transport for sending.

## Cross-Thread Communication

Cross-thread communication must also be explicit in `Venturi.cpp`.

The required pattern is:

- module 1 exposes output
- module 2 exposes an accept method
- if the boundary crosses threads, the accept method is implemented using `TraceBuffer`
- `Venturi.cpp` performs the handoff explicitly

This removes hidden ownership chains and makes both control flow and data flow visible in one file.

## Alternatives Considered

### Keep Current Attached-Module Wiring

The current pattern lets modules own downstream references and bury latency pushes inside internal `run()` functions.

This is not chosen because it hides data flow, hides thread communication, and makes stage latency ownership unclear.

### Put Latency Recording Inside Each Module

This would reduce call-site work in `Venturi.cpp`.

It is not chosen because it makes the latency path harder to understand from the application entry point, which directly conflicts with the design goal.

### Introduce A New Runtime Coordinator Class

This could centralize the pipeline without growing `Venturi.cpp`.

It is not chosen because the requirement is to keep `Venturi.cpp` explicit and avoid introducing another top-level owner.

## Error Handling

The refactor should preserve current failure behavior at the application level.

In particular:

- sync initialization failure should still stop startup cleanly
- queue push failure behavior at explicit cross-thread boundaries must remain deterministic
- latency recording failure must not silently change business-path behavior
- logging must remain asynchronous and must not add blocking work to hot stages

## Testing And Validation

The implementation must verify all of the following:

- `Venturi.cpp` is readable as the full pipeline and thread wiring map
- sync initialization moved into `SyncHandler`
- regression status generation moved into `Regression`
- printing moved into `LogPrinter`
- `LatencyLogPrinter` rename to `LogPrinter` is complete
- no main-pipeline module still owns downstream forwarding references where explicit wiring now applies
- stage latency is recorded for:
  `FRAME_START -> DMA_EMIT -> DECODE -> STRATEGY -> EXECUTOR -> TX_ENQUEUE -> TX_SEND`
- cross-thread handoff latency is visible through explicit send and receive stage records
- hot-path queue boundaries remain fixed-capacity and allocation-free

## Implementation Notes

- Keep the refactor focused on existing classes; do not introduce a new coordinator class
- Prefer small, stage-shaped methods over one large replacement `run()` method
- Preserve existing thread model unless an explicit boundary change is required by the new message flow
- Keep the hot path free of synchronous printing and formatting
