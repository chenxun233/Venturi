# Latency Log Dequeue Field Design

## Goal

Make the human-facing latency log output show only the dequeue interval the user wants:

- `EXECUTION_ACCEPTED -> EXECUTION_DEQUEUE`

The printed latency line should stop showing:

- `EXECUTOR -> EXECUTION_DEQUEUE`

## Why This Change Is Needed

The current latency printout shows both:

- `execution_accepted_to_execution_dequeue_ns`
- `executor_to_execution_dequeue_ns`

These are different intervals, but they are both dequeue-adjacent and easy to read as duplicates during runtime inspection.

For the current investigation, the user wants the printed output to represent only the sender-local dequeue wait after `TxSender::acceptExecution(...)`.

That means the human-facing output should keep the narrower interval and remove the broader aggregate from the printed line.

## Chosen Approach

Chosen approach:

- update the latency print format in `LogPrinter`
- keep printing `execution_accepted_to_execution_dequeue_ns`
- stop printing `executor_to_execution_dequeue_ns`
- keep internal tracking unchanged
- keep `LatencyLogRecord` unchanged for now

Rejected alternatives:

- remove `executor_to_execution_dequeue_ns` from `LatencyLogRecord`
  - rejected because the user asked to fix printing, not to change the tracked schema
- remove `executor_to_execution_dequeue_ns` from `LatencyTracker`
  - rejected because that is a broader telemetry change and not required to fix the confusing output

## Scope

Files expected to change:

- `cpp_src/FPGA_boost_demo/latency/log_printer.cpp`

Files intentionally left unchanged:

- `cpp_src/FPGA_boost_demo/latency/latency_tracker.cpp`
- `cpp_src/FPGA_boost_demo/common/shared_types.h`
- tests unrelated to log text

## Behavior After This Change

The latency log line should still include:

- `execution_accepted_to_execution_dequeue_ns`

The latency log line should no longer include:

- `executor_to_execution_dequeue_ns`

No stage timing capture changes are part of this design.

No runtime threading changes are part of this design.

No latency aggregation changes are part of this design.

## Testing

Verification should confirm:

1. latency log output still prints successfully
2. `execution_accepted_to_execution_dequeue_ns` remains present
3. `executor_to_execution_dequeue_ns` is absent from the printed latency line
4. existing latency tracking continues to compile and run

If there is an existing log-printer or latency output test that checks the printed field list, update it to match the narrowed output.

## Risks

Main risk:

- a test may currently assert the old printed field list

Mitigation:

- update only the output expectation, not the underlying latency computation

## Stop Condition

This change is complete when:

1. the printed latency line includes `execution_accepted_to_execution_dequeue_ns`
2. the printed latency line no longer includes `executor_to_execution_dequeue_ns`
3. the rest of the latency line remains intact
