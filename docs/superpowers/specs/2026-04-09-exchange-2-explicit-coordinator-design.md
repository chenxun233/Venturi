# Exchange 2 Explicit Coordinator Design

Date: 2026-04-09

## Goal

Finish `cpp_src/FPGA_boost_demo/exchange_2/` so it preserves the current dummy exchange wire behavior from `cpp_src/FPGA_boost_demo/exchange/`, while making the rebuilt structure simpler and more explicit.

The target shape is:

- `Server` coordinates transport and data state explicitly
- `Transport` owns only connection and byte I/O concerns
- `DataLayer` owns only per-session protocol and exchange state
- outbound frames are represented only as raw bytes ready to send
- function boundaries are narrow and single-purpose

## Scope

### In Scope

- Complete `Server::_handle()` in `exchange_2`
- Add the minimal helper functions needed to keep `_handle()` short and explicit
- Preserve existing SOUP and OUCH behavior from `exchange/`
- Keep `Transport` and `DataLayer` fully separated
- Replace nested outbound frame structures with one explicit raw-byte frame type
- Make byte-building functions explicit in name and responsibility

### Out of Scope

- Changing the wire protocol or exchange behavior
- Generalizing `exchange_2` into a reusable framework
- Adding new message types beyond the current dummy exchange behavior
- Hiding coordination inside a new abstraction layer

## Chosen Approach

Use `Server` as the only coordinator.

- `Server` handles epoll, timerfd, accept, close, receive, and send orchestration.
- `Transport` handles live connection ownership and socket syscalls only.
- `DataLayer` handles session state, login state, sequencing state, replay state, timeout marking, parsing, and outbound frame generation only.

This keeps control flow visible in one place without re-coupling the layers.

## Disallowed Structures

The redesign must not use either of the following patterns.

### Large DataLayer Operations

Do not add bigger `DataLayer` entry points such as `handleReadableSession()` or `handleTimerTick()`.

These hide orchestration inside `DataLayer`, recreate long multi-job functions, and make the control flow less explicit.

### Third Protocol Service Layer

Do not insert another class between `Server` and `DataLayer`.

This adds another jump in the call path and weakens the explicit coordinator flow required for `exchange_2`.

## Architecture

### Server

`Server` remains the top-level runtime coordinator and owns:

- listen socket lifecycle
- timer fd lifecycle
- epoll lifecycle
- accept path
- stale event rejection using generation checks
- unified session close path
- receive path orchestration
- timer maintenance orchestration
- outbound flush orchestration

`Server::_handle()` should stay shallow and dispatch to short helpers:

- `_handleListenEvent()`
- `_handleTimerEvent()`
- `_handleConnectedEvent(const epoll_event& event, const EpollEventTag& tag)`
- `_flushSessionOutbound(int session_idx)`
- `_closeSession(int session_idx)`

Each helper must do one job only.

### Transport

`Transport` is passive and owns:

- mapping a live connection id to fd state
- `recv()` calls
- `send()` calls
- releasing a live connection
- resetting all live connections on startup or teardown

`Transport` must not know about login state, timers, replay, heartbeats, or order validation.

### DataLayer

`DataLayer` owns only session-side protocol and exchange state:

- login state
- close-request state
- session id and sequencing state
- inbound byte buffer
- outbound raw frame queue
- pending fills
- replay entries
- timeout marking
- parse and validation logic
- explicit outbound byte formation

`DataLayer` must not own fds, epoll registration, or socket close behavior.

## Frame Model

`exchange_2` should keep one outbound frame type:

- `FrameRaw`: exact bytes ready to send on the socket

`FrameRaw` remains responsible only for:

- `payload`
- `size`
- `offset`

There should not be a separate nested hot-path structure for “Soup packet”, “Soup frame”, and “OUCH frame” on the outbound path.

Instead, the byte-building functions should state exactly what bytes they form, for example:

- `buildSoupLoginAcceptedFrame(...)`
- `buildSoupLoginRejectedFrame(...)`
- `buildSoupServerHeartbeatFrame()`
- `buildSoupSequencedAcceptedFrame(...)`
- `buildSoupSequencedRejectedFrame(...)`
- `buildSoupSequencedExecutedFrame(...)`

