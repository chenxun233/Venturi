# Venturi TX Thread And Buffer Design

Date: 2026-03-31

## Goal

Refine the TX architecture in `FPGA_boost_demo` so that:

- `Venturi.cpp` remains the top-level client application
- `dummy_exchange_server` remains a separate executable
- `Executor` and `TxEngine` have an explicit producer/consumer boundary
- `Executor` writes TX-ready outbound payload records into a TX-owned buffer
- `Executor` writes outbound payloads that are already ready to be sent by `TxEngine`
- `Venturi.cpp` owns the TX thread explicitly
- `TxEngine` runs on that thread and handles send, reconnect, heartbeat, and feedback receive

The transport remains OUCH-over-SoupBinTCP over the direct physical NIC link.

## Scope

### In Scope

- Keep client logic inside `Venturi.cpp`
- Keep server logic in `dummy_exchange_server`
- Add a dedicated TX worker thread owned by `Venturi.cpp`
- Reuse the existing `TraceBuffer<T>` for the executor-to-TX handoff
- Keep queue overflow policy as drop-oldest
- Push TX feedback to the existing async log printer
- Print key TX events immediately
- Limit behavior changes to `cpp_src/FPGA_boost_demo`

### Out of Scope

- Any behavioral change to `Intel_demo`
- A new custom queue implementation unrelated to `TraceBuffer`
- Returning to direct executor-driven transport progress
- Removing the separate exchange server

## Existing Problem

The current relationship between `Executor` and `TxEngine` is still too implicit. Even if transport is logically separated, the current design makes `Executor` aware of transport progression and lets it call into `TxEngine` directly to perform send-side behavior.

That is not the desired boundary.

The desired boundary is:

- `Executor` only produces outbound execution records
- `TxEngine` owns transport and feedback handling

## Architecture

### `Venturi.cpp`

`Venturi.cpp` remains the top-level application and continues to own:

- RX path
- strategy
- executor
- latency tracker
- async log printer

New responsibility:

- create `TxEngine`
- attach the shared log printer to it
- create the TX `std::thread` at startup
- join the TX `std::thread` at shutdown

### `Executor`

`Executor` becomes a pure producer for TX.

Responsibilities:

- consume strategy-generated intents from existing per-producer intent buffers
- record execution-side logs as it already does
- convert the internal execution decision into a TX handoff payload that is already ready for the TX module to send
- push that record into `TxEngine`’s exposed buffer interface

`Executor` no longer drives socket progress, reconnect, heartbeat, or feedback polling.

### `TxEngine`

`TxEngine` becomes a passive TX subsystem with a blocking run loop.

Responsibilities:

- expose a producer-facing buffer write interface for executor
- own the SoupBinTCP session and socket
- continuously:
  - read from the TX handoff buffer
  - send queued orders immediately if connected
  - attempt reconnect when disconnected
  - resend pending unconfirmed orders after reconnect
  - emit Soup client heartbeats
  - check for feedback from the server
  - decode OUCH responses
  - push feedback into the log printer

Interface shape:

- `pushPayload(const TxOutboundRecord&)`
- `run(const std::atomic<bool>& running)`

`TxEngine` should not own thread lifecycle internally. It owns TX state and behavior, while `Venturi.cpp` owns the thread that calls `run(...)`.

### `dummy_exchange_server`

No structural change:

- remains a separate executable
- remains the only server-side process

## Explicit Executor/TxEngine Boundary

The key design rule is:

- `Executor` writes
- `TxEngine` reads

There should be no hidden transport progression embedded inside executor logic, and no hidden thread lifecycle embedded inside `TxEngine`.

The interface should read naturally as a producer/consumer handoff, for example:

- `executor.attachTx(tx_engine)`
- `tx_engine.pushPayload(...)`

and `Venturi.cpp` should create the worker thread explicitly around `tx_engine.run(running)`.

## Buffer Design

The executor-to-TX handoff must reuse the existing `TraceBuffer<T>` implementation already present in:

- `cpp_src/FPGA_boost_demo/latency/trace_buffer.h`

No new general-purpose queue type should be introduced for this handoff.

### Producer/Consumer Model

- producer: `Executor`
- consumer: the TX thread running `TxEngine::run(...)`
- queue type: `TraceBuffer<TxOutboundRecord>`

This matches the intended single-producer/single-consumer usage pattern of `TraceBuffer`.

### TX Record Type

The design should add a focused TX handoff record inside `FPGA_boost_demo`, carrying a payload that is already ready to be sent plus the metadata `TxEngine` needs for tracking and logging, such as:

