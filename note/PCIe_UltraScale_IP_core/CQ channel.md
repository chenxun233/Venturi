## CQ (Completer Request) Channel Overview

The **Completer Request (CQ)** channel is the AXI4-Stream master interface used when the **Root Complex** delivers requests initiated by the **Host (CPU/Software)** to the **FPGA Logic**. This is the primary path for register configuration and "Inbound" data movement.

---

### Role Clarity: Who Does What?

To understand the CQ channel, we must distinguish between the three participants:

* **Host (CPU/Software):** **The Initiator.** An application or driver executes a memory-mapped I/O (MMIO) instruction (e.g., `*ptr = 0x1234`). This triggers the CPU to generate a request targeting the FPGA's Base Address Register (BAR).
* **Root Complex (Hardware):** **The Messenger.** It captures the CPU's request, wraps it into a PCIe Transaction Layer Packet (TLP), and sends it across the physical link to the FPGA.
* **FPGA Logic:** **The Target.** It acts as the "Completer." It receives the request via the CQ channel, parses the address to identify which internal register or memory is being accessed, and either stores the data (for Writes) or prepares a response (for Reads).

---

### Signal Interface & Directions (256-bit)

| Signal Name | Direction | Width | Description |
| --- | --- | --- | --- |
| **`m_axis_cq_tdata`** | **Output** | 256 | 128-bit CQ Descriptor + 128-bit Payload. |
| **`m_axis_cq_tuser`** | **Output** | 85 | Sideband signals (see tuser bit mapping below). |
| **`m_axis_cq_tlast`** | **Output** | 1 | Final clock cycle of the host request. |
| **`m_axis_cq_tvalid`** | **Output** | 1 | Asserted by the **PCIe Core** when a host request is arriving. |
| **`m_axis_cq_tready`** | **Input** | 1 | Asserted by **FPGA Logic** to accept the request. |
| **`pcie_cq_np_req`** | **Input** | 1 | **Critical:** Pulse to allow the core to deliver a Read request. |

### CQ `tuser` Bit Mapping (85 bits)

| Bit(s) | Field Name | Description |
| --- | --- | --- |
| **[3:0]** | First BE | First Dword Byte Enables (only valid for first beat). |
| **[7:4]** | Last BE | Last Dword Byte Enables. |
| **[39:24]** | **Completer ID** | **Our device's Bus:Dev:Func** - use this as requester_id for DMA. |
| **[40]** | **SOP** | Start of Packet - asserted on first beat of multi-beat transfer. |
| **[41]** | Discontinue | Packet aborted mid-transfer (rare). |
| **[42]** | TPH Present | Transaction Processing Hints present. |
| **[44:43]** | TPH Type | Type of TPH steering tag. |
| **[52:45]** | TPH Steering Tag | Steering tag for cache/QoS hints. |
| **[60:54]** | Reserved | - |
| **[61]** | Parity | Odd parity over tuser and tdata. |
| **[84:62]** | Reserved | - |

**Key Fields:**
- **`tuser[39:24]`** = **Our Completer ID** - This is the FPGA's own Bus:Dev:Func assigned during PCIe enumeration. Use this as `requester_id` when initiating DMA transfers via RQ channel.
- **`tuser[40]`** = **SOP** - Use to detect the first beat when processing multi-beat CQ transfers.

---

### Transaction Layout & Use Cases

In 256-bit mode, the first cycle (**SOP**) contains the descriptor and the start of the "message" from the host.

#### **1. Register Write (Host CPU  FPGA)**

* **Goal:** The CPU driver sets a configuration bit in the FPGA.
* **Data Flow:** `tdata[127:0]` contains the Descriptor (BAR ID, Address). `tdata[255:128]` contains the first 4 Dwords of the value the CPU is writing.
* **FPGA Logic:** Sees `Req Type = 4'b0001` (Write), extracts the address, and updates the local register with the data in the payload bits.

#### **2. Register Read (Host CPU  FPGA)**

* **Goal:** The CPU driver queries a status register from the FPGA.
* **Data Flow:** `tdata[127:0]` contains the Descriptor. The payload bits `[255:128]` are **ignored**.
* **FPGA Logic:** Sees `Req Type = 4'b0000` (Read). It **must** save the **Tag** and **Requester ID** from the descriptor. It then fetches the register value and sends it back to the **Root Complex** via the **CC Channel**.

---

### CQ Descriptor Bit Mapping (`tdata[127:0]`)

| Bit(s) | Field Name | Description |
| --- | --- | --- |
| **[1:0]** | **Address Type** | `00`: Untranslated; `10`: Translated (with IOMMU). |
| **[63:2]** | **Address** | Target address within the FPGA (Dword aligned). |
| **[74:64]** | **Dword Count** | Number of Dwords the host is requesting to read/write. |
| **[78:75]** | **Req Type** | `0000`: Mem Read; `0001`: Mem Write. |
| **[79]** | **Poisoned** | Error indicator (TLP poisoned). |
| **[95:80]** | **Requester ID** | ID of the **Root Complex** (needed for your CC response). |
| **[103:96]** | **Tag** | Unique identifier from the host (must be returned in CC). |
| **[111:104]** | **Target Function** | Target function number. |
| **[114:112]** | **BAR ID** | Identifies which BAR (0–5) the **Root Complex** is targeting. |
| **[117:115]** | **BAR Aperture** | BAR aperture size encoding. |
| **[120:118]** | **Attr** | Transaction attributes (cacheability, ordering). |
| **[123:121]** | **TC** | Traffic Class (QoS priority). |
| **[127:124]** | **Reserved** | Reserved bits. |

**Note:** I/O transactions (`IORd`/`IOWr`) are deprecated for 64-bit addressing designs and not decoded in the parser.

