// MAC RX: XGMII -> AXI Stream (frame delineation, tdata/tkeep/tvalid/tlast).
// Optionally strips preamble+SFD (first 8 bytes) and/or the 4-byte FCS (CRC) from each frame.
// First byte of frame at [63:56] (big-endian style) per project convention.
`timescale 1ns / 1ps

module MAC_layer_rx #(
    parameter DATA_WIDTH = 64,
    parameter CTRL_WIDTH  = 8,
    parameter STRIP_CRC       = 1,   // 1 = remove last 4 bytes (FCS) from AXI output; 0 = pass through
    parameter STRIP_PREAMBLE  = 1    // 1 = remove first 8 bytes (7×preamble + SFD); 0 = pass through
)(
    input  wire                     i_xgmii_rx_clk   ,
    input  wire                     i_xgmii_rx_rst   ,
    input  wire [DATA_WIDTH-1:0]    i_xgmii_rxd      ,
    input  wire [CTRL_WIDTH-1:0]    i_xgmii_rxc      ,
    input  wire                     i_rx_status      ,

    output reg [DATA_WIDTH-1:0]     axi_rx_data      ,
    output reg                      axi_rx_valid     ,
    output reg [CTRL_WIDTH-1:0]     axi_rx_keep       ,
    output reg                      axi_rx_last      ,
    input  wire                     axi_rx_ready
);

    localparam XGMII_START = 8'hFB;
    localparam XGMII_TERM  = 8'hFD;

    // -------------------------------------------------------------------------
    // SOF / EOF detection (combo)
    // -------------------------------------------------------------------------
    wire [7:0] xgmii_term;
    genvar   j;
    generate
        for (j = 0; j < 8; j = j + 1) begin: terminate
            assign xgmii_term[j] = i_xgmii_rxc[j] & (i_xgmii_rxd[j*8 +: 8] == XGMII_TERM);
        end
    endgenerate

    wire sof_lane0 = i_xgmii_rxc[0] & (i_xgmii_rxd[7:0] == XGMII_START);
    wire sof_lane4 = i_xgmii_rxc[4] & (i_xgmii_rxd[39:32] == XGMII_START);
    wire sof      = sof_lane0 | sof_lane4;

    // Mask control to 0 for payload
    wire [DATA_WIDTH-1:0] xgmii_rxd_masked;
    genvar gi;
    generate
        for (gi = 0; gi < 8; gi = gi + 1) begin : gen_mask
            assign xgmii_rxd_masked[gi*8 +: 8] = i_xgmii_rxc[gi] ? 8'd0 : i_xgmii_rxd[gi*8 +: 8];
        end
    endgenerate

    // -------------------------------------------------------------------------
    // Pipeline: lane swap when START in lane 4 (like Corundum), then 1-cycle delay
    // -------------------------------------------------------------------------
    reg lanes_swapped;
    reg [31:0] swap_rxd;
    reg [DATA_WIDTH-1:0] xgmii_rxd_d0;
    reg [DATA_WIDTH-1:0] xgmii_rxd_d1;
    reg                  xgmii_start_d0;
    reg                  xgmii_start_d1;
    reg [7:0]            detect_term_d0;
    reg [7:0]            detect_term_d1;

    // Byte-reverse 64-bit for big-endian: first frame byte at [63:56]
    wire [63:0] rev64 = { xgmii_rxd_d1[ 7: 0], xgmii_rxd_d1[15: 8], xgmii_rxd_d1[23:16], xgmii_rxd_d1[31:24],
                          xgmii_rxd_d1[39:32], xgmii_rxd_d1[47:40], xgmii_rxd_d1[55:48], xgmii_rxd_d1[63:56] };

    localparam STATE_IDLE    = 2'd0;
    localparam STATE_PAYLOAD = 2'd1;
    localparam STATE_LAST    = 2'd2;

    reg [1:0] state;
    wire stall = axi_rx_valid & ~axi_rx_ready;

    reg [7:0] last_beat_tkeep_reg;
    reg [3:0] swap_rxc_term;  // upper 4 lanes term from previous cycle (for START-in-lane4 path)
    reg       skip_first_beat;  // when STRIP_PREAMBLE: skip first 8-byte beat (preamble+SFD)
    reg       need_first_beat;   // gap after last: first beat of new frame is output next cycle
    reg [63:0] first_beat_data;

    // Strip FCS: last 4 bytes of frame are CRC; keep only bytes 0..(K-4) on last beat when K>4
    wire [7:0] last_beat_keep_stripped = (last_beat_tkeep_reg > 8'h0F) ? (last_beat_tkeep_reg >> 4) : 8'd0;

    always @(posedge i_xgmii_rx_clk) begin
        if (i_xgmii_rx_rst) begin
            state                   <= STATE_IDLE;
            axi_rx_data             <= 64'd0;
            axi_rx_valid            <= 1'b0;
            axi_rx_keep             <= 8'd0;
            axi_rx_last             <= 1'b0;
            lanes_swapped           <= 1'b0;
            xgmii_rxd_d0            <= 64'd0;
            xgmii_rxd_d1            <= 64'd0;
            xgmii_start_d0          <= 1'b0;
            xgmii_start_d1          <= 1'b0;
            detect_term_d0          <= 8'd0;
            detect_term_d1          <= 8'd0;
            last_beat_tkeep_reg     <= 8'd0;
            swap_rxc_term           <= 4'd0;
            skip_first_beat         <= 1'b0;
            need_first_beat         <= 1'b0;
            first_beat_data         <= 64'd0;
        end else if (!stall) begin
            // Pipeline: capture current XGMII
            if (sof_lane0) begin
                lanes_swapped <= 1'b0;
                xgmii_rxd_d0  <= xgmii_rxd_masked;
                xgmii_start_d0 <= 1'b1;
                detect_term_d0 <= xgmii_term;
            end else if (sof_lane4) begin
                lanes_swapped <= 1'b1;
                xgmii_rxd_d0  <= {xgmii_rxd_masked[31:0], swap_rxd};
                xgmii_start_d0 <= 1'b1;
                detect_term_d0 <= {xgmii_term[3:0], swap_rxc_term};
            end else if (lanes_swapped) begin
                xgmii_rxd_d0  <= {xgmii_rxd_masked[31:0], swap_rxd};
                xgmii_start_d0 <= 1'b0;
                detect_term_d0 <= {xgmii_term[3:0], swap_rxc_term};
            end else begin
                xgmii_rxd_d0  <= xgmii_rxd_masked;
                xgmii_start_d0 <= 1'b0;
                detect_term_d0 <= xgmii_term;
            end

            swap_rxd     <= xgmii_rxd_masked[63:32];
            swap_rxc_term<= xgmii_term[7:4];

            xgmii_rxd_d1   <= xgmii_rxd_d0;
            xgmii_start_d1 <= xgmii_start_d0;
            detect_term_d1 <= detect_term_d0;

            // FSM
            case (state)
                STATE_IDLE: begin
                    if (xgmii_start_d1) begin
                        state <= STATE_PAYLOAD;
                        skip_first_beat <= (STRIP_PREAMBLE != 0);
                        need_first_beat <= 1'b1;
                        first_beat_data <= rev64;
                        axi_rx_data  <= 64'd0;
                        axi_rx_valid <= 1'b0;   // gap: valid=0 one period after last
                        axi_rx_keep  <= 8'd0;
                        axi_rx_last  <= 1'b0;
                    end else begin
                        axi_rx_data  <= 64'd0;
                        axi_rx_valid <= 1'b0;
                        axi_rx_keep  <= 8'd0;
                        axi_rx_last  <= 1'b0;
                    end
                end
                STATE_PAYLOAD: begin
                    if (need_first_beat) begin
                        need_first_beat <= 1'b0;
                        if (skip_first_beat) begin
                            skip_first_beat <= 1'b0;
                            if (|detect_term_d1) begin
                                state <= STATE_IDLE;
                                axi_rx_valid <= 1'b0;
                                axi_rx_keep  <= 8'd0;
                                axi_rx_last  <= 1'b0;
                            end else begin
                                state <= STATE_PAYLOAD;
                                axi_rx_data  <= rev64;
                                axi_rx_valid <= 1'b1;
                                axi_rx_keep  <= 8'hFF;
                                axi_rx_last  <= 1'b0;
                            end
                        end else begin
                            state <= STATE_PAYLOAD;
                            axi_rx_data  <= first_beat_data;
                            axi_rx_valid <= 1'b1;
                            axi_rx_keep  <= 8'hFF;
                            axi_rx_last  <= 1'b0;
                        end
                    end else if (skip_first_beat) begin
                        skip_first_beat <= 1'b0;
                        if (|detect_term_d1) begin
                            // Frame ended in first 8 bytes (preamble only); nothing to output
                            state <= STATE_IDLE;
                            axi_rx_valid <= 1'b0;
                            axi_rx_keep  <= 8'd0;
                            axi_rx_last  <= 1'b0;
                        end else begin
                            // First payload beat after stripping preamble: bytes 8–15 (dest MAC, etc.)
                            state <= STATE_PAYLOAD;
                            axi_rx_data  <= rev64;
                            axi_rx_valid <= 1'b1;
                            axi_rx_keep  <= 8'hFF;
                            axi_rx_last  <= 1'b0;
                        end
                    end else if (|detect_term_d1) begin
                        if (detect_term_d1[7:5] != 3'd0) begin
                            // TERM in lane 5,6,7: last beat appears in d1 next cycle; defer to STATE_LAST
                            state <= STATE_LAST;
                            axi_rx_valid <= 1'b0;
                            last_beat_tkeep_reg <= detect_term_d1[5] ? 8'h1F :
                                                   (detect_term_d1[6] ? 8'h3F : 8'h7F);
                        end else begin
                            // TERM in lane 0-4: output last beat now (0..4 bytes; all FCS if STRIP_CRC)
                            state <= STATE_IDLE;
                            axi_rx_data  <= rev64;
                            axi_rx_valid <= 1'b1;
                            axi_rx_keep  <= (STRIP_CRC != 0) ? 8'd0 :
                                            (detect_term_d1[0] ? 8'd0  :
                                            (detect_term_d1[1] ? 8'h01 :
                                            (detect_term_d1[2] ? 8'h03 :
                                            (detect_term_d1[3] ? 8'h07 : 8'h0F))));
                            axi_rx_last  <= 1'b1;
                        end
                    end else begin
                        state <= STATE_PAYLOAD;
                        axi_rx_data  <= rev64;
                        axi_rx_valid <= 1'b1;
                        axi_rx_keep  <= 8'hFF;
                        axi_rx_last  <= 1'b0;
                    end
                end
                STATE_LAST: begin
                    state <= STATE_IDLE;
                    axi_rx_data  <= rev64;
                    axi_rx_valid <= 1'b1;
                    axi_rx_keep  <= (STRIP_CRC != 0) ? last_beat_keep_stripped : last_beat_tkeep_reg;
                    axi_rx_last  <= 1'b1;
                end
                default: begin
                    state <= STATE_IDLE;
                    axi_rx_data  <= 64'd0;
                    axi_rx_valid <= 1'b0;
                    axi_rx_keep  <= 8'd0;
                    axi_rx_last  <= 1'b0;
                end
            endcase
        end
        // when stall: keep state and outputs unchanged; pipeline still advances (XGMII has no back-pressure)
        // For no-drop behavior, add an input FIFO between XGMII and this block.
    end

endmodule
