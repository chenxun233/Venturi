# FPGARegression Remove Test-Only Reader APIs Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove the test-only `readSnapshot()` and `returnParaSnapshot()` APIs from `FPGARegression` while keeping runtime behavior unchanged and preserving regression coverage through behavior-based tests.

**Architecture:** Narrow the `FPGARegression` public interface to runtime-relevant operations only. Replace test assertions that inspect internal state with assertions on observable behavior through `isFrozen()`, `tryAcceptSnapshot(...)`, `readStatusLogRecord()`, and `convertFpgaToHostTime(...)`.

**Tech Stack:** C++20, GoogleTest, CMake

---

### Task 1: Remove the test-only public readers from FPGARegression

**Files:**
- Modify: `cpp_src/FPGA_boost_demo/sync/FPGA_regression.h`
- Modify: `cpp_src/FPGA_boost_demo/sync/FPGA_regression.cpp`

- [ ] **Step 1: Remove the declarations from the header**

Edit `cpp_src/FPGA_boost_demo/sync/FPGA_regression.h` and delete these two declarations:

```cpp
    FpgaSyncSnapshot readSnapshot() const;
    RegressionPara returnParaSnapshot() const;
```

The remaining public interface in that section should look like:

```cpp
    void run(CapSignal& cap_signal);
    bool tryAcceptSnapshot(const FpgaSyncSnapshot& snapshot, uint64_t accepted_interval_ns);
    void updateModel(const FpgaSyncSnapshot* snapshot);
    RegressionStatusLogRecord readStatusLogRecord() const;
    bool isFrozen() const;
    bool convertFpgaToHostTime(uint64_t fpga_tick, uint64_t& host_time_ns) const;
```

- [ ] **Step 2: Remove the two definitions from the implementation**

Edit `cpp_src/FPGA_boost_demo/sync/FPGA_regression.cpp` and delete these two functions:

```cpp
FpgaSyncSnapshot FPGARegression::readSnapshot() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    return m_latest_snapshot;
}

RegressionPara FPGARegression::returnParaSnapshot() const {
    std::lock_guard<std::mutex> lock(m_regression_mutex);
    return m_regression_para;
}
```

Do not change `readStatusLogRecord()`, `isFrozen()`, or `convertFpgaToHostTime(...)`.

- [ ] **Step 3: Build the regression test target to catch compile fallout**

Run: `cmake --build build --target regression_test -j4`

Expected: the build fails in `regression_test.cpp` because it still references the removed APIs.

- [ ] **Step 4: Commit the API shrink**

```bash
git add cpp_src/FPGA_boost_demo/sync/FPGA_regression.h cpp_src/FPGA_boost_demo/sync/FPGA_regression.cpp
git commit -m "refactor: drop FPGARegression test-only readers"
```

### Task 2: Rewrite regression tests to validate behavior instead of internals

**Files:**
- Modify: `cpp_src/FPGA_boost_demo/tests/regression_test.cpp`
- Test: `cpp_src/FPGA_boost_demo/tests/regression_test.cpp`

- [ ] **Step 1: Rewrite the slope-fit test to use status and conversion behavior**

In `cpp_src/FPGA_boost_demo/tests/regression_test.cpp`, replace the `returnParaSnapshot()` assertions in `RegressionTest.estimatesSlopeNearFpgaTickPeriod` with `readStatusLogRecord()` assertions.

Use this assertion block:

```cpp
    const RegressionStatusLogRecord status = regression.readStatusLogRecord();
    ASSERT_TRUE(status.has_para);
    EXPECT_TRUE(regression.isFrozen());
    EXPECT_NEAR(status.a_ns_per_tick, static_cast<double>(kSlopeNsPerTick), 5e-5);
```

Keep the existing `convertFpgaToHostTime(...)` assertion at the end of the test.

- [ ] **Step 2: Rewrite the frozen-update test to prove latest-snapshot behavior through conversion**

