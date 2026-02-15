// =============================================================================
// Host <- PCIe_IP_core <- CC_formatter.v <- Logic
// Does not support multi-beat completions (max 4 DWords)
// =============================================================================

module CC_formatter #(
    parameter DATA_WIDTH = 256
)(
// =========================================================================
// User Logic Interface (completion request)
// =========================================================================


input  wire                         cc_valid,          // (user logic -> cc -> IP core)
input  wire [15:0]                  cc_requester_id,   // (cq -> cc -> IP core)
input  wire [7:0]                   cc_tag,            // (cq -> cc -> IP core)
input  wire [2:0]                   cc_tc,             // (cq -> cc -> IP core)
input  wire [6:0]                   cc_lower_addr,     // (cq -> cc -> IP core)
input  wire [10:0]                  cc_dword_count,    // (cq -> cc -> IP core)
output wire                         cc_ready,          // (IP core -> cc -> user logic)
// Our ID (FPGA's Bus:Dev:Func) 

// Completion status
// 3'b000 = sucessful completion
// 3'b001 = unsupported request
// 3'b010 = configuration request retry Status
// 3'b100 = completer abort (FPGA internal error, aborted)
input  wire [2:0]                   cc_status,     
// Read data payload    
input  wire [127:0]                 cc_payload,           // Up to 4 DWords of read data
input  wire                         cc_last,              // Last beat of this completion

// =========================================================================
// AXI-Stream to PCIe IP (s_axis_cc_*)
// =========================================================================
output wire [DATA_WIDTH-1:0]        s_axis_cc_tdata,
output wire                         s_axis_cc_tvalid,
output wire [32:0]                  s_axis_cc_tuser,
output wire [DATA_WIDTH / 32-1:0]   s_axis_cc_tkeep,
output wire                         s_axis_cc_tlast,
input  wire [3:0]                   s_axis_cc_tready    // 4-bit ready (use bit 0)
);

// tuser: Simple - just discontinue and parity
assign s_axis_cc_tuser      = 33'h0;

wire [95:0] descriptor;
assign descriptor[6:0]      = cc_lower_addr;        // Lower address
assign descriptor[7]        = 1'b0;                  // Reserved
assign descriptor[9:8]      = 2'b00;                 // Address Type
assign descriptor[15:10]    = 6'b0;                  // Reserved
assign descriptor[28:16]    = cc_dword_count<<2;      // Byte count
assign descriptor[29]       = 1'b0;                  // Not locked read
assign descriptor[31:30]    = 2'b0;                  // Reserved
assign descriptor[42:32]    = cc_dword_count;       // size of the data payload of the current packet in Dwords
assign descriptor[45:43]    = cc_status;            // Completion status
assign descriptor[46]       = 1'b0;                  // Not poisoned
assign descriptor[47]       = 1'b0;                  // Reserved
assign descriptor[63:48]    = cc_requester_id;      // Requester ID (from CQ)
assign descriptor[71:64]    = cc_tag;               // Tag (from CQ)
assign descriptor[87:72]    = 16'h0000;      // Completer ID (from IP Core Status)
assign descriptor[88]       = 1'b0;                  // As an end point, it must be 0.
assign descriptor[91:89]    = cc_tc;                // Traffic Class
assign descriptor[94:92]    = 3'b000;              // Attributes
assign descriptor[95]       = 1'b0;                  // Reserved

assign s_axis_cc_tdata      = {32'h0, cc_payload[127:0], descriptor};

// tvalid: Pass through completion valid
assign s_axis_cc_tvalid     = cc_valid;
assign s_axis_cc_tkeep      = (cc_dword_count == 11'd2) ? 8'h1F :           // uint64_t read
                              (cc_dword_count == 11'd1) ? 8'h0F : 8'hFF;    // uint32_t read
assign s_axis_cc_tlast      = cc_last;
assign cc_ready             = s_axis_cc_tready[0];


endmodule


