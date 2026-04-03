# TX Translator Design

## Goal

Replace the current `GatewayEngine`, `OrderGatewaySession`, and `protocol/` codec split with one translator module in `cpp_src/FPGA_boost_demo/tx/` that sits between `Executor` and `TxEngine`.

The translator is the only module responsible for:
- accepting `OrderIntent`
- translating intents into `TxOutboundRecord` objects that `TxEngine` can send directly
- receiving raw inbound payload feedback from `TxEngine`
- tracking pending outbound orders for resend on disconnect
- owning OUCH and SOUP protocol structs and binary encode/decode logic as private implementation detail

After migration, files under `cpp_src/FPGA_boost_demo/protocol/` must be removed. Stale files from the previous split, including `gateway_engine.*` and `order_gateway_session.*`, must also be removed after callers and tests are updated.

## Scope

This design covers:
- the new translator module API and ownership boundaries
- movement of OUCH and SOUP protocol logic into translator-owned internals
- migration of resend and feedback handling into the translator
- hot-path no-allocation constraints after construction
- deletion of the obsolete protocol and gateway/session files

This design keeps `TxEngine` as the socket transport owner, but it removes protocol ownership from `TxEngine`. After the refactor, `TxEngine` remains responsible for socket I/O, connection lifecycle, buffering outbound transport-ready bytes carried by `TxOutboundRecord`, exposing inbound raw framed bytes, and surfacing disconnect events. `TxEngine` must not own OUCH or SOUP message definitions or codec rules.

## Recommended Approach

Use one public module, tentatively `TxTranslator`, with internal private helper types and functions in `tx_translator.cpp`.

Why this approach:
- it matches the desired architecture of a single adaptor between `Executor` and `TxEngine`
- it removes the separate `protocol/` layer completely
- it avoids pushing business translation and resend state into `TxEngine`
- it keeps the public surface small while allowing internal organization inside one implementation file

Alternatives rejected:
- keeping a separate reusable `protocol/` layer does not satisfy the requested ownership model
- collapsing translation logic into `TxEngine` would mix transport concerns with intent translation and pending-order state

## Public Architecture

### Module Placement

Create a new module under `cpp_src/FPGA_boost_demo/tx/`:
- `tx_translator.h`
- `tx_translator.cpp`

### Public Responsibilities

`TxTranslator` is the single module between `Executor` and `TxEngine`. Its public methods should cover the current gateway-facing workflow:
- `attachLogPrinter(LatencyLogPrinter*)`
- `pushIntent(const OrderIntent&)`
- `drainOutbound() -> std::vector<TxOutboundRecord>`
- `restoreOutbound(std::vector<TxOutboundRecord>)` if retained as part of the resend pipeline
- `handleInboundPayload(const std::vector<uint8_t>&)`
- `handleTransportDisconnect()`

The translator accepts intent-domain inputs and emits transport-ready records. Those outbound records must already contain the bytes that `TxEngine` can send directly on the socket, including any required protocol framing. The translator also consumes inbound raw framed bytes from `TxEngine` and updates translator-owned pending-order state.

### Caller Changes

- `Executor` depends on `TxTranslator` instead of `GatewayEngine`
- `TxEngine` remains transport-oriented and does not learn `OrderIntent`
- no caller includes headers from `cpp_src/FPGA_boost_demo/protocol/`
- `TxEngine` no longer depends on `SoupPacketType`, `SoupPacket`, or any protocol-owned message struct

## Internal Structure

Externally there is one module. Internally, `tx_translator.cpp` is organized into private helper areas in the anonymous namespace.

### Private Protocol Types And Helpers

Move these into translator-private implementation detail:
- OUCH message structs currently defined for order entry and exchange feedback
- SOUP packet structs currently used for login and framed transport packets
- endian conversion helpers
- sequence and fixed-width text-field helpers
- binary write/read helpers for OUCH and SOUP payloads

These types must not be exposed through `tx_translator.h`. They exist only to support translator behavior and transport interaction.

### Private Pending-Order State

Absorb the current `OrderGatewaySession` behavior into the translator as private state or a translator-private helper struct. This internal state remains responsible for:
- tracking pending orders by tag
- preserving resend order
- dropping the oldest pending order when capacity is full
- removing pending orders on accepted, executed, or rejected feedback

There is no separate public session class after the refactor.

## Data Flow

### Outbound Path

1. `Executor` pushes `OrderIntent` into the translator.
2. The translator validates the action and derives the order side.
3. The translator allocates the next monotonic tag.
4. The translator encodes the OUCH enter-order payload.
5. The translator applies any required SOUP framing so the result is directly sendable by `TxEngine`.
6. The translator writes the final transport-ready bytes into the fixed payload storage of a `TxOutboundRecord`.
7. The translator stages the outbound record in pending-order state.
8. The translator returns ready outbound records for `TxEngine::pushPayload`.

