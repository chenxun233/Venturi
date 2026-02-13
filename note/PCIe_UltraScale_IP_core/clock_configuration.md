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
   pcie_clk_100 ─────────► Transceiver │    │ (Your Logic)   │  │
   (100 MHz)        │  │             │    │                │  │
                    │  │ SerDes PLL  │    │ CLB, BRAM, DSP │  │
                    │  │ 8 GHz       │    │                │  │
                    │  └──────┬──────┘    └───────▲────────┘  │
                    │         │                   │           │
                    │         │   user_clk_250 (250 MHz)          │
                    │         └───────────────────┘           │
                    │                                         │
   pcie_clk_50 ─────────────► Core Logic (clock buffers, PLLs)    │
   (50 MHz)         │                                         │
                    └─────────────────────────────────────────┘
```

### Reason 1: GT Transceiver Jitter Requirements

The GT transceiver generates **8 GHz SerDes clocks** from the 100 MHz reference:

```
pcie_clk_100 (100 MHz) → GT PLL → 8 GHz SerDes clock
                               ↓
                         PCIe lane @ 8 GT/s
```

**Strict jitter requirement:** <1 ps RMS for reliable data recovery at 8 Gbps. Any buffering (BUFG, MMCM) adds jitter, so `pcie_clk_100` must be **direct from IBUFDS_GTE4.O**.

### Reason 2: Dedicated Routing Resources

```
pcie_clk_100 path:
  IBUFDS_GTE4.O → Dedicated GT clock network → GT REFCLK pin
  (No fabric routing, minimal jitter)

pcie_clk_50 path:
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
pcie_clk_100: Fixed at 100 MHz (PCIe spec requirement)
pcie_clk_50:    Can be 50 MHz, 100 MHz, or custom (more flexible)
```

The core logic clock can be derived differently without affecting the GT reference.

### Summary: Why Not One Clock?

| Clock | Why Needed |
|-------|------------|
| `pcie_clk_100` | GT SerDes needs low-jitter reference for 8 GHz PLL |
| `pcie_clk_50` | FPGA fabric needs buffered clock for internal logic |

**You can't use one clock for both because:**
1. GT requires unbuffered, low-jitter clock on dedicated pins
2. FPGA fabric requires buffered clock on global clock network
3. They are physically different clock distribution networks in the chip

## Clock Ports

| Port | Frequency | Purpose |
|------|-----------|---------|
| `pcie_clk_100` | 100 MHz | GT transceiver reference (feeds internal 8 GHz PLL) |
| `pcie_clk_50` | 50-100 MHz | Core logic clock (internal buffering) |
| `user_clk_250` (output) | 250 MHz | Your logic runs on this |

---

## Clock Generation

### Standard Configuration (IBUFDS_GTE4)

```verilog
IBUFDS_GTE4 refclk_ibuf (
    .I     (i_host_clk_100_p),      // 100 MHz differential from PCIe slot
    .IB    (i_host_clk_100_n),
    .O     (pcie_clk_100),     // 100 MHz → GT reference (MUST use this output)
    .ODIV2 (pcie_clk_50),        // 50 MHz  → Core logic (divided by 2)
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
│  O ────────────────► pcie_clk_100 (100 MHz) → GT Transceiver
│  ODIV2 ────────────► pcie_clk_50 (50 MHz)     → Core Logic
└─────────────────┘
```

**The 2:1 ratio (pcie_clk_100 = 2× pcie_clk_50) is built into the IBUFDS_GTE4 primitive.**

---

## Constraints

### pcie_clk_100 (Strict)

- **MUST** come from `IBUFDS_GTE4.O` (or `IBUFDS_GTE3` for 7-series)
- **Cannot** use MMCM, PLL, BUFG, or any other primitive
- Dedicated routing path to GT transceiver REFCLK input
- Must match IP configuration (100 MHz for Gen3)

### pcie_clk_50 (Flexible)

Can be generated from:

| Source | Frequency | Notes |
|--------|-----------|-------|
| `IBUFDS_GTE4.ODIV2` | 50 MHz | Default, simplest |
| `IBUFDS_GTE4.O` → `BUFG_GT` | 100 MHz | Same as pcie_clk_100 |
| `IBUFDS_GTE4.O` → `MMCM` | Any | Advanced, for custom frequencies |

### user_clk_250 (Output from IP)

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
    .I     (i_host_clk_100_p),
    .IB    (i_host_clk_100_n),
    .O     (pcie_clk_100),     // 100 MHz
    .ODIV2 (sys_clk_odiv2),  // 50 MHz (unused)
    .CEB   (1'b0)
);

BUFG_GT bufg_gt_inst (
    .I       (pcie_clk_100),
    .O       (pcie_clk_50),      // 100 MHz (buffered)
    .CE      (1'b1),
    .CEMASK  (1'b0),
    .CLR     (1'b0),
    .CLRMASK (1'b0),
    .DIV     (3'b000)        // No division
);
```

### Option 2: Custom pcie_clk_50 via MMCM

```verilog
// pcie_clk_100 directly from IBUFDS_GTE4
IBUFDS_GTE4 refclk_ibuf (
    .O     (pcie_clk_100),     // 100 MHz to GT
    .ODIV2 (refclk_div2),    // 50 MHz to MMCM input
    ...
);

// Generate custom pcie_clk_50
MMCME4_BASE mmcm_inst (
    .CLKIN1  (refclk_div2),  // 50 MHz in
    .CLKOUT0 (pcie_clk_50),      // Custom frequency out
    ...
);
```

---

## IP Configuration (Vivado)

When configuring PCIe IP in Vivado:

| Setting | Value | Notes |
|---------|-------|-------|
| Reference Clock Frequency | 100 MHz | Must match pcie_clk_100 |
| AXI-ST Alignment Mode | DWORD Aligned | Matches parser/formatter design |
| System reset polarity | ACTIVE LOW | Standard for PCIe |

**The IP does not have a setting for pcie_clk_50 frequency** - it accepts whatever you provide and handles it internally.

---

## Frequency Summary

```
PCIe Slot Reference Clock: 100 MHz
         │
         ├──► pcie_clk_100: 100 MHz (to GT transceiver)
         │         │
         │         └──► Internal GT PLL: 8 GHz (Gen3 line rate)
         │
         └──► pcie_clk_50: 50 MHz or 100 MHz (to core logic)
                   │
                   └──► Internal PLL: 250 MHz (user_clk_250 output)
```

---

## Common Mistakes

1. **Using BUFG instead of BUFG_GT for pcie_clk_50**
   - BUFG cannot drive GT-related clock networks
   - Use BUFG_GT for clocks related to GT transceivers

2. **Generating pcie_clk_100 from MMCM**
   - GT reference must come directly from IBUFDS_GTE4.O
   - No other clock source is valid

3. **Mismatched reference clock frequency**
   - IP expects 100 MHz (configured value)
   - Using 125 MHz or other frequencies will cause link training failure

4. **Forgetting clock domain crossing**
   - Your logic runs on `user_clk_250` (250 MHz)
   - If interfacing with other clock domains, use proper CDC

---

## Reference

- Xilinx PG156: UltraScale Devices Gen3 Integrated Block for PCI Express v4.4
  - Chapter 4: Clocking
- Xilinx UG576: UltraScale GTH/GTY Transceivers User Guide
  - Reference Clock Requirements
