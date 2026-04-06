# Dummy Exchange Server and Protocol Split Design

Date: 2026-04-06

## Goal

Split the current dummy exchange implementation into two modules:

- a transport-oriented server module
- a protocol-session module that owns OUCH-over-SoupBinTCP behavior

The purpose of the split is to keep the server runtime clean and stable so that:

- exchange behavior can be changed in one focused place
- protocol behavior can be changed without rewriting socket and epoll code
- future protocol replacement is bounded to the protocol module rather than the whole server

## Scope

### In Scope

- Split `dummy_exchange_server` into a transport module and a protocol-session module
- Move SoupBinTCP framing, login, heartbeat, logout, and sequencing into the protocol-session module
- Move OUCH encode/decode, order validation, replay handling, and delayed fill scheduling into the protocol-session module
- Keep live-session slot pooling and epoll dispatch in the server module
- Preserve the existing fixed-capacity, no-`unordered_map` storage model
- Re-center tests so protocol behavior is tested mostly at the protocol-session layer

### Out of Scope

- Designing a generic reusable OUCH/Soup library for other components
- Changing the external network model, process model, or executable layout
- Adding new exchange features beyond the current dummy-exchange subset
- Generalizing the server into a multi-protocol framework in this change

## Chosen Approach

Use a two-module split with a state-owning protocol-session object per session slot.

- `dummy_exchange_server.*` becomes transport runtime only
- `dummy_exchange_protocol_session.*` owns all per-session Soup and OUCH behavior plus dummy-exchange order handling

This is the best fit for the stated goal because it removes protocol details from the server instead of only relocating helper functions. The server remains responsible for sockets, epoll, timer events, and slot lifetime. The protocol-session module becomes the only place that understands login state, Soup packet rules, OUCH messages, replay behavior, validation, and fill timing.

## Alternatives Considered

### Thin Codec Extraction

Move only parse/build helpers into separate files and keep login flow, replay, fills, and sequencing in `DummyExchangeServer`.

This has the lowest refactor cost, but it does not produce a clean server. The server would still contain most protocol behavior, so changing exchange behavior or replacing the protocol would still require editing transport code.

### Fully Generic Protocol Engine

Create a protocol-agnostic session interface intended to support many future protocols.

This is attractive in theory, but it is too abstract for the current codebase. The existing problem is local and concrete. Solving it with a generic framework would add design surface and indirection before there is evidence that the extra abstraction is needed.

## Architecture

### Module 1: Dummy Exchange Server

`dummy_exchange_server.*` should own only transport and runtime orchestration:

- listen socket setup
- nonblocking socket configuration
- epoll registration and wakeup dispatch
- timerfd creation and readout
- live/test session slot allocation and reuse
- stale-event protection through generation checks
- socket read and write syscalls
- fd cleanup and slot release

The server should not inspect Soup packet types, OUCH message types, login fields, sequence numbers, replay entries, or pending fills.

### Module 2: Dummy Exchange Protocol Session

`dummy_exchange_protocol_session.*` should own all per-session protocol and exchange behavior:

- SoupBinTCP read-frame parsing
- Soup login request validation and login accepted/rejected responses
- Soup heartbeat and logout handling
- OUCH enter-order decode
- OUCH accepted, rejected, and executed encode
- per-session next sequence, order reference, and match number state
- duplicate-order replay storage and lookup
- validation rules for price, shares, and symbol
- pending-fill scheduling and release
- outbound frame queue ownership
- protocol-requested session close state

This module remains dummy-exchange-specific. It is intended to isolate protocol and exchange behavior from transport code, not to become a shared library at this stage.

## Session Slot Structure

The existing startup-sized slot pool remains the core runtime model. The split changes what a slot owns.

Each slot should contain:

- slot mode and generation
- transport-facing fields such as socket fd and epoll token
- one protocol-session object

The protocol-session object should internally own:

- read buffer
- outbound frame queue
- login and sequencing state
- replay table
- pending fills

This keeps the slot pool stable while moving protocol-specific state behind one cohesive type.