### Inbound Path

1. `TxEngine` reads framed raw bytes from the socket without owning protocol semantics.
2. `TxEngine` surfaces the full framed bytes to the translator.
3. The translator parses SOUP framing privately.
4. The translator decodes OUCH feedback privately when the framed packet carries exchange feedback.
5. The translator updates pending-order state based on accepted, executed, or rejected messages.
6. The translator emits log events through `LatencyLogPrinter`.

### Disconnect Path

1. `TxEngine` reports a disconnect event.
2. The translator restores pending resendable outbound records from its internal pending-order state.
3. The restored records become available again through the translator outbound drain path.

## Performance Constraints

### Hard Requirement

No heap allocation is permitted on the translator hot path after object construction.

The hot path includes:
- `pushIntent(...)`
- intent draining and outbound-record construction
- OUCH payload encoding
- inbound payload feedback decoding
- resend restoration on disconnect

### Required Design Consequences

- do not build temporary `std::vector<uint8_t>` payloads on the hot path
- write the final transport-ready bytes directly into the fixed payload buffer already present in `TxOutboundRecord`
- convert codec helpers from vector-returning APIs to fixed-size writer/reader helpers that operate on caller-provided buffers or fixed arrays
- preallocate pending-order tracking storage at construction time
- any `reserve()` use is construction-time only, never in steady-state message processing
- avoid growth-oriented containers in the translator hot path unless they are fully pre-sized and never reallocate after construction

### Implication For Existing Interfaces

If `drainOutbound()` returning `std::vector<TxOutboundRecord>` cannot satisfy the no-allocation rule in steady state, the translator API should be adjusted to use caller-owned output storage or a fixed-capacity queue abstraction. The implementation plan must verify whether the current return-by-vector interface can remain without violating the rule.

## Correctness Rules

- unsupported `OrderIntentAction` values produce no outbound record
- a specific order tag never changes after assignment
- malformed inbound payloads must fail deterministically and must not corrupt pending-order state
- fixed-capacity overflow behavior must be explicit and testable
- performance optimizations must not weaken resend semantics or protocol correctness

## File-Level Changes

### Add

- `cpp_src/FPGA_boost_demo/tx/tx_translator.h`
- `cpp_src/FPGA_boost_demo/tx/tx_translator.cpp`

### Update

- `cpp_src/FPGA_boost_demo/tx/executor.h`
- `cpp_src/FPGA_boost_demo/tx/executor.cpp`
- `cpp_src/FPGA_boost_demo/tx/tx_engine.h`
- `cpp_src/FPGA_boost_demo/tx/tx_engine.cpp`
- tests and any build files that reference the old modules

`TxEngine` updates must remove protocol-specific enums, packet structs, and codec includes so it operates only on raw sendable and received byte buffers.

### Remove As Stale

- `cpp_src/FPGA_boost_demo/protocol/ouch_codec.h`
- `cpp_src/FPGA_boost_demo/protocol/ouch_codec.cpp`
- `cpp_src/FPGA_boost_demo/protocol/soup_codec.h`
- `cpp_src/FPGA_boost_demo/protocol/soup_codec.cpp`
- `cpp_src/FPGA_boost_demo/tx/gateway_engine.h`
- `cpp_src/FPGA_boost_demo/tx/gateway_engine.cpp`
- `cpp_src/FPGA_boost_demo/tx/order_gateway_session.h`
- `cpp_src/FPGA_boost_demo/tx/order_gateway_session.cpp`

Remove these files only after all includes, call sites, tests, and build configuration have been migrated.

## Testing And Verification

Update or replace the existing tests so they validate the translator-centered design:
- intent translation produces increasing tags and correct transport-ready outbound records
- invalid intents produce no outbound record
- accepted, executed, and rejected feedback remove the correct pending order
- resend restoration preserves pending order behavior after disconnect
- fixed-capacity eviction drops the oldest pending order deterministically
- malformed inbound payloads are rejected without corrupting translator state
- outbound records contain bytes that `TxEngine` can send without additional protocol encoding
- codec hot paths do not rely on temporary vector construction

Verification for implementation completion should include:
- build success after removal of the stale files
- passing tests for translator behavior and transport integration
- confirmation that no source file still includes headers from `cpp_src/FPGA_boost_demo/protocol/`

## Migration Sequence

1. Introduce `TxTranslator` with the public interface that replaces `GatewayEngine`.
2. Move gateway behavior and session state into the translator.
3. Move OUCH and SOUP structs plus codec helpers into translator-private internals.
4. Update `Executor` to attach and use the translator.
5. Update `TxEngine` so it no longer depends on `protocol/` files.
6. Migrate tests to the new translator-centered seams.
7. Remove stale `protocol/`, `gateway_engine.*`, and `order_gateway_session.*` files.

This sequence keeps the refactor incremental while enforcing the final ownership model.
