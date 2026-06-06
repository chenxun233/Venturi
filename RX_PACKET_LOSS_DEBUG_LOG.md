# RX Packet Loss Debug Log

This file records temporary diagnostics and targeted RTL reversions added while
debugging the `total_received` / packet-count mismatch. Keep it updated until
the issue is fixed, then use the cleanup checklist below to remove the debug
surface deliberately.

## Wrap-Up

Conclusion:

- Repeated long runs with `scripts/port_isolation.sh enp6s0f1` showed no packet loss across hundreds of thousands of packets.
- The packet-count mismatch was caused by non-isolated replay/emitter port traffic, not by parser logic or the downstream builder/CDC/DMA/host path.

Cleanup performed:

- Removed parser `o_dbg_*` outputs and last-bad capture registers.
- Removed builder accept/drop/done/noevent debug ports.
- Removed event CDC write/drop debug wires.
- Removed DMA-stage temporary debug counters.
- Removed debug BAR0 registers and host-side `readRxDebug*` methods.
- Removed `printRxDebugSummary()` from the app shutdown path.
- Removed `rx_debug_counters.v`, `rx_debug_counter_bridge.v`, and `tb_rx_debug_counters.v`.
- Removed debug counter modules from `Vivado/Venturi.xpr`.

Kept:

- `bus_stable_synchronizer.v` and its use for coherent stock-locate/price-base config CDC.
- Runtime BRAM scrub in `bram.v` and `bram_dp.v`.
- Vivado project source-list cleanup for stale missing files.

Current operational requirement:

- Run `scripts/port_isolation.sh enp6s0f1` before packet-count benchmark/replay runs.

## Current Evidence

Latest useful run showed:

```text
pcs_frames=108005 mac_frames=108005 msgs=107999
parser_zero=2 parser_unknown=3 parser_error=3 parser_unmatched=0
q0_parser=53999 ... q0_dma_send=53999 q0_event=53999 total_received=53999
q1_parser=54000 ... q1_dma_send=54000 q1_event=54000 total_received=54000
```

Conclusion from that run:

- Hardware path after parser is clean for both queues.
- Queue 0 is missing one record before `o_msg_valid`.
- `parser_unmatched=0`, so the miss was not observed as a stock-locate routing mismatch.
- Extra/noise frames exist and are currently counted by `parser_zero`, `parser_unknown`, and `parser_error`.

## Functional/RTL Changes To Keep Or Reassess

### `verilog_src/async_fifo.v`

- Reverted async FIFO design to the older implementation from the known-good frame-loss era.
- Purpose: remove later async FIFO redesign as a possible loss source.
- Status from counters: not the current loss source; downstream counts match once parser emits an event.
- Cleanup decision: keep only if this older FIFO is desired; otherwise restore newer FIFO after packet-loss root cause is fixed and retest.

### BRAM scrub rollback

Files:

- `verilog_src/order_book_related/dependencies/book_builder.v`
- `verilog_src/order_book_related/dependencies/bram.v`
- `verilog_src/order_book_related/dependencies/bram_dp.v`
- `verilog_src/order_book_related/dependencies/qty_builder.v`
- `verilog_src/order_book_related/dependencies/qty_book_wrapper.v`

Change:

- Reverted BRAM scrub / `*_init_done` gating added by commit `aba5baa`.
- That gating could drop parser messages before order-book memories finished scrubbing.

Status from counters:

- Current run shows no builder loss after parser, but this rollback removed a real startup-drop risk.

Cleanup decision:

- If BRAM scrub is needed long term, reintroduce it with upstream RX gating/backpressure instead of silently blocking `ff_push`.

### `verilog_src/bus_stable_synchronizer.v`

- Added coherent stable-bus CDC helper.
- Used for host-configured stock-locate and price-base buses crossing from `user_clk_250` to `w_xgmii_rx_clk`.

Reason:

- Previous `bit_synchronizer` was bitwise and unsafe for multi-bit config buses.

