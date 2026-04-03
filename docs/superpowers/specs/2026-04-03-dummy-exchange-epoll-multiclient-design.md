# Dummy Exchange Epoll Multi-Client Design

Date: 2026-04-03

## Goal

Remove the obvious single-client bottleneck from `dummy_exchange_server` while keeping the code simple and preserving the existing host-side demo semantics.

The server should accept and service multiple concurrent TCP clients using a single-threaded `epoll` event loop, with each client treated as an isolated session.

## Scope

### In Scope

- replace the current single-client blocking `accept` plus `select` loop with a single-threaded `epoll` loop
- support multiple concurrent TCP client connections
- use non-blocking listen and client sockets
- keep one SoupBinTCP / OUCH connection per client
- isolate login state, sequencing, duplicate detection, and pending fills per client
- separate order handling from heartbeat and delayed-fill timing inside the server event loop
- drop slow or blocked clients instead of retaining large outbound backlogs
- treat reconnects as fresh sessions with no replayed prior history

### Out of Scope

- multi-threaded server execution
- separate TCP channels for heartbeats or control traffic
- shared exchange-wide sequenced history across clients
- durable session recovery across disconnect or process restart
- full SoupBinTCP recovery semantics
- protocol expansion beyond the current simplified login, heartbeat, logout, and OUCH subset
- production-grade flow control or matching-engine behavior

## Current Problem

The current server implementation accepts exactly one TCP connection, stores it in a single `client_fd`, services only that socket, then exits once that client disconnects or times out.

That creates three immediate limitations:

- only one client can be connected at a time
- one client lifecycle ends the whole server process
- the current event loop structure cannot scale beyond a single socket without a redesign

For the demo, the requirement is not extreme scalability. The requirement is to remove the obvious one-client bottleneck without turning the code into a complicated threaded server.

## Design Summary

Convert `DummyExchangeServer` into a long-running single-threaded reactor built around:

- one non-blocking listen socket
- one `epoll` instance
- one timer source for periodic control-plane work
- one per-client session object per active connection

Each client session owns all protocol and order state for that socket. The top-level server owns only connection orchestration and event dispatch.

The event loop is responsible for four categories of work:

1. accepting new client sockets
2. reading and parsing inbound client traffic
3. flushing queued outbound traffic
4. handling periodic timer-driven tasks such as heartbeats, fill release, and idle timeouts

This removes the single-client bottleneck while keeping state ownership simple and avoiding cross-thread synchronization.

## Recommended Approach

Three implementation shapes were considered:

1. single-threaded `epoll` with non-blocking sockets and one periodic timer source
2. single-threaded `epoll` with more complex deadline-aware scheduling
3. multi-threaded acceptor plus worker or sharded event loops

The recommended approach is option 1.

It is the best fit for the current demo because:

- it removes the one-client bottleneck directly
- it keeps all protocol state on one thread
- it avoids locks and cross-thread ownership complexity
- it preserves deterministic behavior and easy debugging

The design deliberately does not optimize for extreme scale. It optimizes for simplicity, correctness, and a clear path from the current implementation.

## Architecture

### Top-Level Server

`DummyExchangeServer` becomes a process-lifetime event loop that:

- creates and configures the listen socket
- creates `epoll`
- creates a periodic timer fd
- registers the listen fd and timer fd with `epoll`
- accepts new clients and registers their sockets
- dispatches readable and writable events to the corresponding client session
- removes and destroys failed or disconnected sessions

The server no longer exits when one client disconnects. It keeps running until a fatal process-level setup or event-loop failure occurs.

### Per-Client Session

Each connected client gets a dedicated session object owned by the server.

Each session contains:

- socket fd
- login/authentication state
- inbound read buffer
- outbound send queue
- last-send timestamp
- last-receive timestamp
- per-client next sequence number
- per-client next order reference number
- per-client next match number
- per-client duplicate-detection map
- per-client sequenced history buffer
- per-client pending-fill list

This preserves the existing behavior model, but scopes it to one client instead of the entire process.

### Event Sources

The server uses three kinds of `epoll` participants:

- listen fd
  - accepts new clients until `accept` returns `EAGAIN`
- client fds
  - handle inbound and outbound socket progress
- timer fd
  - drives periodic heartbeat, fill, and idle-timeout checks

This is the only event split required. The design does not introduce separate transports or extra threads.

## Runtime Behavior

### Connection Handling

The listen socket is configured non-blocking and registered with `epoll`.

When the listen fd becomes readable, the server:

- accepts clients in a loop until the accept queue is drained
- configures each client socket non-blocking
- creates a fresh per-client session object
- registers that client fd for `EPOLLIN`

### Login And Session Start

Each new client begins in a pre-login state.

The session waits for a valid Soup login request. On successful credential validation, the server:

- sends `Login Accepted`
- marks the client as logged in
- starts the client session sequence state at `1`

On login failure or malformed login data, the server rejects and closes only that client.

### Reconnect Semantics

Reconnects are treated as fresh sessions.

That means:

- disconnected-client state is destroyed
- no prior sequenced history is retained for replay
- requested sequence numbers from a reconnecting client do not restore an old session and are answered with fresh-session login state
- the new client session starts from empty history and fresh counters

