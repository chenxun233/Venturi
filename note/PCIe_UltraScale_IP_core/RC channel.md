## RC (Requester Completion) Channel Overview

The **Requester Completion (RC)** channel is the master AXI4-Stream interface used by the **Root Complex** to deliver data back to the **FPGA Logic** in response to an FPGA-initiated Read request (DMA Read).

---

### Role Clarity: Who Does What?

* **Host (CPU/Software):** **Passive.** Similar to a DMA Write, the software is not involved in the actual data movement. It simply waits for the FPGA to signal (via interrupt) that the requested data has arrived in the FPGA's local buffers.
* **Root Complex (Hardware):** **The Provider.** After receiving a Read Request from the FPGA (via the RQ channel), the Root Complex fetches the data from the **Host RAM**, packages it into a Completion TLP, and sends it to the FPGA.
* **FPGA Logic:** **The Consumer.** It acts as the "Requester." It monitors the RC channel, matches the incoming **Tag** with its internal list of pending requests, and stores the incoming data into its local memory (e.g., a packet buffer or a descriptor cache).

---

### Signal Interface & Directions (256-bit)

| Signal Name | Direction | Width | Description |
| --- | --- | --- | --- |
| **`m_axis_rc_tdata`** | **Output** | 256 | 128-bit RC Descriptor + 128-bit Payload (Data). |
| **`m_axis_rc_tuser`** | **Output** | 75 | Sideband signals (see tuser bit mapping below). |
| **`m_axis_rc_tlast`** | **Output** | 1 | Final clock cycle of the completion data. |
| **`m_axis_rc_tvalid`** | **Output** | 1 | Asserted by **PCIe Core** when data is being delivered. |
| **`m_axis_rc_tready`** | **Input** | 1 | Asserted by **FPGA Logic** to accept the data. |

### RC `tuser` Bit Mapping (75 bits)

| Bit(s) | Field Name | Description |
| --- | --- | --- |
| **[7:0]** | Byte Enables | First Dword byte enables for first beat. |
| **[8]** | is_sof_0 | Start of frame for first packet (slot 0). |
| **[9]** | is_sof_1 | Start of frame for second packet (slot 1). |
| **[31:10]** | Reserved | - |
| **[32]** | **SOP** | **Start of Packet** - asserted on first beat of completion. |
| **[33]** | Discontinue | Packet aborted mid-transfer (rare). |
| **[41:34]** | Reserved | - |
| **[42]** | **Error Poison** | TLP was marked as poisoned (data corrupted). |
| **[74:43]** | Parity | Odd parity over tuser and tdata. |

**Key Fields:**
- **`tuser[32]`** = **SOP** - Use to detect the first beat when a multi-beat completion arrives.
- **`tuser[42]`** = **Error Poison** - Check this bit; if set, the data is corrupted and should be discarded.

---

### Transaction Layout & Use Case

In 256-bit mode, the RC channel delivers the descriptor and the first 4 Dwords of host data in the same cycle.

#### **DMA Read Completion (Host RAM  FPGA)**

* **Goal:** The FPGA receives the data it previously requested from host memory.
* **Data Flow:** `tdata[127:0]` is the RC Descriptor (containing the matching Tag). `tdata[255:128]` contains the first 4 Dwords of the data from RAM.
* **FPGA Logic:** Sees `m_axis_rc_tvalid` and `m_axis_rc_tuser[32]` (SOP). It checks the **Tag** in the descriptor. If the Tag matches a pending DMA Read, it accepts the data and routes it to the correct internal buffer.

---

### RC Descriptor Bit Mapping (`tdata[127:0]`)

| Bit(s) | Field Name | Description |
| --- | --- | --- |
| **[11:0]** | **Lower Address** | Byte address of the first enabled byte of the first DW. |
| **[27:16]** | **Byte Count** | Total remaining bytes to be delivered in this transaction. |
| **[42:32]** | **Dword Count** | Number of Dwords of data in **this** TLP. |
| **[45:43]** | **Status** | `000`: Success; `001`: Unsupported; `100`: Abort. |
| **[71:64]** | **Tag** | **Crucial:** Matches the **Tag** you assigned in the **RQ request**. |
| **[95:80]** | **Completer ID** | The ID of the **Root Complex** (the responder). |
| **[94:89]** | **Attr/TC** | Attributes and Traffic Class of the completion. |

---

### Implementation Checklist for FPGA Logic

1. **Tag Matching:** The **FPGA Logic** must have a lookup table or FIFO to remember which internal buffer belongs to which **Tag**. Because completions can arrive out-of-order, the Tag is the only way to know what the data is.
2. **Handling "Split" Completions:** The **Root Complex** may break a large Read request (e.g., 512 bytes) into multiple smaller RC packets (e.g., four 128-byte packets). You must use the **Byte Count** and **Lower Address** fields to reassemble them correctly.
3. **Error Handling:** Always check the **Status** field. If the Root Complex returns `001` (Unsupported Request), it means the address you sent in the RQ channel was invalid or blocked by the IOMMU.
4. **TREADY Backpressure:** If your internal buffers are full, you can de-assert `m_axis_rc_tready`. The **PCIe Core** will hold the data in its internal RX FIFO until you are ready.

---
