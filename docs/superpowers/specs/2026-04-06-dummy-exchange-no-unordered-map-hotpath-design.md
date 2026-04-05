# Dummy Exchange No-Unordered-Map Hot-Path Design

Date: 2026-04-06

## Goal

Remove `std::unordered_map` from the dummy exchange runtime and test paths so the server can:

- allocate all required storage once at startup
- perform no further dynamic allocation afterward
- keep client and session objects in contiguous storage
- avoid hash-map lookup on the live epoll event hot path
- reject cleanly on capacity exhaustion instead of allocating or aborting

This design applies to both live server sessions and test-created sessions.

## Scope

### In Scope

- Replace live client storage based on `m_live_clients`
- Replace test session storage based on `m_test_clients`
- Replace per-client duplicate-order replay storage based on `ClientState::order_results`
- Keep session and client state in startup-sized contiguous storage
- Move live epoll dispatch to stable pointer/token handles in `epoll_event.data.ptr`
- Reject on slot-table or replay-table exhaustion
- Add tests for pool reuse, replay lookup, overflow handling, and stale-event safety

### Out of Scope

- Changing SoupBinTCP or OUCH protocol behavior beyond exhaustion handling
- Changing fixed-ring-buffer behavior
- Persisting session state across process restart
- General-purpose reusable container library work outside the dummy exchange module

## Chosen Approach

Use contiguous object pools plus preallocated flat hash tables where key-based lookup is still required.

Specifically:

- live sessions move from `std::unordered_map<int, ClientState>` to a startup-sized contiguous slot pool
- live epoll dispatch uses `epoll_event.data.ptr` so the event hot path does not do `fd -> session` lookup
- test sessions move from `std::unordered_map<uint64_t, ClientState>` to the same slot-pool model used by the live path
- duplicate-order replay moves from per-client `std::unordered_map<uint32_t, HandledOrderResult>` to a startup-sized flat hash table owned by each session slot

This is the best fit because it preserves contiguous storage, removes post-startup allocation, avoids `unordered_map` entirely, and still keeps expected O(1) lookup for replay state.

## Alternatives Considered

### Direct-Index Tables Everywhere

Direct indexing gives true worst-case O(1), but it only works well when the key space is small and bounded. It is not practical for sparse 32-bit `user_ref_num` values, so it would either waste large amounts of memory or force protocol assumptions that do not belong here.

### Sorted Lookup Vectors

Keeping key/index pairs in sorted vectors simplifies implementation but makes lookup O(log N) and makes inserts and erases more expensive. That is acceptable for offline utilities, but it is the wrong trade-off for duplicate-order replay and live session management.

## Architecture

### Session Slot Pool

Introduce one startup-sized pool of session slots. A slot contains:

- slot metadata such as `is_in_use`, generation, and mode
- the existing `ClientState` payload
- startup-allocated replay storage owned by that slot
- a stable event token for the live epoll path

The slot pool is allocated once during server construction or initialization and never resized afterward.

The pool supports two modes:

- live server sessions created from accepted sockets
- test sessions created through helper APIs

Using one slot model for both paths keeps test behavior aligned with production behavior instead of maintaining separate storage semantics.

### Free-List Allocation

Free slots are tracked by a startup-sized free-list of slot indexes.

- acquire: pop one index from the free-list
- release: reset the slot state in place and push its index back

This keeps slot allocation and reuse O(1) without moving objects in memory.

### Live Event Dispatch

The live path does not maintain an `fd -> slot` hash table.

Instead:

- `accept()` acquires a free slot
- the slot is initialized in place
- the slot's stable event token is registered with `epoll_event.data.ptr`
- the event loop casts `data.ptr` back to the token, validates the generation, and accesses the slot directly

This removes live-session lookup from the epoll hot path entirely.

### Test Session Access

Test sessions use the same slot pool semantics as live sessions.

- `createSessionForTest()` acquires a free slot and returns a stable handle containing at least slot index and generation
- test helper APIs take that handle and resolve directly to the slot
- `session_id` remains session metadata for protocol behavior and logs, not a container key

This keeps tests structurally aligned with the real path and avoids introducing a separate map-shaped test-only storage model.

### Per-Session Replay Table

