# TX Send-Enter Latency Split Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the aggregate `tx_enqueue -> tx_send_ns` latency line with the split sender-tail latencies `tx_enqueue -> tx_send_enter_ns` and `tx_send_enter -> tx_send_ns`.

**Architecture:** Introduce one new sender-local latency stage, `TX_SEND_ENTER`, emitted inside `_sendPayload(...)` after the early guards and immediately before the send loop. Update the latency schema, tracker decode path, printed output, and focused tests so the sender tail is decoded as `TX_EXECUTION_ACCEPTED -> TX_ENQUEUE -> TX_SEND_ENTER -> TX_SEND`, with no unrelated telemetry changes.

**Tech Stack:** C++20, CMake, GoogleTest

---

## File Map

- Modify: `cpp_src/FPGA_boost_demo/common/shared_types.h`
  Add `TX_SEND_ENTER` to the `stage` enum, remove `tx_enqueue_to_tx_send_ns`, and add `tx_enqueue_to_tx_send_enter_ns` plus `tx_send_enter_to_tx_send_ns` to `LatencyLogRecord`.
- Modify: `cpp_src/FPGA_boost_demo/tx_engine/tx_sender.cpp`
  Emit `TX_SEND_ENTER` in `_sendPayload(...)` after the early validity checks and before the send loop.
- Modify: `cpp_src/FPGA_boost_demo/latency/latency_tracker.h`
  Store the `TX_SEND_ENTER` host tick in pending state and replace the old aggregate sender-tail field with the two split fields.
- Modify: `cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp`
  Decode `TX_ENQUEUE -> TX_SEND_ENTER -> TX_SEND`, derive the two split latencies, and remove the old aggregate sender-tail edge.
- Modify: `cpp_src/FPGA_boost_demo/latency/log_printer.cpp`
  Print the two split sender-tail lines and remove the old aggregate line.
- Modify: `cpp_src/FPGA_boost_demo/tests/log_printer_test.cpp`
  Update schema assertions and expected printed output to the two split sender-tail lines.
- Modify: `cpp_src/FPGA_boost_demo/tests/latency_tracker_test.cpp`
  Update the sender-tail test chain and expected output to use `TX_SEND_ENTER`.
- Modify: `cpp_src/FPGA_boost_demo/tests/tx_translator_test.cpp`
  Add focused assertions that `TxSender` emits `TX_SEND_ENTER` only after the `_sendPayload(...)` guards succeed and before `TX_SEND`.

## Guardrails

- Do not touch any latency boundary before `TX_ENQUEUE`.
- Do not modify CPU affinity, transport behavior, sender queue behavior, socket behavior, or unrelated telemetry.
- Do not retain the old aggregate `tx_enqueue -> tx_send_ns` line in the schema or printed output.
- If a file outside the file map appears to need changes, stop and treat that as out of scope.

### Task 1: Update The Latency Schema And Printed Output For The Split Sender Tail

**Files:**
- Modify: `cpp_src/FPGA_boost_demo/common/shared_types.h`
- Modify: `cpp_src/FPGA_boost_demo/latency/log_printer.cpp`
- Modify: `cpp_src/FPGA_boost_demo/tests/log_printer_test.cpp`

- [ ] **Step 1: Write the failing schema and print test updates**

Update `cpp_src/FPGA_boost_demo/tests/log_printer_test.cpp` so the sender-tail fields and expected output use the split lines:

```cpp
static_assert(std::is_same_v<
    decltype(LatencyLogRecord{}.tx_enqueue_to_tx_send_enter_ns),
    int64_t>);
static_assert(std::is_same_v<
    decltype(LatencyLogRecord{}.tx_send_enter_to_tx_send_ns),
    int64_t>);
```

Use this fixture:

```cpp
LatencyLogRecord record {
    .que_idx = 3,
    .event_tag = 42,
    .frame_start_to_dma_emit_ns = 17,
    .batch_duration_ns = -5,
    .batch_end_to_strategy_start_ns = -11,
    .strategy_start_to_tx_execution_accepted_ns = 13,
    .tx_execution_accepted_to_tx_enqueue_ns = -19,
    .tx_enqueue_to_tx_send_enter_ns = 23,
    .tx_send_enter_to_tx_send_ns = -29,
};
```

And this expected tail:

```cpp
expected += formatSignedLine("tx_execution_accepted -> tx_enqueue_ns", -19LL);
expected += formatSignedLine("tx_enqueue -> tx_send_enter_ns", 23LL);
expected += formatSignedLine("tx_send_enter -> tx_send_ns", -29LL);
```