Cleanup decision:

- This is a real fix candidate, not just debug instrumentation. Keep unless replaced by a proper handshake-based config CDC.

### `verilog_src/Top.v`

Functional fix:

- Replaced stock-locate and price-base `bit_synchronizer` instances with `bus_stable_synchronizer`.

Temporary diagnostics:

- Wires parser debug pulses/counters into `rx_debug_counters`.
- Wires per-symbol builder debug pulses.
- Counts event CDC FIFO write/drop.
- Wires DMA-stage counters.
- Crosses parser last-bad snapshot to `user_clk_250`.

Cleanup decision:

- Keep the config CDC fix.
- Remove debug-counter wiring after root cause is fixed.

## Temporary Parser Diagnostics

### `verilog_src/order_book_related/order_book_parser.v`

Added debug outputs:

- `o_dbg_zero_msg_count`
- `o_dbg_unknown_msg_type`
- `o_dbg_parse_error`
- `o_dbg_last_bad_reason`
- `o_dbg_last_bad_seq_num`
- `o_dbg_last_bad_frame_ts`
- `o_dbg_last_bad_msg_count`
- `o_dbg_last_bad_msg_len`
- `o_dbg_last_bad_msg_type`
- `o_dbg_last_bad_stock_locate`
- `o_dbg_last_bad_buffered_bytes`
- `o_dbg_last_bad_valid_msg_ptr`

Reason mapping:

```text
1 = zero message count
2 = unknown message type
3 = parser error state
```

Purpose:

- Identify exactly which rejected packet/message caused `msgs` to fall below expected.

Cleanup decision:

- Remove all `o_dbg_*` ports and capture registers after parser root cause is found.

## Temporary Boundary Counters

### `verilog_src/debug_related/rx_debug_counters.v`

Added counters for:

- parser zero/unknown/error
- parser per-symbol match and unmatched
- builder FIFO accept/drop
- builder op done/noevent
- event CDC FIFO write/drop
- final event count remains

Cleanup decision:

- Remove extra debug counters after root cause is fixed, or keep a small permanent subset if useful.

### `verilog_src/debug_related/rx_debug_counter_bridge.v`

Added gray-code CDC for the extra RX-domain counters.

Cleanup decision:

- Remove corresponding bridge paths when removing debug counters.

### `verilog_src/DMA_related/rx_dma_stage.v`

Added user-clock counters:

- event pop count
- event valid count
- DMA send count
- RQ stall count

Cleanup decision:

- Remove if only needed for this debug session.

### `verilog_src/DMA_related/rx_dma_config.v`

Added BAR0 register readout for:

- parser counters and last-bad snapshot
- builder counters
- event CDC counters
- DMA counters

Important offsets:

```text
0x200 parser_zero
0x208 parser_unknown
0x210 parser_error
0x220 builder_accept base
0x230 builder_drop base
0x240 builder_done base
0x250 builder_noevent base
0x260 parser_symbol base
0x270 parser_unmatched
0x280 cdc_wr base
0x290 cdc_drop base
0x2a0 dma_pop base
0x2b0 dma_valid base
0x2c0 dma_send base
0x2d0 dma_stall base
0x2e0 parser_last_bad_info0
0x2e8 parser_last_bad_seq
0x2f0 parser_last_bad_info1
```

Cleanup decision:

- Remove temporary BAR0 registers after debug.

## Temporary Host Diagnostics

### `host_src/src/fpga_dev/fpga_dev.h`

Added read methods and BAR0 offsets for RX debug counters/snapshot.

Cleanup decision:

- Remove methods and constants matching removed BAR0 registers.

### `host_src/src/fpga_dev/fpga_dev.cpp`

Implemented the added debug read methods.

Cleanup decision:

- Remove implementations matching removed declarations.

### `host_src/src/app/venturi.cpp`

Added `printRxDebugSummary()` fields for all debug counters and the decoded parser last-bad snapshot.

