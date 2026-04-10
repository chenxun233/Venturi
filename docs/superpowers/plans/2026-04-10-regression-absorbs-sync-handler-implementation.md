# Regression Absorbs SyncHandler Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove `SyncHandler` and move its startup sync convergence and periodic capture trigger responsibilities into `Regression`, while simplifying `Venturi.cpp`.

**Architecture:** Keep `Regression` as the single owner of sync-for-regression policy: it should initialize sync, decide whether snapshots qualify, keep the latest accepted snapshot, and trigger periodic capture requests. `Venturi.cpp` should only orchestrate the runtime and stop carrying sync policy checks or a separate `SyncHandler` object.

**Tech Stack:** C++20, CMake, GoogleTest, existing FPGA driver interfaces

---

## File Map

**Core runtime**
- Modify: `cpp_src/FPGA_boost_demo/sync/regression.h`
  - Add trigger-period constructor, `initSync(...)`, `run(CapSignal&)`, `tryAcceptSnapshot(...)`, and private acceptance helper/state.
- Modify: `cpp_src/FPGA_boost_demo/sync/regression.cpp`
  - Implement the absorbed `SyncHandler` behavior using existing regression-owned snapshot/model state.
- Modify: `cpp_src/FPGA_boost_demo/app/Venturi.cpp`
  - Remove `SyncHandler` construction/use and replace inline snapshot qualification with `Regression` calls.

**Deletion**
- Delete: `cpp_src/FPGA_boost_demo/sync/sync_handler.h`
- Delete: `cpp_src/FPGA_boost_demo/sync/sync_handler.cpp`

**Build/test**
- Modify: `cpp_src/CMakeLists.txt`
  - Remove `sync_handler.cpp` from `Venturi`, remove `sync_handler_test`, and keep regression-focused tests wired.
- Modify: `cpp_src/FPGA_boost_demo/tests/regression_test.cpp`
  - Absorb sync-init and trigger-cadence coverage here.
- Delete: `cpp_src/FPGA_boost_demo/tests/sync_handler_test.cpp`

### Task 1: Move SyncHandler Behavior Into Regression

**Files:**
- Modify: `cpp_src/FPGA_boost_demo/sync/regression.h`
- Modify: `cpp_src/FPGA_boost_demo/sync/regression.cpp`
- Modify: `cpp_src/FPGA_boost_demo/tests/regression_test.cpp`

- [ ] **Step 1: Add regression API checks and failing sync-owned tests**

Update `cpp_src/FPGA_boost_demo/tests/regression_test.cpp` to require the new surface:

```cpp
static_assert(std::is_member_function_pointer_v<decltype(&Regression::run)>);
static_assert(std::is_member_function_pointer_v<decltype(&Regression::tryAcceptSnapshot)>);
```

Append two tests adapted from the current `sync_handler_test.cpp`:

```cpp
TEST(RegressionTest, initSyncStopsWhenRegressionFreezes) {
    SequencedSyncDevice device {};
    device.snapshots.reserve(32);
    for (uint64_t idx = 0; idx < 32; ++idx) {
        device.snapshots.push_back(FpgaSyncSnapshot {
            .fpga_tick = 1000 + idx * 200,
            .host_time_ns = 500000 + idx * 1250,
            .interval_ns = 1000
        });
    }

    Regression regression(64);

    EXPECT_TRUE(regression.initSync(device, 64, 2000));
    EXPECT_TRUE(regression.isFrozen());
    EXPECT_LT(device.read_count, 64U);
}

TEST(RegressionTest, runRequestsCaptureAtConfiguredCadence) {
    Regression regression(3);
    CapSignal capture_signal {};

    regression.run(capture_signal);
    EXPECT_TRUE(capture_signal.request.exchange(false));

    regression.run(capture_signal);
    EXPECT_FALSE(capture_signal.request.exchange(false));

    regression.run(capture_signal);
    EXPECT_FALSE(capture_signal.request.exchange(false));

    regression.run(capture_signal);
    EXPECT_TRUE(capture_signal.request.exchange(false));
}
```

- [ ] **Step 2: Run regression_test to confirm it fails before implementation**

Run: `cmake --build build --target regression_test -j4 && ./build/regression_test`

Expected: FAIL because `Regression::run`, `Regression::initSync`, and `Regression::tryAcceptSnapshot` do not exist yet.

- [ ] **Step 3: Extend Regression’s header with absorbed SyncHandler state and APIs**

Update `cpp_src/FPGA_boost_demo/sync/regression.h` so the class starts like:

