// =============================================================================
//  Logic <- RC_parser <- PCIe_IP_core <- Host
// =============================================================================

module RC_parser #(
parameter DATA_WIDTH = 256
)(
// AXI-Stream from PCIe IP
input  wire [DATA_WIDTH-1:0]        m_axis_rc_tdata,
input  wire                         m_axis_rc_tvalid,
input  wire [74:0]                  m_axis_rc_tuser,
input  wire [DATA_WIDTH / 32-1:0]   m_axis_rc_tkeep,
input  wire                         m_axis_rc_tlast,
output wire                         m_axis_rc_tready,

// =========================================================================
// Descriptor Channel (SOP beat only)
// =========================================================================
output wire                         rc_desc_valid,          // Descriptor valid (tvalid && SOP)
output wire [7:0]                   rc_tag,                 // Tag - matches RQ request
output wire [2:0]                   rc_status,              // 000=SC, 001=UR, 010=CRS, 100=CA
output wire [10:0]                  rc_dword_count,         // DWords in this completion
output wire [12:0]                  rc_byte_count,          // Remaining bytes in transaction
output wire [11:0]                  rc_lower_addr,          // Lower address (for split completions)
output wire                         rc_request_completed,   // Last completion for this request
output wire [3:0]                   rc_error_code,          // Error code (0=normal)

// =========================================================================
// Data Channel (all beats)
// =========================================================================
output wire                         rc_data_valid,          // Payload valid this cycle
output wire                         rc_data_sop,            // Start of packet
output wire                         rc_data_eop,            // End of packet
output wire [255:0]                 rc_payload,             // Full 256-bit payload
output wire [DATA_WIDTH / 32-1:0]   rc_payload_keep         // Byte enables
);

// =========================================================================
// tuser Sideband Extraction (PG156 Table 10)
// =========================================================================
wire        sop         = m_axis_rc_tuser[32];
wire        sop_beat    = m_axis_rc_tvalid && sop;

// =========================================================================
// Flow control - always ready (HFT: no backpressure)
// =========================================================================
assign m_axis_rc_tready = 1'b1;

// =========================================================================
// Descriptor Channel - RC Descriptor is 96 bits in tdata[95:0]
// =========================================================================
// Only valid on SOP beat
assign rc_desc_valid        = sop_beat;
assign rc_tag               = m_axis_rc_tdata[71:64];
assign rc_status            = m_axis_rc_tdata[45:43];
assign rc_dword_count       = m_axis_rc_tdata[42:32];
assign rc_byte_count        = m_axis_rc_tdata[28:16];
assign rc_lower_addr        = m_axis_rc_tdata[11:0];
assign rc_request_completed = m_axis_rc_tdata[30];
assign rc_error_code        = m_axis_rc_tdata[15:12];

// =========================================================================
// Data Channel - Payload extraction (Multi-Beat)
// =========================================================================
// For 256-bit interface (DWord-aligned mode):
//   SOP beat:     Descriptor in [95:0], reserved [127:96], payload in [255:128]
//   Non-SOP beat: Full 256-bit payload in [255:0]

assign rc_data_valid = m_axis_rc_tvalid;
assign rc_data_sop   = sop_beat;
assign rc_data_eop   = m_axis_rc_tvalid && m_axis_rc_tlast;

// Payload extraction:
//   SOP:     Zero lower 128 bits (descriptor area), payload in upper 128
//   Non-SOP: Full 256 bits are payload
assign rc_payload = sop ? {m_axis_rc_tdata[255:128], 128'h0}
                        : m_axis_rc_tdata[255:0];

// Keep bits: downstream uses this to know valid bytes
assign rc_payload_keep = m_axis_rc_tkeep;

endmodule

// =============================================================================
// Usage Examples
// =============================================================================
//
// Example 1: Single-Beat Completion (4 DWords / 16 Bytes)
// -------------------------------------------------------
// Beat 0 (SOP & EOP):
//   m_axis_rc_tvalid = 1
//   m_axis_rc_tuser[32] (sop) = 1
//   m_axis_rc_tlast = 1
//   m_axis_rc_tdata[95:0] = descriptor (tag, status, etc.)
//   m_axis_rc_tdata[255:128] = 16 bytes of data
//
// Output:
//   rc_desc_valid = 1
//   rc_tag = ... (from descriptor)
//   rc_data_valid = 1, rc_data_sop = 1, rc_data_eop = 1
//   rc_payload = {16_bytes, 128'h0}
//
// Example 2: Multi-Beat Completion (256 Bytes)
// --------------------------------------------
// 256 bytes = 64 DWords
// SOP beat: 128 bits (16 bytes) payload
// Non-SOP beats: 256 bits (32 bytes) per beat
// Remaining: 240 bytes / 32 = 7.5 → 8 more beats
// Total: 1 + 8 = 9 beats
//
// Beat 0 (SOP):
//   rc_desc_valid = 1, rc_tag = 0x42
//   rc_data_sop = 1, rc_data_eop = 0
//   rc_payload = {bytes[0:15], 128'h0}
//
// Beat 1-7:
//   rc_desc_valid = 0
//   rc_data_sop = 0, rc_data_eop = 0
//   rc_payload = bytes[16:47], bytes[48:79], ...
//
// Beat 8 (EOP):
//   rc_desc_valid = 0
//   rc_data_sop = 0, rc_data_eop = 1
//   rc_payload = bytes[240:255], padding
//   rc_payload_keep = partial (only valid DWords)
//
// Example 3: TX DMA Flow - FPGA Fetches 1500-Byte Packet
// ------------------------------------------------------
// 1. FPGA issues RQ read: req_addr=buffer_addr, req_dword_count=375, req_tag=0x50
// 2. RC_parser receives multi-beat completion:
//    Beat 0: rc_desc_valid=1, rc_tag=0x50, rc_payload[255:128]=bytes[0:15]
//    Beat 1: rc_payload=bytes[16:47]
//    ...
//    Beat N: rc_data_eop=1
// 3. TX engine stores payload in FPGA buffer, sends to MAC
//
// =============================================================================

// =============================================================================
// Status Codes (rc_status)
// =============================================================================
// 3'b000 = Successful Completion (SC)
// 3'b001 = Unsupported Request (UR)
// 3'b010 = Configuration Request Retry Status (CRS)
// 3'b100 = Completer Abort (CA)
// =============================================================================

// =============================================================================
// Error Codes (rc_error_code)
// =============================================================================
// 4'b0000 = Normal termination
// 4'b0001 = Poisoned completion
// 4'b0010 = UR/CA/CRS status
// 4'b0110 = Invalid tag
// 4'b1001 = Completion timeout
// =============================================================================
