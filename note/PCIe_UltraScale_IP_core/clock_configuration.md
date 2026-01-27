# PCIe Clock Configuration

## Overview

The Xilinx UltraScale PCIe IP requires two clock inputs from the same 100 MHz PCIe reference clock source:

---

## Why Two Clocks?

The two clocks serve **fundamentally different purposes** in the FPGA:

```
                    ┌─────────────────────────────────────────┐
                    │           FPGA Chip                     │
                    │                                         │
                    │  ┌─────────────┐    ┌────────────────┐  │
                    │  │ GT          │    │ FPGA Fabric    │  │
   sys_clk_gt ─────────► Transceiver │    │ (Your Logic)   │  │
   (100 MHz)        │  │             │    │                │  │
                    │  │ SerDes PLL  │    │ CLB, BRAM, DSP │  │
                    │  │ 8 GHz       │    │                │  │
                    │  └──────┬──────┘    └───────▲────────┘  │
                    │         │                   │           │
                    │         │   user_clk (250 MHz)          │
                    │         └───────────────────┘           │
                    │                                         │
   sys_clk ─────────────► Core Logic (clock buffers, PLLs)    │
   (50 MHz)         │                                         │
                    └─────────────────────────────────────────┘
```

### Reason 1: GT Transceiver Jitter Requirements

The GT transceiver generates **8 GHz SerDes clocks** from the 100 MHz reference:

```
sys_clk_gt (100 MHz) → GT PLL → 8 GHz SerDes clock
                               ↓
                         PCIe lane @ 8 GT/s
```

**Strict jitter requirement:** <1 ps RMS for reliable data recovery at 8 Gbps. Any buffering (BUFG, MMCM) adds jitter, so `sys_clk_gt` must be **direct from IBUFDS_GTE4.O**.

### Reason 2: Dedicated Routing Resources

```
sys_clk_gt path:
  IBUFDS_GTE4.O → Dedicated GT clock network → GT REFCLK pin
  (No fabric routing, minimal jitter)

sys_clk path:
  IBUFDS_GTE4.ODIV2 → BUFG_GT → Global clock network → Fabric FFs
  (Buffered, can drive thousands of flip-flops)
```

GT transceivers have **physically separate clock input pins** that only connect to specific clock buffers.

### Reason 3: Different Clock Networks

| Network | Drives | Jitter Tolerance |
|---------|--------|------------------|
| GT Reference Clock | GT transceiver PLL | Very low (<1 ps) |
| Global Clock (BUFG) | FPGA fabric logic | Higher (~50 ps OK) |

You **cannot** use a BUFG output to drive a GT reference clock - the routing doesn't exist.

### Reason 4: Frequency Flexibility

```
sys_clk_gt: Fixed at 100 MHz (PCIe spec requirement)
sys_clk:    Can be 50 MHz, 100 MHz, or custom (more flexible)
```

The core logic clock can be derived differently without affecting the GT reference.

### Summary: Why Not One Clock?

| Clock | Why Needed |
|-------|------------|
| `sys_clk_gt` | GT SerDes needs low-jitter reference for 8 GHz PLL |
| `sys_clk` | FPGA fabric needs buffered clock for internal logic |

**You can't use one clock for both because:**
1. GT requires unbuffered, low-jitter clock on dedicated pins
2. FPGA fabric requires buffered clock on global clock network
3. They are physically different clock distribution networks in the chip

## Clock Ports

| Port | Frequency | Purpose |
|------|-----------|---------|
| `sys_clk_gt` | 100 MHz | GT transceiver reference (feeds internal 8 GHz PLL) |
| `sys_clk` | 50-100 MHz | Core logic clock (internal buffering) |
| `user_clk` (output) | 250 MHz | Your logic runs on this |

---

## Clock Generation

### Standard Configuration (IBUFDS_GTE4)

```verilog
IBUFDS_GTE4 refclk_ibuf (
    .I     (sys_clk_p),      // 100 MHz differential from PCIe slot
    .IB    (sys_clk_n),
    .O     (sys_clk_gt),     // 100 MHz → GT reference (MUST use this output)
    .ODIV2 (sys_clk),        // 50 MHz  → Core logic (divided by 2)
    .CEB   (1'b0)            // Clock enable (active low)
);
```

### Clock Relationship

```
PCIe Slot (100 MHz differential)
         │
         ▼
┌─────────────────┐
│   IBUFDS_GTE4   │
├─────────────────┤
│  O ────────────────► sys_clk_gt (100 MHz) → GT Transceiver
│  ODIV2 ────────────► sys_clk (50 MHz)     → Core Logic
└─────────────────┘
```