---

## CQ Parser Module Design

The `CQ_parser` module separates the CQ channel into two logical interfaces:

### 1. Descriptor Channel (First Beat Only)

Parsed descriptor fields, valid only on the first beat (SOP):

| Output Signal | Width | Description |
| --- | --- | --- |
| `cq_valid` | 1 | Descriptor valid (asserted when `tvalid && SOP`) |
| `cq_addr_type` | 2 | Address type (from `tdata[1:0]`) |
| `cq_reg_addr` | `BAR0_SIZE` | Register address (from `tdata[63:2]`, masked to BAR0_SIZE) |
| `cq_dword_count` | 11 | Number of Dwords (from `tdata[74:64]`) |
| `cq_poisoned` | 1 | Poisoned flag (from `tdata[79]`) |
| `cq_requester_id` | 16 | Requester ID (from `tdata[95:80]`) |
| `cq_tag` | 8 | Transaction tag (from `tdata[103:96]`) |
| `cq_target_func` | 8 | Target function (from `tdata[111:104]`) |
| `cq_bar_id` | 6 | BAR ID (from `tdata[114:112]`, padded to 6 bits) |
| `cq_bar_aperture` | 3 | BAR aperture (from `tdata[117:115]`) |
| `cq_tc` | 3 | Traffic class (from `tdata[123:121]`) |
| `cq_completer_id` | 16 | Completer ID (from `tuser[39:24]`) |

**Key Design Points:**
- All descriptor fields are **gated by `tvalid && SOP`** - they are only valid on the first beat
- When invalid, fields are zero-filled for safe downstream logic
- `cq_reg_addr` is extracted from the full 62-bit address, masked to `BAR0_SIZE` bits

### 2. Data Channel (All Beats)

Full 256-bit data payload, valid on all beats:

| Output Signal | Width | Description |
| --- | --- | --- |
| `cq_data_valid` | 1 | Data valid (same as `tvalid`) |
| `cq_data_sop` | 1 | Start of packet indicator (from `tuser[40]`) |
| `cq_data` | 256 | Full data payload |

**Key Design Points:**
- On **SOP beat**: `cq_data = {128'h0, tdata[255:128]}` - descriptor bits zeroed, only payload preserved
- On **subsequent beats**: `cq_data = tdata[255:0]` - full data (for multi-beat transfers)
- Downstream logic can extract write data directly:
  - `cq_data[159:128]` = Write Data DW0
  - `cq_data[191:160]` = Write Data DW1
  - `cq_data[255:192]` = Write Data DW2-3

### 3. Transaction Type Decoding

| Output Signal | Description | Request Type Code |
| --- | --- | --- |
| `is_mem_read` | Memory Read transaction | `4'b0000` |
| `is_mem_write` | Memory Write transaction | `4'b0001` |

**Note:** I/O transactions (`IORd`/`IOWr`) are not decoded as they are only used with 16-bit addressing, which is deprecated for 64-bit designs.

### Module Interface

```verilog
module CQ_parser #(
    parameter DATA_WIDTH    = 256,
    parameter KEEP_WIDTH    = 8,
    parameter BAR0_SIZE     = 16  // 2^16 = 64 kB
)(
    // AXI-Stream inputs
    input wire [DATA_WIDTH-1:0]     m_axis_cq_tdata,
    input wire                      m_axis_cq_tvalid,
    input wire [84:0]               m_axis_cq_tuser,
    input wire [KEEP_WIDTH-1:0]     m_axis_cq_tkeep,
    input wire                      m_axis_cq_tlast,
    
    // Descriptor Channel outputs (SOP only)
    output wire                     cq_valid,
    output wire [1:0]               cq_addr_type,
    output wire [BAR0_SIZE-1:0]     cq_reg_addr,
    // ... (other descriptor fields)
    
    // Data Channel outputs (all beats)
    output wire                    cq_data_valid,
    output wire                    cq_data_sop,
    output wire [DATA_WIDTH-1:0]   cq_data,
    
    // Transaction type flags
    output wire                    is_mem_read,
    output wire                    is_mem_write
);
```

### Usage Example

```verilog
// Instantiate parser
CQ_parser #(
    .DATA_WIDTH(256),
    .KEEP_WIDTH(8),
    .BAR0_SIZE(16)
) cq_parser_inst (
    .m_axis_cq_tdata(m_axis_cq_tdata),
    .m_axis_cq_tvalid(m_axis_cq_tvalid),
    .m_axis_cq_tuser(m_axis_cq_tuser),
    .m_axis_cq_tkeep(m_axis_cq_tkeep),
    .m_axis_cq_tlast(m_axis_cq_tlast),
    // ... connect outputs
);

// Handle Memory Write
if (is_mem_write && cq_valid) begin
    case (cq_reg_addr)
        16'h0100: control_reg <= cq_data[159:128];  // Extract DW0
        16'h0104: status_reg  <= cq_data[159:128];
    endcase
end

// Handle Memory Read
if (is_mem_read && cq_valid) begin
    // Save tag and requester_id for CC response
    saved_tag <= cq_tag;
    saved_requester_id <= cq_requester_id;
    // ... prepare read data for CC channel
end
```

---

### Critical Flow Control: `pcie_cq_np_req`

This signal is the gatekeeper for **Non-Posted (Read)** requests.

* **Mechanism:** The **PCIe Core** will not deliver a Read request to your logic unless it has a "credit."
* **FPGA Logic Action:** You must drive this signal (Corundum defaults to `1'b1`) to tell the **Root Complex** you are ready for Reads.
* **If Tied to 0:** The **Root Complex** will hold the Read request in its internal buffer. The **Host CPU** will wait indefinitely for the data, eventually timing out and likely crashing the driver.

---
