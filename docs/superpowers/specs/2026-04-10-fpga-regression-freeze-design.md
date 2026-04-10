# FPGA Regression Freeze Design

## Goal

Fix the current `FPGARegression` behavior so that:

- regression computes `a` only during an initial calibration phase
- once converged, `a` is frozen and never updated again
- after freeze, each newly accepted snapshot becomes the latest conversion anchor
- `convertFpgaToHostTime()` converts both forward and backward relative to the latest anchor
- the convergence knobs are explicit variables instead of hidden file-scope constants

## Current Problems

- The current implementation mixes calibration state and runtime anchor state.
- `_update_a()` compares smoothed values in a way that can freeze incorrectly or too early.
- `convertFpgaToHostTime()` only extrapolates forward from `m_cur_snapshot`.
- After freeze, snapshot refresh and regression update are not clearly separated.
- The convergence knobs are implicit constants rather than explicit regression state.

## Proposed Design

### Phase Model

`FPGARegression` operates in two phases:

1. Calibration phase
   - accepted snapshots are used to compute raw slope estimates
   - the implementation tracks convergence across consecutive raw estimates
   - no stable runtime parameter is exposed until convergence is reached
2. Frozen phase
   - the frozen slope `a_q32` is immutable
   - accepted snapshots only update the latest anchor snapshot
   - no more regression calculation is performed

### Explicit Variables

The convergence controls become explicit class members with constructor defaults:

- `m_required_stable_updates`
- `m_convergence_threshold_ns_per_tick`

The regression state is also made explicit:

- `m_prev_raw_a_q32`
- `m_has_prev_raw_a`
- `m_stable_update_count`
- `m_frozen_a_q32`
- `m_is_frozen`

The latest accepted snapshot used for conversion remains explicit:

- `m_latest_snapshot`

The calibration path continues to retain the previous accepted snapshot needed to compute a new raw estimate:

- `m_pre_snapshot`
- `m_cur_snapshot`

### Snapshot Acceptance

`tryAcceptSnapshot()` keeps the existing interval filter in both phases.

Before freeze:

- reject snapshots with invalid interval
- shift `m_pre_snapshot <- m_cur_snapshot`
- store the new snapshot in `m_cur_snapshot`
- allow the caller to run the regression update step

After freeze:

- reject snapshots with invalid interval
- store the new snapshot as `m_latest_snapshot`
- do not modify the frozen slope or any calibration counters

This keeps the runtime conversion anchor fresh without reopening regression.

### Freeze Rule

Freeze is based on consecutive accepted raw slope estimates.

For each valid pair of accepted snapshots:

- compute `tick_delta`
- compute `host_delta_ns`
- derive `raw_a_q32 = round(host_delta_ns * Q32 / tick_delta)`

Convergence logic:

- if there is no previous raw estimate, save this one and start the streak
- otherwise compare the new raw estimate against the previous raw estimate
- if the absolute difference is less than or equal to the configured threshold in Q32 units, increment `m_stable_update_count`
- otherwise reset `m_stable_update_count` to zero
- store the new raw estimate as `m_prev_raw_a_q32`

Freeze occurs when the streak reaches `m_required_stable_updates`.
At that point:

- `m_frozen_a_q32` is set to the current raw estimate
- regression status becomes available
- `m_is_frozen` becomes `true`
- `m_latest_snapshot` is set to the most recent accepted snapshot

No smoothing is used after this change. The design uses the converged raw estimate directly.

## Conversion Behavior

`convertFpgaToHostTime()` requires:

- frozen regression
- a valid latest anchor snapshot

Given an input `fpga_tick`:

- if `fpga_tick >= m_latest_snapshot.fpga_tick`, compute a positive delta and add it to `m_latest_snapshot.host_time_ns`
- if `fpga_tick < m_latest_snapshot.fpga_tick`, compute the absolute tick delta, scale it with frozen `a_q32`, and subtract it from `m_latest_snapshot.host_time_ns`

The method returns `false` only when conversion is not yet possible.

This supports both future and past FPGA ticks relative to the most recent accepted anchor.

## API and State Semantics

- `readStatusLogRecord()` reports `has_para = true` only after freeze and reports the frozen `a`
- `isFrozen()` reflects whether calibration is complete
- `initSync()` keeps sampling until the regression freezes or the attempt budget is exhausted
- `run()` remains a cadence helper for requesting new snapshots and does not contain regression logic

## Error Handling

- snapshots with zero interval or interval greater than the configured acceptance bound are rejected
- zero `tick_delta` between consecutive accepted snapshots does not advance convergence
- backward conversion must guard against unsigned underflow by branching on tick order before subtraction
- if no frozen parameter exists yet, conversion fails cleanly

## Testing

Update and extend regression tests to cover:

- freeze after the configured number of consecutive stable raw estimates
- no further regression updates after freeze
- latest accepted snapshot refresh after freeze
- forward conversion relative to the latest anchor
- backward conversion relative to the latest anchor
- rejection of invalid snapshots in both calibration and frozen phases
- `initSync()` stopping once frozen

## Non-Goals

- adaptive re-calibration after freeze
- multi-parameter regression beyond the slope `a`
- changing the external call pattern in `Venturi.cpp`
