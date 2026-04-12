# Buffer Rename Hard Cutover Design

## Goal

Rename the two existing buffer primitives so their names match their real semantics:

- `TraceBuffer<T>` becomes `SpscRingQueue<T>`
- `RingBuffer<T, Capacity>` becomes `FixedCircularBuffer<T, Capacity>`

This is a repo-wide hard cutover inside `cpp_src/FPGA_boost_demo` with no compatibility aliases.

## Why This Change

The current names no longer describe the actual abstractions:

- `TraceBuffer` is now the general bounded single-producer/single-consumer queue primitive used in latency, executor, and TX paths. It is not trace-specific anymore.
- `RingBuffer` is a fixed-capacity, single-threaded circular container with richer indexed and prefix operations than a plain queue.

The rename should improve readability without changing runtime behavior.

## Scope

### In Scope

- Rename type names, file names, include paths, test names, and obvious type-shaped member names.
- Update all `FPGA_boost_demo` call sites in one pass.
- Update `cpp_src/CMakeLists.txt` to use the renamed files and renamed tests.
- Keep behavior unchanged.

### Out Of Scope

- No compatibility typedefs, wrapper headers, or staged aliases.
- No API redesign beyond name changes required by the type rename.
- No queue ownership refactor.
- No implementation merge between the two buffer types.
- No changes to runtime semantics, lock behavior, or drop behavior.

## Chosen Rename Set

### Runtime-Capacity SPSC Queue

- File: `latency/trace_buffer.h` -> `common/spsc_ring_queue.h`
- File: `latency/trace_buffer.cpp` -> `common/spsc_ring_queue.cpp`
- Type: `TraceBuffer<T>` -> `SpscRingQueue<T>`
- Test: `trace_buffer_overwrite_test.cpp` -> `spsc_ring_queue_overwrite_test.cpp`
- Test suite: `TraceBufferOverwriteTest` -> `SpscRingQueueOverwriteTest`

This type keeps its current queue-style API:

- `push`
- `pushDropOldest`
- `pop`
- `readCapacity`
- `readDropCount`

### Fixed-Capacity Single-Thread Circular Container

- File: `common/ring_buffer.h` -> `common/fixed_circular_buffer.h`
- Type: `RingBuffer<T, Capacity>` -> `FixedCircularBuffer<T, Capacity>`
- Test: `fixed_ring_buffer_test.cpp` -> `fixed_circular_buffer_test.cpp`
- Test suite: `FixedRingBufferTest` -> `FixedCircularBufferTest`

This type keeps its current richer container API:

- `pushBack`
- `write`
- `eraseFront`
- `eraseFrontN`
- `readFront`
- `readAt`
- `copyFrom`
- `readSize`
- `readCapacity`

## File-Level Design

### `common/spsc_ring_queue.h` / `.cpp`

These files become the canonical home of the runtime-capacity SPSC queue currently called `TraceBuffer`.

The implementation remains templated and behaviorally identical:

- runtime-configured capacity
- power-of-two validation
- atomic head/tail indices
- drop counting
- `pushDropOldest` mutex behavior preserved exactly as-is

Only naming and file placement change.

### `common/fixed_circular_buffer.h`

This file becomes the canonical home of the fixed-capacity circular container currently called `RingBuffer`.

The implementation remains:

- `std::array`-backed
- compile-time power-of-two capacity
- single-threaded
- rich indexed/prefix operations

Only naming changes.

### Member Naming Rules

Member names should be renamed only when they are mechanically tied to the old type name.

Use domain names rather than literal type names where possible:

- `LatencyTracker::m_trace_buffer` should become a domain name such as `m_latency_queues`
- `Executor::m_intent_buffers` should remain `m_intent_buffers`
- exchange protocol members such as `m_inbound_bytes` should keep their domain names and only change the type

## Affected Call-Site Areas

### Latency Path

Current `TraceBuffer` users in latency tracking must be updated to `SpscRingQueue`, including:

- `latency_tracker.h/.cpp`
- tests that inspect queue internals

### Executor And TX Path

Current `TraceBuffer` users in executor and TX code must be updated to `SpscRingQueue`, including:

- `executor.h/.cpp`
- `tx_sender.h/.cpp`
- related tests

### Exchange Protocol Path

Current `RingBuffer` users in exchange protocol must be updated to `FixedCircularBuffer`, including:

- `exchange_protocol.h`
- fixed buffer tests

## Build And Include Strategy

Because this is a hard cutover, all include directives must be updated in the same change.

Required include migration:

- `#include "../latency/trace_buffer.h"` -> `#include "../common/spsc_ring_queue.h"`
- `#include "../common/ring_buffer.h"` -> `#include "../common/fixed_circular_buffer.h"`

`cpp_src/CMakeLists.txt` must be updated in the same patch so no target depends on stale file names.

## Testing Strategy

### Rename-Specific Tests

The renamed test files and suites must still cover the same behavior:

- `fixed_circular_buffer_test.cpp`
- `spsc_ring_queue_overwrite_test.cpp`

### Behavioral Regression Coverage

Existing tests in these subsystems should continue to pass with behavior unchanged:

- latency tracker
- executor
- TX sender/receiver runtime
- exchange protocol

### Acceptance Checks

The hard cutover is complete when all of the following are true:

1. No references to `TraceBuffer` remain in `cpp_src/FPGA_boost_demo`.
2. No references to `RingBuffer` remain in `cpp_src/FPGA_boost_demo`.
3. No includes of `trace_buffer.h` remain.
4. No includes of `ring_buffer.h` remain.
5. The renamed tests pass with unchanged semantics.
6. `cpp_src/CMakeLists.txt` points only to the new file names.

## Risks

### Include-Path Breakage

This is the highest-probability migration error because `trace_buffer.h` currently lives under `latency/` while the chosen renamed file lives under `common/`.

Mitigation:

- update all includes repo-wide in the same patch
- verify with repo-wide search after the rename

### Explicit Template Instantiations

The current `trace_buffer.cpp` contains explicit instantiations.

Mitigation:

- rename those to the new type in the same pass
- verify link success in targets that consume `TimeRecord` and `OrderIntent`

### Type-Shaped Member Renames

Members like `m_trace_buffer` will ripple through tests and internal white-box assertions.

Mitigation:

- rename those members deliberately to domain terms
- update white-box tests in the same patch

## Rejected Alternatives

### Rename Plus API Cleanup

Rejected for this change because it would mix vocabulary cleanup with behavior and call-site refactoring, making the review surface much larger.

### Compatibility Alias Phase

Rejected because the user explicitly wants a hard cutover, and keeping old names would extend the ambiguity this rename is meant to remove.

### Rename Plus Implementation Consolidation

Rejected because the two buffers solve different problems:

- one is a runtime-capacity SPSC queue
- one is a fixed-capacity single-thread circular container with richer access patterns

Sharing a name boundary is useful; forcing a structural merge is not part of this goal.

## Implementation Summary

This design performs a repo-wide hard rename with no compatibility layer:

- `TraceBuffer` -> `SpscRingQueue`
- `RingBuffer` -> `FixedCircularBuffer`

The patch should be purely semantic:

- names change
- behavior stays the same
- build and tests move in one pass
