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
    input  wire [DATA_WIDTH-1:0]        m_axis_rc_tdata,
    input  wire                         m_axis_rc_tvalid,
    input  wire [74:0]                  m_axis_rc_tuser,
    input  wire [DATA_WIDTH/32-1:0]     m_axis_rc_tkeep,
    input  wire                         m_axis_rc_tlast,
    output wire                         m_axis_rc_tready,

    // =========================================================================
    // User Interface (aligned data output)
    // =========================================================================
    output reg                          rc_valid,
    output reg                          rc_sop,
    output reg                          rc_eop,
    output reg  [255:0]                 rc_data,
    output reg  [7:0]                   rc_keep,
    
    // Descriptor (extracted on SOP, directly to user for e.g. TAG lookup)
    output reg                          rc_desc_valid,
    output reg  [7:0]                   rc_tag,
    output reg  [2:0]                   rc_status,
    output reg  [10:0]                  rc_dword_count,
    output reg  [12:0]                  rc_byte_count,
    output reg                          rc_request_completed
);

    // =========================================================================
    // Internal Signals
    // =========================================================================
    wire        sop_in      = m_axis_rc_tuser[32];
    wire        sop_beat    = m_axis_rc_tvalid && sop_in;
    
    reg [127:0] data_saver;
    reg         pending;            // Have saved data that needs output
    reg [10:0]  dword_count_reg;    // Latched dword count for keep calculation
    
    // Always ready - no backpressure to PCIe
    assign m_axis_rc_tready = 1'b1;

    // =========================================================================
    // Keep Calculation - Based on dword_count % 8
    // For RC, we need to calculate valid DWords in last beat
    // =========================================================================
    // RC payload alignment:
    //   SOP has 4 DW payload, then each beat has 8 DW
    //   Total payload DWs = dword_count
    //   After SOP: remaining = dword_count - 4 (if > 4)
    //   
    // On last beat:
    //   (dword_count - 4) % 8 gives remainder after SOP
    //   0 -> 8 DW valid (0xFF), but actually means aligned
    //   1-7 -> partial
    //
    // Simpler: (dword_count + 4) % 8 (same as RQ direction)
    function [7:0] calc_tail_keep(input [10:0] count);
        case (count & 11'd7)
            11'd0: calc_tail_keep = 8'b1111_1111;  // 8 DW valid, but may need extra beat
            11'd1: calc_tail_keep = 8'b0001_1111;  // 5 total = desc(0) + 5 DW
            11'd2: calc_tail_keep = 8'b0011_1111;
            11'd3: calc_tail_keep = 8'b0111_1111;
            11'd4: calc_tail_keep = 8'b1111_1111;
            11'd5: calc_tail_keep = 8'b0000_0001;  // Extra beat needed
            11'd6: calc_tail_keep = 8'b0000_0011;
            11'd7: calc_tail_keep = 8'b0000_0111;
            default: calc_tail_keep = 8'hFF;
        endcase
    endfunction

    // Check if extra output beat needed after tlast
    // When (dword_count % 8) > 4, the last PCIe beat leaves data in saver
    function one_more(input [10:0] count);
        one_more = (count & 11'd7) > 11'd4;
    endfunction

    // =========================================================================
    // Main Logic
    // =========================================================================
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            rc_valid            <= 1'b0;
            rc_sop              <= 1'b0;
            rc_eop              <= 1'b0;
            rc_data             <= 256'b0;
            rc_keep             <= 8'b0;
            rc_desc_valid       <= 1'b0;
            rc_tag              <= 8'b0;
            rc_status           <= 3'b0;
            rc_dword_count      <= 11'b0;
            rc_byte_count       <= 13'b0;
            rc_request_completed <= 1'b0;
            data_saver          <= 128'b0;
            pending             <= 1'b0;
            dword_count_reg     <= 11'b0;
        end
        else begin
            // Default: clear single-cycle signals
            rc_desc_valid <= 1'b0;
            
            // -----------------------------------------------------------------
            // Case 1: Pending data to flush (after tlast, extra beat needed)
            // -----------------------------------------------------------------
            if (pending && !m_axis_rc_tvalid) begin
                rc_valid    <= 1'b1;
                rc_sop      <= 1'b0;
                rc_eop      <= 1'b1;
                rc_data     <= {128'b0, data_saver};
                rc_keep     <= calc_tail_keep(dword_count_reg);
                pending     <= 1'b0;
                data_saver  <= 128'b0;
            end
            // -----------------------------------------------------------------
            // Case 2: Valid input from PCIe
            // -----------------------------------------------------------------
            else if (m_axis_rc_tvalid) begin
                // -------------------------------------------------------------
                // SOP Beat - Extract descriptor, save payload
                // -------------------------------------------------------------
                if (sop_in) begin
                    // Extract descriptor fields
                    rc_desc_valid       <= 1'b1;
                    rc_tag              <= m_axis_rc_tdata[71:64];
                    rc_status           <= m_axis_rc_tdata[45:43];
                    rc_dword_count      <= m_axis_rc_tdata[42:32];
                    rc_byte_count       <= m_axis_rc_tdata[28:16];
                    rc_request_completed <= m_axis_rc_tdata[30];
                    dword_count_reg     <= m_axis_rc_tdata[42:32];
                    
                    // Save upper 128 bits (first 4 DW of payload)
                    data_saver <= m_axis_rc_tdata[255:128];
                    
                    // Small packet: dword_count <= 4, completes in SOP beat
                    if (m_axis_rc_tdata[42:32] <= 11'd4) begin
                        rc_valid <= 1'b1;
                        rc_sop   <= 1'b1;
                        rc_eop   <= 1'b1;
                        rc_data  <= {128'b0, m_axis_rc_tdata[255:128]};
                        rc_keep  <= calc_tail_keep(m_axis_rc_tdata[42:32]);
                        pending  <= 1'b0;
                    end
                    // Large packet: more beats coming
                    else begin
                        rc_valid <= 1'b0;  // No output yet, wait for next beat
                        rc_sop   <= 1'b0;
                        rc_eop   <= 1'b0;
                        pending  <= 1'b0;
                    end
                end
                // -------------------------------------------------------------
                // Non-SOP Beat - Combine saved + new data
                // -------------------------------------------------------------
                else begin
                    // Output: {new_lower_128, saved_128}
                    rc_valid <= 1'b1;
                    rc_sop   <= (data_saver != 128'b0) && !pending;  // First data output
                    rc_data  <= {m_axis_rc_tdata[127:0], data_saver};
                    
                    // Save new upper 128 bits
                    data_saver <= m_axis_rc_tdata[255:128];
                    
                    // Last PCIe beat
                    if (m_axis_rc_tlast) begin
                        if (one_more(dword_count_reg)) begin
                            // Need extra beat to flush remaining data
                            rc_eop   <= 1'b0;
                            rc_keep  <= 8'hFF;
                            pending  <= 1'b1;
                        end
                        else begin
                            // No extra beat needed
                            rc_eop   <= 1'b1;
                            rc_keep  <= calc_tail_keep(dword_count_reg);
                            pending  <= 1'b0;
                        end
                    end
                    else begin
                        rc_eop  <= 1'b0;
                        rc_keep <= 8'hFF;
                    end
                end
            end
            // -----------------------------------------------------------------
            // Case 3: Idle
            // -----------------------------------------------------------------
            else if (!pending) begin
                rc_valid <= 1'b0;
                rc_sop   <= 1'b0;
                rc_eop   <= 1'b0;
            end
        end
    end

endmodule