```cpp
class Regression {
public:
    explicit Regression(uint64_t trigger_period = 0);

    template <typename DeviceT>
    bool initSync(DeviceT& device,
                  std::size_t max_attempts,
                  uint64_t accepted_interval_ns) {
        FpgaSyncSnapshot snapshot {};
        for (std::size_t attempt = 0; attempt < max_attempts; ++attempt) {
            if (!device.readSyncTimestamp(snapshot)) {
                continue;
            }
            (void)tryAcceptSnapshot(snapshot, accepted_interval_ns);
            if (isFrozen()) {
                return true;
            }
        }
        return false;
    }

    void run(CapSignal& cap_signal);
    bool tryAcceptSnapshot(const FpgaSyncSnapshot& snapshot, uint64_t accepted_interval_ns);
    void updateSnapshot(const FpgaSyncSnapshot* snapshot);
    FpgaSyncSnapshot readSnapshot() const;
    RegressionPara returnParaSnapshot() const;
    RegressionStatusLogRecord readStatusLogRecord() const;
    bool isFrozen() const;
    bool convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const;

private:
    bool _tryAcceptSnapshot(const FpgaSyncSnapshot& snapshot, uint64_t accepted_interval_ns);
    void _updateModel(const FpgaSyncSnapshot* snapshot);
    static uint64_t _scaleTicksToNs(uint64_t tick_delta, uint64_t a_q32);

    mutable std::mutex m_regression_mutex {};
    RegressionPara m_regression_para {};
    uint32_t m_snapshot_count {0};
    bool m_is_frozen {false};
    FpgaSyncSnapshot m_latest_snapshot {};
    long double m_sum_fpga_ticks {0.0L};
    long double m_sum_host_time_ns {0.0L};
    long double m_sum_fpga_ticks_sq {0.0L};
    long double m_sum_fpga_host_product {0.0L};
    long double m_last_a_ns_per_tick {0.0L};
    std::size_t m_stable_count {0};
    uint64_t m_trigger_period {0};
    uint64_t m_trigger_countdown {0};
};
```

- [ ] **Step 4: Implement the absorbed logic in regression.cpp**

Add the constructor and trigger/acceptance methods in `cpp_src/FPGA_boost_demo/sync/regression.cpp`:

```cpp
Regression::Regression(uint64_t trigger_period)
    : m_trigger_period(trigger_period),
      m_trigger_countdown(0) {}

void Regression::run(CapSignal& cap_signal) {
    if (m_trigger_period == 0) {
        return;
    }
    if (m_trigger_countdown == 0) {
        cap_signal.request.store(true, std::memory_order_release);
        m_trigger_countdown = m_trigger_period - 1;
        return;
    }
    --m_trigger_countdown;
}

bool Regression::tryAcceptSnapshot(const FpgaSyncSnapshot& snapshot,
                                   uint64_t accepted_interval_ns) {
    return _tryAcceptSnapshot(snapshot, accepted_interval_ns);
}

bool Regression::_tryAcceptSnapshot(const FpgaSyncSnapshot& snapshot,
                                    uint64_t accepted_interval_ns) {
    if (snapshot.interval_ns == 0 || snapshot.interval_ns > accepted_interval_ns) {
        return false;
    }
    updateSnapshot(&snapshot);
    return true;
}
```

Leave the existing regression math in `_updateModel(...)` unchanged.

- [ ] **Step 5: Re-run regression_test and commit the absorbed Regression behavior**

Run: `cmake --build build --target regression_test -j4 && ./build/regression_test`

Expected: PASS with all regression tests green, including absorbed sync-init and trigger-cadence coverage.

Commit:

```bash
git add cpp_src/FPGA_boost_demo/sync/regression.h \
        cpp_src/FPGA_boost_demo/sync/regression.cpp \
        cpp_src/FPGA_boost_demo/tests/regression_test.cpp
git commit -m "refactor: move sync handling into regression"
```

### Task 2: Remove SyncHandler From Venturi And Build/Test Wiring

**Files:**
- Modify: `cpp_src/FPGA_boost_demo/app/Venturi.cpp`
- Modify: `cpp_src/CMakeLists.txt`
- Delete: `cpp_src/FPGA_boost_demo/sync/sync_handler.h`
- Delete: `cpp_src/FPGA_boost_demo/sync/sync_handler.cpp`
- Delete: `cpp_src/FPGA_boost_demo/tests/sync_handler_test.cpp`

- [ ] **Step 1: Update Venturi to use Regression as the only sync owner**

In `cpp_src/FPGA_boost_demo/app/Venturi.cpp`:

1. Remove:

```cpp
#include "../sync/sync_handler.h"
```

2. Replace:

```cpp
SyncHandler sync_handler(kSnapshotSamplePeriod);
```

with:

```cpp
Regression regression(kSnapshotSamplePeriod);
```

3. Replace startup init:

```cpp
if (!sync_handler.initSync(device,
                           regression,
                           sync_snapshot,
                           kInitSyncMaxAttempts,
                           accepted_interval_ns)) {
```

with:

