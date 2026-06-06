# FPGA Drill Weak Points Log

Use this file after each drill session to turn mistakes into review notes. Keep each entry concrete: what was confusing, the corrected mental model, and a small follow-up drill.

## Standard Drill Interface Catalog

Each drill should leave behind a reusable interface. The interface is part of the lesson: it names the contract, the direction of data/control, and the module shape interviewers expect.

### Ready/Valid Register Slice

```verilog
module rv_register_slice #(
    parameter DATA_W = 64
) (
    input  wire              clk,
    input  wire              rst,

    input  wire              i_valid,
    output wire              o_ready,
    input  wire [DATA_W-1:0] i_data,

    output wire              o_valid,
    input  wire              i_ready,
    output wire [DATA_W-1:0] o_data
);
```

Contract:

- input transfer: `i_valid && o_ready`
- output transfer: `o_valid && i_ready`
- active-high synchronous reset
- 1-cycle latency when empty and downstream ready
- 1 item/cycle sustained throughput when upstream valid and downstream ready
- hold `o_valid` and `o_data` stable while `o_valid && !i_ready`
- `o_ready = empty || downstream_ready`

What this interface teaches:

- ready travels backward, valid/data travel forward
- storage changes the ready equation
- low latency does not mean no registers; it means correctly placed registers

### Skid Buffer

```verilog
module skid_buffer #(
    parameter DATA_W = 64
) (
    input  wire              clk,
    input  wire              rst,

    input  wire              i_valid,
    output wire              o_ready,
    input  wire [DATA_W-1:0] i_data,

    output wire              o_valid,
    input  wire              i_ready,
    output wire [DATA_W-1:0] o_data
);
```

Contract:

- input transfer: `i_valid && o_ready`
- output transfer: `o_valid && i_ready`
- active-high synchronous reset
- preserves ready/valid ordering with no drop or duplication
- supports 1 item/cycle when downstream is ready
- holds output stable while `o_valid && !i_ready`
- has enough internal storage to absorb one extra input item when downstream stalls suddenly

What this interface teaches:

- why a simple register slice can still leave a long combinational ready path
- how an extra "skid" slot protects one cycle of backpressure latency
- difference between throughput, latency, and stall absorption

### Fixed-Priority Arbiter

```verilog
module fixed_priority_arbiter #(
    parameter N = 4
) (
    input  wire [N-1:0] i_req,
    output wire [N-1:0] o_grant,
    output wire         o_valid
);
```

Contract:

- purely combinational
- `i_req[0]` has highest priority
- if `i_req == 0`, then `o_valid = 0` and `o_grant = 0`
- if one or more requests are high, `o_valid = 1`
- `o_grant` is one-hot and grants the lowest-index asserted request

What this interface teaches:

- priority encoding
- one-hot output contracts
- combinational `for` loop reasoning
- hardware cost of priority chains as `N` grows

## Ready/Valid Register Slice

| Topic | Mistake / Confusion | Correct Mental Model | Follow-Up Drill |
|---|---|---|---|
| Register slice purpose | Unclear where a ready/valid register slice is used. | It is a 1-entry elastic pipeline stage between streaming blocks. It breaks timing paths and preserves backpressure behavior. | Draw `parser -> register slice -> book builder` and label `valid`, `ready`, and `data`. |
| `i_ready` vs `o_ready` | Thought `i_ready` should simply be sent back as `o_ready`. | `i_ready` means downstream can take from this module. `o_ready` means this module can take from upstream. For a 1-entry slice, `o_ready = !full || i_ready`. | Make a 4-row table for empty/full and downstream ready/not ready. |
| Transfer condition | Thought data is received only because `i_valid` is high. | A transfer happens only when both sides agree: input transfer is `i_valid && o_ready`; output transfer is `o_valid && i_ready`. | Mark input/output transfer cycles on a timing table. |
| Streaming behavior | Expected `o_valid` to alternate `1,0,1,0` through the slice. | A correct slice has 1-cycle latency but can sustain 1 item/cycle when upstream keeps `i_valid=1` and downstream keeps `i_ready=1`. | Simulate three back-to-back words and check `o_valid` stays high after the first word. |
| Stall behavior | Original RTL allowed valid/data to change during downstream stall. | When `o_valid && !i_ready`, the output item is not consumed, so `o_valid` and `o_data` must remain stable. | Add a test that changes `i_data` while stalled and checks `o_data` does not change. |
| Synchronous reset stimulus | Used `#5 rst = 0` while the first clock edge also occurred at 5 ns. | For synchronous reset, hold reset active across clock edges using `repeat (N) @(posedge clk); rst = 0;`. | Write reset stimulus that holds reset for 4 rising edges. |

## Verilog Testbench Basics

