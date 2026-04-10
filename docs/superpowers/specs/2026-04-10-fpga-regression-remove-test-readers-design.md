# FPGARegression Remove Test-Only Reader APIs

## Goal

Shrink the `FPGARegression` public API by removing reader methods that are only used by tests and are not needed by the runtime path.

## Current State

`FPGARegression` currently exposes three reader functions:

- `readSnapshot()`
- `returnParaSnapshot()`
- `readStatusLogRecord()`

In the current tree:

- `readSnapshot()` is only used by `regression_test.cpp`
- `returnParaSnapshot()` is only used by `regression_test.cpp`
- `readStatusLogRecord()` is still used by `Venturi.cpp`

This means the first two functions are test-only API surface.

## Decision

Remove:

- `FpgaSyncSnapshot readSnapshot() const;`
- `RegressionPara returnParaSnapshot() const;`

Keep:

- `RegressionStatusLogRecord readStatusLogRecord() const;`

`readStatusLogRecord()` stays because it is part of the runtime logging path used by `Venturi`.

## Testing Strategy

Update `regression_test.cpp` so it no longer inspects raw internal state through those removed readers.

Tests should validate `FPGARegression` through observable behavior:

- `isFrozen()`
- `tryAcceptSnapshot(...)`
- `readStatusLogRecord()`
- `convertFpgaToHostTime(...)`

This keeps tests aligned with behavior instead of implementation details.

## File Changes

- Update `cpp_src/FPGA_boost_demo/sync/FPGA_regression.h`
- Update `cpp_src/FPGA_boost_demo/sync/FPGA_regression.cpp`
- Update `cpp_src/FPGA_boost_demo/tests/regression_test.cpp`

No runtime call sites outside tests should need changes, because `Venturi` only depends on `readStatusLogRecord()`.

## Risks

The main risk is weakening regression coverage when converting tests away from internal readers. The replacement assertions should still prove:

- the fit converges
- the fit remains stable after freeze
- accepted snapshots affect runtime conversion behavior
- logging-facing status stays correct
