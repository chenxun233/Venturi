# Log Printer ProcessLog Queues Design

Date: 2026-04-14

## Goal

Simplify the non-latency side of `LogPrinter` so it follows the same queue ownership pattern as `LatencyTracker`:

- keep `pushLatency(const LatencyLogRecord&)` as the dedicated latency path
- replace non-latency typed push APIs with one unified `pushProcessLog(const ProcessLog&)`
- remove the `ProducerSlot` and `LogProducer` abstraction
- use `std::vector<std::unique_ptr<SpscRingQueue<...>>>` directly inside `LogPrinter`

## Why This Change Is Needed

The current registration-based producer design works, but it adds an extra abstraction layer:

- `ProducerSlot`
- `LogProducer`
- registration lifecycle rules
- handle ownership rules separate from queue index routing

The user wants `LogPrinter` to be structurally closer to `LatencyTracker`, which already uses the preferred pattern:

- queue arrays stored directly as `std::vector<std::unique_ptr<SpscRingQueue<T>>>`
- explicit `queue_idx` carried in the pushed record
- no extra per-producer wrapper object

For the non-latency path, the user also wants producers to build the final sentence themselves so `LogPrinter` only prints text in real time instead of formatting multiple record types internally.

## Chosen Approach

Chosen approach:

- keep `pushLatency(const LatencyLogRecord&)` unchanged for latency logs only
- add a new `ProcessLog` transport record with:
  - `queue_idx`
  - fixed-size `log_sentence`
  - explicit `message_length`
- add `pushProcessLog(const ProcessLog&)`
- store process-log queues directly as:
  - `std::vector<std::unique_ptr<SpscRingQueue<ProcessLog>>> m_process_log_queues`
- make producer call sites build the final human-readable sentence before enqueue
- make `LogPrinter` drain latency queues and process-log queues and print them

Rejected alternatives:

- keep the registration-based `LogProducer` handle model
  - rejected because the user explicitly wants the simpler `LatencyTracker`-style queue pattern
- convert latency logs to `ProcessLog` too
  - rejected because the user explicitly wants `pushLatency(...)` left alone
- keep multiple typed non-latency APIs (`pushExecution`, `pushTxEvent`, `pushRegressionStatus`)
  - rejected because the user wants one unified non-latency push interface

## Data Types

### `ProcessLog`

`ProcessLog` should be added to `cpp_src/FPGA_boost_demo/common/shared_types.h`.

Required fields:

- `uint16_t queue_idx`
- `uint16_t message_length`
- fixed-size character storage for the sentence

Recommended shape:

- `std::array<char, kProcessLogSentenceCapacity> log_sentence`

This avoids dynamic allocation in producer paths and keeps enqueue behavior deterministic.

The sentence buffer should be treated as raw stored bytes with an explicit length, not as a required null-terminated C string.

## Architecture

### `LogPrinter`

`LogPrinter` should own two queue families:

- latency queues for typed `LatencyLogRecord`
- process-log queues for preformatted `ProcessLog`

Both queue families should use the same direct storage pattern already used by `LatencyTracker`:

- `std::vector<std::unique_ptr<SpscRingQueue<...>>>`

The non-latency path should not use:

- `ProducerSlot`
- `LogProducer`
- registration-time producer handles

`LogPrinter` becomes simpler:

- route by `queue_idx`
- drain queues
- print records

### Producer Responsibilities

The caller of `pushProcessLog(...)` is responsible for:

- choosing the correct `queue_idx`
- formatting the final human-readable message
- filling the fixed-size buffer
- setting `message_length`

`LogPrinter` should not interpret or format execution or TX records after this change.

That logic moves to the producer call sites such as:

- `Executor`
- `TxConnection`
- `TxSender`
- startup status publication in `Venturi.cpp`

## Queue Model

The queue model should match `LatencyTracker` as closely as practical.

Expected members in `LogPrinter`:

- `std::vector<std::unique_ptr<SpscRingQueue<LatencyLogRecord>>> m_latency_queues`
- `std::vector<std::unique_ptr<SpscRingQueue<ProcessLog>>> m_process_log_queues`
- queue count and round-robin state for draining

