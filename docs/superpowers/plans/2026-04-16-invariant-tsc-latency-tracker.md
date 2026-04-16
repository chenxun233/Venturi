# Invariant TSC Latency Tracker Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace syscall-based host timestamp capture in the latency-tracking path with invariant-TSC reads, while keeping logged latency values in nanoseconds and touching no unrelated code.

**Architecture:** Keep all existing latency stage names and log output unchanged. Change [`readMonotonicRawNs()`](/home/chenxun/Documents/Project/Venturi/cpp_src/FPGA_boost_demo/common/time_utils.h) so host-side call sites capture raw TSC ticks, then teach [`LatencyTracker`](/home/chenxun/Documents/Project/Venturi/cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp) to convert only host-host deltas to nanoseconds with integer math.

**Tech Stack:** C++20, inline x86 intrinsics/assembly, CMake, GoogleTest

---

## File Map

- Modify: `cpp_src/FPGA_boost_demo/common/time_utils.h`
  Add invariant-TSC read helpers, one-time frequency calibration, integer tick-to-ns scale data, and keep a fallback path behind the same public function name.
- Modify: `cpp_src/FPGA_boost_demo/latency/latency_tracker.h`
  Rename or reframe host-side pending timestamps so they are stored as raw host ticks, and add helper declarations for host tick-to-ns conversion.
- Modify: `cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp`
  Convert host-host deltas from ticks to nanoseconds where each latency field is derived, without changing FPGA-derived calculations or printed fields.
- Modify: `cpp_src/FPGA_boost_demo/tests/latency_tracker_test.cpp`
  Add focused tracker coverage that proves host-side deltas are converted from ticks into the same nanosecond values the logs expect.
- Modify only if required: `cpp_src/FPGA_boost_demo/tests/fpga_rx_engine_test.cpp`
  Keep the existing ordering assertions valid if the TSC-backed function changes the semantics of “before” and “after” timestamps in those tests.

## Guardrails

- Do not modify sender, strategy, executor, RX-engine production logic, transport logic, log format, stage schema, CPU pinning, or any unrelated code.
- Do not broaden this into a full timestamp-domain redesign.
- Do not convert ticks to nanoseconds on the hot path.
- Do not introduce floating-point conversion in the hot path or tracker.

### Task 1: Add A TSC-Backed Host Timestamp Source Behind `readMonotonicRawNs()`

**Files:**
- Modify: `cpp_src/FPGA_boost_demo/common/time_utils.h`

- [ ] **Step 1: Add a focused calibration/ordering test scaffold in the timing utility header consumer**

Use an existing latency-path test file rather than creating a new test target. Add a minimal assertion block that can validate monotonic host captures through the unchanged interface:

```cpp
const uint64_t first = readMonotonicRawNs();
const uint64_t second = readMonotonicRawNs();
EXPECT_GT(first, 0U);
EXPECT_GT(second, 0U);
EXPECT_LE(first, second);
```

Put this in `cpp_src/FPGA_boost_demo/tests/latency_tracker_test.cpp` as a small helper assertion or standalone test so the implementation has a direct check that the replacement path still behaves monotonically.

- [ ] **Step 2: Run the focused tests to establish the current baseline**

Run:

```bash
cmake --build cpp_src/build --target latency_tracker_test fpga_rx_engine_test -j8
./cpp_src/build/latency_tracker_test
./cpp_src/build/fpga_rx_engine_test
```

Expected: PASS on the current clock-based implementation.

- [ ] **Step 3: Replace the timing utility internals with a TSC-backed implementation**

Modify `cpp_src/FPGA_boost_demo/common/time_utils.h` so the public entrypoint stays:

```cpp
inline uint64_t readMonotonicRawNs();
```

but its internals become TSC-backed. Add private inline helpers in the same header along these lines:

```cpp
struct HostTickScale {
    uint64_t tsc_hz {0};
    bool use_clock_fallback {false};
};

inline uint64_t readInvariantTscTicks() {
    unsigned int aux = 0;
    return __rdtscp(&aux);
}

inline const HostTickScale& readHostTickScale() {
    static const HostTickScale scale = []() {
        HostTickScale value {};
        // First choice: kernel-exposed TSC frequency.
        // Fallback: one-time CLOCK_MONOTONIC_RAW calibration window.
        // If neither works, set use_clock_fallback = true.
        return value;
    }();
    return scale;
}

inline uint64_t readMonotonicRawNs() {
    const HostTickScale& scale = readHostTickScale();
    if (scale.use_clock_fallback) {
        timespec ts {};
        if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) != 0) {
            throw std::runtime_error("clock_gettime(CLOCK_MONOTONIC_RAW) failed");
        }
        return (static_cast<uint64_t>(ts.tv_sec) * 1000000000ULL) +
               static_cast<uint64_t>(ts.tv_nsec);
    }
    return readInvariantTscTicks();
}
```

