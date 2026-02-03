# PCIe Interface Layer - HFT NIC

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              HOST (CPU + RAM)                               │
└───────────────────────────────────┬─────────────────────────────────────────┘
                                    │ PCIe Link
┌───────────────────────────────────┴─────────────────────────────────────────┐
│                         Xilinx PCIe IP Core (PG156)                         │
│    CQ (out) ──►     ◄── CC (in)     RQ (in) ──►     ◄── RC (out)           │
└───────┬─────────────────┬───────────────┬───────────────────┬───────────────┘
        ▼                 ▲               ▲                   ▼
   CQ_parser         CC_formatter    RQ_formatter        RC_parser
        │                 │               │                   │
        ▼                 ▲               ▲                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           FPGA User Logic (HFT NIC)                         │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Channel Summary

| Channel | Direction | Initiator | Function |
|---------|-----------|-----------|----------|
| **CQ** | PCIe IP → FPGA | Host | MMIO requests (doorbells, register access) |
| **CC** | FPGA → PCIe IP | FPGA | MMIO read responses |
| **RQ** | FPGA → PCIe IP | FPGA | DMA requests (descriptor/packet fetch) |
| **RC** | PCIe IP → FPGA | Host | DMA read completions |

**Key insight:** CQ/CC = control plane (host triggers work via doorbells), RQ/RC = data plane (FPGA masters data transfer).

---

## Descriptor Sizes (64-bit Addressing)

| Channel | Descriptor | Reason |
|---------|------------|--------|
| **CQ** | 128 bits | Carries 64-bit target address from host |
| **RQ** | 128 bits | Carries 64-bit target address to host |
| **CC** | **96 bits** | No address needed - completion response only |
| **RC** | **96 bits** | No address needed - completion response only |

**Why CC/RC are smaller:**
- Completions don't carry addresses - they're *responses* to prior requests
- Only need: Requester ID + Tag (match original request), byte/DWord count, status
- Lower address (7 bits) = byte position within first DWord, not full address

**256-bit tdata Layout (DWord-aligned mode):**

```
CQ/RQ (128-bit descriptor):
  [127:0]   = Descriptor (128 bits)
  [255:128] = Payload DW0-DW3

CC/RC (96-bit descriptor):
  [95:0]    = Descriptor (96 bits)
  [127:96]  = Payload DW0        ← Payload starts at bit 96!
  [159:128] = Payload DW1
  [191:160] = Payload DW2
  [223:192] = Payload DW3
  [255:224] = Payload DW4 (or padding)
```

**Common Bug:** Putting padding between descriptor and payload in CC:
```verilog
// WRONG: Payload starts at bit 128, wastes DW0 position
assign s_axis_cc_tdata = {cc_payload, 32'h0, descriptor};

// CORRECT: Payload starts immediately after 96-bit descriptor
assign s_axis_cc_tdata = {32'h0, cc_payload[127:0], descriptor};
```

---

## HFT Design Principles

1. **Purely Combinational**: No pipeline registers, zero added latency
2. **No Backpressure**: Parsers always ready; formatters pass-through ready
3. **No Output Gating**: Fields always driven; downstream checks valid signal
4. **QWord Alignment**: 64-bit register operations for atomic access

---

## Module Interfaces

### CQ_parser (Host → FPGA MMIO)

```verilog
module CQ_parser #(parameter BAR0_SIZE = 16)(
    // AXI-Stream from PCIe IP
    input  wire [255:0]          m_axis_cq_tdata,
    input  wire                  m_axis_cq_tvalid,
    input  wire [84:0]           m_axis_cq_tuser,
    input  wire [7:0]            m_axis_cq_tkeep,
    input  wire                  m_axis_cq_tlast,
    output wire                  m_axis_cq_tready,   // Always 1

    // Parsed outputs
    output wire                  cq_valid,          // Valid transaction
    output wire                  cq_is_write,       // Memory Write
    output wire                  cq_is_read,        // Memory Read
    output wire [BAR0_SIZE-4:0] cq_dw_addr,        // QWord address
    output wire [63:0]           cq_payload,        // Write data
    output wire [2:0]            cq_bar_id,
    output wire [15:0]           cq_requester_id,   // Echo to CC
    output wire [7:0]            cq_tag,            // Echo to CC
    output wire [2:0]            cq_tc,             // Echo to CC
    output wire [6:0]            cq_lower_addr,     // Echo to CC
    output wire [10:0]           cq_payload_dw_count
);
```

