# SFP 10GBASE-R Debug Notes

## Summary

Getting the 10GBASE-R link up on the Venturi board (Kintex UltraScale, xcku040)
required solving two independent problems:

1. **Corundum's custom TX/RX reset FSM** failed to properly initialize the GT transceiver
2. **Wrong RX polarity** prevented CDR lock on the incoming SFP signal

The final working design uses the **GT Wizard example design's reset infrastructure**
(example wrapper + init block) with Corundum's `eth_phy_10g` for 10GBASE-R encoding,
and correct polarity settings: `txpolarity = 1, rxpolarity = 0`.

---

## Bug 1: Corundum's Custom Reset FSM

### Symptom

ILA showed the RX reset state machine stuck in `WAIT_CDR` (state 2):
- `rxcdrlock = 0`
- `rx_prgdiv_reset_done = 0`
- `rx_block_lock = 0`
- `rx_reset_done = 0`
- TX path worked fine

### Root Cause

Corundum's `eth_xcvr_phy_10g_gty_wrapper.v` implements its own TX and RX reset
state machines that manually control `GTRXRESET`, `RXPROGDIVRESET`, `RXUSERRDY`,
and other GT reset signals. These FSMs are designed for Corundum's multi-channel
NIC architecture and make assumptions about clock domain crossings and timing
that don't hold in our single-channel configuration.

Specific issues found:

1. **`FREERUN_FREQUENCY` mismatch**: The GT Wizard IP was generated with
   `freerun_freq = 125` MHz, but the Venturi board provides a 100 MHz freerun
   clock. This caused internal timer miscalculations in the reset controller.
   Fixed by regenerating the IP with `freerun_freq = 100`.

2. **`gtwiz_userclk_rx_reset_in` feedback loop**: An attempt to align the FSM
   with the GT Wizard's sequence by changing `gtwiz_userclk_rx_reset_in` to
   `~(gt_rx_prgdiv_reset_done & gt_rx_pma_reset_done)` caused the RX user clock
   (`rxusrclk2`) to stop during `WAIT_CDR` (because `rxprogdivreset` was asserted).
   This froze the `rxusrclk2`-domain synchronizers with stale values, causing the
   FSM to prematurely rush through subsequent states.

3. **Clock domain crossing issues**: The FSM uses `drp_clk` (100 MHz freerun) but
   many status signals originate in `rxusrclk2` (~156.25 MHz). The synchronizer
   pipeline depth was insufficient, and when `rxusrclk2` stopped (during reset),
   the synchronized values went stale, leading to incorrect FSM transitions.

### Attempted Fixes (Did Not Fully Work)

- Bypassed `WAIT_CDR` state
- Tied `gtwiz_reset_rx_done_in` to `1'b1`
- Switched to LPM equalization (`GT_RX_LPM_EN = 1`)
- Added unconditional counters in `WAIT_USRCLK` and `WAIT_RESETDONE`
- Replaced Corundum FSMs with GT Wizard's `eth_xcvr_gth_full_example_gtwiz_reset`
  controller (still failed due to incorrect trigger signal routing)

### Why Corundum Doesn't Work Here

Corundum's reset FSM was designed for a specific board (ExaNIC X10/X25) with:
- Multiple GT channels sharing QPLLs
- A specific reset ordering between channels
- DRP-based dynamic reconfiguration of GT parameters
- External reset coordination from a PCIe control plane

In our single-channel standalone design without PCIe control, many of these
mechanisms don't trigger properly. The `xcvr_ctrl_rst` input, which Corundum
uses to initiate the reset sequence, is just a board-level reset that doesn't
follow Corundum's expected timing protocol.

### Final Fix

**Replaced the entire Corundum GT wrapper** with the GT Wizard example design's
proven infrastructure:
- `eth_xcvr_gth_full_example_wrapper` — contains the GT channel, QPLL, user
  clocking helpers, and the built-in reset controller
- `eth_xcvr_gth_full_example_init` — retry logic that re-resets on timeout
  or link loss
- `eth_phy_10g` — Corundum's 10GBASE-R PHY (encoding/decoding only, no GT
  management) connected to the wrapper's data and gearbox ports

---

## Bug 2: Wrong RX Polarity

### Symptom