Decision, serialization, and queueing must stay separate.

## Data Flow

### Listen Event

`Server::_handleListenEvent()` performs exactly this sequence:

1. accept one client fd
2. return immediately if no pending client exists
3. allocate one session in `DataLayer`
4. register one live connection in `Transport`
5. add the connected fd to epoll with the correct generation token

### Connected Event

`Server::_handleConnectedEvent(...)` performs exactly this sequence:

1. reject stale generation tags
2. close immediately on `EPOLLRDHUP`, `EPOLLHUP`, or `EPOLLERR`
3. receive raw bytes through `Transport`
4. append received bytes into `DataLayer`
5. parse complete messages in `DataLayer`
6. close the session if the data layer requests close
7. flush any queued outbound frames

### Timer Event

`Server::_handleTimerEvent()` performs exactly this sequence:

1. drain the timer fd
2. mark timed-out sessions in `DataLayer`
3. close each marked session through the unified close path
4. ask `DataLayer` to queue heartbeat frames
5. ask `DataLayer` to queue due fill frames
6. flush queued outbound frames for live sessions

This keeps timeout detection in `DataLayer` and timeout teardown in `Server`.

## API Shape

The following API direction is required.

### DataLayer Public API

- `int setSession()`
- `void releaseSession(int session_idx)`
- `void resetSessions()`
- `void markTimedOutSessions(std::chrono::steady_clock::time_point now, std::chrono::seconds timeout)`
- `bool hasTimedOutSession() const`
- `int readTimedOutSession()`
- `bool appendSessionBytes(int session_idx, const uint8_t* bytes, std::size_t size, std::chrono::steady_clock::time_point now)`
- `bool parseSessionBytes(int session_idx, std::chrono::steady_clock::time_point now)`
- `bool shouldCloseSession(int session_idx) const`
- `void queueHeartbeatFrames(std::chrono::steady_clock::time_point now)`
- `void queueDueFillFrames(std::chrono::steady_clock::time_point now)`
- `bool hasOutboundFrame(int session_idx) const`
- `FrameRaw& readFrontFrame(int session_idx)`
- `void eraseFrontFrame(int session_idx)`

These functions are the implementation target for the redesign.

### Server Close API

`Server` needs one unified close helper:

- `_closeSession(int session_idx)`

It performs exactly three actions in order:

1. remove the fd from epoll if needed
2. release the connection from `Transport`
3. release the session from `DataLayer`

No other function should partially close a session.

## Error Handling

Transport-side failures handled by `Server`:

- accept failure
- epoll add or modify failure
- receive failure
- send failure
- stale epoll event
- peer disconnect

Protocol-side failures handled by `DataLayer`:

- invalid login
- malformed frame
- invalid field values
- duplicate order replay decision
- replay capacity exhaustion
- timeout marking
- protocol-requested close state

Actual teardown always remains in `Server`.

## Behavioral Parity Requirements

`exchange_2` must preserve the same externally visible behavior as `exchange/` for:

- login accepted and login rejected responses
- client and server heartbeat handling
- logout handling
- enter order accepted, rejected, and executed responses
- sequence advancement
- delayed fill release timing
- duplicate replay behavior
- idle timeout behavior

This change is internal simplification and decoupling, not a behavior change.

## Testing

Implementation should verify at least:

- login accepted path
- login rejected path
- timer-generated heartbeat path
- accepted order response path
- rejected order response path
- delayed executed response path
- timeout causes both connection release and session release
- stale epoll event does not touch a recycled session
- partial send resumes using `FrameRaw.offset`

## Implementation Notes

- Keep `Server::_handle()` short by using the explicit helpers listed above.
- Avoid helpers that both decide and act on multiple subsystems.
- Avoid functions that both build bytes and queue them.
- Avoid introducing a new abstraction layer between `Server` and `DataLayer`.
- Do not reintroduce nested outbound frame structs into `exchange_2`.
