// =============================================================================
//  Logic <- RC_parser <- RC_gearbox256 <- PCIe_IP_core <- Host
// =============================================================================
// This module parses RC completions and realigns payload data using RC_gearbox256.
// The gearbox handles the 128-bit offset in SOP beats, providing clean 256-bit
// aligned data to user logic.
// =============================================================================

module RC_parser #(
parameter DATA_WIDTH = 256
)(
// =========================================================================
// Clock and Reset
// =========================================================================
input  wire                         clk,
input  wire                         rst_n,

// =========================================================================
// PCIe IP core Interface
// =========================================================================
input  wire [DATA_WIDTH-1:0]        m_axis_rc_tdata,
input  wire                         m_axis_rc_tvalid,
input  wire [74:0]                  m_axis_rc_tuser,
input  wire [DATA_WIDTH / 32-1:0]   m_axis_rc_tkeep,
input  wire                         m_axis_rc_tlast, // if axi straddle is enabled, it will always be 0.
output wire                         m_axis_rc_tready,

// =========================================================================
// Logic Interface - Descriptor (valid on SOP)
// =========================================================================
output wire  [11:0]                  rc_lower_addr,
output wire  [3:0]                   rc_err_code,
output wire  [12:0]                  rc_payload_byte_count,
output wire                          rc_request_completed,
output wire  [15:0]                  rc_requester_id,
output wire  [15:0]                  rc_completer_id, //not used generally
output wire  [7:0]                   rc_tag,
output wire  [12:0]                  rc_payload_dw_count,
output wire                          rc_posioned,


// =========================================================================
// Logic Interface - Realigned Data Channel
// =========================================================================
output wire                         rc_valid,               // valid for all
output wire                         rc_payload_last,         // End of packet
output wire [255:0]                 rc_payload,             // Realigned 256-bit payload
output wire [DATA_WIDTH / 32-1:0]   rc_payload_dw_keep         // DW enables
);

// =========================================================================
// RC_gearbox256 Instantiation - Handles payload realignment
// =========================================================================
wire [95:0] rc_descriptor;

assign    rc_lower_addr           = rc_descriptor[11:0];
assign    rc_err_code             = rc_descriptor[15:12];
assign    rc_payload_byte_count   = rc_descriptor[28:16] ;
assign    rc_request_completed    = rc_descriptor[30];
assign    rc_payload_dw_count    = rc_descriptor[42:32];
assign    rc_requester_id         = rc_descriptor[63:48];
assign    rc_completer_id         = rc_descriptor[87:72];
assign    rc_tag                  = rc_descriptor[71:64];
assign    rc_posioned             = rc_descriptor[46];


RC_gearbox256 #(
    .DATA_WIDTH(DATA_WIDTH)
) rc_gearbox_inst (
    .clk                    (clk),
    .rst_n                  (rst_n),

    // PCIe IP Core Interface
    .m_axis_rc_tdata        (m_axis_rc_tdata),
    .m_axis_rc_tvalid       (m_axis_rc_tvalid),
    .m_axis_rc_tuser        (m_axis_rc_tuser),
    .m_axis_rc_tkeep        (m_axis_rc_tkeep),
    .m_axis_rc_tlast        (m_axis_rc_tlast),
    .m_axis_rc_tready       (m_axis_rc_tready),

    // Realigned User Interface
    .rc_valid               (rc_valid),
    .rc_payload_last        (rc_payload_last),
    .rc_payload             (rc_payload),
    .rc_payload_dw_keep     (rc_payload_dw_keep),
    .rc_descriptor          (rc_descriptor)
);

endmodule

