# Verilog: reg vs wire, and always @* vs clocked

Short reference for why signals like `xgmii_term` are `reg` and use `always @*` with blocking assignment, while `xgmii_rxd_masked` is `wire` and uses `assign`.

---

## 1. reg vs wire: it's about **how** the signal is driven

In Verilog the type is determined by **where** the signal is assigned, not by whether the hardware is a register or combinational logic.

| Type  | Rule |
|-------|------|
| **reg**  | Must be used for any variable that is **assigned inside** an `always` or `initial` block. |
| **wire** | Must be used for any signal that is **driven by** an `assign` statement or by a module output. |

Examples from `MAC_layer_rx.v`:

- **xgmii_term** is assigned in `always @*` → must be declared **reg**.
- **xgmii_rxd_masked** is driven only by `assign` (in a generate loop) → must be declared **wire**.

Both synthesize to **combinational logic**; a `reg` in a combinational `always @*` block does **not** become a flip-flop.

---

## 2. always @* (combinational) vs clocked always

| Block                | Use when                         | Infers            |
|----------------------|-----------------------------------|-------------------|
| **always @***        | Output is a **pure function of current inputs** (no state, no memory). | Combinational logic. |
| **always @(posedge clk)** | Output depends on **previous state** or you want **one cycle delay**. | Registers (flip-flops). |

Example: `xgmii_term` answers “which lanes have TERM in **this** beat?” using only current `i_xgmii_rxd` / `i_xgmii_rxc`. So it uses **always @*** to get the result in the same cycle with no clock delay. A clocked block would add a cycle of latency and require aligning the rest of the pipeline.

---

## 3. Blocking (=) vs non-blocking (<=)

| Assignment | Use in              | Purpose |
|------------|---------------------|--------|
| **Blocking (`=`)**   | **Combinational** `always @*` | Models “update immediately”; standard for combinational always blocks. |
| **Non-blocking (`<=`)** | **Clocked** `always @(posedge clk)` | Updates at end of time step; correct for registers and avoids races. |

So:

- In **always @*** use **`=`** (e.g. `xgmii_term[j] = ...`).
- In **always @(posedge clk)** use **`<=`** (e.g. `state <= STATE_IDLE`).

Using `<=` in an `always @*` can lead to confusing or tool-dependent behavior; blocking assignment is the right choice for combinational logic.

---

## 4. Summary table

| Signal / block type     | Declaration | Assignment style | Result in hardware   |
|-------------------------|------------|-------------------|----------------------|
| Driven by **assign**    | wire       | N/A (continuous)  | Combinational        |
| Assigned in **always @*** | reg      | `=` (blocking)    | Combinational        |
| Assigned in **always @(posedge clk)** | reg | `<=` (non-blocking) | Registers (flip-flops) |

---

## 5. Reference

- `verilog_src/MAC_layer/MAC_layer_rx.v`: `xgmii_term` (reg, always @*), `xgmii_rxd_masked` (wire, assign in generate).