### CC_formatter (FPGA → Host MMIO Response)

```verilog
module CC_formatter #(parameter DATA_WIDTH = 256, KEEP_WIDTH = 8)(
    // User interface
    input  wire                  cc_valid,
    output wire                  cc_ready,
    input  wire [15:0]           cc_requester_id,     // From CQ
    input  wire [7:0]            cc_tag,              // From CQ
    input  wire [2:0]            cc_tc,               // From CQ
    input  wire [2:0]            cc_attr,
    input  wire [6:0]            cc_lower_addr,       // From CQ
    input  wire [10:0]           cc_dword_count,
    input  wire [12:0]           cc_byte_count,
    input  wire [2:0]            cc_status,           // 000=Success
    input  wire [127:0]          cc_payload,
    input  wire                  cc_last,

    // AXI-Stream to PCIe IP
    output wire [DATA_WIDTH-1:0] s_axis_cc_tdata,
    output wire                  s_axis_cc_tvalid,
    output wire [32:0]           s_axis_cc_tuser,
    output wire [KEEP_WIDTH-1:0] s_axis_cc_tkeep,
    output wire                  s_axis_cc_tlast,
    input  wire [3:0]            s_axis_cc_tready     // Use bit [0]
);
```

### RQ_formatter (FPGA → Host DMA) - Multi-Beat

```verilog
module RQ_formatter #(parameter DATA_WIDTH = 256, KEEP_WIDTH = 8)(
    // User interface
    input  wire                  rq_valid,
    output wire                  rq_ready,
    input 
    input  wire                  rq_payload_sop,              // First beat (descriptor)
    input  wire                  rq_payload_last,             // Last beat
    
    // Descriptor fields (SOP beat only)
    input  wire [63:0]           rq_addr,
    input  wire [10:0]           rq_payload_dw_count,
    input  wire [7:0]            rq_tag,
    input  wire [15:0]           rq_requester_id,
    input  wire [2:0]            rq_tc,
    input  wire [2:0]            rq_attr,
    
    // Payload (256 bits, SOP uses [127:0], non-SOP uses [255:0])
    input  wire [255:0]          rq_payload,
    input  wire [KEEP_WIDTH-1:0] rq_payload_keep,

    // AXI-Stream to PCIe IP
    output wire [DATA_WIDTH-1:0] s_axis_rq_tdata,
    output wire                  s_axis_rq_tvalid,
    output wire [59:0]           s_axis_rq_tuser,
    output wire [KEEP_WIDTH-1:0] s_axis_rq_tkeep,
    output wire                  s_axis_rq_tlast,
    input  wire [3:0]            s_axis_rq_tready
);
```

**Multi-Beat RX DMA Write (1500-byte packet):**
```
Beat 0 (SOP): tdata = {payload[127:0], descriptor}
Beat 1:       tdata = payload[255:0]
Beat 2:       tdata = payload[255:0]
...
Beat N (EOP): tdata = payload, tkeep = partial
```

### RC_parser (Host → FPGA DMA Completion) - Multi-Beat

```verilog
module RC_parser #(parameter DATA_WIDTH = 256, KEEP_WIDTH = 8)(
    // AXI-Stream from PCIe IP
    input  wire [DATA_WIDTH-1:0] m_axis_rc_tdata,
    input  wire                  m_axis_rc_tvalid,
    input  wire [74:0]           m_axis_rc_tuser,
    input  wire [KEEP_WIDTH-1:0] m_axis_rc_tkeep,
    input  wire                  m_axis_rc_tlast,
    output wire                  m_axis_rc_tready,

    // Descriptor (SOP only)
    output wire                  rc_valid,
    output wire [7:0]            rc_tag,
    output wire [12:0]           rc_payload_byte_count,
    output wire                  rc_request_completed,
    output wire [3:0]            rc_error_code,

    // Data (all beats)
    output wire                  rc_payload_sop,
    output wire                  rc_payload_last,
    output wire [255:0]          rc_payload,          // Full 256 bits
    output wire [KEEP_WIDTH-1:0] rc_payload_dw_keep
);
```

