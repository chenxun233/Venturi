# Venturi Executor To TX Boundary Design

Date: 2026-04-03

## Goal

Clarify the module boundary between `Executor` and `TxEngine` by moving order-to-wire preparation out of `Executor` and into `TxEngine`.

After this change, `Executor` forwards pure `OrderIntent` values, while `TxEngine` owns transport-facing record construction, `user_ref_num` assignment, OUCH encoding, and session staging.

## Scope

### In Scope

- change the `Executor` to `TxEngine` handoff from transport-ready records to pure intents
- move `user_ref_num` assignment from `Executor` into `TxEngine`
- move OUCH `Enter Order` construction and encoding into `TxEngine`
- make `TxOutboundRecord` a TX-internal transport/session type rather than an executor-facing handoff type
- update tests to reflect the new boundary

### Out of Scope

- strategy producer queue redesign
- SOUP transport behavior changes
- session recovery semantics changes
- unrelated cleanup in latency or FPGA event handling

## Current Problem

The current boundary mixes responsibilities:

- `Executor` receives `OrderIntent`
- `Executor` assigns `user_ref_num`
- `Executor` builds `OuchEnterOrder`
- `Executor` encodes OUCH bytes
- `Executor` builds `TxOutboundRecord`
- `TxEngine` accepts the already-prepared record and handles session/SOUP transport

This makes `Executor` know transport-facing details that belong more naturally to the TX side:

- OUCH field layout
- payload buffer shape
- transport/session resend record format
- order identity generation used by the gateway session

If `TxEngine` is the module most likely to evolve, the current split makes that evolution harder because transport-specific logic is spread across both modules.

## Design Summary

Move the order-entry encoding boundary into `TxEngine`.

After the refactor:

- `Executor` owns intent intake, queue draining, and execution logging
- `TxEngine` owns `user_ref_num` assignment, OUCH `Enter Order` construction, payload encoding, transport record creation, session staging, and SOUP sending

The cross-module handoff becomes `OrderIntent` rather than `TxOutboundRecord`.

## Architecture

### Executor Responsibilities

`Executor` remains responsible for:

- per-producer `OrderIntent` queue ownership
- draining queued intents
- pushing execution-intent log records
- forwarding pure intents to `TxEngine`

`Executor` no longer:

- assign `user_ref_num`
- build `OuchEnterOrder`
- call `writeEnterOrder()`
- construct `TxOutboundRecord`

This keeps `Executor` focused on execution-stage orchestration rather than gateway protocol details.

### TX Engine Responsibilities

`TxEngine` owns:

- translating `OrderIntent` into gateway-ready outbound orders
- assigning the next `user_ref_num`
- building `OuchEnterOrder`
- encoding OUCH bytes
- constructing `TxOutboundRecord`
- staging the record through `OrderGatewaySession`
- sending the record over SOUP when connected

This makes `TxEngine` the single module that understands the full gateway transmit path from trading intent to wire bytes.

### Shared Types

`OrderIntent` remains the cross-module handoff type.

`TxOutboundRecord` remains useful, but it becomes a TX/session-facing data structure rather than an executor-facing one. It is treated as internal to the TX layer even if it still lives in shared types for practical reasons.

## Data Flow

The intended data flow is:

1. strategy creates an `OrderIntent`
2. strategy pushes it into `Executor`
3. `Executor` logs the execution intent
4. `Executor` forwards the `OrderIntent` to `TxEngine`
5. `TxEngine` validates the action can map to an outbound order
6. `TxEngine` allocates the next `user_ref_num`
7. `TxEngine` builds `OuchEnterOrder`
8. `TxEngine` encodes the OUCH payload
9. `TxEngine` creates the `TxOutboundRecord`
10. `TxEngine` stages and sends through the existing session/SOUP path

This flow makes the boundary easier to explain:

- `Executor`: "what intent should be sent"
- `TxEngine`: "how that intent becomes gateway traffic"

## Error Handling

`Executor` does not need to understand gateway encoding failures.

Expected behavior:

- `Executor` forwards every queued `OrderIntent`
- `TxEngine` rejects intents that cannot map to a valid outbound order
- invalid actions remain a TX-boundary concern, not an executor concern

Logging split remains:

- execution-intent logging stays in `Executor`
- send, drop, accept, reject, and fill logging stays in `TxEngine`

This preserves the current conceptual logging split while making the transport boundary clearer.

## Ownership Changes

The following ownership moves from `Executor` to `TxEngine`:

- `m_next_user_ref_num`
- the helper that currently builds `TxOutboundRecord`
- direct dependency on `ouch_codec` for outbound encode

As a result, `Executor` no longer needs to know about:

- `OuchEnterOrder`
- `writeEnterOrder()`
- `TxOutboundRecord`

except indirectly through any transitional type exposure that remains during the refactor.

## Trade-Offs

### Benefits

- clearer module boundary
- one place for gateway transport evolution
- less transport/protocol knowledge in `Executor`
- easier reasoning about TX behavior because all order-entry wire preparation lives in one module

### Costs

- `TxEngine` becomes slightly larger
- tests need to move with the new ownership model
- some shared type exposure may remain for pragmatic reasons even if the conceptual boundary is improved

This is an acceptable trade if the TX side is expected to keep evolving.

## Testing

Update or add tests to verify:

- `Executor` forwards intents without owning transport record construction
- `TxEngine` assigns monotonically increasing `user_ref_num`
- valid intents become transport-ready TX records
- invalid `OrderIntentAction` values are dropped at the TX boundary
- `OrderGatewaySession` behavior remains unchanged once given a constructed `TxOutboundRecord`

Existing protocol and session tests continue to validate:

- OUCH encoding correctness
- session resend behavior
- transport send behavior

Expectations move so that TX-owned helpers are the place where outbound record construction is validated.

## Migration Notes

This refactor is done without changing SOUP/session semantics.

Recommended implementation sequence:

1. add or update tests that describe the new boundary
2. add a pure-intent enqueue/forward path into `TxEngine`
3. move record construction and `user_ref_num` ownership into `TxEngine`
4. remove the old executor-side builder path
5. update tests and call sites to the new interface

## Non-Goals

- do not redesign the producer buffering model
- do not fold strategy logic into `Executor`
- do not combine this refactor with transport optimizations or protocol feature changes

## Recommendation

Refactor to the pure-intent boundary:

- `Executor` hands off `OrderIntent`
- `TxEngine` owns the full gateway-order preparation path

This is the clearest split for a codebase where TX transport behavior is likely to change over time.
