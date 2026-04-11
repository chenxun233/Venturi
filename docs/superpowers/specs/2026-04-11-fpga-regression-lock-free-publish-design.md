# FPGA Regression Lock-Free Publish Design

## Goal

Remove the reader-side mutex from `FPGARegression` by introducing a lock-free publish path for reader-visible regression state.

The new design should:

- keep regression fitting as a single-writer workflow
- publish a self-consistent immutable reader snapshot
- remove `std::mutex` and `std::lock_guard` from reader-facing paths
- preserve current public APIs and external behavior
- keep the full-fit startup regression model and freeze logic

## Current Problems

- `readStatusLogRecord()`, `isFrozen()`, `convertFpgaToHostTime()`, and `_update_a()` currently share mutable state behind `m_regression_mutex`
- the mutex protects correctness, but it couples fast read paths to writer-side fitting internals
- readers only need a compact published view, not the full mutable regression state
- separate lock-based reads make it easy to accidentally expand the critical section as the class evolves

## Chosen Direction

Split `FPGARegression` into:

- a writer-only internal fit state
- an immutable published state loaded atomically by readers

The writer path remains the only code that mutates regression internals. After each accepted update that changes externally visible state, it constructs a new immutable published snapshot and atomically publishes it through:

- `std::atomic<std::shared_ptr<const PublishedState>>`

Readers perform one atomic load and operate only on that published object.

## Scope

### Files In Scope

- `cpp_src/FPGA_boost_demo/sync/FPGA_regression.h`
- `cpp_src/FPGA_boost_demo/sync/FPGA_regression.cpp`
- `cpp_src/FPGA_boost_demo/tests/regression_test.cpp`

### Files Out Of Scope

- changing call sites in `LatencyTracker`, `Venturi`, or RX engine code
- changing `RegressionPara`
- changing regression acceptance criteria or cadence policy beyond what is needed for publication
- making the writer-side fit itself multi-writer or wait-free

## Published State

Add a compact immutable structure that contains only the state readers need:

- `RegressionPara regression_para`
- `bool is_frozen`
- `FpgaSyncSnapshot anchor_snapshot`

This published state is the only source of truth for:

- `readStatusLogRecord()`
- `isFrozen()`
- `convertFpgaToHostTime()`

Readers must not access mutable fit accumulators, convergence counters, or candidate snapshots.

## Writer-Owned Internal State

Keep all fit internals private to the single writer:

- accepted-snapshot regression sums
- previous fitted-`a` state used for convergence
- convergence count
- candidate snapshot used by `run()`
- working snapshots used while evaluating a new accepted sample

The writer-owned state does not need a mutex as long as mutation remains confined to the existing sync-update path.

## Publish Flow

The publish flow should work as follows:

1. `tryAcceptSnapshot()` validates the incoming snapshot against the current acceptance rules.
2. If rejected, no published state changes.
3. If accepted, the writer updates its private working state.
4. If the model is still unfrozen, the writer updates the full-fit regression and may update `a_q32`.
5. If the model is frozen, the writer does not change the fitted parameter but may refresh the latest accepted anchor snapshot.
6. The writer constructs a new `PublishedState` from the current externally visible state.
7. The writer stores it with release semantics.

Readers load the published pointer with acquire semantics and use only that loaded object for the entire read operation.

## API Behavior

Public method behavior remains the same:

- `readStatusLogRecord()` returns `has_para` and `a_ns_per_tick` from the published state
- `isFrozen()` returns the published frozen flag
- `convertFpgaToHostTime()` returns `false` until a published state with valid parameters exists
- `convertFpgaToHostTime()` uses the published anchor snapshot and published `a_q32`

No public signature changes are required.

## Conversion Consistency

The publish boundary must guarantee that a reader sees a matching set of:

- `has_para`
- `a_q32`
- `is_frozen`
- anchor snapshot

This avoids mixed reads such as:

- a new anchor snapshot paired with an older `a_q32`
- `has_para == false` paired with a non-default fitted parameter

Using one atomically loaded immutable object is the chosen mechanism for that consistency.

## Freeze Semantics

Freeze remains writer-owned.

Once convergence is reached:

- published `a_q32` stops changing
- published `is_frozen` becomes `true`
- subsequent accepted snapshots may still publish a new anchor snapshot

This preserves the current model where post-freeze conversion follows the latest accepted anchor while keeping the fitted slope fixed.

## Initialization

Construction should publish an initial default state so readers always have a valid object to load.

The initial published state should represent:

- `has_para == false`
- default `a_q32`
- `is_frozen == false`
- zero-initialized anchor snapshot

This removes the need for null published-pointer checks in normal reader code.

## Error Handling

- Rejected snapshots do not publish anything new.
- Invalid or not-yet-ready regression fits do not publish a parameterized state prematurely.
- Conversion failures continue to return `false` rather than throwing or blocking.
- Any heap allocation failure while constructing a new published state can surface as a normal exception path; no special recovery mechanism is added in this change.

## Test Strategy

Update regression tests to cover the publish model:

- readers see the default unpublished state before enough samples are accepted
- accepted updates publish coherent `a` and anchor state
- once frozen, `a` stops changing while anchor refresh still works
- `convertFpgaToHostTime()` succeeds from published state only
- repeated reads during successive publishes do not observe impossible combinations

Tests can stay single-process and deterministic; they do not need to prove lock-freedom formally.

## Risks

- `std::shared_ptr` publication introduces allocation on each publish, which is acceptable here because snapshot publish frequency is low relative to packet processing
- if future code adds multiple writer threads, this design becomes invalid without further synchronization
- reader consistency depends on strictly avoiding direct reads of writer-owned mutable state after the refactor

## Verification

After implementation:

- build `regression_test`
- run the regression test binary
- run `ctest --test-dir build/cpp --output-on-failure -R RegressionTest`

## Non-Goals

- removing all dynamic allocation from regression publication
- changing regression math or convergence thresholds in this design
- redesigning surrounding thread ownership outside `FPGARegression`
