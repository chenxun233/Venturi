# pcie_wrapper
This module connects the Xilinx UltraScale PCIe Gen3 IP core to the rest of the design. It is the top-level PCIe integration block and exposes a simpler logic-side interface for MMIO and requester traffic.
## Introduction
The implementation is in `verilog_src/PCIe_related/pcie_wrapper.v`.

This wrapper mainly does three things:

1. instantiates the PCIe IP core and connects the external PCIe pins
2. routes the four PCIe AXI-Stream channels to parser or formatter submodules
3. exports the PCIe user clock, reset, and a few configuration signals to the rest of the logic
## Design Logic
`pcie_wrapper` is mostly an integration layer. It does not contain much protocol-specific processing by itself. Instead, it connects each PCIe channel to a dedicated helper module and keeps the top-level wiring manageable.

At a high level:

- `CQ` and `CC` are used for MMIO-style **control** transactions
- `RQ` and `RC` are used for requester **traffic** such as DMA-style transfers

The detailed behavior of each path is documented in the corresponding subpages.
## Hierarchy
The internal structure is shown below:

![pcie_wrapper](../../figures/FPGA/top%20level/pcie_wrapper.png)

The main submodules are:

1. `pcie3_ultrascale_0`: PCIe hard IP wrapper
2. [CQ_parser](CQ_parser.md): decodes the `CQ` channel
3. [CC_formatter](CC_formatter.md): formats the `CC` channel
4. [RQ_formatter](RQ_formatter/overview.md): formats the `RQ` channel
5. [RC_parser](RC_parser/overview.md): parses the `RC` channel
## Clock And Reset
The PCIe IP generates the user-side signals exported by the wrapper:

- `o_user_clk_250`
- `o_user_reset_p`

This `250 MHz` clock domain is used by the PCIe-side logic in the design.
## How It Is Used In Top
In `verilog_src/Top.v`, `pcie_wrapper` sits between the external PCIe pins and the internal control/data path.

- the `CQ/CC` path connects to the control-plane logic
- the `RQ/RC` path connects to requester-side data movement logic
