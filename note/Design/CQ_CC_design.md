# HFT PCIe Control Plane Design: CQ Parser & CC Formatter

## 1. Design Philosophy & Rationale

This design targets **High-Frequency Trading (HFT)** applications where **Latency** and **Deterministic Behavior** are paramount. The architecture deviates from standard general-purpose NIC designs to achieve the lowest possible round-trip time (RTT) for Register Reads and the fastest possible reaction time for Doorbell Writes.

### 1.1 Zero-Latency "Wire-Through" Architecture

* **Standard Approach:** Most IP cores use registered pipelines or FIFOs to buffer incoming requests (CQ) and outgoing completions (CC) to ease timing closure.
* **HFT Approach (Your Design):**
* **Combinational Only:** Both modules use pure `assign` statements. There are **0 clock cycles** of latency added by this logic.
* **Direct Mapping:** The TLP headers are sliced and re-mapped immediately. A signal arriving at the PCIe IP boundary is available to the User Logic in the same clock cycle.



### 1.2 The "Software Contract" Optimization

* **Standard Approach:** Hardware usually handles every edge case defined in the PCIe Spec (unaligned reads, byte enables, straddled packets) to support generic drivers.
* **HFT Approach:** We enforce a strict contract with the specific user-space driver.
* **Alignment:** Addresses are assumed to be 32-bit or 64-bit aligned.
* **Granularity:** Only Full-DWord or QWord accesses are supported.
* **Benefit:** Removes complex barrel shifters and alignment logic, saving LUTs and routing resources.



---

## 2. Module Breakdown

### 2.1 `CQ_parser.v` (Completer Request)

This module acts as the "Trigger." It parses memory requests from the Host (Root Complex).

* **Flow Control (`m_axis_cq_tready = 1`):**
* **Rationale:** Backpressure is disabled. The FPGA must process requests at line rate. In HFT, if the logic stalls the PCIe bus, latency spikes occur.
* **Risk:** If downstream logic (Register File) cannot consume a write in 1 cycle, the packet is lost. (Design Constraint: Register writes must be fast).


* **Address Extraction (`cq_dw_addr` + `cq_lower_addr`):**
* **Logic:** `cq_dw_addr = m_axis_cq_tdata[2 +: (BAR0_SIZE-2)]`, `cq_lower_addr = {m_axis_cq_tdata[6:2], 2'b00}`
* **Rationale:** `cq_dw_addr` indexes DWORDs for register file addressing, while `cq_lower_addr` provides the byte offset field for completions.
* **With The address in cpp**  `cq_dw_addr` is `offset >> 2` while `cq_lower_addr` equals to `offset to BAR0` if bit <= 128.


* **Data Concatenation (`cq_wr_data`):**
* **Logic:** Concatenates `DW1` and `DW0` (`[191:160]` and `[159:128]`).
* **Rationale:** Optimizes for 64-bit Doorbell writes (common in HFT) so the TX engine gets the full queue pointer/metadata in a single beat.

* **Read Length (`cq_dword_count`):**
* **Logic:** Directly uses `m_axis_cq_tdata[74:64]`.
* **Rationale:** The formatter relies on the CPU access width (1 DW vs 2 DW) to size completions.



### 2.2 `CC_formatter.v` (Completer Completion)

This module acts as the "Responder." It builds the reply packet for Host memory reads.

* **Hardcoded Completer ID (`descriptor[87:72] = 0`):**
* **Rationale:** The Xilinx UltraScale+ Hard IP automatically overwrites the Bus Number field before transmission. Setting it to 0 saves logic and simplifies the interface.


* **Dynamic `TKEEP` Logic:**
* **Logic:** `0x1F` for 2 DWs, `0x0F` for 1 DW, otherwise `0xFF` (full 8 DW beat).
* **Rationale:** Supports `uint32_t` and `uint64_t` reads while allowing larger payloads if the request is wider.



---

## 3. FPGA Integration Guidelines (Points of Attention)

### 3.1 Timing Closure & Fanout

Since `CQ_parser` is purely combinational, the `cq_valid` signal has a long path:
`PCIe Hard IP -> CQ_parser -> Register File -> User Logic Muxes`.

* **Attention:** If you struggle with timing at 250MHz+, consider pipelining the outputs of the parser (adding 1 cycle latency) or using `keep_hierarchy` to prevent the tool from spreading this logic too thin.


### 3.2 Unsupported Requests

* **Attention:** This parser only recognizes Read (`0000`) and Write (`0001`). Other TLP types will not assert `cq_is_read`/`cq_is_write` even though `cq_valid` still follows `m_axis_cq_tvalid`.

---

## 4. Driver Design Guidelines (VFIO / User Space)

The driver **must** adhere to the hardware contract implied by this Verilog design. A standard kernel driver might work, but a custom VFIO driver is safer.

### 4.1 Strict Type Usage (`uint64_t` vs `uint32_t`)

The `CC_formatter` uses `cc_dword_count` to decide packet length. This value comes directly from the CPU instruction used.

* **Rule:** If the FPGA register is 64-bit, you **MUST** use a 64-bit pointer.
* **Code Example:**
```cpp
// Example: Writing to Queue 0 Doorbell at Offset 0x08
// The pointer MUST be 64-bit and the offset MUST be a multiple of 8
volatile uint64_t* tx_doorbell = (volatile uint64_t*)(bar0_ptr + 0x08);

// The value to write (e.g., Queue ID in upper 32, Tail Pointer in lower 32)
uint64_t doorbell_cmd = 0x0000000100000020; // QID=1, Tail=32

// Memory Barrier: Ensure all descriptor updates in RAM are visible 
// BEFORE kicking the doorbell.
__asm__ volatile("" ::: "memory"); 

// The Trigger: Generates one atomic 64-bit write TLP
*tx_doorbell = doorbell_cmd;
```
```cpp
// Example: Reading a hardware timestamp or packet counter at Offset 0x10
volatile uint64_t* status_reg = (volatile uint64_t*)(bar0_ptr + 0x10);

// The Trigger: Generates a 2-DW Read Request
uint64_t current_status = *status_reg;

// Memory Barrier: Prevent the compiler from reordering this read 
// (e.g., ensure we read the status AFTER sending a command)
__asm__ volatile("" ::: "memory"); 

// Use the data
printf("FPGA Status: 0x%016lx\n", current_status);
```

```cpp
// DANGEROUS mismatch
// If FPGA expects 64-bit but you do this:
volatile uint32_t *reg32 = (uint32_t*)(bar0 + OFFSET);
uint32_t val = *reg32; 
// Result: FPGA sees dword_count=1. 
// If FPGA logic ignores count and tries to return 64-bits, 
// the CC_formatter might cut it off or send malformed TLP.

```



### 4.2 Alignment is Mandatory

`cq_lower_addr` forces the lowest 2 address bits to zero for completion formatting, and `cq_dw_addr` is a DWORD index.

* **Rule:** Use aligned 32-bit or 64-bit accesses; do not issue byte/short operations.
* **Impact:** Misaligned reads/writes may return or update the wrong DWORD lane.
* **Note:** Byte offsets like `0x01` are not addressable; the parser drops the lowest two bits, so `0x01` maps to the `0x00` DWORD.




### 4.3 Compiler Reordering

* **Rule:** Always use `volatile` and memory barriers.
```cpp
__asm__ volatile("" ::: "memory"); // Prevent compiler reordering
*doorbell_reg = tx_desc_index;

```


* **Why:** The FPGA has no buffering. The order in which TLPs arrive is the order they are processed. You must ensure the descriptor is valid in Host RAM *before* the doorbell write TLP is generated.