Implementation details required in this step:

- use `RDTSCP`
- keep everything integer-based
- cache calibration in a function-local static
- do not change any production call sites

- [ ] **Step 4: Rebuild the focused tests**

Run:

```bash
cmake --build cpp_src/build --target latency_tracker_test fpga_rx_engine_test -j8
```

Expected: compile succeeds. Some runtime expectations may fail until `LatencyTracker` is taught that host-captured timestamps are now ticks instead of nanoseconds.

- [ ] **Step 5: Commit the timing-source change**

```bash
git add cpp_src/FPGA_boost_demo/common/time_utils.h \
        cpp_src/FPGA_boost_demo/tests/latency_tracker_test.cpp
git commit -m "refactor: read host latency timestamps from invariant tsc"
```

### Task 2: Teach `LatencyTracker` To Convert Host Ticks To Nanoseconds

**Files:**
- Modify: `cpp_src/FPGA_boost_demo/latency/latency_tracker.h`
- Modify: `cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp`
- Modify: `cpp_src/FPGA_boost_demo/tests/latency_tracker_test.cpp`
- Modify only if required: `cpp_src/FPGA_boost_demo/tests/fpga_rx_engine_test.cpp`

- [ ] **Step 1: Update the tracker test to model host timestamps as ticks, but still expect nanosecond output**

In `cpp_src/FPGA_boost_demo/tests/latency_tracker_test.cpp`, stop treating host-side test timestamps as already being nanoseconds. Introduce a local helper scale and construct host-side captured values as synthetic ticks:

```cpp
constexpr uint64_t kTicksPerNs = 10ULL;

const uint64_t batch_start_tick = 1000ULL * kTicksPerNs;
const uint64_t batch_end_tick = 1300ULL * kTicksPerNs;
const uint64_t strategy_start_tick = 1400ULL * kTicksPerNs;
const uint64_t tx_execution_accepted_tick = 1500ULL * kTicksPerNs;
const uint64_t tx_enqueue_tick = 1540ULL * kTicksPerNs;
const uint64_t tx_send_tick = 1550ULL * kTicksPerNs;
```

Keep the expected log block in nanoseconds:

```cpp
expected += formatSignedLine("batch_duration_ns", 300LL);
expected += formatSignedLine("batch_end -> strategy_start_ns", 100LL);
expected += formatSignedLine("strategy_start -> tx_execution_accepted_ns", 100LL);
expected += formatSignedLine("tx_execution_accepted -> tx_enqueue_ns", 40LL);
expected += formatSignedLine("tx_enqueue -> tx_send_ns", 10LL);
```

- [ ] **Step 2: Run the tracker test to verify it fails before conversion is added**

Run:

```bash
cmake --build cpp_src/build --target latency_tracker_test -j8
./cpp_src/build/latency_tracker_test
```

Expected: FAIL because `LatencyTracker` still subtracts host timestamps as if they were already nanoseconds.

- [ ] **Step 3: Change `PendingEventState` to store raw host ticks**

Modify `cpp_src/FPGA_boost_demo/latency/latency_tracker.h` so the host-side pending timestamps are explicitly host ticks rather than host nanoseconds. Update these members:

```cpp
struct PendingEventState {
    uint64_t frame_start_tick {0};
    uint64_t dma_emit_tick {0};
    uint64_t batch_start_tick {0};
    uint64_t batch_end_tick {0};
    uint64_t strategy_start_tick {0};
    uint64_t tx_execution_accepted_tick {0};
    uint64_t tx_enqueue_tick {0};
    uint64_t frame_start_to_dma_emit_ns {0};
    int64_t batch_duration_ns {0};
    int64_t batch_end_to_strategy_start_ns {0};
    int64_t strategy_start_to_tx_execution_accepted_ns {0};
    int64_t tx_execution_accepted_to_tx_enqueue_ns {0};
    int64_t tx_enqueue_to_tx_send_ns {0};
    bool has_dma_emit {false};
    bool has_batch_start {false};
    bool has_batch_end {false};
    bool has_strategy_start {false};
    bool has_tx_execution_accepted {false};
    bool has_tx_enqueue {false};
};
```

Also add helper declarations:

```cpp
static int64_t _readSignedDelta(uint64_t later_tick, uint64_t earlier_tick);
static int64_t _readSignedHostDeltaNs(uint64_t later_tick, uint64_t earlier_tick);
```

- [ ] **Step 4: Implement integer host tick-to-nanosecond conversion in `LatencyTracker`**

