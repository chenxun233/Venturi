# Dummy Exchange Fill Debug Design

## Goal

Investigate and fix the regression where the dummy exchange appears to stop delivering fill-related `Executed` messages after the recent queue/ring-buffer changes, while also adding exchange-side logging for every outbound SoupBinTCP packet actually sent to the client.

## Scope

This change is limited to the dummy exchange server path.

Included:
- outbound exchange-side logging for every Soup packet after a successful socket send
- one focused regression test proving an accepted order eventually produces an outbound `Executed` message after timer maintenance
- a minimal server-side fix only if the regression test shows the exchange is failing to emit the fill

Excluded:
- broader client-side logging changes
- protocol changes outside the currently supported message set
- unrelated refactors outside the exchange send/fill path

## Current Behavior

The dummy exchange still has the intended fill lifecycle in code:

1. `_handleEnterOrder()` accepts an order and pushes a `PendingFill`
2. `_maintainClients()` checks due fills on timer ticks
3. due fills are encoded as OUCH `Executed` payloads and wrapped in Soup `Sequenced Data`
4. `_flushQueuedPackets()` sends queued Soup packets to the client

After the queue refactor, live behavior suggests the client only observes outbound order sends and not later fills. The failure could be in:
- `PendingFill` scheduling
- timer maintenance draining logic
- outbound queueing/flushing on the exchange
- client receipt/parsing after the exchange already sent the fill

## Design

### 1. Outbound Logging

Add one helper in the dummy exchange that logs every outbound Soup packet after a successful `send()` in `_flushQueuedPackets()`.

Logged fields:
- session/client identifier
- bytes sent
- Soup packet type
- when Soup type is `S`, decode inner OUCH type and key fields

For inner OUCH messages:
- `A` Accepted: user ref, shares, price, order ref
- `E` Executed: user ref, executed shares, price, match number
- `J` Rejected: user ref, reject reason

For non-OUCH Soup control packets:
- `A` Login Accepted
- `J` Login Rejected
- `H` Server Heartbeat
- `S` Sequenced Data with decoded inner OUCH details when supported

The log point is the successful send path rather than queue insertion so the message reflects what actually left the exchange.

### 2. Regression Test

Add one deterministic regression test to `dummy_exchange_server_test.cpp`:

1. create a test session
2. submit one valid order
3. mark the session logged in
4. age the last-send timestamp if needed for maintenance behavior
5. sleep or otherwise advance past `fill_delay`
6. call `handleTimerTickForTest()`
7. verify a later outbound Soup packet is queued
8. verify that packet is Soup `S`
9. inspect the inner payload to confirm it is OUCH `E`

This test should fail if the server no longer turns queued `PendingFill` entries into outbound `Executed` packets.

### 3. Fix Rule

If the regression test fails:
- fix only the minimum server path needed to restore `PendingFill -> Executed -> queued/sent Soup S`

If the regression test passes:
- do not change exchange behavior further in this task
- use the new exchange outbound logs to diagnose client-side receipt/parsing

## Error Handling

- If outbound logging sees an unsupported or malformed inner OUCH payload, print the Soup type and payload length rather than failing the send path.
- Logging must not throw or alter message delivery.
- The regression test should use the existing test helpers and avoid introducing network timing dependencies beyond the fill delay itself.

## Verification

Required verification steps after implementation:

- build the exchange and test targets
- run `dummy_exchange_server_test`
- run the standalone `dummy_server`
- exercise a client connection and confirm outbound logs show both accepted and executed traffic

## Risks

- Logging at the send point may produce multiple lines for partial socket writes if a packet is transmitted across multiple `send()` calls. The implementation should log a packet once only when the full packet has been sent, or clearly mark partial sends.
- Timer-based tests can become flaky if they rely on overly small sleep margins. The test should use a fill delay that gives enough room to be deterministic.

## Success Criteria

The task is complete when:
- the exchange logs every outbound Soup packet at send time
- the regression test proves accepted orders later emit executed messages
- if a server regression exists, it is fixed and verified
- if no server regression exists, the logs provide enough signal to continue diagnosis on the client side without further exchange guesses