| Topic | Mistake / Confusion | Correct Mental Model | Follow-Up Drill |
|---|---|---|---|
| Testbench ports | Asked whether a testbench needs inputs/outputs. | A top-level unit testbench usually has no ports; it creates internal regs/wires and instantiates the DUT. | Write `module tb_name; ... endmodule` and instantiate one DUT. |
| DUT input/output signal types | Asked whether only `reg` can be modified. | In plain Verilog TBs, DUT inputs are usually `reg` because the TB drives them; DUT outputs are usually `wire` because the DUT drives them. | Label every DUT port as TB-driven or DUT-driven. |
| Blocking assignment in `initial` | Worried about using `=` for a `reg` clock in an `initial` block. | Blocking assignment is normal for scripted testbench stimulus. Use nonblocking mainly for synthesizable sequential RTL. | Write a clock generator with `clk = 0; forever #5 clk = ~clk;`. |
| Clock generation | Asked whether `forever #5 clk = ~clk;` is common. | It is the classic TB clock generator. With `timescale 1ns/1ps`, `#5` gives a 10 ns period, 100 MHz clock. | Change `#5` to another half-period and compute frequency. |
| `tick` task | Needed clarification on `@(posedge clk); #1;`. | `tick()` advances to the next clock edge, then waits slightly so DUT nonblocking updates settle before checks. | Use `tick(); check(...);` after driving one input item. |
| Timeout block | Needed clarification on `repeat (200) @(posedge clk); ... $finish;`. | A timeout watchdog prevents broken tests from running forever. It fails if the main test does not finish in time. | Add a 200-cycle timeout to every TB. |
| `repeat` timing | Needed clarification on what `repeat (N) @(posedge clk);` does and whether `#` delays move clock edges. | `repeat` waits for N occurrences of the following event. `#` delays in one process do not move the independently generated clock; they only change when that process starts waiting. | Write a small TB that prints `$time` before/after `#1; repeat(1) @(posedge clk);`. |
| Sampling after clock edge | Checked output immediately after `repeat(1) @(posedge clk);`. | For DUTs using nonblocking assignments, check after a tiny delay: `@(posedge clk); #1; check(...)`. | Convert repeated edge waits into a `tick` task and use it before every output check. |
| `task` vs `function` | Task/function syntax looked similar. | A function computes and returns a value without consuming time. A task performs actions and may consume time, so it is common for TB helpers. | Write one `check` task and one combinational helper function. |
| Message width | Did not know how wide a message input should be. | Plain Verilog has no dynamic string type. Use a fixed byte vector such as `[8*96-1:0]`, meaning up to 96 ASCII characters. | Change message width to `[8*128-1:0]` and pass a longer string. |
| `$display` formatting | Asked what `%0s` means. | `%s` prints a string. `%0s` prints it without extra padding spaces. `%0t` prints simulation time. | Print `time`, `o_valid`, and `o_data` in one `$display`. |
| `$time` | Needed clarification on simulation time. | `$time` returns current simulation time, useful in failure logs and waveform correlation. | Make a check fail and observe the printed time. |
| `$finish` scope | Thought `$finish` in the stimulus block might only end that one initial block while the clock runs forever. | `$finish` ends the entire simulation: clock block, timeout block, stimulus block, and all module processes stop. | Add `$display("PASS"); $finish;` at the end of a TB and confirm the forever clock stops. |
| Fatal vs accumulating checks | Noticed `fails = fails + 1` has little value if `$finish` immediately follows. | A fatal check stops at the first failure and does not need a counter. A nonfatal check increments `fails` and reports all failures at the end. | Write both versions of `check`: fatal and nonfatal. Use fatal for small drills. |
| Vivado simulation behavior | Asked whether `$finish` works in Vivado. | `$finish` works in Vivado/XSIM simulation, but it is simulation-only and not synthesizable. `$stop` pauses; `$finish` exits. | Run the same TB in XSIM later and confirm PASS/finish behavior. |
| Larger testbench structure | Asked what should be included for a more complex task. | Add helper tasks, protocol checks, scoreboard, randomized stalls, corner cases, drain phase, timeout, and a clear PASS condition. | For the FIFO drill, implement a small scoreboard array and compare output order. |

## Verilator Workflow

| Topic | Mistake / Confusion | Correct Mental Model | Follow-Up Drill |
|---|---|---|---|
| `--timing` | Asked whether it is always needed. | Use `--timing` for DUT + Verilog testbench because TBs use `#`, `@(posedge clk)`, and `repeat`. DUT-only lint often does not need it. | Run DUT-only lint without `--timing`, then DUT+TB lint with `--timing`. |
| Lint scope | Thought no `--timing` might mean syntax-only checking. | Verilator lint checks syntax plus elaboration, widths, ports, unsupported constructs, and suspicious logic. | Intentionally connect a wrong-width signal and observe lint. |
| Build before run | Needed clarification that `--binary` builds before simulation. | Verilator translates Verilog to C++ and builds an executable. You then run the executable. | Run `verilator --binary ...`, then `./obj_dir/Vtb_ready_valid`. |
| `obj_dir` | Asked how to know binaries are in `obj_dir`. | Verilator's default output directory is `obj_dir`; binary name is `V<top_module>`. | Build with top `tb_ready_valid`, then list `obj_dir/Vtb_ready_valid`. |
| Verilator vs Vivado | Asked whether Verilator or Vivado should run the TB. | Use Verilator for fast small RTL drills. Use Vivado/XSIM when Xilinx IP/primitives or GUI waveform workflow matter. | Run this TB in Verilator first; later try XSIM for comparison. |
