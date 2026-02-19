```md
# FPGA High-Speed Streaming Design — Core Design Skills Notes

A concise checklist distilled from practical 10G / PCIe / NIC / low-latency FPGA design principles.

---

# 1. Hardware Mental Model

## Think **circuits**, not software
- RTL describes **physical topology**, not execution order.
- Performance depends on:
  - combinational depth
  - fanout
  - reconvergent paths
  - clock boundaries

> Design goal: **short, predictable register-to-register paths**

---

# 2. Combinational Logic & Hazards

## Problem
Combinational logic introduces:
- race conditions
- glitches (hazards)
- timing unpredictability

Cause:
- unequal propagation delays
- reconvergent fanout

## Rule
✅ Critical control signals must be **registered**  
❌ Avoid deep combinational control trees.

---

# 3. Procedural vs Generate

| Style | Result |
|---|---|
| large `always @*` | shared optimized logic |
| `generate` blocks | replicated structure |

### Guideline
- Prefer **structural replication** for high-speed datapaths.
- Avoid large shared combinational networks.

---

# 4. Wide MUX = Timing & Glitch Risk

Wide mux creates:
- multi-level LUT trees
- selector glitches
- long critical paths

### Safe usage
✅ Inputs registered  
✅ Select (`grant`) registered  
✅ Output registered immediately

```

reg → MUX → reg   (safe)

```

### Avoid
- combinational grant
- mux driving control signals directly
- mux inside READY path

---

# 5. Control Path vs Data Path

## Key Insight
Control paths limit frequency more than datapaths.

### Best practice
> Control decisions act on **next cycle’s data**.

```

data → pipeline → action
↑
delayed control

```

Benefits:
- shorter timing paths
- stable control
- fewer glitches

---

# 6. Pipeline + Valid Flow (Replace FSM Thinking)

### FSM model (slow)
```

state decides what happens now

```

### Flow model (fast)
```

data moves every cycle
valid bit travels with data

```

Pipeline example:
```

[S0] → [S1] → [S2] → [S3]
valid valid valid

```

Advantages:
- local control
- predictable timing
- scalable frequency

---

# 7. VALID Rule (Critical)

## Golden Rule
```

DATA latency == VALID latency

````

VALID is:
> the time coordinate of DATA.

### Correct
```verilog
data_s1  <= data_s0;
valid_s1 <= valid_s0;
````

### Dangerous

```verilog
valid_s1 = condition(data_now);
```

Result of violation:

* misaligned packets
* random hardware bugs

---

# 8. READY / Backpressure Design

READY travels upstream → timing danger.

### Never allow:

* long combinational READY chains
* valid↔ready combinational loops

### Solution

Insert register slices / skid buffers.

```
stage → REG → stage → REG
```

READY must be locally generated.

---

# 9. Bubble-Free Pipeline

## Goal

```
1 word per clock forever
```

Bubble = empty cycle → throughput loss.

### Avoid stalls

Do NOT stop pipeline.

Instead:

* propagate data
* attach drop/mask flags

```
flow always continues
decision happens later
```

---

# 10. Distributed Ownership (Avoid Central Arbitration)

### Bad (central arbiter)

```
many inputs → global mux → bottleneck
```

Problems:

* wide mux
* global timing cone
* scaling failure

### Good (distributed pipeline)

```
per-input pipeline → late merge
```

Principle:

> Data owns the pipeline once admitted.

---

# 11. Arbitration & MUX Safe Pattern

Safe arbitration stage:

```
rx*_r (registered inputs)
        ↓
   grant_r (registered)
        ↓
      MUX
        ↓
     out_r (registered)
```

Mux must sit **between registers only**.

---

# 12. Throughput > Raw Latency

Counter-intuitive truth:

| Design                      | Result             |
| --------------------------- | ------------------ |
| fewer stages + stalls       | slower system      |
| deeper pipeline + no stalls | lower real latency |

Stable high Fmax wins.

---

# 13. Core Engineering Heuristics

### Always do

* pipeline aggressively
* register control signals
* keep logic local
* design for continuous flow

### Avoid

* global decisions
* wide combinational mux trees
* regenerated valid signals
* long READY paths
* centralized FSM control

---

# 14. One-Sentence Summary

> Move data every cycle, keep control registered, avoid global decisions, and design pipelines that never stop.

---

