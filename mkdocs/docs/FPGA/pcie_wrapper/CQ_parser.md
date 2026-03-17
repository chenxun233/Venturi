# CQ_parser
Its job is to decode the infomation in descriptors from the PCIe IP core, and separate the payload as well. It does not support multi-beat operation, as it is only used for control signals.

## Design Logic
`CQ_parser` assumes the PCIe IP presents the request descriptor in the low part of `m_axis_cq_tdata`. It slices those descriptor fields into project-specific outputs and always asserts ready:
```verilog
assign m_axis_cq_tready = 1'b1;
assign cq_valid         = m_axis_cq_tvalid;
```
The most important fields are:

- `cq_reg_addr`: BAR-relative byte address, forced to `4-byte` alignment
- `cq_type`: request type used by the project register block
- `cq_payload_dw_count`: host request length in DWords
- `cq_payload`: `64-bit` write data

The current register address mapping is:
```verilog
assign cq_reg_addr = {m_axis_cq_tdata[2 +: (BAR0_SIZE-2)], 2'b00};
```
So the parser drops the two LSBs and re-appends `2'b00`, which means the downstream register block sees aligned byte addresses.

For writes, the current payload extraction combines two DWords from the request beat:
```verilog
assign cq_payload = {m_axis_cq_tdata[191:160], m_axis_cq_tdata[159:128]};
```
This is the `64-bit` data field consumed by the MMIO register logic.
## Interface
### Inputs
- `m_axis_cq_tdata`: raw `256-bit` CQ beat.
- `m_axis_cq_tvalid`
- `m_axis_cq_tuser`
- `m_axis_cq_tkeep`
- `m_axis_cq_tlast`
### Outputs
- `m_axis_cq_tready`: always asserted in the current implementation.
- `cq_valid`: decoded request valid.
- `cq_type`: request type.
- `cq_reg_addr`: aligned BAR-relative register address.
- `cq_payload`: `64-bit` write payload.
- `cq_bar_id`: target BAR ID.
- `cq_requester_id`: requester ID of the root complex.
- `cq_tag`: request tag.
- `cq_tc`: traffic class.
- `cq_lower_addr`: lower address bits used later by [CC_formatter](CC_formatter.md).
- `cq_payload_dw_count`: request length in DWords.
- `cq_last`: forwarded last-beat indication.
## Limits
- `CQ_parser` is effectively a **single-beat** parser in the current project usage.
- `m_axis_cq_tuser` and `m_axis_cq_tkeep` are not used by the current RTL.
- The parser always asserts ready, so there is no backpressure on the `CQ` channel.
