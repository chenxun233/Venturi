# Venturi SOUP Hot-Path Send Design

Date: 2026-04-03

## Goal

Remove per-packet heap allocation and payload copying from the steady-state SOUP send paths in the order gateway TX engine and the dummy exchange server.

The design keeps the existing vector-returning SOUP codec helpers for control-path and test code, but introduces a dedicated hot-path send helper for data-path traffic.

## Scope

### In Scope

- `TxEngine` steady-state outbound order sends
- `TxEngine` replay sends for pending resends
- `TxEngine` client heartbeat sends
- `dummy_exchange_server` sequenced replay sends after login
- `dummy_exchange_server` steady-state sequenced response sends
- `dummy_exchange_server` server heartbeat sends
- a reusable SOUP send helper that transmits header and payload without materializing a packet `std::vector`

### Out of Scope

- read-path decoding changes
- full `soup_codec` API replacement
- async or nonblocking socket redesign
- unrelated protocol cleanup
- login request / login accepted / login rejected control-path serialization changes unless required by the new helper

## Current Problem

The current hot path performs avoidable heap work and copying:

- `TxEngine::_sendPayload()` creates a temporary payload `std::vector`
- `writeUnsequencedPacket()` and `writeSequencedPacket()` call `writePacket()`
- `writePacket()` allocates a second `std::vector` for the final SOUP frame
- the send helper transmits from that materialized packet buffer

This means one outbound order currently incurs:

- one allocation and copy to build the temporary payload vector
- one allocation and copy to build the SOUP packet vector

The dummy exchange server has the same materialization pattern for sequenced replay, accepted, rejected, executed, and heartbeat sends.

For the HFT-oriented demo, the hot path should not allocate per packet when the payload already exists in contiguous storage.

## Design Summary

Introduce a dedicated hot-path SOUP send helper with this logical interface:

- `sendSoupPacket(int fd, SoupPacketType type, const uint8_t* payload, std::size_t payload_size)`

The helper will:

- validate that `payload_size + 1` fits in the SOUP 16-bit length field
- build the 3-byte SOUP header on the stack
- transmit the header and payload as two segments
- handle short writes until the full message is sent or `send` fails
- support zero-length payloads for heartbeat-style packets

This helper becomes the send boundary for hot-path traffic. The payload remains in its original storage and is not recopied into a temporary packet buffer.

## Architecture

### TX Engine

`TxEngine::_sendPayload()` will stop constructing a temporary payload vector from `TxOutboundRecord::payload`.

Instead, it will:

- validate `payload_length`
- call the new hot-path SOUP send helper using:
  - packet type `UnsequencedData`
  - `record.payload.data()`
  - `record.payload_length`

Replay sends use the same path because `_replayPendingOrders()` already routes through `_sendPayload()`.

`TxEngine` client heartbeats will also use the new helper with a zero-length payload instead of building a vector packet through `writeClientHeartbeat()`.

Control-path login sends may remain on the existing vector-returning helper path because they are not part of the steady-state hot path.

### Dummy Exchange Server

The dummy exchange server will use the same hot-path helper for:

- sequenced replay sends from `m_sequenced_history`
- live sequenced responses emitted after inbound order handling
- delayed fill sends from `m_pending_fills`
- server heartbeats with zero-length payload

Control-path login accepted and login rejected sends may remain on the existing vector-returning helper path because they are not hot-path traffic.

### Codec Boundary

The existing `writePacket()` and vector-returning wrappers remain available for:

- login helpers
- tests that verify packet construction
- non-hot control paths

This keeps the protocol module stable while moving only the hot paths to a no-allocation send boundary.

## Data Flow

### Hot-Path Send

For each outbound data-path packet:

1. caller provides socket fd, SOUP type, payload pointer, and payload size
2. helper encodes the 2-byte big-endian SOUP length plus 1-byte packet type into a 3-byte stack buffer
3. helper sends header and payload without creating a combined packet buffer
4. helper returns `true` only after the full message is transmitted

### Zero-Payload Send

For heartbeats:

1. caller provides socket fd, heartbeat packet type, null or ignored payload pointer, and payload size `0`
2. helper sends only the 3-byte SOUP header
3. helper returns success or failure using the same rules as data packets

## Partial Write Handling

The helper must correctly handle short writes across two segments.

Required behavior:

- track a total byte offset across the logical packet
- if the offset is within the 3-byte header, send the unsent header suffix
- once the header is fully sent, send the remaining payload bytes
- continue until `3 + payload_size` bytes are written or `send` fails

This keeps behavior equivalent to the current `sendAll()` semantics while avoiding packet materialization.

The implementation should use scatter/gather send semantics for the logical two-buffer packet:

- use `writev` or `sendmsg` for the header-plus-payload send
- on short write, advance the logical packet offset and retry until completion or failure
- preserve the no-allocation property across all retries

## Error Handling

The new helper returns `false` when:

- the payload size exceeds the representable SOUP packet length
- a socket send call returns `<= 0`

Caller behavior remains unchanged:

- `TxEngine` disconnects and schedules reconnect on send failure
- `dummy_exchange_server` breaks the session loop and closes the client connection on send failure

No new recovery semantics are introduced by this change.

## Performance Expectations

This design removes the following hot-path work:

- temporary `std::vector` construction for outbound payloads in `TxEngine`
- `std::vector` allocation and growth in `writePacket()`
- payload copy into a combined SOUP packet buffer

The remaining hot-path work is:

- stack header construction
- socket syscall work
- payload transmission directly from existing contiguous storage

This is a practical and cache-friendly improvement for the current HFT-style demo because it reduces allocator traffic and memory copying without widening the protocol refactor scope.

## File-Level Change Plan

- Modify `cpp_src/FPGA_boost_demo/tx/tx_engine.cpp`
  - add or use the new hot-path SOUP send helper
  - route `_sendPayload()` and heartbeat sends through it
- Modify `cpp_src/FPGA_boost_demo/exchange/dummy_exchange_server.cpp`
  - route sequenced replay, live sequenced responses, delayed fills, and heartbeats through the helper
- Modify `cpp_src/FPGA_boost_demo/protocol/soup_codec.h` and `cpp_src/FPGA_boost_demo/protocol/soup_codec.cpp` only if needed to expose reusable SOUP header utilities cleanly
- Modify or add tests under `cpp_src/FPGA_boost_demo/tests/` as needed

## Testing

Keep existing protocol serialization tests and add focused coverage for the new hot-path helper behavior where practical.

Expected verification:

- existing `soup_codec_test` still passes
- existing `dummy_exchange_server_test` still passes
- existing TX-related tests still pass
- new helper tests verify:
  - correct bytes for non-empty payload
  - correct bytes for zero-length payload
  - rejection of oversize payload lengths
  - correct behavior under short-write conditions if test harness support is practical

Short-write coverage should use either a local socket-based integration test or a send shim that can force partial writes. If unit-level injection is awkward in the current test structure, local socket integration coverage is sufficient for this change.

## Non-Goals And Constraints

- Do not zero reusable buffers after send because this design does not require a reusable packet buffer
- Do not introduce asynchronous send lifetimes or queued packet ownership
- Do not change the receive path
- Do not force all control-path code onto the new helper

## Recommendation

Implement the new helper first as a focused transport-layer optimization, then update only the confirmed hot-path call sites in `TxEngine` and `dummy_exchange_server`.

This yields the HFT-relevant improvement with minimal interface churn and avoids turning a send-path optimization into an unnecessary full codec rewrite.
