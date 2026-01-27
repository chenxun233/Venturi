## CC (Completer Completion) Channel Overview

The **Completer Completion (CC)** channel is the AXI4-Stream slave interface used by the **FPGA Logic** to send responses (Completions) back to the **Root Complex** following a host-initiated Read request.

---

### Role Clarity: Who Does What?

* **Host (CPU/Software):** **The Waiter.** After the CPU sends a Read request (via the CQ channel), the specific CPU thread usually "stalls" or waits for the data to return. If the completion takes too long, the CPU triggers a **Completion Timeout (CTO)**.
* **Root Complex (Hardware):** **The Router.** It receives the Completion TLP from the FPGA via the physical link, matches the **Tag**, and delivers the data to the correct CPU cache/register that initiated the original request.
* **FPGA Logic:** **The Responder.** It acts as the "Completer." It fetches the requested data from its internal registers or memory, constructs the CC Descriptor using the **Tag** saved from the CQ request, and drives the CC interface.

---

### Signal Interface & Directions (256-bit)

| Signal Name | Direction | Width | Description |
| --- | --- | --- | --- |
| **`s_axis_cc_tdata`** | **Input** | 256 | 128-bit CC Descriptor + 128-bit Payload (Data). |
| **`s_axis_cc_tuser`** | **Input** | 33 | Sideband signals (see tuser bit mapping below). |
| **`s_axis_cc_tlast`** | **Input** | 1 | Final clock cycle of the completion. |
| **`s_axis_cc_tvalid`** | **Input** | 1 | Asserted by **FPGA Logic** to send the response. |
| **`s_axis_cc_tready`** | **Output** | 4 | Asserted by the **PCIe Core** when it can accept the packet. |

### CC `tuser` Bit Mapping (33 bits)

| Bit(s) | Field Name | Description |
| --- | --- | --- |
| **[0]** | Discontinue | Set to `1'b1` to abort packet mid-transfer (rare). |
| **[32:1]** | Parity | Odd parity over tuser and tdata (typically set to 0). |

**Typical Usage:**
- Set `s_axis_cc_tuser = 33'h0` for normal completions.
- Discontinue is only used for error conditions where you need to abort a multi-beat transfer.

---

### Transaction Layout & Data Mapping

In 256-bit mode, the Completion Descriptor and the start of the data are packed into the first cycle (**SOP**).

| Bit Range | Component | Usage for **CplD** (Completion with Data) |
| --- | --- | --- |
| **`[127:0]`** | **CC Descriptor** | **Used** (Routing & Status Header) |
| **`[255:128]`** | **Payload (Data)** | **Used** (First 4 Dwords of returned data) |

---

### CC Descriptor Bit Mapping (`tdata[127:0]`)

To ensure the **Root Complex** accepts the data, the descriptor must perfectly mirror the original request.

| Bit(s) | Field Name | Description |
| --- | --- | --- |
| **[9:8]** | **AT** | Address Type (Must match **CQ Descriptor [1:0]**). |
| **[28:16]** | **Byte Count** | Total bytes remaining to be sent (e.g., 8 for a 64-bit read). |
| **[42:32]** | **Dword Count** | Number of Dwords in **this** TLP. |
| **[45:43]** | **Status** | `000`: Success; `001`: Unsupported Req; `100`: Abort. |
| **[63:48]** | **Requester ID** | **Must match** `Requester ID` from the **CQ Descriptor**. |
| **[71:64]** | **Tag** | **Must match** `Tag` from the **CQ Descriptor**. |
| **[95:80]** | **Completer ID** | The Bus/Device/Function ID of your **FPGA**. |
| **[94:89]** | **Attr/TC** | Attributes and Traffic Class (Must match **CQ Descriptor**). |

---

### Implementation Checklist for FPGA Logic

1. **Alignment Warning:** The first Dword of your data (DW0) must start at **`tdata[159:128]`**. If you accidentally start at `tdata[127:96]`, you will corrupt the **Tag** and **Requester ID**, and the **Root Complex** will discard the packet as an "Unexpected Completion."
2. **The Matching Rule:** You cannot send a completion whenever you want. You must only send one in response to a valid Read request received on the CQ channel.
3. **TLAST Timing:** For a single-register read (1 or 2 Dwords), the entire packet fits in the first 256-bit cycle. Therefore, `s_axis_cc_tlast` must be asserted in the same cycle as `s_axis_cc_tvalid`.
4. **Byte Count vs. Dword Count:** `Byte Count` is the total length of the intended transfer; `Dword Count` is what is inside this specific packet. For standard register reads, they usually describe the same amount of data.

---
