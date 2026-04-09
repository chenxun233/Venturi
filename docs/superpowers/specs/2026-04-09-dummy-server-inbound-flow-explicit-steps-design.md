# Dummy Server Inbound Flow Explicit Steps Design

Date: 2026-04-09

## Goal

Refactor the readable-session path so `DummyServer::_handleConnectedEvent(...)` shows the three inbound processing steps directly:

1. append bytes
2. try read one inbound message
3. handle the message

The purpose of this change is readability. A reader should be able to understand the receive-side control flow from the event handler without opening `ProtocolSession::parseBytes()`.

## Scope

### In Scope

- Make the inbound event path in `dummy_server.cpp` show the explicit message-processing loop
- Keep per-session protocol logic inside `ProtocolSession`
- Add the narrow forwarding APIs needed on `ExchangeProtocol`
- Preserve existing SOUP and OUCH runtime behavior

### Out of Scope

- Changing message formats or validation behavior
- Moving protocol logic into `DummyServer`
- Reworking timer handling or outbound flush behavior
- Larger protocol-layer redesign beyond the receive path

## Chosen Approach

Keep the real inbound state-machine operations on `ProtocolSession`, and expose them through slot-based forwarding methods on `ExchangeProtocol`.

`DummyServer::_handleConnectedEvent(...)` remains the coordinator for socket-readable work. It should orchestrate the receive path in this order:

1. receive bytes from `ExchangeTransport`
2. append those bytes into `ExchangeProtocol`
3. repeatedly ask for the next parsed inbound message
4. hand each message back to `ExchangeProtocol` for handling
5. continue with existing close and outbound flush checks

This is the best fit because it makes the event-handler flow explicit without breaking the existing ownership boundary.

## Why These Functions Belong To ProtocolSession

The three operations belong to `ProtocolSession` because they operate on one connection's protocol state:

- inbound byte ring buffer
- login state
- close-request state
- sequencing state
- replay state
- pending fills
- per-session timestamps

Those are all session-local invariants. `ExchangeProtocol` is the slot-indexed facade over many sessions. Its role is to find the active slot and forward calls to the correct `ProtocolSession`. It should not become the primary owner of one session's parsing and message-handling rules.

## Alternatives Considered

### Keep `parseBytes()` And Only Rename Or Comment It

This keeps the current encapsulation and has the smallest code diff.

It is not the chosen approach because it does not achieve the readability goal. The three-step flow would still be hidden inside a helper.

### Move The Entire Inbound Pipeline Into A Larger Protocol Entry Point

Examples would be functions shaped like `processReadableSession(...)` or `processReceivedBytes(...)`.

This is worse for the stated goal because it hides even more of the event flow. It makes `DummyServer::_handleConnectedEvent(...)` shorter, but less explicit.

## API Shape

### ProtocolSession

Keep:

- `bool appendBytes(const uint8_t* bytes, std::size_t size, std::chrono::steady_clock::time_point now)`

Add public methods:

- `std::optional<InboundMessage> tryReadInboundMessage()`
- `bool handleInboundMessage(const InboundMessage& message, std::chrono::steady_clock::time_point now)`

`handleInboundMessage(...)` should preserve current close behavior:

- if message handling fails, set `m_should_close = true`
- return whether handling succeeded

The existing private helpers `tryReadInboundMessage()` and `handleInboundMessage(...)` remain the implementation core. The new public methods are thin wrappers that preserve ownership while making the orchestration visible at the caller.

### ExchangeProtocol

Add slot-based forwarding methods:

- `std::optional<InboundMessage> tryReadInboundMessage(int slot_idx)`
- `bool handleInboundMessage(int slot_idx, const InboundMessage& message, std::chrono::steady_clock::time_point now)`

These methods forward to the active slot's `ProtocolSession`.

### parseBytes

`parseBytes(...)` should no longer be used by `DummyServer::_handleConnectedEvent(...)`.

`parseBytes(...)` should be retained for now as a convenience wrapper implemented in terms of the two explicit public methods. This keeps the refactor narrow, avoids unnecessary interface churn, and still removes the hidden loop from the event-handler hot path.

## Control Flow In DummyServer

After the change, the receive path inside `DummyServer::_handleConnectedEvent(...)` should read conceptually as:

1. receive bytes from transport
2. if bytes were received, append them into protocol
3. while a complete inbound message is available:
4. handle that message through protocol
5. if the transport receive failed, close the slot
6. flush outbound data
7. close the slot if the protocol requests close

The event handler should coordinate the loop, but it must not interpret message contents.

## Error Handling

Behavior should remain unchanged in the following cases:

- inbound ring buffer full still marks the session for close
- invalid or rejected inbound message handling still marks the session for close
- transport receive failure still closes the slot
- outbound flush failure still closes the slot
- protocol-requested close still uses the existing unified close path

The refactor must not introduce a path where `DummyServer` handles protocol failure differently from the current `parseBytes(...)` behavior.

## Testing

This is a structure-focused refactor, so verification is regression-based.

### Required Verification

- project still builds
- existing exchange or dummy-server tests still pass
- message parsing and handling behavior is unchanged for login, heartbeat, logout, and enter-order traffic
- close behavior remains unchanged when append or message handling fails

### Specific Risks To Check

- `tryReadInboundMessage(...)` accidentally exposing a partially parsed message
- duplicated `m_should_close` handling between wrapper and internal helper
- event-handler loop changing the previous receive-path ordering
- `parseBytes(...)` left behind in a stale or inconsistent state if it is retained