Also assert the old line is absent:

```cpp
EXPECT_EQ(output.find("tx_enqueue -> tx_send_ns"), std::string::npos);
```

- [ ] **Step 2: Run the focused print test to verify it fails**

Run:

```bash
cmake --build cpp_src/build --target log_printer_test -j8
./cpp_src/build/log_printer_test
```

Expected: FAIL at compile time because the new fields do not exist yet and the printer still emits the old aggregate line.

- [ ] **Step 3: Update the stage enum and latency record schema**

Modify `cpp_src/FPGA_boost_demo/common/shared_types.h` so:

```cpp
enum stage {
    FRAME_START = 0,
    DMA_EMIT = 1,
    DECODE = 2,
    STRATEGY_START = 3,
    EXECUTOR = 4,
    EXECUTION_TAKEN = 5,
    TX_EXECUTION_ACCEPTED = 6,
    TX_EXECUTION_DEQUEUE = 7,
    TX_ORDER_FRAME_BUILT = 8,
    TX_PENDING_RECORDED = 9,
    TX_ENQUEUE = 10,
    TX_SEND_ENTER = 11,
    TX_SEND = 12,
    BATCH_START = 13,
    BATCH_END = 14
};
```

and `LatencyLogRecord` becomes:

```cpp
struct LatencyLogRecord {
    uint16_t    que_idx {0};
    uint64_t    event_tag {0};
    uint64_t    frame_start_to_dma_emit_ns {0};
    int64_t     batch_duration_ns {0};
    int64_t     batch_end_to_strategy_start_ns {0};
    int64_t     strategy_start_to_tx_execution_accepted_ns {0};
    int64_t     tx_execution_accepted_to_tx_enqueue_ns {0};
    int64_t     tx_enqueue_to_tx_send_enter_ns {0};
    int64_t     tx_send_enter_to_tx_send_ns {0};
};
```

- [ ] **Step 4: Update the printer to the two split sender-tail lines**

Modify `cpp_src/FPGA_boost_demo/latency/log_printer.cpp` so the negativity check and printed latency block use:

```cpp
const bool has_negative = (record.batch_duration_ns < 0) ||
                          (record.batch_end_to_strategy_start_ns < 0) ||
                          (record.strategy_start_to_tx_execution_accepted_ns < 0) ||
                          (record.tx_execution_accepted_to_tx_enqueue_ns < 0) ||
                          (record.tx_enqueue_to_tx_send_enter_ns < 0) ||
                          (record.tx_send_enter_to_tx_send_ns < 0);
```

and:

```cpp
printSignedLatency("tx_execution_accepted -> tx_enqueue_ns",
                   static_cast<long long>(record.tx_execution_accepted_to_tx_enqueue_ns));
printSignedLatency("tx_enqueue -> tx_send_enter_ns",
                   static_cast<long long>(record.tx_enqueue_to_tx_send_enter_ns));
printSignedLatency("tx_send_enter -> tx_send_ns",
                   static_cast<long long>(record.tx_send_enter_to_tx_send_ns));
```

Remove the old:

```cpp
printSignedLatency("tx_enqueue -> tx_send_ns",
                   static_cast<long long>(record.tx_enqueue_to_tx_send_ns));
```

- [ ] **Step 5: Rebuild and rerun the focused print test**

Run:

```bash
cmake --build cpp_src/build --target log_printer_test -j8
./cpp_src/build/log_printer_test
```

Expected: PASS.

- [ ] **Step 6: Commit the schema and printer split**

```bash
git add cpp_src/FPGA_boost_demo/common/shared_types.h \
        cpp_src/FPGA_boost_demo/latency/log_printer.cpp \
        cpp_src/FPGA_boost_demo/tests/log_printer_test.cpp
git commit -m "refactor: split tx send latency fields"
```

### Task 2: Emit `TX_SEND_ENTER` At The Send-Loop Boundary

**Files:**
- Modify: `cpp_src/FPGA_boost_demo/tx_engine/tx_sender.cpp`
- Modify: `cpp_src/FPGA_boost_demo/tests/tx_translator_test.cpp`

- [ ] **Step 1: Write the failing sender-emission test update**

Add or update a focused test in `cpp_src/FPGA_boost_demo/tests/tx_translator_test.cpp` so a successfully connected send path emits the sender-tail stages in this order:

```cpp
ASSERT_TRUE(tracker.m_latency_queues[1]->pop(record));
EXPECT_EQ(record.event_stage, stage::TX_EXECUTION_ACCEPTED);

ASSERT_TRUE(sender.buildOutboundFrames());
ASSERT_TRUE(sender.trySendOutbound(outbound));

std::vector<stage> seen;
while (tracker.m_latency_queues[1]->pop(record)) {
    seen.push_back(record.event_stage);
}

EXPECT_NE(std::find(seen.begin(), seen.end(), stage::TX_SEND_ENTER), seen.end());
EXPECT_NE(std::find(seen.begin(), seen.end(), stage::TX_SEND), seen.end());
EXPECT_LT(std::find(seen.begin(), seen.end(), stage::TX_ENQUEUE),
          std::find(seen.begin(), seen.end(), stage::TX_SEND_ENTER));
EXPECT_LT(std::find(seen.begin(), seen.end(), stage::TX_SEND_ENTER),
          std::find(seen.begin(), seen.end(), stage::TX_SEND));
```

Also add a guard-path assertion that failed `_sendPayload(...)` preconditions do not emit `TX_SEND_ENTER`.

- [ ] **Step 2: Run the focused sender test to verify it fails**

Run:

```bash
cmake --build cpp_src/build --target tx_translator_test -j8
./cpp_src/build/tx_translator_test
```

Expected: FAIL because `TX_SEND_ENTER` is not emitted yet.

- [ ] **Step 3: Emit `TX_SEND_ENTER` after the early guards and before the send loop**

Modify `cpp_src/FPGA_boost_demo/tx_engine/tx_sender.cpp` inside `_sendPayload(...)` so the new stage is emitted here:

```cpp
if (record.payload_length == 0 ||
    record.payload_length > record.payload.size() ||
    m_send_fd < 0) {
    return false;
}

if (record.event_tag != 0 && m_latency_tracker != nullptr) {
    try {
        m_latency_tracker->pushRecord(TimeRecord {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .event_stage = stage::TX_SEND_ENTER,
            .time_captured = readMonotonicRawNs(),
        });
    } catch (...) {
    }
}

std::size_t offset = 0;
while (offset < static_cast<std::size_t>(record.payload_length)) {
```

Do not move the existing `TX_SEND` emission.

- [ ] **Step 4: Rebuild and rerun the focused sender test**

Run:

```bash
cmake --build cpp_src/build --target tx_translator_test -j8
./cpp_src/build/tx_translator_test
```

Expected: PASS for the `TX_SEND_ENTER` emission behavior.

- [ ] **Step 5: Commit the `TX_SEND_ENTER` emission**

```bash
git add cpp_src/FPGA_boost_demo/tx_engine/tx_sender.cpp \
        cpp_src/FPGA_boost_demo/tests/tx_translator_test.cpp
git commit -m "refactor: emit tx send enter latency stage"
```

### Task 3: Split The Tracker Decode Path For The Sender Tail

**Files:**
- Modify: `cpp_src/FPGA_boost_demo/latency/latency_tracker.h`
- Modify: `cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp`
- Modify: `cpp_src/FPGA_boost_demo/tests/latency_tracker_test.cpp`

- [ ] **Step 1: Write the failing tracker test update for the split sender tail**

Update `cpp_src/FPGA_boost_demo/tests/latency_tracker_test.cpp` so the sender-tail record stream is:

```cpp
tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_EXECUTION_ACCEPTED,
                              tx_execution_accepted_tick));
tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_ENQUEUE, tx_enqueue_tick));
tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_SEND_ENTER, tx_send_enter_tick));
tracker.pushRecord(makeRecord(que_idx, event_tag, stage::TX_SEND, tx_send_tick));
```

Use split expected lines:

```cpp
expected += formatSignedLine("tx_execution_accepted -> tx_enqueue_ns", 40LL);
expected += formatSignedLine("tx_enqueue -> tx_send_enter_ns", 10LL);
expected += formatSignedLine("tx_send_enter -> tx_send_ns", 5LL);
```

And assert the old line is absent:

```cpp
EXPECT_EQ(output.find("tx_enqueue -> tx_send_ns"), std::string::npos);
```

- [ ] **Step 2: Run the focused tracker test to verify it fails**

Run:

```bash
cmake --build cpp_src/build --target latency_tracker_test -j8
./cpp_src/build/latency_tracker_test
```

