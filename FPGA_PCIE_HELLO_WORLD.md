# PCIe Hello World - Design Reference

Practical guide for understanding signal flow in PCIe register access. Focus on **what signals to check** and **how values map between host and FPGA**.

---

## 1. Register Address Mapping (C++ ↔ Verilog)

### How BAR0 Offset Maps to reg_addr

**In C++ (Host):**
```cpp
volatile uint64_t* reg = (uint64_t*)(bar0 + offset);
```

**In Verilog (FPGA):**
```verilog
wire [63:0] cq_address = {m_axis_cq_tdata[63:2], 2'b00};  // From CQ descriptor
wire [2:0] reg_addr = cq_address[5:3];  // Extract bits [5:3] from byte address
```

### What is cq_address?

**cq_address is the OFFSET within BAR0, NOT the full physical address!**

### Address Calculation

```
C++ offset (bytes) → cq_address (offset within BAR0) → reg_addr[2:0]

offset = 0x00  →  cq_address = 0x00  →  reg_addr = 0x00[5:3] = 3'b000 = 0
offset = 0x08  →  cq_address = 0x08  →  reg_addr = 0x08[5:3] = 3'b001 = 1
offset = 0x10  →  cq_address = 0x10  →  reg_addr = 0x10[5:3] = 3'b010 = 2
offset = 0x18  →  cq_address = 0x18  →  reg_addr = 0x18[5:3] = 3'b011 = 3
offset = 0x20  →  cq_address = 0x20  →  reg_addr = 0x20[5:3] = 3'b100 = 4
...
offset = 0x38  →  cq_address = 0x38  →  reg_addr = 0x38[5:3] = 3'b111 = 7
```

### Why Bits \[5:3]?

**Binary breakdown:**
```
Offset 0x18 = 0b00011000
                  |||+++-- bits [2:0] = 000 (byte select, ignored)
                  +++----- bits [5:3] = 011 = 3 (register select)

So: reg_addr = 3'h3 when accessing BAR0 + 0x18
```

---

## 2. CQ tdata Field Details (256 bits)

### Complete Field Breakdown

| Bits | Field | Meaning | Expected Values |
|------|-------|---------|-----------------|
| [1:0] | `cq_addr_type` | Address translation | `00`=untranslated (typical) |
| [63:2] | `cq_address[63:2]` | Byte address >> 2 | BAR0 offset (e.g., 0x18 >> 2 = 6) |
| [74:64] | `cq_dword_count` | DWORDs to transfer | `2` for 64-bit, `1` for 32-bit |
| [78:75] | `cq_req_type` | Request type | `0000`=Read, `0001`=Write |
| [79] | `cq_poisoned` | Data integrity | `0` (should always be 0) |
| [95:80] | `cq_requester_id` | Host Bus:Dev:Func | `0x0000` typical (00:00.0) |
| [103:96] | `cq_tag` | Transaction ID | `0x00`-`0xFF` (increments) |
| [111:104] | `cq_target_func` | Target function | `0x00` (single function) |
| [114:112] | `cq_bar_id` | Which BAR hit | `0` for BAR0 |
| [117:115] | `cq_bar_aperture` | BAR aperture | Depends on BAR size config |
| [123:121] | `cq_tc` | Traffic class | `0` (best effort) |
| [127] | `cq_tph_present` | TPH hint present | `0` (not used) |
| [159:128] | `cq_write_data_dw0` | Write data low 32b | Payload for writes |
| [191:160] | `cq_write_data_dw1` | Write data high 32b | Payload for writes |
| [255:192] | (unused) | More write data | For >64-bit writes |

### Key Derived Signals

```verilog
wire is_mem_read  = (cq_req_type == 4'b0000);  // Memory Read
wire is_mem_write = (cq_req_type == 4'b0001);  // Memory Write
wire [2:0] reg_addr = cq_address[5:3];         // Register select
```

---

## 3. CC tdata Field Details (256 bits)

### Complete Field Breakdown

| Bits | Field | Meaning | What to Set |
|------|-------|---------|-------------|
| [2:0] | Reserved | - | `3'b000` |
| [6:3] | `lower_addr[3:0]` | Lower address bits | From CQ: `cq_address[3:0]` |
| [9:7] | `at` | Address type | `3'b000` |
| [28:16] | `byte_count` | Bytes remaining | `8` for 64-bit read |
| [29] | `locked_read` | Locked operation | `0` |
| [30] | `request_completed` | Last completion | `1` |
| [42:32] | `dword_count` | DWORDs in this completion | `2` for 64-bit |
| [45:43] | `completion_status` | Status code | `3'b000` = Success |
| [46] | `poisoned` | Data poisoned | `0` |
| [63:48] | `requester_id` | Echo from CQ | **Must match CQ!** |
| [71:64] | `tag` | Echo from CQ | **Must match CQ!** |
| [79:72] | `completer_id[7:0]` | Your device ID low | From config |
| [87:80] | `completer_id[15:8]` | Your device ID high | From config |
| [88] | `completer_id_en` | Enable completer ID | `0` |
| [91:89] | `tc` | Traffic class | From CQ |
| [94:92] | `attr` | Attributes | `3'b000` |
| [95] | Reserved | - | `0` |
| [127:96] | Reserved | - | `32'h0` |
| [159:128] | `data_dw0` | Read data low 32b | **Your register value** |
| [191:160] | `data_dw1` | Read data high 32b | **Your register value** |
| [255:192] | More data | For >64-bit | As needed |

