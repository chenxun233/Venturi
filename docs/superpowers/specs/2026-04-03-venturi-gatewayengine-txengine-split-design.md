# Venturi GatewayEngine And TxEngine Split Design

Date: 2026-04-03

## Goal

Make `TxEngine` reusable across projects by removing gateway-specific protocol logic from it and introducing a gateway-specific protocol layer above it.

The intended stack becomes:

- `Executor`
- `GatewayEngine`
- `TxEngine`

`GatewayEngine` owns all OUCH and SOUP protocol/session behavior for this project, while `TxEngine` becomes a generic transport engine.

## Scope

### In Scope

- introduce a gateway-specific layer between `Executor` and `TxEngine`
- move OUCH and SOUP-specific behavior out of `TxEngine`
- keep `GatewayEngine` and `TxEngine` in the same TX thread
- define a message/payload-oriented interface between `GatewayEngine` and `TxEngine`
- update tests to match the new layering

### Out of Scope

- multi-threading between `GatewayEngine` and `TxEngine`
- transport optimization changes unrelated to the split
- strategy/executor producer queue redesign
- changing the functional gateway protocol behavior

## Current Problem

`TxEngine` currently mixes several responsibilities:

- outbound order/session record handling
- SOUP login, heartbeat, reconnect, and framing
- socket I/O and connection lifecycle
- inbound SOUP packet polling and parsing
- OUCH response interpretation
- protocol-specific logging of gateway events

This makes `TxEngine` specific to the current OUCH-over-SOUP gateway design instead of reusable as a generic transport module.

If another project needs a different protocol format, or the current project changes the gateway protocol stack, the current `TxEngine` shape forces protocol-specific changes into the transport engine.

## Design Summary

Refactor the TX path into three layers:

- `Executor`
  - execution-stage queue draining and intent forwarding
- `GatewayEngine`
  - all gateway-specific protocol/session semantics for this project
- `TxEngine`
  - generic transport runtime and payload movement

The design keeps `GatewayEngine` and `TxEngine` in the same thread so the runtime ordering model and latency profile stay simple.

## Architecture

### Executor

`Executor` remains responsible for:

- owning the producer-side `OrderIntent` queues
- draining those queues
- emitting execution-intent log records
- forwarding pure `OrderIntent` values into the TX-side stack

`Executor` does not own OUCH, SOUP, connection state, or resend/session behavior.

### GatewayEngine

`GatewayEngine` is the protocol-specific layer for this project. It owns:

- `OrderIntent` to outbound order mapping
- `user_ref_num` assignment
- OUCH outbound encode
- OUCH inbound decode
- SOUP login, heartbeat, reconnect, framing, and protocol rules
- protocol-specific resend/session behavior
- interpretation of server feedback into order/session outcomes

`GatewayEngine` is the only layer that understands the gateway protocol stack used by this project.

### TxEngine

`TxEngine` becomes a generic transport layer. It owns:

- generic outbound payload queueing
- generic payload send/receive runtime behavior
- generic connection lifecycle hooks if those hooks are transport-generic
- delivering raw inbound payloads or transport events back to the caller

`TxEngine` does not understand:

- OUCH
- SOUP
- order state
- `user_ref_num`
- exchange-specific feedback semantics

This is what makes it reusable.

## Threading Model

`GatewayEngine` and `TxEngine` run in the same TX thread.

This is intentional:

- preserves the current low-latency execution model
- avoids another queue and synchronization boundary
- keeps outbound ordering and inbound feedback handling easier to reason about
- avoids shutdown and replay complexity introduced by cross-thread coordination

The split is a module boundary, not a thread boundary.

## Thread Arrangement In Venturi.cpp

`Venturi.cpp` should make the module-to-thread arrangement explicit instead of hiding it inside a combined TX module.

The intended top-level arrangement is:

- control thread
- executor thread
- TX thread
- RX thread per queue

Within that arrangement:

- `Executor` runs in the executor thread
- `GatewayEngine` and `TxEngine` are both constructed explicitly in `Venturi.cpp`
- the TX thread runs the gateway/transport pipeline explicitly, not through a monolithic protocol-specific `TxEngine`

The wiring in `Venturi.cpp` should make it obvious which modules share a thread and which modules do not. A reader should be able to identify, from the top-level application wiring alone:

