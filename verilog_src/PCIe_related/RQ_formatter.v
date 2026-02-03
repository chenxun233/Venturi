// =============================================================================
// Logic -> RQ_formatter -> PCIe_IP_core -> Host
// =============================================================================
module RQ_formatter #(
parameter DATA_WIDTH = 256
)(
// =========================================================================
// PCIe Interface
// =========================================================================
input clk,
input rst_n,
output wire [DATA_WIDTH-1:0]        s_axis_rq_tdata,    // to PCIe IP core
output wire                         s_axis_rq_tvalid,   // to PCIe IP core
output wire [59:0]                  s_axis_rq_tuser,    // to PCIe IP core
output wire [DATA_WIDTH / 32-1:0]   s_axis_rq_tkeep,    // to PCIe IP core
output wire                         s_axis_rq_tlast,    // to PCIe IP core
input  wire                         s_axis_rq_tready,   // from PCIe IP core
// =========================================================================
// Logic Interface
// =========================================================================
input  wire [3:0]                   rq_type,            // 4'b0000=Mem Read, 4'b0001=Mem Write
input  wire [63:0]                  rq_addr,            // Host physical address
input  wire [10:0]                  rq_payload_dw_count,     // Total Data DWords (1-1024) in this burst. Does not include header.
input  wire [7:0]                   rq_tag,             // Tag for tracking
input  wire [2:0]                   rq_tc,              // Traffic Class
input  wire                         rq_valid,           // Request valid
input  wire                         rq_payload_sop,     // Start of packet (first beat)
input  wire                         rq_payload_last,    // Last of payload. It only cares about data. Not rq_descriptor.
input  wire [255:0]                 rq_payload,
output wire                         rq_ready            // Ready (RQ_formatter->Logic)
);

// =========================================================================
// RQ rq_descriptor Format (tdata[127:0]) - PG156 Table 38
// =========================================================================
// Request type: one-hot encoded
// Only one of rq_is_write or rq_is_read should be asserted

wire [127:0] rq_descriptor;
assign rq_descriptor[1:0]     = 2'b00;                 // Not translated
assign rq_descriptor[63:2]    = rq_addr[63:2];         // DWord-aligned address (IOVA)
assign rq_descriptor[74:64]   = rq_payload_dw_count;   // DWord count, keep the same all the way
assign rq_descriptor[78:75]   = rq_type;               // Request type
assign rq_descriptor[79]      = 1'b0;                  // Not poisoned
assign rq_descriptor[95:80]   = 16'h0000;              // Requester ID. If works as an endpoint, must be 0000.
assign rq_descriptor[103:96]  = rq_tag;                // Tag
// The address must be 32-bit aligned, as the data is sent in DWords. Either 4 bytes are all invalid or all valid.
assign rq_descriptor[107:104] = (rq_payload_dw_count == 11'd1) ? 4'b0000 : 4'b1111; //LBE, indicating all 4 bytes in one DW are invalid/valid
assign rq_descriptor[111:108] = 4'b1111;                 //FBE, indicating all 4 bytes in one DW are valid 
assign rq_descriptor[112]     = 1'b0;                  // Requester ID Enable
assign rq_descriptor[114:113] = 2'b00;                 // Reserved
assign rq_descriptor[119:115] = 5'b00000;              // Reserved
assign rq_descriptor[120]     = 1'b0;                  // No Force ECRC
assign rq_descriptor[123:121] = 3'b000;                // Attributes, Snooping Enabled
assign rq_descriptor[126:124] = rq_tc;                 // Traffic Class
assign rq_descriptor[127]     = 1'b0;                  // Reserved

// =========================================================================
// AXI-Stream Output - Multi-Beat Support
// =========================================================================

RQ_gearbox256 RQ_gearbox256_inst (
    .clk                (clk),
    .rst_n              (rst_n),
    // User Interface
    .rq_descriptor        (rq_descriptor),
    .rq_payload           (rq_payload),
    .rq_payload_dw_count  (rq_payload_dw_count),
    .rq_payload_last      (rq_payload_last),
    .rq_valid             (rq_valid),
    .rq_payload_sop       (rq_payload_sop),
    .rq_ready             (rq_ready),

    // PCIe IP Core Interface
    .s_axis_rq_tdata   (s_axis_rq_tdata),
    .s_axis_rq_tvalid  (s_axis_rq_tvalid),
    .s_axis_rq_tuser   (s_axis_rq_tuser),
    .s_axis_rq_tkeep   (s_axis_rq_tkeep),
    .s_axis_rq_tlast   (s_axis_rq_tlast),
    .s_axis_rq_tready  (s_axis_rq_tready)
);



endmodule
