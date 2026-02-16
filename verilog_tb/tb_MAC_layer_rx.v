`timescale 1ns / 1ps

module tb_MAC_layer_rx;

    localparam DATA_WIDTH = 64;
    localparam CTRL_WIDTH = 8;
    localparam CLK_PERIOD = 10;
    // Switches:
    // SOF_SEL  = 0 -> SOF in lane0  (rxc=0000_0001, rxd=...55FB)
    // SOF_SEL  = 1 -> SOF in lane4  (rxc=0001_0000, rxd=5555_55FB_xxxx_xxxx)
    // EOF_LANE = 0..7 -> FD byte lane position in final XGMII word
    // EOF_LANE = 8    -> insert one extra full data word, then put FD at lane0 next cycle
    // DATA_64B_BETWEEN >= 1 -> payload bytes between SOF and EOF, in 8-byte units
    // Example: 1->8 bytes, 2->16 bytes, ...
    parameter SOF_SEL  = 0;
    parameter EOF_LANE = 7;
    parameter DATA_64B_BETWEEN = 2;

    localparam [63:0] IDLE_WORD    = 64'h0707_0707_0707_0707;
    localparam [63:0] SOF_L0_WORD = 64'hD555_5555_5555_55FB;
    // Frame payload pattern is digit bytes (0x30..0x39).
    // FCS bytes are fixed to AA BB CC DD in this TB.
    // START in lane4: first cycle carries ...55FB_xxxx_xxxx, second carries dddd_dddd_d555_5555.
    // Use the same payload bytes as SOF_SEL=0: xxxx_xxxx=4433_2211, dddd_dddd=8877_6655.
    localparam [63:0] SOF_L4_WORD   = 64'h5555_55FB_4433_2211;
    localparam [63:0] SOF_L4_WORD_2 = 64'h8877_6655_D555_5555;

    reg                     i_xgmii_rx_clk;
    reg                     i_xgmii_rx_rst;
    reg  [DATA_WIDTH-1:0]   i_xgmii_rxd;
    reg  [CTRL_WIDTH-1:0]   i_xgmii_rxc;
    reg                     i_rx_status;
    wire [DATA_WIDTH-1:0]   o_axi_rx_data;
    wire                    o_axi_rx_valid;
    wire [CTRL_WIDTH-1:0]   o_axi_rx_keep;
    wire                    o_axi_rx_last;
    reg                     i_axi_rx_ready;

    integer err_count;
    integer cfg_sof_sel;
    integer cfg_eof_lane;
    integer cfg_data_64b_between;

    MAC_layer_rx dut (
        .i_xgmii_rx_clk  (i_xgmii_rx_clk),
        .i_xgmii_rx_rst  (i_xgmii_rx_rst),
        .i_xgmii_rxd     (i_xgmii_rxd),
        .i_xgmii_rxc     (i_xgmii_rxc),
        .i_rx_status     (i_rx_status),
        .o_axi_rx_data   (o_axi_rx_data),
        .o_axi_rx_valid  (o_axi_rx_valid),
        .o_axi_rx_keep   (o_axi_rx_keep),
        .o_axi_rx_last   (o_axi_rx_last),
        .i_axi_rx_ready  (i_axi_rx_ready)
    );

    always #(CLK_PERIOD/2) i_xgmii_rx_clk = ~i_xgmii_rx_clk;

    function [63:0] make_eof_word;
        input integer term_lane;
        integer b;
        reg [63:0] tmp;
        begin
            tmp = 64'd0;
            for (b = 0; b < 8; b = b + 1) begin
                if (b < term_lane) begin
                    if (term_lane >= 4 && b >= term_lane-4) begin
                        // Full CRC in final word: ... AA BB CC DD FD
                        case (b-(term_lane-4))
                            0: tmp[b*8 +: 8] = 8'hAA;
                            1: tmp[b*8 +: 8] = 8'hBB;
                            2: tmp[b*8 +: 8] = 8'hCC;
                            3: tmp[b*8 +: 8] = 8'hDD;
                            default: tmp[b*8 +: 8] = 8'h30;
                        endcase
                    end else if (term_lane < 4) begin
                        // CRC split across previous+final word: final word carries suffix.
                        case ((4-term_lane)+b)
                            0: tmp[b*8 +: 8] = 8'hAA;
                            1: tmp[b*8 +: 8] = 8'hBB;
                            2: tmp[b*8 +: 8] = 8'hCC;
                            3: tmp[b*8 +: 8] = 8'hDD;
                            default: tmp[b*8 +: 8] = 8'h30;
                        endcase
                    end else begin
                        tmp[b*8 +: 8] = 8'h30 + b[7:0] % 10;
                    end
                end else if (b == term_lane) begin
                    tmp[b*8 +: 8] = 8'hFD;
                end else begin
                    tmp[b*8 +: 8] = 8'h07;
                end
            end
            make_eof_word = tmp;
        end
    endfunction

    function [63:0] make_pre_eof_word;
        input integer term_lane;
        integer b;
        integer crc_head_count;
        integer crc_head_start;
        reg [63:0] tmp;
        begin
            tmp = 64'd0;
            crc_head_count = (term_lane < 4) ? (4-term_lane) : 0;
            crc_head_start = 8-crc_head_count;
            for (b = 0; b < 8; b = b + 1) begin
                if (term_lane < 4 && b >= crc_head_start) begin
                    case (b-crc_head_start)
                        0: tmp[b*8 +: 8] = 8'hAA;
                        1: tmp[b*8 +: 8] = 8'hBB;
                        2: tmp[b*8 +: 8] = 8'hCC;
                        3: tmp[b*8 +: 8] = 8'hDD;
                        default: tmp[b*8 +: 8] = 8'h30;
                    endcase
                end else begin
                    tmp[b*8 +: 8] = 8'h30 + b[7:0] % 10;
                end
            end
            make_pre_eof_word = tmp;
        end
    endfunction

    function [7:0] make_eof_ctrl;
        input integer term_lane;
        begin
            make_eof_ctrl = (8'hFF << term_lane);
        end
    endfunction

    function [63:0] make_payload_word;
        input integer word_idx;
        integer b;
        integer d;
        reg [63:0] tmp;
        begin
            tmp = 64'd0;
            for (b = 0; b < 8; b = b + 1) begin
                d = (word_idx*8 + b) % 10;
                tmp[b*8 +: 8] = 8'h30 + d[7:0];
            end
            make_payload_word = tmp;
        end
    endfunction

    task drive_word;
        input [63:0] d;
        input [7:0]  c;
        begin
            @(posedge i_xgmii_rx_clk);
            i_xgmii_rxd <= d;
            i_xgmii_rxc <= c;
        end
    endtask

    task send_idle;
        input integer cycles;
        integer k;
        begin
            for (k = 0; k < cycles; k = k + 1) begin
                drive_word(IDLE_WORD, 8'hFF);
            end
        end
    endtask

    task send_payload_beats;
        input integer beats;
        input integer start_idx;
        integer j;
        begin
            for (j = 0; j < beats; j = j + 1) begin
                drive_word(make_payload_word(start_idx + j), 8'h00);
            end
        end
    endtask

    task send_one_frame;
        input integer sof_sel;
        input integer term_lane;
        input integer data_64b_between;
        integer eof_check_lane;
        integer beats_to_send;
        integer payload_start_idx;
        begin
            if (sof_sel == 0) begin
                drive_word(SOF_L0_WORD, 8'b0000_0001);
                #1;
                if (dut.sof_location !== 2'b01) begin
                    err_count = err_count + 1;
                    $display("[%0t] ERROR: SOF lane0 detect mismatch. sof_location=%b", $time, dut.sof_location);
                end
                payload_start_idx = 0;
            end else begin
                drive_word(SOF_L4_WORD, 8'b0001_0000);
                #1;
                if (dut.sof_location !== 2'b10) begin
                    err_count = err_count + 1;
                    $display("[%0t] ERROR: SOF lane4 detect mismatch. sof_location=%b", $time, dut.sof_location);
                end
                // For START in lane 4, provide the continuation so preamble/SFD spans two cycles.
                drive_word(SOF_L4_WORD_2, 8'h00);
                payload_start_idx = 1;
            end

            beats_to_send = data_64b_between;
            if (term_lane == 8) begin
                // Extra full payload beat between SOF and EOF.
                beats_to_send = beats_to_send + 1;
                eof_check_lane = 0;
            end else begin
                eof_check_lane = term_lane;
            end
            send_payload_beats(beats_to_send, payload_start_idx);
            if (eof_check_lane < 4) begin
                // Need one extra data beat to carry CRC prefix bytes.
                drive_word(make_pre_eof_word(eof_check_lane), 8'h00);
            end
            drive_word(make_eof_word(eof_check_lane), make_eof_ctrl(eof_check_lane));
            #1;
            if (dut.eof_location !== (8'b0000_0001 << eof_check_lane)) begin
                err_count = err_count + 1;
                $display("[%0t] ERROR: EOF lane%0d detect mismatch. eof_location=%b",
                         $time, eof_check_lane, dut.eof_location);
            end

            send_idle(2);
        end
    endtask

    initial begin
        i_xgmii_rx_clk = 1'b0;
        i_xgmii_rx_rst = 1'b1;
        i_xgmii_rxd = IDLE_WORD;
        i_xgmii_rxc = 8'hFF;
        i_rx_status = 1'b1;
        i_axi_rx_ready = 1'b0;
        err_count = 0;
        cfg_sof_sel = SOF_SEL;
        cfg_eof_lane = EOF_LANE;
        cfg_data_64b_between = DATA_64B_BETWEEN;

        if ($value$plusargs("SOF_SEL=%d", cfg_sof_sel)) begin end
        if ($value$plusargs("EOF_LANE=%d", cfg_eof_lane)) begin end
        if ($value$plusargs("DATA_64B_BETWEEN=%d", cfg_data_64b_between)) begin end

        repeat (4) @(posedge i_xgmii_rx_clk);
        i_xgmii_rx_rst = 1'b0;
        i_axi_rx_ready = 1'b1;

        if ((cfg_sof_sel < 0) || (cfg_sof_sel > 1)) begin
            $display("FATAL: SOF_SEL must be 0 or 1, got %0d", cfg_sof_sel);
            $finish;
        end
        if ((cfg_eof_lane < 0) || (cfg_eof_lane > 8)) begin
            $display("FATAL: EOF_LANE must be 0..8, got %0d", cfg_eof_lane);
            $finish;
        end
        if (cfg_data_64b_between < 1) begin
            $display("FATAL: DATA_64B_BETWEEN must be >= 1, got %0d", cfg_data_64b_between);
            $finish;
        end

        $display("Running one-frame test: SOF_SEL=%0d, EOF_LANE=%0d, DATA_64B_BETWEEN=%0d",
                 cfg_sof_sel, cfg_eof_lane, cfg_data_64b_between);

        send_idle(3);
        send_one_frame(cfg_sof_sel, cfg_eof_lane, cfg_data_64b_between);

        send_idle(6);

        if (err_count == 0) begin
            $display("PASS: Selected SOF/EOF pattern generated and detected.");
        end else begin
            $display("FAIL: %0d pattern checks failed.", err_count);
        end

        $finish;
    end

    initial begin
        $dumpfile("tb_MAC_layer_rx.vcd");
        $dumpvars(0, tb_MAC_layer_rx);
    end

endmodule
