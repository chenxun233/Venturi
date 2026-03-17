# CC_formatter
`CC_formatter` packs signals from the user logic side to the PCIe IP core. Since it is mainly for control signals, it only supports single-beat transfer for simplicity.
## Design Logic
The user logic provides the metadata needed for a PCIe completion:

- requester ID
- tag
- traffic class
- lower address
- completion status
- payload DWord count
- up to `128 bits` of read data (the payload)

`CC_formatter` packs those fields into the `96-bit` completion descriptor used on the `CC` channel and concatenates it with the payload:
```text
s_axis_cc_tdata = {32'h0, cc_payload[127:0], descriptor[95:0]}
```
## Interface
### Inputs
- `cc_valid`: completion valid from user logic.
- `cc_requester_id`: requester ID copied from the original host request.
- `cc_tag`: tag copied from the original host request.
- `cc_tc`: traffic class.
- `cc_lower_addr`: lower address bits used by the completion descriptor.
- `cc_dword_count`: completion payload size in DWords.
- `cc_status`: completion status code.
- `cc_payload`: up to `128 bits` of read response data.
- `cc_last`: last beat of the completion. In the current implementation it should always be asserted for a supported transfer.
- `s_axis_cc_tready`: ready from the PCIe IP core.
### Outputs
- `cc_ready`: ready back to user logic. This is `s_axis_cc_tready[0]`.
- `s_axis_cc_tdata`
- `s_axis_cc_tvalid`
- `s_axis_cc_tuser`
- `s_axis_cc_tkeep`
- `s_axis_cc_tlast`
## Keep And Handshake
The current `tkeep` logic handles the small completions used by this project:

- `cc_dword_count == 1`: `tkeep = 8'h0F`
- `cc_dword_count == 2`: `tkeep = 8'h1F`
- otherwise: `tkeep = 8'hFF`

`tuser` is tied to zero in the current RTL, and `cc_ready` simply forwards the PCIe IP ready bit.
## Limits
- The current module does **not** support multi-beat completions.
- The payload interface is limited to `128 bits`.
- The code assumes the completion can be represented in one `256-bit` `CC` beat.
