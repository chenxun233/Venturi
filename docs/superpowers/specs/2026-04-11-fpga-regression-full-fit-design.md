# FPGA Regression Full-Fit Design

## Goal

Replace the current per-delta averaging logic in `FPGARegression` with a full linear regression fit over all accepted startup snapshots until freeze.

The new design should:

- compute `a` from ordinary least squares over the full accepted snapshot history
- fit both slope `a` and intercept `b` internally
- continue to expose only `a` through the current public status/parameter path
- freeze when consecutive fitted `a` values stabilize
- stop updating the model after freeze

## Current Problems

- The current `a` update is based on per-delta averaging rather than a full regression fit.
- Early outliers can distort the running estimate for a long time.
- The current convergence settings are extremely loose and can freeze on a bad estimate.
- The current model uses only local deltas between two snapshots, even though the user wants a full regression calculation over the whole startup history.

## Chosen Direction

Use full-history ordinary least squares during startup.

For each accepted snapshot `(x, y)`:

- `x = fpga_tick`
- `y = host_time_ns`

Maintain the running sums:

- `sum_x`
- `sum_y`
- `sum_xx`
- `sum_xy`
- `sample_count`

From those, compute the fitted line:

- `a = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x^2)`
- `b = (sum_y - a * sum_x) / n`

Only `a` is published into `RegressionPara`.

## Scope

### Files In Scope

- `cpp_src/FPGA_boost_demo/sync/FPGA_regression.h`
- `cpp_src/FPGA_boost_demo/sync/FPGA_regression.cpp`
- `cpp_src/FPGA_boost_demo/tests/regression_test.cpp`

### Files Out Of Scope

- changing external latency-tracker interfaces
- exposing `b` in `RegressionPara`
- redesigning the surrounding sync pipeline

## Model State

The regression state should be reorganized around full-fit accumulation.

Keep:

- `m_cur_regression_para`
- `m_is_frozen`
- `m_trigger_period`
- `m_trigger_countdown`
- `m_candidate_snapshot`
- `m_pre_snapshot`
- `m_cur_snapshot`

Add or repurpose internal fit state:

- `m_sample_count`
- `m_sum_fpga_tick`
- `m_sum_host_time_ns`
- `m_sum_fpga_tick_sq`
- `m_sum_fpga_host_product`
- `m_prev_fitted_a_q32`
- `m_has_prev_fitted_a`
- `m_converge_count`
- optional internal `m_fitted_b_ns`

Remove state that only supports averaging-by-delta:

- `m_sum_a_q32`
- `m_pre_regression_para` if no longer needed for convergence

## Snapshot Handling

`tryAcceptSnapshot()` should continue to:

- reject snapshots with `interval_ns == 0`
- reject snapshots with `interval_ns > accepted_interval_ns`
- update `m_pre_snapshot` and `m_cur_snapshot`

It should not directly decide convergence.

`initSync()` continues to call the model-update function after a successful acceptance.

## Regression Update Logic

On each accepted snapshot before freeze:

1. Add the current snapshot into the running regression sums.
2. If there are fewer than 2 accepted samples, return without publishing a parameter.
3. Compute the regression denominator:
   - `den = n * sum_xx - sum_x * sum_x`
4. If `den <= 0`, return without publishing a parameter.
5. Compute fitted `a` and `b`.
6. If `a <= 0`, reject the update and keep waiting for more data.
7. Convert fitted `a` into `a_q32` and publish it into `m_cur_regression_para`.
8. Compare this fitted `a_q32` against the previous fitted `a_q32`.
9. If the difference is below the configured threshold, increment the consecutive-stable counter; otherwise reset it.
10. Freeze once the stable counter reaches the required count.

After freeze:

- `m_cur_regression_para` remains fixed
- the model sums are no longer updated

## Freeze Rule

Freeze is based on consecutive fitted-slope stability, not on a fixed sample count alone.

The logic should use:

- a minimum sample count before stability checks begin
- a convergence threshold for `a`
- a required number of consecutive stable fitted values

Recommended defaults:

- minimum fit samples: keep the current startup scale or use a similarly conservative number
- consecutive stable updates: at least `4`
- threshold: much tighter than the current `1e-1`

The exact constants can remain file-local unless there is a reason to expose them.

## Intercept Handling

The model should fit both `a` and `b` internally because that produces the correct least-squares slope when the data has an offset.

However:

- `RegressionPara` should continue to publish only `a_q32`
- no external API should depend on `b`
- `b` is internal fit state only

## Conversion Behavior

`convertFpgaToHostTime()` should continue using the existing public behavior unless changed separately.

For this design:

- keep using the currently accepted anchor snapshot for conversion
- use the fitted `a_q32`
- do not expand scope to redesign anchor semantics in this change

## Test Strategy

Update tests to match the new full-fit model:

- verify that full-history regression converges near the expected slope
- verify that conversion works after convergence
- verify that `initSync()` can freeze the model
- verify invalid snapshot rejection still works
- verify cadence logic in `run()` remains unchanged

The tests should use deterministic synthetic snapshots where the expected slope is known.

## Risks

- Large raw sums may need `long double` or wider intermediate storage to avoid precision loss.
- If the accepted snapshots are highly noisy during startup, the minimum sample count and stability threshold matter a lot.
- Keeping current anchor semantics means this change improves `a` estimation only; it does not address other potential conversion-accuracy issues outside the slope fit.

## Verification

After implementation:

- build `regression_test`
- run the regression test binary
- run `ctest --test-dir build/cpp --output-on-failure -R RegressionTest`

## Non-Goals

- exposing `b` outside the class
- changing the transport or latency APIs
- redesigning post-freeze anchor behavior