In `RegressionTest.keepsFrozenSlopeWhileRefreshingSnapshot`, remove all uses of `returnParaSnapshot()` and `readSnapshot()`.

Replace the post-freeze assertions with:

```cpp
    const RegressionStatusLogRecord status_before = regression.readStatusLogRecord();

    regression.updateModel(&refreshed_snapshot);

    const RegressionStatusLogRecord status_after = regression.readStatusLogRecord();
    EXPECT_DOUBLE_EQ(status_after.a_ns_per_tick, status_before.a_ns_per_tick);

    uint64_t converted_host_time_ns = 0;
    ASSERT_TRUE(regression.convertFpgaToHostTime(refreshed_snapshot.fpga_tick, converted_host_time_ns));
    EXPECT_EQ(converted_host_time_ns, refreshed_snapshot.host_time_ns);
```

This proves the slope stays frozen while the most recent accepted snapshot still anchors conversion.

- [ ] **Step 3: Rewrite the logging-status test to compute expected slope directly**

In `RegressionTest.exposesConvertedStatusForLogging`, remove `returnParaSnapshot()` and compare directly against the known slope:

```cpp
    const RegressionStatusLogRecord status = regression.readStatusLogRecord();

    EXPECT_TRUE(status.has_para);
    EXPECT_NEAR(status.a_ns_per_tick, 8.125, 5e-5);
```

Keep the test data unchanged so the expected slope remains `1625 / 200 = 8.125`.

- [ ] **Step 4: Rewrite the accepted-snapshot test to observe acceptance through conversion**

In `RegressionTest.tryAcceptSnapshotRejectsInvalidIntervalsAndAcceptsQualifiedOnes`, remove the final `readSnapshot()` assertions.

Replace them with:

```cpp
    uint64_t converted_host_time_ns = 0;
    ASSERT_TRUE(regression.convertFpgaToHostTime(accepted.fpga_tick, converted_host_time_ns));
    EXPECT_EQ(converted_host_time_ns, accepted.host_time_ns);
```

This proves the accepted snapshot became the active anchor without exposing internal state.

- [ ] **Step 5: Run the focused regression test**

Run: `./build/regression_test`

Expected: all `RegressionTest` cases pass.

- [ ] **Step 6: Commit the test rewrite**

```bash
git add cpp_src/FPGA_boost_demo/tests/regression_test.cpp
git commit -m "test: validate FPGARegression through behavior"
```

### Task 3: Final verification

**Files:**
- Verify: `cpp_src/FPGA_boost_demo/sync/FPGA_regression.h`
- Verify: `cpp_src/FPGA_boost_demo/sync/FPGA_regression.cpp`
- Verify: `cpp_src/FPGA_boost_demo/tests/regression_test.cpp`
- Verify: `cpp_src/FPGA_boost_demo/app/Venturi.cpp`

- [ ] **Step 1: Confirm the removed APIs have no remaining references**

Run: `rg -n "readSnapshot\\(|returnParaSnapshot\\(" cpp_src/FPGA_boost_demo`

Expected: no matches for `FPGARegression` call sites.

- [ ] **Step 2: Rebuild the runtime target that still uses readStatusLogRecord**

Run: `cmake --build build --target Venturi -j4`

Expected: successful build with no changes required in `Venturi.cpp`.

- [ ] **Step 3: Run the full focused verification set**

Run:

```bash
cmake --build build --target regression_test Venturi -j4
./build/regression_test
```

Expected:
- build succeeds
- all regression tests pass

- [ ] **Step 4: Commit the verified final state**

```bash
git add cpp_src/FPGA_boost_demo/sync/FPGA_regression.h cpp_src/FPGA_boost_demo/sync/FPGA_regression.cpp cpp_src/FPGA_boost_demo/tests/regression_test.cpp
git commit -m "refactor: remove FPGARegression test-only readers"
```