Each session slot owns a startup-sized replay table keyed by `user_ref_num`.

The replay table uses open addressing in preallocated contiguous arrays:

- key occupancy state
- stored `user_ref_num`
- stored `HandledOrderResult`

Operations:

- lookup existing `user_ref_num` to detect duplicates
- insert new result after first-time handling
- reject new orders when the replay table is full

This preserves expected O(1) duplicate detection without post-startup allocation.

## Data Flow

### Startup

At startup, allocate all required storage once:

- session slot pool
- slot free-list
- per-slot replay tables
- any event-token storage needed for `listen_fd`, `timer_fd`, and live session slots

Capacity is derived from configuration so sizing is explicit and deterministic.

### Live Session Lifecycle

1. `accept()` succeeds.
2. Acquire one free slot. If none is available, close the new socket and reject the connection.
3. Initialize the slot's `ClientState` in place.
4. Register the slot's stable event token in `epoll_event.data.ptr`.
5. On epoll wakeup, resolve the token directly to the slot.
6. Handle receive, decode, duplicate detection, outbound queueing, and close logic against the slot in place.
7. On close, unregister the fd, close it, reset the slot, increment its generation, and return the index to the free-list.

### Test Session Lifecycle

1. `createSessionForTest()` acquires a free slot. If none is available, fail clearly.
2. Initialize the slot in test mode.
3. Return a stable test handle backed by the slot, using slot index plus generation rather than a lookup key.
4. Test helper calls operate directly on that slot-backed handle.
5. Releasing a test session resets the slot and returns it to the free-list.

### Duplicate Order Handling

1. Probe the slot-owned replay table with `user_ref_num`.
2. If found, return the cached replay result and mark it duplicate.
3. If not found, validate and build the first-time result.
4. Insert the result into the replay table in place.
5. If the replay table is exhausted, reject the order rather than allocate.

## Error Handling

Capacity exhaustion is handled as explicit runtime rejection:

- no free live slots: close the newly accepted socket without registering it in epoll
- no free test slots: fail the test helper call clearly
- per-session replay table full: reject the order through a deterministic server-side rejection path

Invalid or stale event tokens are handled defensively:

- each live slot carries a generation counter
- the event token includes the slot generation observed at registration time
- if an epoll event arrives for a reused slot with an older generation, ignore it

This prevents stale-event use after slot reuse.

## API and Structure Changes

The refactor should make the storage model explicit rather than hiding it behind map-like access.

Expected structural changes:

- replace `m_live_clients` with a session slot pool
- replace `m_test_clients` with the same slot-pool-backed mechanism used by tests
- replace `ClientState::order_results` with a preallocated replay table abstraction
- change close helpers from fd-based lookup to slot/token-based operations
- change test helper APIs from lookup-key semantics to stable slot-backed handle semantics

The exact type names can be chosen during implementation, but the ownership model should stay visible in the type structure.

## Testing

Add or update tests for the following:

- slot acquisition, release, and reuse
- slot exhaustion behavior for live and test sessions
- duplicate-order replay hits and misses
- replay-table collision handling
- replay-table exhaustion rejection
- live epoll dispatch through stable event tokens
- stale-event rejection through generation mismatch
- regression coverage for login, heartbeat, logout, delayed fills, and normal close paths

Tests should verify both correctness and the intended storage semantics where practical, especially slot reuse and exhaustion behavior.

## Implementation Notes

- Startup sizing should be explicit in configuration so capacities are reviewable and testable.
- Slot storage must remain stable in memory after startup; no later resize is allowed.
- The live event loop should never need to recover a session by searching from `fd`.
- The replay-table implementation only needs the operations required by this module; it does not need to be generalized prematurely.

## Success Criteria

The change is complete when all of the following are true:

- no `std::unordered_map` remains in the dummy exchange runtime or test paths
- all required storage is allocated up front before serving traffic
- no runtime path performs further dynamic allocation for session or replay storage
- the live epoll event path resolves directly from `event.data.ptr` to the target session slot
- exhaustion paths reject cleanly and deterministically
- existing protocol behavior is preserved aside from explicit capacity rejection behavior
- tests cover slot reuse, exhaustion, replay behavior, and stale-event safety