**Multi-Beat TX DMA Read Completion:**
```
Beat 0 (SOP): rc_payload = {tdata[255:128], 128'h0}  (descriptor in lower bits)
Beat 1:       rc_payload = tdata[255:0]
Beat 2:       rc_payload = tdata[255:0]
...
Beat N (EOP): rc_payload = tdata, rc_payload_dw_keep = partial
```

---

## Data Flow

### TX Path (Send Packet)

```
1. Host writes doorbell    → CQ: cq_is_write=1, cq_dw_addr=TX_TAIL, cq_payload=5
2. FPGA fetches descriptor → RQ: req_addr=desc_ring_base+5*16, req_tag=0x50
3. Host returns descriptor → RC: cq_tag=0x50, data_payload={buffer_addr, len}
4. FPGA fetches packet     → RQ: req_addr=buffer_addr, req_tag=0x51
5. Host returns packet     → RC: cq_tag=0x51, data_payload={packet}
6. FPGA sends to MAC
```

### RX Path (Receive Packet)

```
1. MAC receives packet into FPGA buffer
2. FPGA writes to host     → RQ: req_is_write=1, req_addr=rx_buf, req_wr_data={pkt}
3. FPGA updates descriptor → RQ: req_is_write=1, req_addr=rx_desc, req_wr_data={len,ts}
4. FPGA signals host (MSI-X or doorbell)
```

---

## BAR0 Register Map (Multi-Queue)

All registers are BAR0 offsets accessed via CQ channel:

```
0x0000 - 0x00FF: Global (DEVICE_ID, STATUS, COMPLETER_ID)
0x0100 - 0x01FF: TX Queue 0 (BASE_ADDR, SIZE, HEAD, TAIL, CTRL)
0x0200 - 0x02FF: TX Queue 1
0x0300 - 0x03FF: TX Queue 2
0x0400 - 0x04FF: TX Queue 3
0x1000 - 0x10FF: RX Queue 0
0x1100 - 0x11FF: RX Queue 1
...
```

**Per-Queue Registers:**
| Offset | Register | Access | Description |
|--------|----------|--------|-------------|
| +0x00 | BASE_ADDR_LO | R/W | Descriptor ring base (lower 32 bits) |
| +0x08 | BASE_ADDR_HI | R/W | Descriptor ring base (upper 32 bits) |
| +0x10 | SIZE | R/W | Ring size in descriptors |
| +0x18 | HEAD | R/O | Current head (FPGA updates) |
| +0x20 | TAIL | R/W | **Doorbell** (driver writes new tail) |
| +0x28 | CTRL | R/W | Enable, reset |

---

## Tag Management

### CQ/CC Tags (Host-managed)
- Host assigns tag → FPGA echoes in CC
- No FPGA tracking needed

### RQ/RC Tags (FPGA-managed)
- FPGA assigns unique tag per DMA read
- Completions arrive out-of-order
- Route based on tag to correct queue

**Tag Partitioning (256 tags):**
```
[7:6] = Queue Type (00=TX, 01=RX)
[5:4] = Queue ID (0-3)
[3:0] = Sequence (16 outstanding per queue)
```

---

## Multi-Queue Arbitration

Multiple queues share RQ_formatter via arbiter:

```
TX0_req ──►┌─────────┐
TX1_req ──►│ Arbiter │──► RQ_formatter
RX0_req ──►│         │
RX1_req ──►└─────────┘
```

RC completions routed by tag back to originating queue.

---

## 4-bit tready Signal

256-bit interface = 4 × 64-bit slots. For non-straddle mode (HFT):
- Use only `tready[0]`
- Ignore `tready[3:1]`

---

## File Locations

```
verilog_src/
├── CQ_parser.v      # Host → FPGA MMIO
├── CC_formatter.v   # FPGA → Host MMIO response
├── RQ_formatter.v   # FPGA → Host DMA
└── RC_parser.v      # Host → FPGA DMA completion
```

## Reference

Xilinx PG156 v4.4: Tables 38-41 for descriptor formats.
