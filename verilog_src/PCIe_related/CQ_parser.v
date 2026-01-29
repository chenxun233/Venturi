// =============================================================================
// Host -> PCIe_IP_core -> CQ_parser.v -> Logic
// Does not support multi-beat completions (max 4 DWords)
// =============================================================================

module CQ_parser #(
parameter DATA_WIDTH = 256,    // AXI-Stream data width
parameter BAR0_SIZE = 16      // Byte address width (2^16 = 64KB BAR)
)(
// =========================================================================
// AXI Channel
// =========================================================================
input  wire [DATA_WIDTH-1:0]    m_axis_cq_tdata,
input  wire                     m_axis_cq_tvalid,
input  wire [84:0]              m_axis_cq_tuser,
input  wire [DATA_WIDTH/32-1:0] m_axis_cq_tkeep,
input  wire                     m_axis_cq_tlast,
output wire                     m_axis_cq_tready,

// =========================================================================
// Descriptor Channel
// =========================================================================
output wire                     cq_valid,          // t1, Transaction valid this cycle, always check it first.
output wire [3:0]               cq_type,           // t1, 4'b0000=Mem Read, 4'b0001=Mem Write
output wire [BAR0_SIZE-1:0]     cq_reg_addr,       // t1, 
output wire [63:0]              cq_wr_data,        // t1, Write data (64-bit)
output wire [2:0]               cq_bar_id,         // t1, Target BAR
output wire [15:0]              cq_requester_id,   // t1, the Root Complex sending the request, feed to cc completer
output wire [7:0]               cq_tag,            // t1, associated with the request
output wire [2:0]               cq_tc,             // t1, associated with the request
output wire [6:0]               cq_lower_addr,     // Lower 7 bits of byte address, need to feed to cc completer
output wire [10:0]              cq_dword_count      // For multi-DWord reads
);


// =========================================================================
// Flow control - always ready
// =========================================================================
assign      m_axis_cq_tready   = 1'b1;

// =========================================================================
// Descriptor outputs
// =========================================================================
assign      cq_valid           = m_axis_cq_tvalid;

// address
assign      cq_reg_addr         = {m_axis_cq_tdata[2 +: (BAR0_SIZE-2)],2'b00};
// how many DW you try to read. In the driver, if you read uint64, cq_dword_count is 2.
assign      cq_dword_count     = m_axis_cq_tdata[74:64];
// request type
assign      cq_type           = m_axis_cq_tdata[78:75];
// request ID
assign      cq_requester_id    = m_axis_cq_tdata[95:80];
// tag      
assign      cq_tag             = m_axis_cq_tdata[103:96];

// BAR ID
assign      cq_bar_id          = m_axis_cq_tdata[114:112];
// tran     saction class
assign      cq_tc              = m_axis_cq_tdata[123:121];

assign      cq_lower_addr      = {m_axis_cq_tdata[6:2], 2'b00};
// Write data: Combine DW1 and DW0 of payload
// DW1 (High 32) = [191:160], DW0 (Low 32) = [159:128]
// Driver MUST use writeq() or equivalent 64-bit atomic write
assign      cq_wr_data         = {m_axis_cq_tdata[191:160], m_axis_cq_tdata[159:128]};






endmodule