Cleanup decision:

- Remove or shrink to the permanent counters only.

## Testbench Updates

### `verilog_tb/tb_rx_debug_counters.v`

Updated to lint/check the expanded debug counter and BAR0 readout path, including parser last-bad packing.

Cleanup decision:

- Remove debug-specific expectations when debug registers are removed.

## Vivado Project Source List Fix

### `Vivado/Venturi.xpr`

Added `verilog_src/bus_stable_synchronizer.v` to the main Vivado source set after adding the config CDC synchronizer module in RTL.

Removed the stale simulation source reference to missing `verilog_src/frame_timestamp.v`; the project already lists the real `verilog_src/timestamper.v`.

Fixed a stale simulation source path from `verilog_src/pcs_pma_related/dependencies/bit_synchronizer.v` to the real `verilog_src/bit_synchronizer.v`.

Cleanup decision:

- Keep the `bus_stable_synchronizer.v` project entry if the CDC fix is kept.
- Keep the stale source-list cleanup regardless; it fixes broken Vivado project metadata, not temporary debug logic.

## Runtime BRAM Scrub Reintroduced

### `verilog_src/order_book_related/dependencies/bram.v`

Added an internal post-reset scrub that writes zero to every single-port BRAM address before normal read/write service resumes.

Reason:

- Hardware run showed `builder_done` and `builder_noevent` were equal while downstream event/CDC/DMA counters stayed at zero.
- This matches quantity/order BRAM contents powering up nonzero: Add operations become nonzero-to-nonzero, so no empty-to-nonempty price-change pulse is generated and the best-price tree never activates.

Cleanup decision:

- Keep a BRAM initialization mechanism.
- Do not reintroduce the old builder-side drop gate unless a separate readiness/backpressure path is added.

### `verilog_src/order_book_related/dependencies/bram_dp.v`

Added the same internal post-reset scrub for dual-port quantity BRAM.

Reason:

- Quantity BRAM contents directly drive `qty_cur`; if `qty_cur` is nonzero at first Add, `qty_builder` does not emit `NON_EMPTY`, so the tree remains empty and `o_event_found` stays low.

Cleanup decision:

- Keep a BRAM initialization mechanism.
- If startup traffic can arrive during the scrub window, add explicit upstream readiness/backpressure instead of silently dropping parser messages.

Verification note:

- Focused Verilator lint passes for `bram.v`, `bram_dp.v`, and the `order_book_builder` dependency set.
- `tb_mac_order_book_builder.v` starts traffic four RX-clock cycles after reset, so it needs an added wait before it is useful for validating the scrubbed BRAM path.

## Known Dirty Files Not Part Of This Debug Surface

These appeared dirty in the working tree and should not be blindly reverted as part of debug cleanup:

- `.gitignore`
- `host_src/third_party/google_test`
- `verilog_backup/async_fifo.v`
- `verilog_tb/tb_MAC_layer_rx.v`

## Cleanup Checklist After Root Cause Is Fixed

1. Decide whether to keep `bus_stable_synchronizer.v` as a real CDC fix.
2. Decide whether to keep old `async_fifo.v` or restore the newer FIFO and retest.
3. Keep BRAM initialization; add explicit upstream readiness/backpressure only if traffic can arrive during the scrub window.
4. Remove parser `o_dbg_*` ports and last-bad registers.
5. Remove temporary fields from `rx_debug_counters.v` and `rx_debug_counter_bridge.v`.
6. Remove temporary DMA counters from `rx_dma_stage.v`.
7. Remove temporary BAR0 registers from `rx_dma_config.v`.
8. Remove temporary FPGADev read methods/constants.
9. Shrink or remove `printRxDebugSummary()` debug output.
10. Update `tb_rx_debug_counters.v` to match the final permanent register map.
11. Run host build and focused RTL lint.
12. Rerun packet-count test and confirm both queues reach expected totals.
