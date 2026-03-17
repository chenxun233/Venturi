# RC_parser
`RC_parser` is the receive-side PCIe completion parser for the `RC` AXI-Stream channel. In this design it is the FPGA block that turns host-to-FPGA DMA read completions into a simpler logic-side descriptor plus payload interface.
## Introduction
The implementation is in `verilog_src/PCIe_related/dependencies/RC_parser.v`.

Inside [pcie_wrapper](../overview.md), this module sits on the `RC` path:

- input side: raw `m_axis_rc_*` completion beats from the PCIe IP core
- output side: parsed completion descriptor fields plus a realigned `256-bit` payload stream

`RC_parser` itself is thin. It delegates beat realignment to [RC_gearbox256](RC_gearbox256.md) and then slices the `96-bit` completion descriptor into named fields.
## Design Logic
The PCIe IP does not present the `RC` stream in a shape that is convenient for user logic:

- the start-of-packet beat contains both a completion descriptor and payload data
- later beats contain only payload
- the payload position is therefore shifted relative to the first beat

`RC_parser` handles this in two steps:

1. [RC_gearbox256](RC_gearbox256.md) removes the beat-to-beat alignment mismatch and produces a stable payload channel
2. `RC_parser` maps the `96-bit` descriptor into named outputs such as `rc_lower_addr`, `rc_tag`, and `rc_payload_dw_count`

The current descriptor field mapping in RTL is:
```text
rc_lower_addr         = rc_descriptor[11:0]
rc_err_code           = rc_descriptor[15:12]
rc_payload_byte_count = rc_descriptor[28:16]
rc_request_completed  = rc_descriptor[30]
rc_payload_dw_count   = rc_descriptor[42:32]
rc_posioned           = rc_descriptor[46]
rc_requester_id       = rc_descriptor[63:48]
rc_tag                = rc_descriptor[71:64]
rc_completer_id       = rc_descriptor[87:72]
```
The signal name `rc_posioned` is spelled exactly as it appears in the current RTL.
## Interface
### Inputs
- `clk`: PCIe user clock.
- `rst_n`: active-low reset.
- `m_axis_rc_tdata`: `256-bit` raw RC data bus from the PCIe IP.
- `m_axis_rc_tvalid`: RC beat valid.
- `m_axis_rc_tuser`: RC sideband bus from the PCIe IP.
- `m_axis_rc_tkeep`: DWord keep mask from the PCIe IP.
- `m_axis_rc_tlast`: RC last-beat indication.
### Outputs
- `m_axis_rc_tready`: ready back to the PCIe IP. In the current design it is always asserted through the gearbox.
- `rc_lower_addr`
- `rc_err_code`
- `rc_payload_byte_count`
- `rc_request_completed`
- `rc_requester_id`
- `rc_completer_id`
- `rc_tag`
- `rc_payload_dw_count`
- `rc_posioned`
- `rc_valid`: payload-channel valid after realignment.
- `rc_payload_last`: last beat of the realigned payload channel.
- `rc_payload`: realigned `256-bit` payload output.
- `rc_payload_dw_keep`: DWord keep mask for `rc_payload`.
## Relation To RC_gearbox256
The split of work is:

- [RC_gearbox256](RC_gearbox256.md): **how** raw completion beats are realigned into a clean payload stream
- `RC_parser`: **what** descriptor metadata is exposed to logic

So `RC_parser` is the descriptor-unpacking wrapper around the gearbox.
## Limits