```cpp
if (!regression.initSync(device, kInitSyncMaxAttempts, accepted_interval_ns)) {
```

4. Replace control-thread trigger call:

```cpp
sync_handler.run(capture_signal);
```

with:

```cpp
regression.run(capture_signal);
```

5. Replace runtime snapshot qualification:

```cpp
if (get_snapshot && snapshot.interval_ns != 0) {
    regression.updateSnapshot(&snapshot);
}
```

with:

```cpp
if (get_snapshot) {
    (void)regression.tryAcceptSnapshot(snapshot, accepted_interval_ns);
}
```

- [ ] **Step 2: Remove SyncHandler from CMake and delete obsolete files**

In `cpp_src/CMakeLists.txt`:
- remove `${FPGA_SYNC_DIR}/sync_handler.cpp` from the `Venturi` target
- remove the entire `sync_handler_test` target
- remove `gtest_discover_tests(sync_handler_test)`

Then delete:

```text
cpp_src/FPGA_boost_demo/sync/sync_handler.h
cpp_src/FPGA_boost_demo/sync/sync_handler.cpp
cpp_src/FPGA_boost_demo/tests/sync_handler_test.cpp
```

- [ ] **Step 3: Build Venturi and the focused sync/regression tests**

Run:

```bash
cmake --build build --target Venturi regression_test -j4 && \
./build/regression_test
```

Expected: PASS, and `Venturi` links without `sync_handler.cpp`.

- [ ] **Step 4: Commit the Venturi/build cleanup**

```bash
git add cpp_src/FPGA_boost_demo/app/Venturi.cpp \
        cpp_src/CMakeLists.txt \
        cpp_src/FPGA_boost_demo/sync/regression.h \
        cpp_src/FPGA_boost_demo/sync/regression.cpp \
        cpp_src/FPGA_boost_demo/tests/regression_test.cpp \
        cpp_src/FPGA_boost_demo/sync/sync_handler.h \
        cpp_src/FPGA_boost_demo/sync/sync_handler.cpp \
        cpp_src/FPGA_boost_demo/tests/sync_handler_test.cpp
git commit -m "refactor: remove sync handler class"
```

### Task 3: Final Verification And Scope Check

**Files:**
- Verify: `cpp_src/FPGA_boost_demo/sync/regression.h`
- Verify: `cpp_src/FPGA_boost_demo/sync/regression.cpp`
- Verify: `cpp_src/FPGA_boost_demo/app/Venturi.cpp`
- Verify: `cpp_src/CMakeLists.txt`
- Verify: `cpp_src/FPGA_boost_demo/tests/regression_test.cpp`

- [ ] **Step 1: Run the focused test/build suite**

Run:

```bash
cmake --build build --target regression_test fpga_rx_engine_test latency_tracker_test Venturi -j4 && \
./build/regression_test && \
./build/fpga_rx_engine_test && \
./build/latency_tracker_test
```

Expected: PASS across the absorbed sync behavior and nearby runtime integrations.

- [ ] **Step 2: Verify SyncHandler is gone from the runtime and build graph**

Run:

```bash
rg -n "SyncHandler|sync_handler" cpp_src/FPGA_boost_demo/app/Venturi.cpp cpp_src/FPGA_boost_demo/sync cpp_src/CMakeLists.txt
```

Expected:
- no `SyncHandler` references in `Venturi.cpp`
- no `sync_handler.cpp` in `CMakeLists.txt`
- only zero results or historical mentions in unrelated docs/comments that are clearly non-runtime

- [ ] **Step 3: Review final diff scope**

Run:

```bash
git diff -- cpp_src/FPGA_boost_demo/app/Venturi.cpp \
           cpp_src/FPGA_boost_demo/sync/regression.h \
           cpp_src/FPGA_boost_demo/sync/regression.cpp \
           cpp_src/CMakeLists.txt \
           cpp_src/FPGA_boost_demo/tests/regression_test.cpp \
           cpp_src/FPGA_boost_demo/sync/sync_handler.h \
           cpp_src/FPGA_boost_demo/sync/sync_handler.cpp \
           cpp_src/FPGA_boost_demo/tests/sync_handler_test.cpp
```

Expected: the diff is limited to removing `SyncHandler`, broadening `Regression` to own sync-for-regression policy, simplifying `Venturi`, and replacing the tests accordingly.

- [ ] **Step 4: Commit any final cleanup**

If the verification steps required final cleanup, commit it with:

```bash
git add cpp_src/FPGA_boost_demo/app/Venturi.cpp \
        cpp_src/FPGA_boost_demo/sync/regression.h \
        cpp_src/FPGA_boost_demo/sync/regression.cpp \
        cpp_src/CMakeLists.txt \
        cpp_src/FPGA_boost_demo/tests/regression_test.cpp
git commit -m "test: verify regression-owned sync flow"
```
