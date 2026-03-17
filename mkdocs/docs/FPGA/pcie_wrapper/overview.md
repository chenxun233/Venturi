# pcie_wrapper
This module connects the Xilinx UltraScale PCIe Gen3 IP core to the rest of the design. It is the top-level PCIe integration block and exposes a simpler logic-side interface for MMIO and requester traffic.

## Design Logic
The implementation is in `verilog_src/PCIe_related/pcie_wrapper.v`.

The main function is to separate the descriptors from the payloads, as they are packed together by the PCIe GEN3 IP core originally.

There are four paths from the IP core:

- `CQ` and `CC` are used for MMIO-style **control** transactions, where the host (PC side, CPU core) is the initiator
- `RQ` and `RC` are used for requester **traffic** such as DMA-style transfers, where the user logic (FPGA side) is the initiator.

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

