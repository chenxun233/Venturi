# FPGA Regression Rollback-In-Spirit Design

## Goal

Restore the older regression behavior without performing a literal pre-`1b04f25` rollback.

This change is intentionally scoped to:

- keep the current `FPGARegression` file names and integration points
- remove the unresolved merge markers in `FPGA_regression.cpp`
- discard the newer freeze-after-convergence implementation from commit `1b04f25`
- restore behavior close to the older implementation currently visible in the `theirs` side of the merge conflict

This change does **not** aim to recreate the exact old tree shape where the code lived in `sync/regression.*` under a different class name.

## Current Problems

- `cpp_src/FPGA_boost_demo/sync/FPGA_regression.cpp` contains unresolved conflict markers and does not represent a coherent implementation.
- The current header and implementation mix two incompatible models:
  - the newer explicit freeze/anchor model
  - the older smoothing-based regression update logic
- The current worktree also still has deleted legacy files `sync/regression.cpp` and `sync/regression.h`, so a literal rollback would require a larger integration reversal than requested for approach `3`.

## Chosen Direction

Use the current `FPGARegression` file names, but restore the older behavior internally.

That means:

- prefer the `theirs` side in the current `FPGA_regression.cpp` conflict for runtime behavior
- keep the code building under the `FPGARegression` class name
- accept that the restored result is behaviorally older, but structurally newer than the exact historical tree

## Scope

### Files In Scope

- `cpp_src/FPGA_boost_demo/sync/FPGA_regression.cpp`
- `cpp_src/FPGA_boost_demo/sync/FPGA_regression.h`
- `cpp_src/FPGA_boost_demo/tests/regression_test.cpp`

### Files Out Of Scope

- restoring `cpp_src/FPGA_boost_demo/sync/regression.cpp`
- restoring `cpp_src/FPGA_boost_demo/sync/regression.h`
- reworking unrelated latency or logging code
- changing external callers outside what is necessary to keep the build coherent

## Target Behavior

### Constructor And Configuration

`FPGARegression` should use the older constructor shape:

- single `trigger_period` argument
- no explicit convergence-tuning parameters exposed in the public constructor

The regression behavior should again be controlled by file-local constants in the implementation.

### Snapshot Acceptance

`tryAcceptSnapshot()` should:

- reject snapshots with `interval_ns == 0`
- reject snapshots with `interval_ns > accepted_interval_ns`
- update `m_pre_snapshot` and `m_cur_snapshot`
- avoid the newer “refresh latest anchor while frozen” behavior

### Regression Update Logic

The class should restore the older `_update_a()` model:

- compute `tick_delta` and `host_time_delta_ns` from `m_cur_snapshot` and `m_pre_snapshot`
- use the older smoothed update path for `a_q32`
- compare the previous and current smoothed values using file-local convergence constants
- freeze once the older convergence condition is met

The newer raw-estimate state should be removed:

- no `updateRegression()`
- no `m_has_prev_raw_a`
- no `m_prev_raw_a_q32`
- no `m_stable_update_count`
- no `m_frozen_regression_para`
- no explicit threshold members in the class

### Conversion

`convertFpgaToHostTime()` should restore the older implementation:

- require `m_cur_regression_para.has_para`
- use `m_cur_snapshot` as the conversion anchor
- extrapolate forward from `m_cur_snapshot`
- do not keep the newer backward-conversion path from the latest-anchor model

This is intentionally not as robust as the newer implementation; it is a rollback in spirit.

### Status Reporting

`readStatusLogRecord()` should report:

- `has_para` from `m_cur_regression_para`
- `a_ns_per_tick` derived from `m_cur_regression_para.a_q32`

## Header Shape

`FPGA_regression.h` should match the older behavior and expose:

- `FPGARegression(uint64_t trigger_period = 0)`
- `initSync(FPGADev& device, std::size_t max_attempts, uint64_t accepted_interval_ns)`
- `run(CapSignal&)`
- `tryAcceptSnapshot(...)`
- `readStatusLogRecord()`
- `isFrozen()`
- `convertFpgaToHostTime(...)`

Private members should revert to the older style:

- `m_pre_regression_para`
- `m_cur_regression_para`
- `m_converge_count`
- `m_is_frozen`
- `m_pre_snapshot`
- `m_cur_snapshot`

The header should no longer expose the newer raw-estimate helpers or explicit convergence state.

## Test Strategy

Because this is not an exact historical rollback, the tests should focus on coherence with the restored current API rather than on recreating the old `Regression` class tests verbatim.

Test updates should:

- remove expectations tied to the raw-estimate freeze model
- remove expectations tied to latest-anchor refresh after freeze
- keep coverage for accepted/rejected snapshots, status visibility, and conversion under the restored behavior
- ensure the target compiles and the regression tests match the rollback-in-spirit API

## Risks

- The result will still differ from exact pre-`1b04f25` history because the file names and class name stay modern.
- Any code that started depending on the newer anchor-refresh or backward-conversion behavior will no longer have those semantics.
- Because the current tree already diverged from the older layout, tests may need small adjustments to target the current file/class names.

## Verification

After implementation:

- build the regression test target
- run the regression test binary
- confirm `FPGA_regression.cpp` no longer contains conflict markers

## Non-Goals

- exact historical restoration of `sync/regression.*`
- preserving the newer freeze-after-convergence design from commit `1b04f25`
- refactoring broader sync architecture