- that `GatewayEngine` and `TxEngine` currently share the TX thread
- that `Executor` is separate
- that the design could later place `GatewayEngine` and `TxEngine` on different threads without redesigning module internals

This does not require splitting them into separate threads now. It requires making the current thread placement a visible application-level decision.

Examples of acceptable explicitness include:

- constructing `GatewayEngine` and `TxEngine` as separate objects in `Venturi.cpp`
- naming the TX thread in terms of the gateway/transport pipeline rather than a single opaque module
- using a top-level lambda or runner object in `Venturi.cpp` that clearly sequences `GatewayEngine` and `TxEngine` within the TX thread

The goal is architectural readability and future reassignment flexibility, not immediate multithreading.

## Interface Boundary

The interface between `GatewayEngine` and `TxEngine` is message/payload oriented.

`GatewayEngine` interacts with `TxEngine` through operations such as:

- queue or send raw outbound payload
- poll or receive raw inbound payload
- observe generic connection state or transport events

`TxEngine` does not expose protocol-specific helper methods.

`GatewayEngine` interacts with `Executor` through pure `OrderIntent` values.

This gives two clear boundaries:

- `Executor` -> `GatewayEngine`: trading intent
- `GatewayEngine` -> `TxEngine`: raw protocol payloads and transport events

## Data Flow

The intended runtime flow is:

1. `Executor` forwards `OrderIntent`
2. the TX thread invokes `GatewayEngine`
3. `GatewayEngine` turns intent into gateway-protocol actions and raw outbound payloads
4. `TxEngine` sends raw payloads and receives raw inbound payloads
5. `GatewayEngine` consumes inbound payloads and interprets them
6. `GatewayEngine` updates session/order state and emits higher-level effects

This keeps protocol transformation and transport movement separate while preserving same-thread ordering.

## Error Handling

Protocol-specific failures stay in `GatewayEngine`, for example:

- invalid order actions
- malformed OUCH payloads
- SOUP login rejection
- protocol-specific disconnect handling rules

Transport-specific failures stay in `TxEngine`, for example:

- socket send failure
- socket receive failure
- generic transport disconnect

`GatewayEngine` decides what protocol-level reaction to take when `TxEngine` reports a generic transport failure.

## Testing

Testing follows the new layering:

- `GatewayEngine` tests:
  - intent-to-payload mapping
  - OUCH encode/decode behavior
  - SOUP protocol/session state behavior
  - order/session state transitions
- `TxEngine` tests:
  - generic payload send/receive behavior
  - connection lifecycle behavior that is not protocol-specific
  - queueing and transport event delivery
- integration tests:
  - `Executor`, `GatewayEngine`, and `TxEngine` cooperation in the same TX thread
  - preserved outbound ordering and inbound feedback flow

This makes each module easier to reason about in isolation.

## Migration Notes

Recommended implementation order:

1. define the new module boundaries and interface types
2. make the thread arrangement explicit in `Venturi.cpp`
3. move protocol-specific logic from `TxEngine` into `GatewayEngine`
4. make `Executor` forward pure intents into `GatewayEngine`
5. reduce `TxEngine` to generic transport behavior
6. update tests around the new layering

The refactor keeps behavior equivalent while changing ownership and boundaries.

## Trade-Offs

### Benefits

- `TxEngine` becomes reusable in other projects
- protocol changes are localized to `GatewayEngine`
- cleaner layering between intent, protocol, and transport
- same-thread runtime keeps latency and ordering behavior simple

### Costs

- adds another module and interface boundary
- requires moving a substantial amount of code out of `TxEngine`
- requires re-anchoring tests to the new ownership model

This is a reasonable trade if generic transport reuse is a real goal.

## Non-Goals

- do not split `GatewayEngine` and `TxEngine` into separate threads
- do not redesign strategy logic or executor queueing in the same change
- do not combine this refactor with unrelated transport/protocol optimizations

## Recommendation

Implement the `Executor -> GatewayEngine -> TxEngine` split.

Keep `GatewayEngine` and `TxEngine` in the same TX thread, but make the module boundary explicit and payload-oriented.

This yields a reusable `TxEngine` without paying the complexity cost of another thread boundary.