## Module Boundary

The boundary should be state-owning rather than callback-driven.

The server interacts with the protocol session through a small transport-facing API such as:

- append received bytes into the protocol session
- notify the protocol session of timer ticks
- inspect the front outbound frame
- consume sent outbound bytes after partial or full socket writes
- query whether the protocol session is requesting close
- reset or initialize the protocol session for a fresh slot

The exact names can be chosen during implementation, but the direction of control should remain the same:

- server owns syscalls and slot lifetime
- protocol session owns protocol state and queued protocol output

The server should be able to drive a session without understanding any Soup or OUCH details.

## Data Flow

### Session Creation

1. The server acquires a free slot.
2. The server initializes transport state such as fd, timestamps, and epoll token.
3. The server constructs or resets the protocol-session object using configured capacities and credentials.

### Receive Path

1. The server reads raw bytes from the socket.
2. The server appends those bytes into the protocol session.
3. The protocol session consumes complete Soup frames from its internal read buffer.
4. The protocol session handles login, heartbeat, logout, and order packets internally.
5. The protocol session queues outbound Soup frames in its own outbound queue.

### Timer Path

1. The server forwards timer ticks with the current time.
2. The protocol session decides whether to queue server heartbeats.
3. The protocol session releases any due fills and queues executed responses.
4. The protocol session may request close on protocol-level timeout or invalid state.

### Send Path

1. The server checks whether the protocol session has queued outbound bytes.
2. The server sends bytes from the front outbound frame.
3. After each successful write, the server reports the consumed byte count back to the protocol session.
4. The protocol session advances or retires the front frame when its bytes are fully consumed.

### Close Path

The protocol session may request close because of:

- invalid login
- logout request
- malformed or unsupported packet handling policy
- idle timeout or other protocol-owned session rule

The server performs the actual epoll removal, socket close, and slot release.

## Error Handling

Transport failures remain server responsibilities:

- `accept()` failure handling
- nonblocking socket setup failure
- read and write syscall failure
- epoll registration failure
- stale event token rejection

Protocol failures remain protocol-session responsibilities:

- login rejection
- invalid packet interpretation
- replay-table exhaustion policy
- pending-fill queue exhaustion policy
- protocol-requested close state

This keeps failure ownership aligned with module responsibility.

## Testing

Testing should be split along the same boundary.

### Protocol-Session Tests

Add or migrate tests for:

- login accepted and rejected flows
- heartbeat generation
- logout handling
- order validation outcomes
- duplicate replay hits and misses
- replay-table collision and exhaustion behavior
- accepted, rejected, and executed message generation
- delayed-fill scheduling and release
- outbound queue consumption after partial writes

### Server Tests

Keep server-focused tests for:

- slot acquisition, release, and reuse
- session-pool exhaustion behavior
- stale-event rejection through generation mismatch
- transport integration around socket read, flush, and close behavior
- close-on-transport-failure behavior

Most detailed protocol assertions should move out of server tests unless the test is specifically about transport integration.

## Migration Plan Shape

The implementation should be staged so behavior stays stable while responsibilities move:

1. Introduce the new protocol-session type and move protocol helpers into it.
2. Move per-session protocol state out of `ClientState` and into the protocol-session object.
3. Rewire the server receive, timer, and send paths to drive the protocol session through the new boundary.
4. Move existing tests toward the protocol-session layer.
5. Reduce `DummyExchangeServer` helpers until it no longer contains Soup or OUCH-specific logic.

This ordering keeps the refactor reviewable and lowers the risk of mixing old and new responsibilities for too long.

## Success Criteria

The change is complete when all of the following are true:

- `DummyExchangeServer` is transport-oriented and does not directly implement Soup or OUCH behavior
- one protocol-session object owns all per-session protocol and exchange behavior
- the slot-pool and no-`unordered_map` storage model remain intact
- protocol behavior can be changed without editing epoll or socket orchestration code
- future protocol replacement would primarily affect the protocol-session module rather than the server module
- tests clearly separate protocol behavior from transport behavior