Modify `cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp` so every host-host derived latency uses the tick conversion helper instead of raw subtraction. The conversion helper should follow this shape:

```cpp
int64_t LatencyTracker::_readSignedHostDeltaNs(uint64_t later_tick, uint64_t earlier_tick) {
    const int64_t signed_tick_delta = _readSignedDelta(later_tick, earlier_tick);
    const uint64_t abs_tick_delta =
        (signed_tick_delta >= 0)
            ? static_cast<uint64_t>(signed_tick_delta)
            : static_cast<uint64_t>(-signed_tick_delta);

    const uint64_t tsc_hz = readHostTickScale().tsc_hz;
    if (tsc_hz == 0) {
        return signed_tick_delta;
    }

    const uint64_t delta_ns = (abs_tick_delta * 1000000000ULL) / tsc_hz;
    return (signed_tick_delta >= 0)
        ? static_cast<int64_t>(delta_ns)
        : -static_cast<int64_t>(delta_ns);
}
```

Use that helper at these existing derivation points:

```cpp
state.batch_duration_ns =
    _readSignedHostDeltaNs(record.time_captured, state.batch_start_tick);

state.batch_end_to_strategy_start_ns =
    _readSignedHostDeltaNs(record.time_captured, state.batch_end_tick);

state.strategy_start_to_tx_execution_accepted_ns =
    _readSignedHostDeltaNs(record.time_captured, state.strategy_start_tick);

state.tx_execution_accepted_to_tx_enqueue_ns =
    _readSignedHostDeltaNs(record.time_captured, state.tx_execution_accepted_tick);

state.tx_enqueue_to_tx_send_ns =
    _readSignedHostDeltaNs(record.time_captured, state.tx_enqueue_tick);
```

Do not change:

- `frame_start_to_dma_emit_ns`
- stage validation flow
- latency log field names
- printing behavior

- [ ] **Step 5: Rebuild and rerun the focused tracker tests**

Run:

```bash
cmake --build cpp_src/build --target latency_tracker_test log_printer_test -j8
./cpp_src/build/latency_tracker_test
./cpp_src/build/log_printer_test
```

Expected: PASS, with logged values still in nanoseconds.

- [ ] **Step 6: Verify the RX-engine tests still pass, or make the minimal required test-only adjustment**

Run:

```bash
cmake --build cpp_src/build --target fpga_rx_engine_test -j8
./cpp_src/build/fpga_rx_engine_test
```

Expected: PASS if the existing monotonic comparisons remain valid with raw TSC ticks.

If the test relies on old “nanosecond” semantics rather than simple monotonic ordering, make the smallest test-only change in `cpp_src/FPGA_boost_demo/tests/fpga_rx_engine_test.cpp` so it asserts:

```cpp
EXPECT_GT(third.time_captured, 0U);
EXPECT_GE(third.time_captured, before_ns);
EXPECT_LE(third.time_captured, after_ns);
```

and similarly for `BATCH_END`, without introducing any production-file changes outside the approved scope.

- [ ] **Step 7: Commit the tracker conversion change**

```bash
git add cpp_src/FPGA_boost_demo/latency/latency_tracker.h \
        cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp \
        cpp_src/FPGA_boost_demo/tests/latency_tracker_test.cpp \
        cpp_src/FPGA_boost_demo/tests/fpga_rx_engine_test.cpp
git commit -m "refactor: convert host latency ticks in tracker"
```

### Task 3: Run Focused Final Verification

**Files:**
- Modify: none

- [ ] **Step 1: Build all affected focused targets**

Run:

```bash
cmake --build cpp_src/build --target latency_tracker_test log_printer_test fpga_rx_engine_test Venturi -j8
```

Expected: all targets build successfully.

- [ ] **Step 2: Run the affected test executables**

Run:

```bash
./cpp_src/build/latency_tracker_test
./cpp_src/build/log_printer_test
./cpp_src/build/fpga_rx_engine_test
```

Expected: all tests pass.

- [ ] **Step 3: Inspect the final diff for scope compliance**

Run:

```bash
git diff --stat HEAD~2..HEAD
git diff --name-only HEAD~2..HEAD
```

Expected: only the approved production files and any required tests are changed.

- [ ] **Step 4: Record the final verification result**

Capture the exact commands and outcomes in the handoff note or final response:

```text
Built: latency_tracker_test, log_printer_test, fpga_rx_engine_test, Venturi
Ran: latency_tracker_test, log_printer_test, fpga_rx_engine_test
Outcome: all passed
```

- [ ] **Step 5: Commit only if verification required follow-up edits**

If Task 3 required no code changes, do not create an extra commit.

If a test-only fix was needed during verification:

```bash
git add <exact touched files>
git commit -m "test: align latency timing verification with tsc capture"
```
