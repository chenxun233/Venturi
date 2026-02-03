// =============================================================================
// RC_gearbox256 - Data Realignment for RC Channel (Host -> FPGA)
// =============================================================================
// Counterpart to RQ_gearbox256. Realigns incoming completion data so user
// logic receives clean 256-bit words.
//
// PCIe RC format:
//   SOP beat:     [255:128]=payload, [127:96]=reserved, [95:0]=descriptor
//   Non-SOP beat: [255:0]=payload
//
// Problem: SOP beat only has 128 bits of payload (4 DW), subsequent beats
// have 256 bits (8 DW). User logic wants aligned 256-bit chunks.
//
// Solution: Buffer upper 128 bits, shift left on subsequent beats.
// =============================================================================

module RC_gearbox256 #(
    parameter DATA_WIDTH = 256
)(
    input  wire                         clk,
    input  wire                         rst_n,

    // =========================================================================
    // PCIe IP Core Interface (from RC)
    // =========================================================================
    (* MARK_DEBUG = "TRUE" *)   input  wire [DATA_WIDTH-1:0]        m_axis_rc_tdata,
    (* MARK_DEBUG = "TRUE" *)   input  wire                         m_axis_rc_tvalid,
    (* MARK_DEBUG = "TRUE" *)   input  wire [74:0]                  m_axis_rc_tuser,
    (* MARK_DEBUG = "TRUE" *)   input  wire [DATA_WIDTH/32-1:0]     m_axis_rc_tkeep,
    (* MARK_DEBUG = "TRUE" *)   input  wire                         m_axis_rc_tlast,
    (* MARK_DEBUG = "TRUE" *)   output wire                         m_axis_rc_tready,

    // =========================================================================
    // User Interface 
    // =========================================================================
    // data, can last for multiple beats
    (* MARK_DEBUG = "TRUE" *)   output reg                          rc_valid,
    (* MARK_DEBUG = "TRUE" *)   output reg                          rc_payload_last,
    (* MARK_DEBUG = "TRUE" *)   output wire  [255:0]                rc_payload     ,
    (* MARK_DEBUG = "TRUE" *)   output reg   [7:0]                  rc_payload_dw_keep,
    // Descriptor, taks [95:0] tdata, only last for the first beat (SOP). Thus, no last signal.
    (* MARK_DEBUG = "TRUE" *)   output reg   [95:0]                 rc_descriptor
);




// =========================================================================
// Internal Signals
// =========================================================================
    (* MARK_DEBUG = "TRUE" *)   wire   sop     = m_axis_rc_tvalid? m_axis_rc_tuser[32] : 1'b0; // SOP indicator
    (* MARK_DEBUG = "TRUE" *)   reg [159:0]    data_saver           ;
    (* MARK_DEBUG = "TRUE" *)   reg [7:0]      rc_last_keep         ;
// save the current state   ==========================

assign rc_payload = {m_axis_rc_tdata[95:0], data_saver};

always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
        rc_valid            <= 1'b0;
        rc_payload_last     <= 1'b0;
        rc_payload_dw_keep  <= 8'h0;
        rc_descriptor       <= 96'h0;
        data_saver          <= 160'h0;
        rc_last_keep        <= 8'h0;
    end else if (m_axis_rc_tvalid) begin
            rc_valid            <= 1'b1;
            data_saver          <= m_axis_rc_tdata[255:96];
            if (sop) begin
                rc_last_keep        <= calc_tail_keep(m_axis_rc_tdata[42:32]);
                rc_descriptor       <= m_axis_rc_tdata[95:0];
            end
            if (m_axis_rc_tdata[42:32] <11'd8) begin
                rc_payload_last     <= 1'b1;
                rc_payload_dw_keep  <= calc_tail_keep(m_axis_rc_tdata[42:32]);  
            end else if (!m_axis_rc_tlast) begin // need more than one cycle
                rc_payload_last     <= 1'b0;
                rc_payload_dw_keep  <= 8'hFF;
            end else if (m_axis_rc_tlast) begin // need more than one cycle
                rc_payload_last     <= 1'b1;
                rc_payload_dw_keep  <= rc_last_keep;
            end
    end else begin
    data_saver          <= 160'h0;
    rc_valid            <= 1'b0;
    rc_payload_last     <= 1'b0;
    rc_payload_dw_keep  <= 8'h0;
    rc_descriptor       <= 96'h0;
    rc_last_keep        <= 8'h0;

    end
end



      
    
    assign m_axis_rc_tready = 1'b1;
// =========================================================================
    // function [7:0] calc_tail_keep(input [DATA_WIDTH/32-1:0] m_axis_rc_tkeep);
    // // rc_payload_byte_count is in bytes, convert to DW first.
    //     case (m_axis_rc_tkeep)
    //         8'b1111_1111: calc_tail_keep = 8'b0001_1111; 
    //         8'b0000_0001: calc_tail_keep = 8'b0011_1111;  
    //         8'b0000_0011: calc_tail_keep = 8'b0111_1111;
    //         8'b0000_0111: calc_tail_keep = 8'b1111_1111;
    //         8'b0000_1111: calc_tail_keep = 8'b0000_0001;
    //         8'b0001_1111: calc_tail_keep = 8'b0000_0011;  
    //         8'b0011_1111: calc_tail_keep = 8'b0000_0111;
    //         8'b0111_1111: calc_tail_keep = 8'b0000_1111;
    //         default: calc_tail_keep = 8'hFF;
    //     endcase
    // endfunction

    function [7:0] calc_tail_keep(input [10:0] dw_count);
    // rc_payload_byte_count is in bytes, convert to DW first.
        case (dw_count[2:0])
            3'd5: calc_tail_keep = 8'b0001_1111; 
            3'd6: calc_tail_keep = 8'b0011_1111;  
            3'd7: calc_tail_keep = 8'b0111_1111;
            3'd0: calc_tail_keep = 8'b1111_1111;
            3'd1: calc_tail_keep = 8'b0000_0001;
            3'd2: calc_tail_keep = 8'b0000_0011;  
            3'd3: calc_tail_keep = 8'b0000_0111;
            3'd4: calc_tail_keep = 8'b0000_1111;
            default: calc_tail_keep = 8'hFF;
        endcase
    endfunction
    // Check if extra output beat needed after tlast
// =========================================================================




endmodule
