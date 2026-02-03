# RQ Formatter Design

## Overview

1. The RQ (Requester) path sends Memory Read/Write requests from FPGA to Host via PCIe.

2. The descriptors must be sent along with the data at the first beat. Thus, there is only one `rq_valid`.
3. Actually, `rq_payload_dw_count` and `rq_payload_last` are redundant, since with one can dedrive the other. However, since `rq_payload_dw_count` is required by the descriptor, the inputs are designed like this.
4. Like above, `rq_payload_sop` is also redundant, but kept as an input since it makes the logic simpler.


```
User Logic → RQ_formatter → RQ_gearbox256 → PCIe IP Core → Host
```


## Module Hierarchy

### RQ_formatter
Builds the 128-bit PCIe descriptor from user inputs and instantiates the gearbox.

### RQ_gearbox256  
Handles data alignment. The descriptor occupies the lower 128 bits of the first beat, so payload data must be shifted across cycles.

## Descriptor Format (128-bit)

| Bits | Field | Description |
|------|-------|-------------|
| 1:0 | AT | Address Type (00 = untranslated) |
| 63:2 | Address | DWord-aligned host address |
| 74:64 | DW Count | Payload size in DWords (1-1024) |
| 78:75 | Type | 0000=MemRd, 0001=MemWr |
| 95:80 | Requester ID | Bus:Dev:Func |
| 103:96 | Tag | Transaction tag |
| 107:104 | LBE | Last Byte Enable |
| 111:108 | FBE | First Byte Enable |
| 126:124 | TC | Traffic Class |

## Data Alignment Problem

The PCIe IP expects: `[255:128]=Data, [127:0]=Descriptor` on first beat.

User provides 256-bit data per cycle. Since descriptor takes lower 128 bits, only lower 128 bits of user data fits in beat 1. The upper 128 bits must be saved and output in the next cycle.

```
Beat 1: [user_data[127:0], descriptor]
Beat 2: [user_data_new[127:0], user_data_prev[255:128]]
...
```

## one_more_cycle Logic

When `(dw_count % 8) > 4`, the last user beat leaves remnant data in `data_saver` that requires an extra output cycle.

| dw_count % 8 | tkeep (last) | one_more_cycle |
|--------------|--------------|----------------|
| 1 | 0x1F | No |
| 2 | 0x3F | No |
| 3 | 0x7F | No |
| 4 | 0xFF | No |
| 5 | 0x01 | Yes |
| 6 | 0x03 | Yes |
| 7 | 0x07 | Yes |
| 0 | 0x0F | Yes |

## Interface Signals

### User Interface (RQ_formatter inputs)
| Signal | Width | Description |
|--------|-------|-------------|
| rq_type | 4 | Request type |
| rq_addr | 64 | Host physical address |
| rq_payload_dw_count | 11 | Payload DWords |
| rq_tag | 8 | Transaction tag |
| rq_requester_id | 16 | BDF |
| rq_tc | 3 | Traffic class |
| rq_valid | 1 | Data valid |
| rq_payload_sop | 1 | Start of packet |
| rq_payload_last | 1 | End of packet |
| rq_payload | 256 | Write data |
| rq_ready | 1 | Ready (output) |

### PCIe Interface (AXI-Stream to IP core)
| Signal | Width | Description |
|--------|-------|-------------|
| s_axis_rq_tdata | 256 | Data + Descriptor |
| s_axis_rq_tvalid | 1 | Valid |
| s_axis_rq_tlast | 1 | Last beat |
| s_axis_rq_tkeep | 8 | Byte enables (per DWord) |
| s_axis_rq_tuser | 60 | Sideband (FBE/LBE) |
| s_axis_rq_tready | 1 | Backpressure |

## Flow Control

```
rq_ready = s_axis_rq_tready && !one_more_cycle
```

- `rq_ready` deasserts when PCIe applies backpressure
- `rq_ready` deasserts during `one_more_cycle` (gearbox flushing remnant data)

## Usage Example

**4 DWord Write (single cycle, no one_more):**
```
Cycle 1: rq_valid=1, rq_payload_sop=1, rq_payload_last=1, rq_payload_dw_count=4, rq_payload={D3,D2,D1,D0}
Output:  tdata={D3,D2,D1,D0,DESC}, tkeep=0xFF, tlast=1
```

**9 DWord Write (2 user beats + one_more):**
```
Cycle 1: rq_valid=1, rq_payload_sop=1, rq_payload_last=0, rq_payload_dw_count=9, rq_payload={D7..D0}
Output:  tdata={D3,D2,D1,D0,DESC}, tkeep=0xFF, tlast=0

Cycle 2: rq_valid=1, rq_payload_sop=0, rq_payload_last=1, rq_payload={0,0,0,0,0,0,0,D8}
Output:  tdata={D8,0,0,0,D7,D6,D5,D4}, tkeep=0xFF, tlast=0

Cycle 3: (one_more_cycle, rq_valid=0)
Output:  tdata={0,0,0,0,0,0,0,0}, tkeep=0x01, tlast=1
```
