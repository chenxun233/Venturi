# Regression Absorbs SyncHandler Design

## Goal

Remove `SyncHandler` as a separate class and merge its responsibilities into `Regression`.

This change is motivated by the current state of the code:
- periodic screenshot printing has already been removed
- `SyncHandler` is now only a thin wrapper around startup sync convergence and periodic capture triggering
- both behaviors exist only to support regression updates

After this change, `Venturi.cpp` should stop knowing about a separate sync-handling object and should treat `Regression` as the owner of sync-for-regression policy.

## Current Problem

The current split is too thin to justify two classes:
- `SyncHandler::initSync(...)` loops on device snapshots until regression freezes
- `SyncHandler::run(CapSignal&)` is only a countdown that requests periodic captures
- `Regression` already owns snapshot ingestion, freeze state, latest accepted snapshot, and FPGA-to-host conversion

This leaves the same concern split across two classes:
- `SyncHandler` decides when sync capture happens
- `Regression` decides how snapshots affect the regression model

That boundary no longer buys meaningful isolation. It makes `Venturi.cpp` construct and call an extra object whose only purpose is to feed `Regression`.

## Recommended Approach

Fold `SyncHandler` fully into `Regression`.

`Regression` should own:
- startup sync convergence
- periodic capture triggering
- snapshot qualification policy
- accepted snapshot state
- regression fitting and freeze logic

`Venturi.cpp` should only orchestrate:
- create `Regression`
- call `regression.initSync(...)`
- call `regression.run(capture_signal)` in the control thread
- pass runtime snapshots to `Regression`

This keeps `Venturi` focused on instance creation and thread orchestration, while moving all sync-for-regression policy into one owner.

## Rejected Alternatives

### Keep SyncHandler Separate

This preserves an extra class that no longer has enough independent behavior to justify itself.

The remaining `SyncHandler` behavior is too small and too tightly coupled to regression update policy.

### Move Only initSync Into Regression

This removes one layer but still leaves periodic capture policy outside the owner of regression updates.

That keeps the same conceptual split:
- one class decides when to capture
- another class decides what captures mean

The design goal here is to unify those policies.

### Move Trigger Logic Into Venturi

This makes `Venturi` responsible for regression-specific timing policy again.

That works mechanically, but it weakens the composition-root boundary and moves domain policy back into orchestration code.

## Ownership Model

After the merge:

### Regression Owns

- latest accepted `FpgaSyncSnapshot`
- regression parameters and freeze state
- snapshot acceptance rule
- startup sync initialization loop
- periodic capture trigger countdown

### Venturi Owns

- object construction
- thread creation
- worker-loop orchestration
- passing snapshots and control signals between runtime components

### Removed

- standalone `SyncHandler`

## API Shape

`Regression` should absorb the externally visible `SyncHandler` behavior.

Recommended public API additions:
- `explicit Regression(uint64_t trigger_period = 0);`
- `template <typename DeviceT> bool initSync(DeviceT& device, std::size_t max_attempts, uint64_t accepted_interval_ns);`
- `void run(CapSignal& cap_signal);`
- `bool tryAcceptSnapshot(const FpgaSyncSnapshot& snapshot, uint64_t accepted_interval_ns);`

Existing `Regression` APIs should remain:
- `void updateSnapshot(const FpgaSyncSnapshot* snapshot);`
- `FpgaSyncSnapshot readSnapshot() const;`
- `RegressionPara returnParaSnapshot() const;`
- `RegressionStatusLogRecord readStatusLogRecord() const;`
- `bool isFrozen() const;`
- `bool convertFpgaToHostTime(...) const;`

## Internal Structure

`Regression` should conceptually own three kinds of state:

### Snapshot State

- latest accepted `FpgaSyncSnapshot`

### Regression Model State

- regression sums
- fitted parameters
- stable-update count
- frozen flag

### Trigger State

- `m_trigger_period`
- `m_trigger_countdown`

The startup and runtime snapshot paths should share one acceptance rule through a private helper.

Recommended private helper:
- `_tryAcceptSnapshot(const FpgaSyncSnapshot& snapshot, uint64_t accepted_interval_ns);`

This helper should:
1. reject unusable snapshots
   - `interval_ns == 0`
   - `interval_ns > accepted_interval_ns`
2. accept usable snapshots into regression-owned state
3. update the model through the existing regression update path
4. report whether the snapshot was accepted

## Venturi Changes

`Venturi.cpp` should simplify to:

- construct `Regression regression(kSnapshotSamplePeriod);`
- remove `SyncHandler sync_handler(...)`
- replace:
  - `sync_handler.initSync(...)`
  with:
  - `regression.initSync(device, kInitSyncMaxAttempts, accepted_interval_ns)`
- replace:
  - `sync_handler.run(capture_signal)`
  with:
  - `regression.run(capture_signal)`
- replace inline runtime snapshot qualification like:
  - `if (get_snapshot && snapshot.interval_ns != 0) { ... }`
  with:
  - `regression.tryAcceptSnapshot(snapshot, accepted_interval_ns);`

`Venturi` should not contain snapshot acceptance policy after this change.

## File Changes

### Modify

- `cpp_src/FPGA_boost_demo/sync/regression.h`
- `cpp_src/FPGA_boost_demo/sync/regression.cpp`
- `cpp_src/FPGA_boost_demo/app/Venturi.cpp`
- `cpp_src/CMakeLists.txt`
- regression-focused tests that currently depend on `SyncHandler`

### Delete

- `cpp_src/FPGA_boost_demo/sync/sync_handler.h`
- `cpp_src/FPGA_boost_demo/sync/sync_handler.cpp`

### Replace Tests

Current `SyncHandler` behavior should be covered by regression-owned tests instead:
- startup init success/failure
- periodic trigger countdown
- snapshot acceptance rule

## Behavior To Preserve

The merge must preserve:
- startup convergence still gates process startup
- periodic capture requests still happen at the same cadence
- only qualified snapshots update regression state
- `readSnapshot()` still returns the latest accepted snapshot
- existing regression freeze behavior remains unchanged

## Risks

### Regression Gets Broader

This is intentional. The class is becoming the owner of sync-for-regression coordination, not just the mathematical fit.

That is still a coherent responsibility because both startup sync and periodic capture exist only to support regression updates.

### Runtime Policy Drift

The main regression risk is changing snapshot acceptance behavior during the merge.

To avoid that, both startup and runtime paths should use the same acceptance helper instead of duplicating conditions in multiple places.

## Testing

Testing should verify:
- `Regression::initSync(...)` returns true when valid snapshots converge
- `Regression::initSync(...)` returns false when no valid snapshots qualify
- `Regression::run(CapSignal&)` preserves the current trigger cadence
- `Regression::tryAcceptSnapshot(...)` rejects invalid intervals and accepts qualified ones
- `Venturi.cpp` no longer references `SyncHandler`

## Scope

This change is intentionally narrow:
- merge `SyncHandler` into `Regression`
- simplify `Venturi` accordingly
- replace tests

It does not redesign regression math, latency tracking, RX flow, or TX flow.
