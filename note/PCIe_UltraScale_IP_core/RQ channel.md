## RQ (Requester Request) Channel Overview

The **Requester Request (RQ)** channel is the AXI4-Stream slave interface used when the **FPGA Logic** initiates transactions to access **Host RAM**. This is the core mechanism for **DMA** (Direct Memory Access).

---

### Role Clarity: Who Does What?

To understand the RQ channel, we must distinguish between the three participants:

* **Host (CPU/Software):** **Passive.** The software driver allocates a buffer in system RAM and provides its **Physical Address** to the FPGA (usually via a BAR write on the CQ channel). The CPU does not move the data; it simply waits for the FPGA to finish.
* **Root Complex (Hardware):** **The Executioner.** Located in the CPU/Chipset, it receives TLPs from the FPGA. For Writes, it pushes data into RAM. For Reads, it fetches data from RAM and sends it back to the FPGA via the **RC Channel**.
* **FPGA Logic:** **The Master.** It initiates the transaction. It constructs the RQ descriptor, provides the address, manages the data payload (for writes), and tracks transaction **Tags** (for reads).

---

### Signal Interface & Directions (256-bit)

| Signal Name | Direction | Width | Description |
| --- | --- | --- | --- |
| **`s_axis_rq_tdata`** | **Input** | 256 | 128-bit RQ Descriptor + 128-bit Payload. |
| **`s_axis_rq_tuser`** | **Input** | 60 | Sideband signals (see tuser bit mapping below). |
| **`s_axis_rq_tlast`** | **Input** | 1 | Final clock cycle of the request. |
| **`s_axis_rq_tvalid`** | **Input** | 1 | Asserted by **FPGA Logic** to start a request. |
| **`s_axis_rq_tready`** | **Output** | 4 | Asserted by the **PCIe Core** when it can accept the packet. |

### RQ `tuser` Bit Mapping (60 bits)

| Bit(s) | Field Name | Description |
| --- | --- | --- |
| **[3:0]** | First BE | First Dword Byte Enables (which bytes valid in first Dword). |
| **[7:4]** | Last BE | Last Dword Byte Enables (which bytes valid in last Dword). |
| **[10:8]** | Addr Offset | Address offset for non-Dword-aligned transfers. |
| **[11]** | Discontinue | Set to `1'b1` to abort packet mid-transfer (rare). |
| **[12]** | TPH Present | Transaction Processing Hints present. |
| **[14:13]** | TPH Type | Type of TPH steering tag. |
| **[23:15]** | TPH Steering Tag | Steering tag for cache/QoS hints. |
| **[27:24]** | Seq Number | Sequence number (4 bits per function). |
| **[59:28]** | Parity | Odd parity over tuser and tdata. |

**Typical Usage for Simple DMA:**
- **First/Last BE**: Set to `4'hF` for fully aligned Dword transfers.
- **All others**: Set to `0`.
- Example: `s_axis_rq_tuser = 60'h0` (with proper byte enables in descriptor bits [119:112]).

---

### Transaction Layout & Use Cases

In 256-bit mode, the first cycle (**SOP**) packed with metadata and data.

#### **1. DMA Write (FPGA  Host RAM)**

* **Goal:** Move data from FPGA to system memory.
* **FPGA Logic:** Sets `Req Type = 4'b0001`. Drives descriptor on `tdata[127:0]` and first 4 Dwords of data on `tdata[255:128]`.
* **Root Complex:** Receives the TLP and updates the RAM. This is a **Posted** transaction (no response packet required).

#### **2. DMA Read (Host RAM  FPGA)**

* **Goal:** Fetch data from system memory into the FPGA.
* **FPGA Logic:** Sets `Req Type = 4'b0000`. Assigns a unique **Tag**. Descriptor on `tdata[127:0]`; payload bits `[255:128]` are **ignored**.
* **Root Complex:** Reads RAM and generates a **Completion TLP** containing the requested data and the original **Tag**.
* **RC Channel:** The data is delivered back to the FPGA on the **RC interface**.

---

### RQ Descriptor Bit Mapping (`tdata[127:0]`)

| Bit(s) | Field Name | Description |
| --- | --- | --- |
| **[63:2]** | **Address** | Physical address in **Host RAM** (Dword aligned). |
| **[74:64]** | **Dword Count** | Length of transfer in Dwords (1–1024). |
| **[78:75]** | **Req Type** | `0000`: Mem Read; `0001`: Mem Write. |
| **[103:96]** | **Tag** | Identifier assigned by **FPGA Logic** to match future RC data. |
| **[95:80]** | **Requester ID** | Bus/Device/Function ID of the **FPGA Logic**. |
| **[120:114]** | **Target Func** | Specific PF/VF in the FPGA initiating the request. |

---

### Implementation Checklist

1. **Tag Management:** The **FPGA Logic** must ensure that no two outstanding Read requests share the same Tag.
2. **Credit Check:** Before `tvalid`, monitor `pcie_tfc_nph_av` (Read credits) or `pcie_tfc_ph_av` (Write credits) to ensure the **PCIe Core** has buffer space.
3. **Alignment:** `tuser[3:0]` (First Byte Enable) and `tuser[7:4]` (Last Byte Enable) must be set correctly if the transfer is not Dword-aligned at the boundaries.

---