**The 2:1 ratio (sys_clk_gt = 2× sys_clk) is built into the IBUFDS_GTE4 primitive.**

---

## Constraints

### sys_clk_gt (Strict)

- **MUST** come from `IBUFDS_GTE4.O` (or `IBUFDS_GTE3` for 7-series)
- **Cannot** use MMCM, PLL, BUFG, or any other primitive
- Dedicated routing path to GT transceiver REFCLK input
- Must match IP configuration (100 MHz for Gen3)

### sys_clk (Flexible)

Can be generated from:

| Source | Frequency | Notes |
|--------|-----------|-------|
| `IBUFDS_GTE4.ODIV2` | 50 MHz | Default, simplest |
| `IBUFDS_GTE4.O` → `BUFG_GT` | 100 MHz | Same as sys_clk_gt |
| `IBUFDS_GTE4.O` → `MMCM` | Any | Advanced, for custom frequencies |

### user_clk (Output from IP)

- Generated internally by PCIe IP
- Frequency depends on interface width and link speed:
  - Gen3 x8, 256-bit: 250 MHz
  - Gen3 x4, 128-bit: 250 MHz
  - Gen2 x8, 256-bit: 125 MHz

---

## Alternative Configurations

### Option 1: Both Clocks at 100 MHz

```verilog
IBUFDS_GTE4 refclk_ibuf (
    .I     (sys_clk_p),
    .IB    (sys_clk_n),
    .O     (sys_clk_gt),     // 100 MHz
    .ODIV2 (sys_clk_odiv2),  // 50 MHz (unused)
    .CEB   (1'b0)
);

BUFG_GT bufg_gt_inst (
    .I       (sys_clk_gt),
    .O       (sys_clk),      // 100 MHz (buffered)
    .CE      (1'b1),
    .CEMASK  (1'b0),
    .CLR     (1'b0),
    .CLRMASK (1'b0),
    .DIV     (3'b000)        // No division
);
```

### Option 2: Custom sys_clk via MMCM

```verilog
// sys_clk_gt directly from IBUFDS_GTE4
IBUFDS_GTE4 refclk_ibuf (
    .O     (sys_clk_gt),     // 100 MHz to GT
    .ODIV2 (refclk_div2),    // 50 MHz to MMCM input
    ...
);

// Generate custom sys_clk
MMCME4_BASE mmcm_inst (
    .CLKIN1  (refclk_div2),  // 50 MHz in
    .CLKOUT0 (sys_clk),      // Custom frequency out
    ...
);
```

---

## IP Configuration (Vivado)

When configuring PCIe IP in Vivado:

| Setting | Value | Notes |
|---------|-------|-------|
| Reference Clock Frequency | 100 MHz | Must match sys_clk_gt |
| AXI-ST Alignment Mode | DWORD Aligned | Matches parser/formatter design |
| System reset polarity | ACTIVE LOW | Standard for PCIe |

**The IP does not have a setting for sys_clk frequency** - it accepts whatever you provide and handles it internally.

---

## Frequency Summary

```
PCIe Slot Reference Clock: 100 MHz
         │
         ├──► sys_clk_gt: 100 MHz (to GT transceiver)
         │         │
         │         └──► Internal GT PLL: 8 GHz (Gen3 line rate)
         │
         └──► sys_clk: 50 MHz or 100 MHz (to core logic)
                   │
                   └──► Internal PLL: 250 MHz (user_clk output)
```

---

## Common Mistakes

1. **Using BUFG instead of BUFG_GT for sys_clk**
   - BUFG cannot drive GT-related clock networks
   - Use BUFG_GT for clocks related to GT transceivers

2. **Generating sys_clk_gt from MMCM**
   - GT reference must come directly from IBUFDS_GTE4.O
   - No other clock source is valid

3. **Mismatched reference clock frequency**
   - IP expects 100 MHz (configured value)
   - Using 125 MHz or other frequencies will cause link training failure

4. **Forgetting clock domain crossing**
   - Your logic runs on `user_clk` (250 MHz)
   - If interfacing with other clock domains, use proper CDC

---

## Reference

- Xilinx PG156: UltraScale Devices Gen3 Integrated Block for PCI Express v4.4
  - Chapter 4: Clocking
- Xilinx UG576: UltraScale GTH/GTY Transceivers User Guide
  - Reference Clock Requirements
