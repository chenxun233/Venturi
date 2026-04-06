# Dummy Exchange Server Run Refactor Design

Date: 2026-04-06

## Goal

Refactor `DummyExchangeServer::run()` in `dummy_exchange_server.cpp` into smaller private methods with clearer responsibility boundaries.

The purpose of this change is to make the server loop easier to read and maintain without changing runtime behavior.

## Scope

### In Scope

- Split `DummyExchangeServer::run()` into smaller private helper methods
- Keep `run()` as the public orchestration entry point
- Preserve existing epoll, timer, socket, and session-slot behavior
- Preserve existing return codes, logging, and cleanup semantics

### Out of Scope

- Changing protocol behavior
- Introducing new abstractions such as RAII fd wrappers or a separate runtime object
- Changing public APIs other than adding private helper declarations
- Changing test-session behavior or session-pool logic

## Chosen Approach

Use a small-helper refactor inside `DummyExchangeServer`.

`run()` becomes a high-level coordinator that:

- resets live-session state
- opens and registers infrastructure fds
- runs the epoll event loop
- shuts down live sessions
- closes infrastructure fds

This is the best fit because it follows the class's existing style. The file already uses private helpers for slot management, transport reads, transport writes, and live-session close handling. Extending that pattern to `run()` produces a smaller function without introducing a wider design change.

## Alternatives Considered

### Helper Refactor with Run Context Struct

Extract the same logical phases but package `listen_fd`, `epoll_fd`, and `timer_fd` into a small context struct passed between helpers.

This would reduce parameter count, but it introduces a new abstraction only for one function. The current code does not otherwise use a runtime-context object, so this would add refactor surface without a clear payoff.

### RAII-Based Cleanup Refactor

Replace explicit `::close(...)` calls with scoped fd wrappers and then split `run()`.

This would improve cleanup ergonomics, but it changes the resource-management model at the same time as the function split. That is a larger change than required for the current goal and increases regression risk.

## Proposed Helper Structure

Add private helpers with one clear decision boundary each:

- `_resetLiveSessionsBeforeRun()`
- `_openListenFd()`
- `_openEpollFd()`
- `_registerInfrastructureFds(int epoll_fd, int listen_fd, int timer_fd)`
- `_runEventLoop(int epoll_fd, int listen_fd, int timer_fd)`
- `_handleReadyEvent(int epoll_fd, int listen_fd, int timer_fd, const epoll_event& event)`
- `_handleTimerTick(int epoll_fd, int timer_fd)`
- `_handleLiveSessionEvent(int epoll_fd, const epoll_event& event, const EventToken& token)`
- `_shutdownLiveSessions()`
- `_closeRunFds(int listen_fd, int epoll_fd, int timer_fd)`

These helper names are the intended implementation shape for this refactor.

## Responsibilities

### `run()`

`run()` should remain responsible for the top-level sequence and for coordinating setup, looping, and teardown. It should read like an outline of the server lifecycle rather than a full implementation of each phase.

### `_resetLiveSessionsBeforeRun()`

Release any slot still marked `Live` before a fresh run starts. This preserves current behavior where stale live sessions are cleared up front.

### `_openListenFd()`

Open the listening socket, make it nonblocking, print the existing listen message, and either return a valid fd or throw.

### `_openEpollFd()`

Create the epoll instance and either return a valid fd or throw.

### `_registerInfrastructureFds(...)`

Register the listen token and timer token with epoll. On failure, throw so the caller can perform centralized cleanup for already-opened fds.

### `_runEventLoop(...)`

Contain the `epoll_wait` loop and return the same status codes as today:

- continue on `EINTR`
- return `1` on non-`EINTR` `epoll_wait` failure
- return `0` when stop is requested and the loop exits normally

### `_handleReadyEvent(...)`

Dispatch one ready event by token kind:

- listen token: accept clients
- timer token: process timer work
- live-session token: handle socket-side work for one session

### `_handleTimerTick(...)`

Read timer expirations, tolerate `EAGAIN` and `EWOULDBLOCK`, then drive all live protocol sessions with `onTimerTick()`, outbound flush, and close-on-request behavior. Return success or failure so `_runEventLoop(...)` can preserve the current `1` return on timer read failure.

### `_handleLiveSessionEvent(...)`

Resolve the slot from the event token, ignore stale events, and preserve the current ordering:

1. close on epoll error or hangup flags
2. receive bytes into the protocol
3. flush outbound frames
4. close if the protocol requests shutdown

### `_shutdownLiveSessions()`

Close any remaining live client fds and release their slots. This preserves the current post-loop shutdown behavior.

### `_closeRunFds(...)`

Best-effort close for `timer_fd`, `epoll_fd`, and `listen_fd`. This helper exists to reduce repeated close sequences without changing the explicit cleanup model.

## Control Flow

The refactored lifecycle should remain:

1. Clear the stop flag.
2. Reset any previously live slots.
3. Open `listen_fd`.
4. Open `epoll_fd`.
5. Open `timer_fd`.
6. Register listen and timer tokens with epoll.
7. Enter the event loop.
8. On loop exit, shut down live sessions.
9. Close infrastructure fds.
10. Return the event-loop status.

This preserves the existing ownership model and sequencing while making each phase locally understandable.

## Error Handling

This refactor is behavior-preserving. Error behavior should remain materially the same:

- listen socket setup failure still throws
- listen socket nonblocking failure still throws after closing `listen_fd`
- epoll creation failure still throws after closing `listen_fd`
- timer creation failure still propagates as an exception from `_openTimerFd()`
- epoll registration failure still throws after closing already-opened fds
- non-`EINTR` `epoll_wait` failure still returns `1`
- timer read failure other than `EAGAIN` and `EWOULDBLOCK` still returns `1`

The split should not silently convert throw paths into return paths or vice versa.

## Testing

This change is primarily structural, so verification is regression-focused.

### Required Verification

- project still builds
- existing dummy exchange server tests still pass
- no changes to observable log messages from startup
- no changes to session acquisition, release, or close behavior during live operation

### Specific Risks to Check

- missing cleanup on partial setup failure
- accidental changes to the order of live-session event handling
- timer handler return-path mistakes
- stale event-token handling regressions after moving logic into helpers

## Implementation Notes

- Keep all new helpers private methods on `DummyExchangeServer`
- Do not move logic into free functions unless it is already file-local today
- Prefer passing raw fds explicitly rather than storing temporary run state on the object
- Keep the diff focused on `dummy_exchange_server.h` and `dummy_exchange_server.cpp`