### Critical: Tag and Requester ID Must Match!

```verilog
// Save from CQ when request arrives:
saved_requester_id <= cq_requester_id;
saved_tag <= cq_tag;

// Echo in CC completion:
s_axis_cc_tdata[63:48] <= saved_requester_id;  // MUST match!
s_axis_cc_tdata[71:64] <= saved_tag;           // MUST match!
```

**If these don't match → Host timeout (CPU hangs waiting for read)**

---

## 4. Signal Flow: Host Read

### Sequence Diagram

```
Host CPU              PCIe Core              User Logic (FPGA)
   |                      |                        |
   | read BAR0+0x18       |                        |
   |--------------------->|                        |
   |                      | m_axis_cq_tvalid=1     |
   |                      | cq_req_type=0000       |
   |                      | cq_address=0x18        |
   |                      | cq_tag=0xAB            |
   |                      |----------------------->|
   |                      |                        | reg_addr = 3
   |                      |                        | is_mem_read = 1
   |                      |                        | Save tag, req_id
   |                      |                        | state → ST_COMPLETE
   |                      |                        |
   |                      | m_axis_cq_tready=1     |
   |                      |<-----------------------|
   |                      |                        |
   |                      |                        | Build CC descriptor
   |                      |                        | cc_tdata = read_data
   |                      | s_axis_cc_tvalid=1     |
   |                      |<-----------------------|
   |                      |                        |
   | Completion TLP       |                        |
   |<---------------------|                        |
   | CPU gets data        |                        |
```

### Expected Signal Values (Read BAR0+0x18 = Status Register)

**Cycle N: CQ Request Arrives**
```
m_axis_cq_tvalid    = 1
m_axis_cq_tready    = 1          // You assert this
m_axis_cq_tlast     = 1
cq_req_type[3:0]    = 4'b0000    // Memory Read
cq_dword_count[10:0]= 11'h002    // Read 2 DWORDs (64 bits)
cq_address[63:0]    = 64'h18     // Offset 0x18
cq_tag[7:0]         = 8'hXX      // Some tag (e.g., 0xAB)
cq_requester_id     = 16'h0000   // Host is 00:00.0
reg_addr[2:0]       = 3'b011     // = 3 (STATUS register)
is_mem_read         = 1
is_mem_write        = 0
```

**Cycle N+1: Send Completion**
```
state               = ST_COMPLETE
s_axis_cc_tvalid    = 1
s_axis_cc_tready    = 1          // Core accepts
s_axis_cc_tlast     = 1
s_axis_cc_tkeep     = 8'h1F      // 5 DWORDs valid (3 header + 2 data)
s_axis_cc_tdata[159:128] = status_low_32bits
s_axis_cc_tdata[191:160] = status_high_32bits
s_axis_cc_tdata[71:64]   = saved_tag      // Echo tag!
s_axis_cc_tdata[63:48]   = saved_req_id   // Echo requester!
s_axis_cc_tdata[45:43]   = 3'b000         // Success
```

**Cycle N+2: Back to Idle**
```
state               = ST_IDLE
s_axis_cc_tvalid    = 0
m_axis_cq_tready    = 1          // Ready for next request
```

---

## 5. Signal Flow: Host Write

### Sequence Diagram

```
Host CPU              PCIe Core              User Logic (FPGA)
   |                      |                        |
   | write 0x42 to BAR0+0x00                       |
   |--------------------->|                        |
   |                      | m_axis_cq_tvalid=1     |
   |                      | cq_req_type=0001       |
   |                      | cq_address=0x00        |
   |                      | cq_write_data=0x42     |
   |                      |----------------------->|
   |                      |                        | reg_addr = 0
   |                      |                        | is_mem_write = 1
   |                      |                        | scratch_reg <= data
   |                      |                        | (NO completion needed!)
   |                      |                        |
   |                      | m_axis_cq_tready=1     |
   |                      |<-----------------------|
   |                      |                        |
   | (Posted - no wait)   |                        |
   | CPU continues        |                        |
```