After switching to the GT Wizard example infrastructure, the same stuck-CDR
symptom persisted: `gtwiz_reset_rx_cdr_stable = 0`, `rxprgdivresetdone = 0`.

However, enabling **near-end PMA loopback** (`loopback = 3'b010`) immediately
fixed everything — all signals went high, block lock achieved, `rx_status = 1`.

### Root Cause

The Venturi board's SFP channel 0 has:
- **TX polarity inverted** on the PCB (requires `txpolarity = 1`)
- **RX polarity NOT inverted** (requires `rxpolarity = 0`)

The initial configuration used `rxpolarity = 1` (copied from Corundum's ExaNIC
settings), which inverts the received bit stream. With inverted polarity, the
CDR cannot lock to the incoming 10GBASE-R signal because the bit transitions
don't match the expected pattern.

### Diagnosis Method

1. Set `loopback = 3'b010` (near-end PMA loopback) — bypasses the SFP entirely,
   GT receives its own TX data. **Result: all signals OK.** This proved the GT
   IP, reset controller, and PHY are all functional.

2. Set `loopback = 3'b000`, tried polarity combinations:
   - `tx=1, rx=1` — CDR fails (this was the original broken config)
   - `tx=1, rx=0` — **CDR locks, block lock, link up**
   - `tx=0, rx=0` — not tested (NIC wouldn't see our TX)
   - `tx=0, rx=1` — not tested

### Final Fix

```verilog
rxpolarity = 1'b0   // RX not inverted
txpolarity = 1'b1   // TX inverted (board trace routing)
```

---

## Working Configuration Summary

| Parameter | Value | Notes |
|-----------|-------|-------|
| `txpolarity` | `1'b1` | Board TX trace is inverted |
| `rxpolarity` | `1'b0` | Board RX trace is NOT inverted |
| `rxlpmen` | `1'b1` | LPM equalization mode |
| `loopback` | `3'b000` | Normal operation |
| `freerun_freq` | 100 MHz | Venturi board LVDS clock |
| GT refclk | 161.1328125 MHz | SFP reference clock |
| Line rate | 10.3125 Gbps | 10GBASE-R |
| Encoding | 64b/66b | With scrambling |
| `BIT_REVERSE` | `1` | Required for GT Wizard data ordering |

## Architecture (Working Design)

```
  Top.v
    └── SFP_wrapper.v
          ├── IBUFDS_GTE3 (refclk buffer)
          ├── eth_xcvr_gth_full_example_wrapper (GT Wizard IP)
          │     ├── GT channel (GTHE3_CHANNEL)
          │     ├── GT common (GTHE3_COMMON + QPLL)
          │     ├── User clocking helpers (BUFG_GT)
          │     └── Reset controller (gtwiz_reset)
          ├── eth_xcvr_gth_full_example_init (retry logic)
          ├── eth_phy_10g (Corundum 10GBASE-R PHY)
          │     ├── eth_phy_10g_tx (TX: XGMII → 64b/66b → scrambler)
          │     └── eth_phy_10g_rx (RX: descrambler → 64b/66b → XGMII)
          └── sync_reset (clock domain crossing for resets)
```

## Files

### GT Wizard Example (from Vivado IP, do not modify):
- `eth_xcvr_gth_full_example_wrapper.v`
- `eth_xcvr_gth_full_example_wrapper_functions.v`
- `eth_xcvr_gth_full_example_gtwiz_reset.v`
- `eth_xcvr_gth_full_example_init.v`
- `eth_xcvr_gth_full_example_bit_sync.v`
- `eth_xcvr_gth_full_example_reset_sync.v`
- `eth_xcvr_gth_full_example_reset_inv_sync.v`

### Corundum PHY (encoding/decoding only):
- `eth_phy_10g.v`, `eth_phy_10g_tx.v`, `eth_phy_10g_rx.v`
- `eth_phy_10g_tx_if.v`, `eth_phy_10g_rx_if.v`
- `eth_phy_10g_rx_frame_sync.v`, `eth_phy_10g_rx_ber_mon.v`, `eth_phy_10g_rx_watchdog.v`
- `xgmii_baser_enc_64.v`, `xgmii_baser_dec_64.v`
- `lfsr.v`, `sync_reset.v`

### Removed (no longer needed):
- `eth_xcvr_phy_10g_gty_wrapper.v` — Corundum's custom GT wrapper with broken reset FSM