This matches the goal of keeping teardown cheap and avoiding recovery complexity.

### Read Path

Readable client events perform the minimal required read-side work:

- receive available bytes into the session input buffer
- parse complete Soup packets from buffered data
- handle packet types:
  - login request
  - unsequenced data carrying enter order
  - client heartbeat
  - logout request
- update the session receive timestamp

Order processing remains deterministic and local to the session.

### Write Path

Each session keeps a bounded outbound queue of already-built packet bytes plus the current send offset for the front packet.

The client fd is monitored for `EPOLLOUT` only when that queue is non-empty.

When writable:

- the server flushes queued outbound data
- successful progress updates the session send timestamp
- once the queue is empty, `EPOLLOUT` interest is removed

If the server cannot make forward progress on queued outbound data, the client is closed instead of allowing unbounded per-client buffering.

## Heartbeat And Order Handling Separation

The server keeps one SoupBinTCP connection per client. Heartbeats are not moved to a separate socket or protocol stream.

The separation is internal:

- readable client events handle order and control packet parsing
- writable client events handle queued transmission progress
- timer events handle heartbeat generation, delayed fill release, and idle timeout checks

This is the intended performance split. It keeps the order path tight while moving periodic control-plane work onto timer-driven processing.

## State Ownership

The current server-level state:

- order results
- sequenced history
- pending fills
- sequence counters
- order reference counters
- match counters

should move into the per-client session object.

The top-level server keeps only:

- configuration
- epoll and timer resources
- listen socket
- the active-session container

This avoids accidental cross-client coupling and makes session teardown straightforward.

## Sequencing And Duplicate Rules

Sequence numbers are scoped per client.

Rules:

- each fresh client session starts with next sequence `1`
- only that client sees its own sequenced history
- duplicate detection applies only within one client session
- one client reusing a tag does not affect another client

This is the correct fit for isolated per-client sessions and keeps the implementation simple.

## Backpressure Policy

The server intentionally does not try to be a sophisticated buffered transport.

Policy:

- keep outbound buffering bounded and simple
- if a client socket cannot keep up, drop the client
- do not allow one slow client to consume server memory or stall the loop
- do not let one slow client affect other active sessions

This matches the stated preference to disconnect slow or blocked clients rather than recover them.

## Timer Behavior

A single periodic timer source wakes the event loop to handle low-frequency control-plane work.

On each timer tick, the server scans active logged-in sessions and:

- queues a server heartbeat for any client with no outbound traffic for at least 1 second
- closes any client with no inbound traffic for at least 15 seconds
- emits any delayed fills whose due time has arrived

This periodic scan is acceptable for the intended goal because the aim is simplicity, not a highly optimized timing wheel or heap scheduler.

## Error Handling

Per-client failures close only that client session.

Client-scoped failure cases include:

- malformed Soup frames
- malformed OUCH payloads
- failed login parsing
- invalid or unsupported packet types
- peer disconnect
- socket read failure
- socket write failure
- blocked outbound progress
- idle timeout

Process-level fatal failures remain setup or infrastructure failures such as:

- `socket`
- `bind`
- `listen`
- `epoll_create1`
- `epoll_ctl`
- `timerfd_create`

In those cases, `run()` may still return failure for the process.

## File-Level Change Plan

- Modify `cpp_src/FPGA_boost_demo/exchange/dummy_exchange_server.h`
  - add per-client session structures and server-owned active-session state
- Modify `cpp_src/FPGA_boost_demo/exchange/dummy_exchange_server.cpp`
  - replace the single-client blocking loop with a non-blocking `epoll` event loop
  - add accept, read, write, timer, and per-session lifecycle helpers
  - move global order/session state into per-client structures
- Modify `cpp_src/FPGA_boost_demo/tests/dummy_exchange_server_test.cpp`
  - adapt unit tests to session-scoped behavior
  - add focused coverage for multi-client isolation behavior where practical

No application-level CLI changes are required for the initial multi-client conversion.

## Testing

Keep the current focused validation tests and extend coverage around isolated per-client behavior.

Expected automated coverage:

- validation still rejects out-of-band price
- duplicate detection still works within one session
- duplicate detection is isolated between two sessions
- fresh sessions start sequence numbering from `1`
- reconnect is treated as a new session

Where direct `epoll` loop integration tests are awkward, prefer extracting small helpers so the important behavior can be verified without building a fragile end-to-end harness for every case.

Manual verification should include:

- start `dummy_exchange_server`
- connect more than one client concurrently
- confirm that both can log in independently
- confirm that disconnecting one client does not stop the server
- confirm that a slow or stalled client is dropped without affecting another active client

## Non-Goals And Constraints

- Do not introduce a second transport channel for heartbeats
- Do not add worker threads
- Do not retain disconnected-client replay state
- Do not implement exchange-wide shared sequencing
- Do not let the implementation grow into a generic framework

## Recommendation

Implement the multi-client conversion as a focused architectural change:

- single-threaded
- non-blocking
- `epoll` driven
- timer-assisted
- per-client isolated session state
- drop-on-backpressure

That removes the obvious single-client bottleneck cleanly while keeping the server understandable and aligned with the demo’s intended scope.
