module RC_gearbox256 #(
    parameter DATA_WIDTH = 256
)(
    input  wire                         clk,
    input  wire                         rst_n,

    // PCIe IP Core Interface
    input  wire [DATA_WIDTH-1:0]        m_axis_rc_tdata,
    input  wire                         m_axis_rc_tvalid,
    input  wire [74:0]                  m_axis_rc_tuser,
    input  wire [DATA_WIDTH/32-1:0]     m_axis_rc_tkeep,
    input  wire                         m_axis_rc_tlast,
    output wire                         m_axis_rc_tready,

    // User Interface 
    output reg                          rc_valid,
    output reg                          rc_payload_last,
    output reg  [255:0]                 rc_payload,
    output reg  [7:0]                   rc_payload_dw_keep,
    output reg  [95:0]                  rc_descriptor // Valid only when rc_valid=1 (First Beat)
);

    // =========================================================================
    // Internal Signals
    // =========================================================================
    reg [159:0] data_saver;
    
    // Captured State (Must be held constant throughout the packet)
    reg [7:0]   r_last_keep;
    reg         r_extra_cycle_needed;
    reg [95:0]  r_desc_reg;

    // Helper Wires
    wire        sop = m_axis_rc_tvalid && m_axis_rc_tuser[32]; // Standard IP SOP
    wire [12:0] current_byte_count = m_axis_rc_tdata[28:16];   // Valid only on SOP

    // FSM States
    localparam STATE_IDLE   = 2'd0;
    localparam STATE_ACTIVE = 2'd1;
    localparam STATE_FLUSH  = 2'd2;
    reg [1:0] state;

    // =========================================================================
    // Helper Functions
    // =========================================================================
    function [7:0] calc_tail_keep(input [12:0] bc);
        case ((bc >> 2) & 11'd7) // (ByteCount / 4) % 8
            11'd1: calc_tail_keep = 8'b0000_0001;  
            11'd2: calc_tail_keep = 8'b0000_0011;
            11'd3: calc_tail_keep = 8'b0000_0111;
            11'd4: calc_tail_keep = 8'b0000_1111;
            11'd5: calc_tail_keep = 8'b0001_1111;  
            11'd6: calc_tail_keep = 8'b0011_1111;
            11'd7: calc_tail_keep = 8'b0111_1111;
            default: calc_tail_keep = 8'hFF; // 0 or >7 (Full beat)
        endcase
    endfunction

    // Does the packet have a remainder > 3 DWords (96 bits)?
    // If yes, after we steal 96 bits for alignment, we still have data left for an extra beat.
    function check_extra_cycle(input [12:0] bc);
        // We look at the remainder modulo 8.
        // If remainder is 1,2,3 -> It fits in the previous cycle alignment.
        // If remainder is 4,5,6,7,0 -> We need an extra cycle to flush.
        // Note: 0 (Exact multiple of 8) implies full 256 bits, so yes, we need the flush cycle
        // because we shifted everything by 96 bits.
        // Actually, simpler logic:
        // Total DWords % 8.
        // We consumed 3 DWords in the shift.
        // If (Total % 8) > 3 or == 0, we need another cycle.
        reg [3:0] mod8;
        mod8 = (bc >> 2) & 4'd7; 
        check_extra_cycle = (mod8 > 3) || (mod8 == 0);
    endfunction

    // =========================================================================
    // Main Logic
    // =========================================================================
    
    // 1. Data Saver & Descriptor Capture
    always @(posedge clk) begin
        if (m_axis_rc_tvalid) begin
            // On SOP, we capture the upper 160 bits (Data starts at bit 96)
            // On Body, we capture upper 160 bits (Data continues)
            data_saver <= m_axis_rc_tdata[255:96];
        end
        
        if (sop) begin
            r_desc_reg           <= m_axis_rc_tdata[95:0];
            r_last_keep          <= calc_tail_keep(current_byte_count);
            r_extra_cycle_needed <= check_extra_cycle(current_byte_count);
        end
    end

    // 2. State Machine & Output Generation
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state               <= STATE_IDLE;
            rc_valid            <= 1'b0;
            rc_payload_last     <= 1'b0;
            rc_payload          <= 256'b0;
            rc_payload_dw_keep  <= 8'b0;
            rc_descriptor       <= 96'b0;
        end 
        else begin
            // Default assignments
            rc_valid <= 1'b0; 
            rc_payload_last <= 1'b0;

            case (state)
                STATE_IDLE: begin
                    if (sop) begin
                        // Load Gearbox only. Do NOT output valid yet.
                        // If it's a single-beat packet (SOP + TLAST), go directly to flush
                        if (m_axis_rc_tlast) begin
                            state <= STATE_FLUSH;
                        end else begin
                            state <= STATE_ACTIVE;
                        end
                    end
                end

                STATE_ACTIVE: begin
                    if (m_axis_rc_tvalid) begin
                        // Output: Combined {New_Low, Old_High}
                        rc_payload         <= {m_axis_rc_tdata[95:0], data_saver};
                        rc_valid           <= 1'b1;
                        rc_descriptor      <= r_desc_reg; // Keep descriptor valid
                        
                        if (m_axis_rc_tlast) begin
                            // This is the last input beat.
                            // Do we need one more cycle to flush the `data_saver` we just updated?
                            if (r_extra_cycle_needed) begin
                                rc_payload_dw_keep <= 8'hFF; // Current beat is full
                                state              <= STATE_FLUSH;
                            end else begin
                                rc_payload_dw_keep <= r_last_keep; // This is the end
                                rc_payload_last    <= 1'b1;
                                state              <= STATE_IDLE;
                            end
                        end else begin
                            rc_payload_dw_keep <= 8'hFF;
                            // Stay in ACTIVE
                        end
                    end
                end

                STATE_FLUSH: begin
                    // Output: {0, Old_High}
                    rc_payload          <= {96'b0, data_saver}; 
                    rc_valid            <= 1'b1;
                    rc_payload_last     <= 1'b1;
                    rc_payload_dw_keep  <= r_last_keep;
                    rc_descriptor       <= r_desc_reg;
                    state               <= STATE_IDLE;
                end
            endcase
        end
    end

    // Always ready to accept data
    assign m_axis_rc_tready = 1'b1;

endmodule