### Expected Signal Values (Write 0x123456789ABCDEF0 to BAR0+0x00)

**Cycle N: CQ Request Arrives**
```
m_axis_cq_tvalid        = 1
m_axis_cq_tready        = 1
m_axis_cq_tlast         = 1
cq_req_type[3:0]        = 4'b0001    // Memory Write
cq_dword_count[10:0]    = 11'h002    // 2 DWORDs (64 bits)
cq_address[63:0]        = 64'h00     // Offset 0x00
cq_write_data_dw0[31:0] = 32'h12345678  // Low 32 bits
cq_write_data_dw1[31:0] = 32'h9ABCDEF0  // High 32 bits
reg_addr[2:0]           = 3'b000     // = 0 (SCRATCH register)
is_mem_read             = 0
is_mem_write            = 1
```

**Cycle N+1: Write Complete (No CC needed!)**
```
scratch_reg             = 64'h123456789ABCDEF0  // Updated!
state                   = ST_IDLE              // Stay in idle
s_axis_cc_tvalid        = 0                    // NO completion
m_axis_cq_tready        = 1                    // Ready for next
```

**Key Difference from Read:**
- Write is "posted" → No completion TLP needed
- FPGA does NOT send CC for writes
- CPU continues immediately (doesn't wait)

---

## 6. Signal Flow: MSI Interrupt

### Sequence Diagram

```
Host CPU              PCIe Core              User Logic (FPGA)
   |                      |                        |
   | write to BAR0+0x10   |                        |
   |--------------------->|                        |
   |                      | cq_tvalid=1            |
   |                      | cq_req_type=0001       |
   |                      | cq_address=0x10        |
   |                      |----------------------->|
   |                      |                        | reg_addr = 2 (INT_CTRL)
   |                      |                        | is_mem_write = 1
   |                      |                        | interrupt_trigger <= 1
   |                      |                        |
   |                      | cfg_interrupt_msi_int  |
   |                      |<-----------------------| (1-cycle pulse)
   |                      |                        |
   |                      | Generates MSI TLP      |
   |<---------------------|                        |
   | Interrupt received!  |                        |
   |                      | msi_sent=1             |
   |                      |----------------------->|
```

### Expected Signal Values

**Cycle N: Write to INT_CTRL**
```
m_axis_cq_tvalid    = 1
cq_req_type         = 4'b0001    // Memory Write
cq_address          = 64'h10     // INT_CTRL offset
reg_addr            = 3'b010     // = 2
is_mem_write        = 1
```

**Cycle N+1: Trigger Interrupt**
```
interrupt_trigger       = 1      // Pulse for 1 cycle
cfg_interrupt_msi_int   = 32'h1  // Vector 0
interrupt_counter       = interrupt_counter + 1
```

**Cycle N+2: Clear Trigger**
```
interrupt_trigger       = 0
cfg_interrupt_msi_int   = 32'h0  // Must clear!
```

**Cycle N+3: Interrupt Sent**
```
cfg_interrupt_msi_sent  = 1      // Core confirms delivery
```

---

## 7. Quick Reference Tables

### CQ: What to Extract

| Signal | Bits | Use For |
|--------|------|---------|
| `cq_req_type` | [78:75] | Determine read vs write |
| `cq_address` | [63:2]+00 | Calculate reg_addr |
| `cq_tag` | [103:96] | Save for CC (reads only) |
| `cq_requester_id` | [95:80] | Save for CC (reads only) |
| `cq_write_data_dw0` | [159:128] | Write payload low |
| `cq_write_data_dw1` | [191:160] | Write payload high |
| `cq_dword_count` | [74:64] | How many DWORDs |

### CC: What to Set

| Signal | Bits | Set To |
|--------|------|--------|
| `cc_tdata[45:43]` | Status | `3'b000` (success) |
| `cc_tdata[42:32]` | DWORD count | `2` for 64-bit read |
| `cc_tdata[63:48]` | Requester ID | **Echo from CQ** |
| `cc_tdata[71:64]` | Tag | **Echo from CQ** |
| `cc_tdata[159:128]` | Data low | Your register value |
| `cc_tdata[191:160]` | Data high | Your register value |
| `cc_tvalid` | - | `1` for 1 cycle |
| `cc_tlast` | - | `1` (single beat) |
| `cc_tkeep` | - | `8'h1F` (5 DWORDs) |

### State Machine Summary

```verilog
case (state)
  ST_IDLE: begin
    m_axis_cq_tready <= 1;           // Accept requests
    s_axis_cc_tvalid <= 0;           // Not sending

    if (cq_tvalid && cq_tready) begin
      if (is_mem_write) begin
        // Handle write, stay in IDLE (no completion)
        case (reg_addr)
          3'h0: scratch_reg <= {cq_write_data_dw1, cq_write_data_dw0};
          3'h2: interrupt_trigger <= 1;
        endcase
      end
      else if (is_mem_read) begin
        // Save metadata, go to COMPLETE
        saved_tag <= cq_tag;
        saved_requester_id <= cq_requester_id;
        state <= ST_COMPLETE;
      end
    end
  end

  ST_COMPLETE: begin
    m_axis_cq_tready <= 0;           // Busy
    s_axis_cc_tvalid <= 1;           // Send completion

    // Set read data based on reg_addr
    case (saved_reg_addr)
      3'h0: read_data <= scratch_reg;
      3'h1: read_data <= MAGIC_ID;
      3'h3: read_data <= {32'h0, interrupt_counter, 15'h0, user_lnk_up};
    endcase

    if (cc_tready) begin
      state <= ST_IDLE;              // Done
    end
  end
endcase
```

---

## 8. Debugging Checklist

### Host Read Timeout?
1. Check `m_axis_cq_tvalid` goes high
2. Check `m_axis_cq_tready` is high (you're accepting)
3. Check `s_axis_cc_tvalid` goes high (you're responding)
4. Check `s_axis_cc_tdata[71:64]` = saved_tag (**must match!**)
5. Check `s_axis_cc_tdata[63:48]` = saved_requester_id (**must match!**)
6. Check `s_axis_cc_tdata[45:43]` = 000 (success status)

### Host Write Not Working?
1. Check `cq_req_type` = 0001 (Memory Write)
2. Check `reg_addr` matches expected register
3. Check `cq_write_data_dw0/dw1` contains expected data
4. Check your register actually updates

### MSI Not Firing?
1. Check `cfg_interrupt_msi_enable` is non-zero (host enabled MSI)
2. Check `cfg_interrupt_msi_int` pulses high for exactly 1 cycle
3. Check `cfg_interrupt_msi_sent` goes high (confirm delivery)
4. Don't hold `msi_int` high - must be 1-cycle pulse!

### ILA Signals to Probe
```
m_axis_cq_tvalid
m_axis_cq_tready
cq_req_type[3:0]
cq_address[63:0] (or just [5:0])
cq_tag[7:0]
cq_write_data_dw0[31:0]
reg_addr[2:0]
is_mem_read
is_mem_write
state[1:0]
s_axis_cc_tvalid
s_axis_cc_tready
s_axis_cc_tdata[191:128] (read data)
```

---

## 9. Code Reference

### Verilog: Key Lines

| Function | File:Line |
|----------|-----------|
| CQ field extraction | pcie_register_interface.v:123-136 |
| reg_addr calculation | pcie_register_interface.v:145 |
| State machine | pcie_register_interface.v:148-250 |
| CC descriptor build | pcie_register_interface.v:200-230 |
| MSI trigger | pcie_register_interface.v:260-280 |

### C++ Test: Register Access

```cpp
// From test_fpga_hello.cpp
volatile uint64_t* scratch = (uint64_t*)(bar0 + 0x00);  // reg_addr=0
volatile uint64_t* id      = (uint64_t*)(bar0 + 0x08);  // reg_addr=1
volatile uint32_t* int_ctrl= (uint32_t*)(bar0 + 0x10);  // reg_addr=2
volatile uint64_t* status  = (uint64_t*)(bar0 + 0x18);  // reg_addr=3

// Test 1: Read ID
uint64_t id_val = *id;  // Triggers: CQ(read) → CC(completion)

// Test 2: Write scratch
*scratch = 0x123456789ABCDEF0ULL;  // Triggers: CQ(write), no CC

// Test 3: Read back scratch
uint64_t scratch_val = *scratch;  // Should match written value

// Test 4: Trigger interrupt
*int_ctrl = 0x1;  // Triggers: CQ(write) → MSI interrupt
```

---

## 10. Summary: Design Workflow

1. **Host initiates access** → `m_axis_cq_tvalid` goes high
2. **Extract CQ fields:**
   - `cq_req_type` → Read (0000) or Write (0001)?
   - `cq_address[5:3]` → Which register?
   - `cq_write_data` → What data (for writes)?
   - `cq_tag`, `cq_requester_id` → Save for reads!
3. **Handle request:**
   - **Write:** Update register, stay in IDLE, no CC needed
   - **Read:** Go to ST_COMPLETE, build CC with data
4. **For reads, send CC:**
   - Echo tag and requester_id exactly
   - Put register value in `cc_tdata[191:128]`
   - Pulse `cc_tvalid` for 1 cycle
5. **For interrupts:**
   - Pulse `cfg_interrupt_msi_int[0]` for 1 cycle
   - Wait for `cfg_interrupt_msi_sent`
