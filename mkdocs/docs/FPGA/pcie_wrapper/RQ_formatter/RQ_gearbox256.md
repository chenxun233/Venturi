# RQ_gearbox256
This module packs the requester descriptor and payload stream into the `256-bit` `s_axis_rq_*` format required by the PCIe IP core. In this design, it is the beat-level packing stage under [RQ_formatter](overview.md).
## Introduction
The implementation is in `verilog_src/PCIe_related/dependencies/RQ_gearbox256.v`.

Inside [RQ_formatter](overview.md), this module receives:

- a `128-bit` requester descriptor
- a `256-bit` payload stream from user logic
- packet-boundary signals such as `rq_payload_sop` and `rq_payload_last`

Its output is the AXI-Stream `RQ` channel toward the PCIe IP core.
## Design Logic
The PCIe `RQ` stream does not carry the request descriptor and payload as two separate channels. They must be packed together into `256-bit` beats with the proper `tkeep`, `tlast`, and `tuser` values.

`RQ_gearbox256` performs that packing.

At a high level:

1. place the `128-bit` descriptor in the lower half of the first beat
2. place payload data beside or after the descriptor depending on packet size
3. save the upper `128-bit` portion of a payload beat when alignment requires it
4. emit a final partial beat when the remaining payload cannot fit into the previous beat

This is why the module is called a **gearbox**: it converts the logic-side payload stream into the beat alignment expected by the PCIe `RQ` interface.
## Packet Cases
The module handles three main cases.

### Read request
For a memory read request, there is no outgoing payload. The module transmits a single beat:

- lower `128 bits`: requester descriptor
- upper `128 bits`: zero

In this case:

- `s_axis_rq_tkeep = 8'h0F`
- `s_axis_rq_tlast = 1`

### Small write request
For a write whose payload is `4 DWords` or less, the descriptor and payload fit into one `256-bit` beat:

- lower `128 bits`: requester descriptor
- upper `128 bits`: payload

In this case, the packet finishes in one beat and `tkeep` is generated from the payload size.

### Large write request
For larger writes, the first beat carries the descriptor and the lower `128 bits` of payload. The module stores the upper `128 bits` of the current payload beat in `data_saver` and combines it with the lower `128 bits` of the next payload beat.

If the payload length does not end on the beat boundary created by this packing, the module asserts `one_more_cycle` and emits one additional final beat containing the saved remainder.
## Interface
### Inputs
- `clk`: PCIe user clock
- `rst_n`: active-low reset
- `rq_descriptor`: `128-bit` requester descriptor
- `rq_payload`: `256-bit` payload input
- `rq_payload_dw_count`: total payload size in DWords
- `rq_payload_last`: final payload beat from user logic
- `rq_valid`: payload beat valid
- `rq_payload_sop`: first payload beat
- `s_axis_rq_tready`: ready from the PCIe IP core
### Outputs
- `rq_ready`: ready back to [RQ_formatter](overview.md)
- `s_axis_rq_tdata`: packed `256-bit` RQ data
- `s_axis_rq_tvalid`
- `s_axis_rq_tuser`
- `s_axis_rq_tkeep`
- `s_axis_rq_tlast`
## Handshake
The module only updates its outputs when `s_axis_rq_tready` is high. On the logic-facing side, `rq_ready` is asserted when the downstream PCIe IP is ready and the gearbox is not holding an extra tail beat.

That means:

- a new input beat is accepted when `rq_valid && rq_ready`
- if `one_more_cycle` is set, `rq_ready` is deasserted until the final remainder beat is sent

This prevents the next request beat from overwriting the saved partial payload before the current packet is finished.
## Helper Logic
Two small helper functions control the output formatting.

- `calc_tail_keep()`: computes the correct `tkeep` pattern for the final beat from `rq_payload_dw_count`
- `one_more()`: determines whether the packet needs one extra output beat after the final input payload beat

Together, these functions let the module handle both aligned and misaligned packet endings without exposing that complexity to [RQ_formatter](overview.md).
## Relation To RQ_formatter
The division of work is:

- [RQ_formatter](overview.md): builds the requester descriptor
- `RQ_gearbox256`: packs descriptor and payload into AXI-Stream beats

So `RQ_formatter` decides **what** request to send, while `RQ_gearbox256` decides **how** that request is placed onto the `256-bit` PCIe stream.
## Limits
- The implementation is written for `DATA_WIDTH = 256`.
- The packing logic assumes a `128-bit` requester descriptor placed only in the first output beat.
- `rq_ready` is deasserted while an internally generated extra tail beat is pending, so upstream logic must hold its current request state until the gearbox is ready again.