Expected: FAIL because `LatencyTracker` still expects the old aggregate sender-tail edge.

- [ ] **Step 3: Update tracker pending state and stage handling**

Modify `cpp_src/FPGA_boost_demo/latency/latency_tracker.h` so `PendingEventState` stores:

```cpp
uint64_t tx_enqueue_tick {0};
uint64_t tx_send_enter_tick {0};
int64_t tx_enqueue_to_tx_send_enter_ns {0};
int64_t tx_send_enter_to_tx_send_ns {0};
```

Remove the old:

```cpp
int64_t tx_enqueue_to_tx_send_ns {0};
```

Add a declaration for:

```cpp
void _handleTxSendEnter(const TimeRecord& record, PendingIterator it);
```

- [ ] **Step 4: Implement the split sender-tail decode in `latency_tracker.cpp`**

Update the stage switch to handle:

```cpp
case stage::TX_SEND_ENTER:
    _handleTxSendEnter(record, it);
    return;
```

Implement the new handler:

```cpp
void LatencyTracker::_handleTxSendEnter(const TimeRecord& record, PendingIterator it) {
    PendingEventState& state = it->second;
    if (!state.has_tx_enqueue) {
        _incrementDrop(record.que_idx, stage::TX_ENQUEUE, stage::TX_SEND_ENTER);
        m_pending_records.erase(it);
        return;
    }

    state.tx_send_enter_tick = record.time_captured;
    state.tx_enqueue_to_tx_send_enter_ns =
        _readSignedHostDeltaNs(record.time_captured, state.tx_enqueue_tick);
    if (state.tx_enqueue_to_tx_send_enter_ns >= 0) {
        _updateStats(StageLatency {
            .que_idx = record.que_idx,
            .event_tag = record.event_tag,
            .prev_stage = stage::TX_ENQUEUE,
            .curr_stage = stage::TX_SEND_ENTER,
            .latency = static_cast<uint64_t>(state.tx_enqueue_to_tx_send_enter_ns)
        });
    }

    state.has_tx_send_enter = true;
}
```

Update `_handleTxSend(...)` to require `TX_SEND_ENTER` and derive:

```cpp
state.tx_send_enter_to_tx_send_ns =
    _readSignedHostDeltaNs(record.time_captured, state.tx_send_enter_tick);
```

and emit:

```cpp
.tx_enqueue_to_tx_send_enter_ns = state.tx_enqueue_to_tx_send_enter_ns,
.tx_send_enter_to_tx_send_ns = state.tx_send_enter_to_tx_send_ns,
```

Do not retain the old aggregate sender-tail field anywhere in the tracker.

- [ ] **Step 5: Rebuild and rerun the focused tracker tests**

Run:

```bash
cmake --build cpp_src/build --target latency_tracker_test log_printer_test -j8
./cpp_src/build/latency_tracker_test
./cpp_src/build/log_printer_test
```

Expected: PASS.

- [ ] **Step 6: Commit the tracker split**

```bash
git add cpp_src/FPGA_boost_demo/latency/latency_tracker.h \
        cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp \
        cpp_src/FPGA_boost_demo/tests/latency_tracker_test.cpp
git commit -m "refactor: split tx sender tail latency decode"
```

### Task 4: Run Focused Final Verification

**Files:**
- Modify: none

- [ ] **Step 1: Build all affected focused targets**

Run:

```bash
cmake --build cpp_src/build --target tx_translator_test latency_tracker_test log_printer_test Venturi -j8
```

Expected: all targets build successfully.

- [ ] **Step 2: Run the affected test executables**

Run:

```bash
./cpp_src/build/tx_translator_test
./cpp_src/build/latency_tracker_test
./cpp_src/build/log_printer_test
```

Expected: all tests pass.

- [ ] **Step 3: Inspect the final diff for scope compliance**

Run:

```bash
git diff --stat HEAD~3..HEAD
git diff --name-only HEAD~3..HEAD
```

Expected: only the approved files changed.

- [ ] **Step 4: Record the final verification result**

Capture the exact outcomes in the handoff note:

```text
Built: tx_translator_test, latency_tracker_test, log_printer_test, Venturi
Ran: tx_translator_test, latency_tracker_test, log_printer_test
Outcome: all passed
```

- [ ] **Step 5: Commit only if verification required follow-up edits**

If Task 4 required no code changes, do not create an extra commit.

If a final test-only fix was needed:

```bash
git add <exact touched files>
git commit -m "test: align tx send enter latency split verification"
```
