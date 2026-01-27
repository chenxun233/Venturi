// =============================================================================
// RQ_formatter.v - Requester Request Formatter for HFT NIC (Multi-Beat)
// =============================================================================
// Formats the Requester Request (RQ) AXI-Stream interface to Xilinx UltraScale
// PCIe Gen3 IP core. Supports both single-beat reads and multi-beat writes.
//
// For HFT RX DMA: Pushes large packets to host memory using multi-beat writes.
// =============================================================================
// Logic->PCIe_IP_core->Host
module RQ_formatter #(
parameter DATA_WIDTH = 256
)(
// =========================================================================
// User Logic Interface
// =========================================================================
output wire                         rq_ready,           // Ready (PCIe_IP_core->Logic)
input  wire                         rq_valid,           // Request valid
input  wire                         rq_is_write,        // 1=DMA Write
input  wire                         rq_is_read,         // 1=DMA Read (mutually exclusive with rq_is_write)
input  wire                         rq_sop,             // Start of packet (first beat)
input  wire                         rq_last,            // Last beat of request

// Descriptor fields (only used on SOP beat)
input  wire [63:0]                  rq_addr,            // Host physical address
input  wire [10:0]                  rq_dword_count,     // Total DWords (1-1024)
input  wire [7:0]                   rq_tag,             // Tag for tracking
input  wire [15:0]                  rq_requester_id,    // Our Bus:Dev:Func
input  wire [2:0]                   rq_tc,              // Traffic Class
input  wire [2:0]                   rq_attr,            // Attributes

// Payload data (used for writes)
// SOP beat:     rq_payload[127:0] goes to tdata[255:128]
// Non-SOP beat: rq_payload[255:0] goes to tdata[255:0]
input  wire [255:0]                 rq_payload,
input  wire [DATA_WIDTH / 32-1:0]   rq_payload_keep,    // Byte enables for payload

// =========================================================================
// AXI-Stream to PCIe IP (s_axis_rq_*)
// =========================================================================
output wire [DATA_WIDTH-1:0]        s_axis_rq_tdata,
output wire                         s_axis_rq_tvalid,
output wire [59:0]                  s_axis_rq_tuser,
output wire [DATA_WIDTH / 32-1:0]   s_axis_rq_tkeep,
output wire                         s_axis_rq_tlast,
input  wire [3:0]                   s_axis_rq_tready
);

// =========================================================================
// RQ Descriptor Format (tdata[127:0]) - PG156 Table 38
// =========================================================================
// Request type: one-hot encoded
// Only one of rq_is_write or rq_is_read should be asserted
wire [3:0] rq_type = rq_is_write ? 4'b0001 : 
                        rq_is_read  ? 4'b0000 : 4'b0000;  // Default to Read if neither

wire [127:0] descriptor;
assign descriptor[1:0]     = 2'b10;                 // Address Type: Translated (IOMMU - IOVA)
assign descriptor[63:2]    = rq_addr[63:2];         // DWord-aligned address (IOVA)
assign descriptor[74:64]   = rq_dword_count;        // DWord count
assign descriptor[78:75]   = rq_type;               // Request type
assign descriptor[79]      = 1'b0;                  // Not poisoned
assign descriptor[95:80]   = rq_requester_id;       // Requester ID
assign descriptor[103:96]  = rq_tag;                // Tag
assign descriptor[111:104] = 8'h00;                 // Target Function
assign descriptor[112]     = 1'b0;                  // Requester ID Enable
assign descriptor[114:113] = 2'b00;                 // Reserved
assign descriptor[119:115] = 5'b00000;              // Reserved
assign descriptor[120]     = 1'b0;                  // No Force ECRC
assign descriptor[123:121] = rq_attr;               // Attributes
assign descriptor[126:124] = rq_tc;                 // Traffic Class
assign descriptor[127]     = 1'b0;                  // Reserved

// =========================================================================
// AXI-Stream Output - Multi-Beat Support
// =========================================================================

// tdata:
//   SOP beat:     [127:0] = descriptor, [255:128] = first 128 bits of payload
//   Non-SOP beat: [255:0] = full 256 bits of payload
// For reads, payload is don't care; for writes, payload contains data
assign s_axis_rq_tdata = rq_sop ? {rq_payload[127:0], descriptor}
                                : rq_payload[255:0];

// tvalid: Pass through
assign s_axis_rq_tvalid = rq_valid;

// tuser: Byte enables
// [3:0]  = First BE (0xF for aligned)
// [7:4]  = Last BE (0xF for aligned)
// [59:8] = Reserved/Parity (0)
assign s_axis_rq_tuser = 60'h0FF;  // First BE=0xF, Last BE=0xF

// tkeep:
//   SOP beat:     All 8 DWords valid (descriptor + payload)
//   Non-SOP beat: Use payload_keep from user
assign s_axis_rq_tkeep = rq_sop ? 8'hFF : rq_payload_keep;

// tlast: Pass through
assign s_axis_rq_tlast = rq_last;

// Ready: Use bit 0 of 4-bit tready
assign rq_ready = s_axis_rq_tready[0];

endmodule

// =============================================================================
// Usage Examples
// =============================================================================
//
// Example 1: DMA Read (Single Beat, No Payload)
// ---------------------------------------------
// rq_valid=1, rq_is_write=0, rq_sop=1, rq_last=1
// rq_addr=0x7F80_0000, rq_dword_count=4, rq_tag=0x10
// rq_payload = don't care (ignored for reads)
//
// Result: Single TLP requesting 4 DWords from host
//
// Example 2: DMA Write 64 Bytes (Single Beat)
// -------------------------------------------
// rq_valid=1, rq_is_write=1, rq_sop=1, rq_last=1
// rq_addr=0x7F80_1000, rq_dword_count=16 (64 bytes)
// rq_payload[127:0] = first 16 bytes
//
// But wait - 64 bytes needs 2 beats!
//
// Example 3: DMA Write 64 Bytes (Multi-Beat)
// ------------------------------------------
// Beat 0 (SOP):
//   rq_valid=1, rq_is_write=1, rq_sop=1, rq_last=0
//   rq_addr=0x7F80_1000, rq_dword_count=16
//   rq_payload[127:0] = bytes[0:15]
//   → tdata = {bytes[0:15], descriptor}
//
// Beat 1:
//   rq_valid=1, rq_sop=0, rq_last=0
//   rq_payload[255:0] = bytes[16:47]
//   → tdata = bytes[16:47]
//
// Beat 2:
//   rq_valid=1, rq_sop=0, rq_last=1
//   rq_payload[127:0] = bytes[48:63], rq_payload_keep=8'h0F (lower 4 DWords)
//   → tdata = {don't care, bytes[48:63]}, tkeep=8'h0F
//
// Example 4: RX DMA - 1500 Byte Ethernet Frame
// --------------------------------------------
// Total: 1500 bytes = 375 DWords = 47 beats (128b SOP + 46*256b)
//
// Beat 0: rq_sop=1, rq_payload[127:0] = pkt[0:127]
// Beat 1: rq_sop=0, rq_payload[255:0] = pkt[128:383]
// Beat 2: rq_sop=0, rq_payload[255:0] = pkt[384:639]
// ...
// Beat 46: rq_sop=0, rq_last=1, rq_payload = pkt[...], rq_payload_keep = ...
//
// =============================================================================