The push path should use the record's `queue_idx` to select the correct queue directly.

No handle registration phase is part of this design.

## API Shape

Public API after this redesign should include:

- `pushLatency(const LatencyLogRecord& record)`
- `pushProcessLog(const ProcessLog& record)`
- `start()`
- `stop()`
- `readDropCount()`

The old non-latency APIs should be removed:

- `pushExecution(...)`
- `pushTxEvent(...)`
- `pushRegressionStatus(...)`

## Data Flow

### Latency Path

The latency path remains unchanged in principle:

1. `LatencyTracker` assembles a `LatencyLogRecord`
2. `LatencyTracker` calls `pushLatency(...)`
3. `LogPrinter` enqueues by `record.que_idx`
4. the worker thread prints the typed latency line

### Non-Latency Path

The process-log path works like this:

1. a producer builds the final printable sentence
2. the producer fills a `ProcessLog`
3. the producer sets `queue_idx`
4. the producer calls `pushProcessLog(...)`
5. `LogPrinter` enqueues into `m_process_log_queues[queue_idx]`
6. the worker thread prints the stored sentence immediately

## Error Handling

Push remains best-effort:

- invalid `queue_idx` returns `false`
- full queue returns `false`
- drop accounting increments on rejected pushes
- producers must not block

For message construction:

- if the formatted sentence is longer than the fixed buffer, it should be truncated deterministically
- `message_length` must reflect the stored byte count

The printer should write exactly `message_length` bytes from `log_sentence` and then print a newline if the sentence format does not already include one.

## Scope

Files expected to change during implementation:

- `cpp_src/FPGA_boost_demo/common/shared_types.h`
- `cpp_src/FPGA_boost_demo/latency/log_printer.h`
- `cpp_src/FPGA_boost_demo/latency/log_printer.cpp`
- producer call sites that currently emit execution, TX, and regression logs
- startup wiring in `cpp_src/FPGA_boost_demo/app/Venturi.cpp`
- tests covering logger behavior

Files intentionally not redesigned in this change:

- latency computation logic in `LatencyTracker`
- `SpscRingQueue` semantics
- the human-facing latency line format

## Testing

Implementation verification should cover:

1. `pushLatency(...)` still works unchanged
2. `pushProcessLog(...)` routes correctly by `queue_idx`
3. process-log queue overflow increments drop accounting
4. overlong sentences are truncated deterministically
5. execution and TX producers still print the expected text after formatting moves out of `LogPrinter`
6. the worker thread drains both latency and process-log queues correctly

## Risks

### Caller Formatting Drift

Moving sentence formatting out of `LogPrinter` means each producer owns its printed string format.

Mitigation:

- keep formatting helpers small and local to each producer site
- update tests to lock in the expected printed strings

### Queue Index Misrouting

If a producer sends the wrong `queue_idx`, log ordering and ownership assumptions become wrong.

Mitigation:

- use existing queue-local context already available at call sites
- validate `queue_idx` in `pushProcessLog(...)`

### Hidden Complexity Reappearing

If the implementation recreates handle-style wrappers or extra slot structs, it misses the point of the redesign.

Mitigation:

- keep queue storage in the same direct vector-of-SPSC shape used by `LatencyTracker`

## Relationship To Earlier Spec

This design supersedes:

- `docs/superpowers/specs/2026-04-14-log-printer-registration-based-producer-design.md`

The earlier same-day design used registration-based producer handles and a `ProducerSlot` wrapper.

This design replaces that approach with:

- direct queue vectors
- explicit `queue_idx`
- a unified non-latency `ProcessLog` record

## Stop Condition

This design is complete when:

1. `pushLatency(...)` remains the dedicated latency interface
2. non-latency logs use `pushProcessLog(...)`
3. `ProcessLog` carries `queue_idx` and a fixed-size sentence buffer with explicit length
4. `LogPrinter` stores queues directly as vector-of-`SpscRingQueue` pointers
5. `ProducerSlot` and `LogProducer` are no longer part of the design