- prebuilt outbound OUCH payload bytes
- `UserRefNum`
- stock locate
- price
- shares
- any local timestamps needed for TX logging

This record should be TX-specific and not leak unrelated executor internals into the transport layer. `TxEngine` should not need to reinterpret strategy intent in order to build the outbound application payload. It should receive something that is already logically ready to transmit.

### Overflow Policy

Queue overflow policy remains:

- drop oldest

Behavior:

- if the TX handoff buffer is full, the oldest pending unsent TX record is discarded
- the drop is printed immediately
- the drop is also sent to the async log printer as a TX event

The policy is intentionally recent-biased so the system keeps the newest strategy decisions under overload.

## Threading Model

The application should end up with five functional threads:

- control/sync thread
- RX thread 0
- RX thread 1
- executor thread
- TX thread

This is acceptable because the TX thread has one clear purpose and makes the subsystem boundary explicit.

The TX thread, created and joined by `Venturi.cpp`, should be the only thread that:

- touches the TX socket
- advances SoupBinTCP session state
- interprets exchange feedback

This keeps ownership clear and avoids shared transport state across threads.

## TX Send/Receive Behavior

### Send Path

1. Strategy produces an order intent.
2. `Executor` drains its existing intent buffer.
3. `Executor` converts the intent into a TX handoff payload record that is already ready to be sent by `TxEngine`.
4. `Executor` pushes that record into `TxEngine`’s `TraceBuffer`.
5. the TX thread running `TxEngine::run(...)` pops the record and sends it immediately if connected.

### Disconnected Path

If disconnected:

- `TxEngine` continues to drain new handoff records into its internal pending-order tracking state
- `TxEngine` retries connection independently
- after reconnect, `TxEngine` resends pending unconfirmed orders in order

### Feedback Path

The TX thread also loops on receive:

- read Soup packets
- decode sequenced OUCH responses
- update pending-order state
- push feedback to `LatencyLogPrinter`

This means feedback handling belongs entirely to `TxEngine`, not to `Executor`.

## Logging Design

TX logging remains dual-path:

### Immediate prints

Print immediately when:

- connection established
- connection lost
- order sent
- order rejected
- order filled
- order dropped due to overflow

### Async log printer

Also push corresponding structured TX events into `LatencyLogPrinter`.

This keeps TX feedback integrated with the rest of `Venturi.cpp` logging while still making important operational events visible immediately.

## Replay And Order Tracking

`TxEngine` still owns replay state.

Rules:

- unconfirmed outbound orders remain replay candidates
- after a sequenced `Accepted` or `Rejected`, the order leaves the replay set
- later `Executed` updates final order outcome

Replay is owned by `TxEngine`, not by executor, because replay is a transport/session responsibility.

## Scope Boundary

All behavioral changes should remain inside:

- `cpp_src/FPGA_boost_demo/app/`
- `cpp_src/FPGA_boost_demo/tx/`
- `cpp_src/FPGA_boost_demo/latency/`
- `cpp_src/FPGA_boost_demo/common/`
- `cpp_src/FPGA_boost_demo/tests/`

The only acceptable non-`FPGA_boost_demo` change is shared build wiring in:

- `cpp_src/CMakeLists.txt`

There should be no change in `Intel_demo` behavior or design.

## Testing Strategy

### Unit-Level

- `TraceBuffer`-backed TX handoff record push/pop
- drop-oldest overflow behavior
- TX replay set maintenance
- TX log event creation

### Integration-Level

- `Venturi.cpp` builds with explicit top-level TX thread ownership
- `dummy_exchange_server` still works with the integrated client
- send path from executor to TX buffer to socket works
- receive path from socket to feedback log printer works
- reconnect path still resends pending unconfirmed orders

### Operator-Level Demo

Two processes:

1. `dummy_exchange_server`
2. `Venturi.cpp`-based top app

Inside the client process, TX is a separate top-level thread created in `Venturi.cpp`, with explicit buffer ownership inside `TxEngine`.

## Implementation Boundary

The first implementation plan should:

- refactor the executor/TX relationship into explicit producer/consumer form
- reuse `TraceBuffer` for executor-to-TX handoff
- move TX thread ownership to `Venturi.cpp`
- make `TxEngine` a passive runnable with `run(const std::atomic<bool>& running)`
- keep the exchange as a separate executable
- keep all feature changes inside `FPGA_boost_demo` plus required CMake wiring

The plan should not:

- add a new generic queue implementation
- move behavior into `Intel_demo`
- revert to executor-driven transport progress
