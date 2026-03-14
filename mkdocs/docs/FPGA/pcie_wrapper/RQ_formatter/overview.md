# RQ_formatter
This module formats **requester requests** from user logic into the `RQ` AXI-Stream interface expected by the PCIe IP core. In this design, it is the transmit-side block for FPGA-initiated host-memory transactions.
## Introduction
The implementation is in `verilog_src/PCIe_related/dependencies/RQ_formatter.v`.

Inside [pcie_wrapper](../overview.md), this module sits on the `RQ` path:

- input side: project-specific request fields from user logic
- output side: AXI-Stream requester packets to the PCIe IP core

`RQ_formatter` itself mainly builds the requester descriptor. The payload beat packing is handled by [RQ_gearbox256](RQ_gearbox256.md).
## Design Logic
The module takes a simpler logic-side request interface:

- request type
- host address
- total payload DWord count
- tag and traffic class
- payload data and packet-boundary signals

From those fields, it constructs the `128-bit` PCIe requester descriptor (only at the first beat) and sends both the descriptor and payload into [RQ_gearbox256](RQ_gearbox256.md).


## Request Types
The module supports the request types used by this design:

- `4'b0000`: Memory Read
- `4'b0001`: Memory Write

For a read request, the outgoing packet contains only the descriptor. For a write request, the descriptor is followed by payload data packed by [RQ_gearbox256](RQ_gearbox256.md).

## Interface
### Inputs
- `clk`: PCIe user clock
- `rst_n`: active-low reset
- `rq_type`: requester type
- `rq_addr`: host physical address
- `rq_payload_dw_count`: total payload size in DWords
- `rq_tag`: requester tag
- `rq_tc`: traffic class
- `rq_valid`: request valid
- `rq_payload_sop`: first payload beat of the request
- `rq_payload_last`: final payload beat of the request
- `rq_payload`: `256-bit` payload input from user logic
- `s_axis_rq_tready`: ready from the PCIe IP core
### Outputs
- `rq_ready`: ready back to user logic
- `s_axis_rq_tdata`
- `s_axis_rq_tvalid`
- `s_axis_rq_tuser`
- `s_axis_rq_tkeep`
- `s_axis_rq_tlast`
## Handshake
On the logic side, `RQ_formatter` uses a `valid/ready` style interface. Below gives the timing diagram:

![timing_RQ](../../../figures/FPGA/top%20level/timing_RQ.png)


## Relation To RQ_gearbox256
[RQ_gearbox256](RQ_gearbox256.md) performs the actual beat-level packing for the PCIe `RQ` stream.

Its job is to:

- combine the `128-bit` descriptor with payload data
- generate the correct `tkeep`
- drive `tlast`
- handle cases where the final payload portion needs an extra output beat

So the division of work is:

- `RQ_formatter`: descriptor generation
- [RQ_gearbox256](RQ_gearbox256.md): stream packing

