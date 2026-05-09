`timescale 1ns / 1ps

module tb_order_book_parser;

    localparam DATA_WIDTH  = 64;
    localparam CTRL_WIDTH  = 8;
    localparam BAR0_SIZE   = 16;
    localparam CLK_PERIOD  = 10;
    localparam FRAME_BYTES = 296;
    localparam NUM_ROUNDS  = 3;

    localparam [63:0] IDLE_WORD  = 64'h0707_0707_0707_0707;
    localparam [63:0] START_WORD = 64'hFB55_5555_5555_55D5;

    reg                         i_clk_156;
    reg                         i_rst;
    reg  [DATA_WIDTH-1:0]       i_axi_rx_data;
    reg                         i_axi_rx_valid;
    reg  [CTRL_WIDTH-1:0]       i_axi_rx_keep;
    reg                         i_axi_rx_last;
    reg  [63:0]                 i_axi_rx_ingress_tick;

    wire                        o_axi_rx_ready;
    wire                        o_msg_valid;
    wire [63:0]                 o_seq_num;
    wire [63:0]                 o_rx_ingress_tick;
    wire [7:0]                  o_msg_type;
    wire [15:0]                 o_stock_locate;
    wire [63:0]                 o_order_ref_num;
    wire [63:0]                 o_new_order_ref_num;
    wire [7:0]                  o_buy_sell;
    wire [31:0]                 o_shares;
    wire [31:0]                 o_price;
    wire [47:0]                 o_timestamp;

    reg [7:0] frame_bytes [0:FRAME_BYTES-1];
    integer r;

    order_book_parser #(
        .DATA_WIDTH (DATA_WIDTH),
        .CTRL_WIDTH (CTRL_WIDTH),
        .BAR0_SIZE  (BAR0_SIZE)
    ) dut (
        .i_clk_156          (i_clk_156),
        .i_rst              (i_rst),
        .i_axi_rx_data      (i_axi_rx_data),
        .i_axi_rx_valid     (i_axi_rx_valid),
        .i_axi_rx_keep      (i_axi_rx_keep),
        .i_axi_rx_last      (i_axi_rx_last),
        .i_axi_rx_ingress_tick(i_axi_rx_ingress_tick),
        .o_axi_rx_ready     (o_axi_rx_ready),
        .o_msg_valid        (o_msg_valid),
        .o_seq_num          (o_seq_num),
        .o_rx_ingress_tick  (o_rx_ingress_tick),
        .o_msg_type         (o_msg_type),
        .o_stock_locate     (o_stock_locate),
        .o_order_ref_num    (o_order_ref_num),
        .o_new_order_ref_num(o_new_order_ref_num),
        .o_buy_sell         (o_buy_sell),
        .o_shares           (o_shares),
        .o_price            (o_price),
        .o_timestamp        (o_timestamp)
    );

    always #(CLK_PERIOD/2) i_clk_156 = ~i_clk_156;

    task automatic drive_cycle;
        input        vld;
        input [63:0] dat;
        input        lst;
        begin
            @(posedge i_clk_156);
            i_axi_rx_valid        <= vld;
            i_axi_rx_data         <= dat;
            i_axi_rx_last         <= lst;
            i_axi_rx_keep         <= 8'hFF;
            i_axi_rx_ingress_tick <= 64'h1;
        end
    endtask

    task automatic set16;
        input integer off;
        input [127:0] line;
        integer i;
        begin
            for (i = 0; i < 16; i = i + 1) begin
                frame_bytes[off+i] = line[127-(i*8) -: 8];
            end
        end
    endtask

    task automatic set2;
        input integer off;
        input [15:0] line;
        begin
            frame_bytes[off+0] = line[15:8];
            frame_bytes[off+1] = line[7:0];
        end
    endtask

    task automatic set8;
        input integer off;
        input [63:0] line;
        integer i;
        begin
            for (i = 0; i < 8; i = i + 1) begin
                frame_bytes[off+i] = line[63-(i*8) -: 8];
            end
        end
    endtask

    task automatic load_snapshot_bytes;
        integer i;
        begin
            for (i = 0; i < FRAME_BYTES; i = i + 1) begin
                frame_bytes[i] = 8'h00;
            end

            set16('h0000, 128'h01_00_5e_00_00_01_00_11_22_33_44_55_08_00_45_00);
            set16('h0010, 128'h01_1a_00_01_00_00_40_11_cc_f3_c0_a8_01_32_e9_01);
            set16('h0020, 128'h02_03_15_b3_04_d2_01_06_f6_49_4e_41_53_44_51_54);
            set16('h0030, 128'h45_53_54_20_00_00_00_00_00_00_00_01_00_07_00_28);
            set16('h0040, 128'h46_00_0d_00_00_1b_d6_53_cf_f0_bf_00_00_00_00_00);
            set16('h0050, 128'h00_00_01_42_00_00_02_64_41_41_50_4c_20_20_20_20);
            set16('h0060, 128'h00_23_5d_e8_54_53_53_4d_00_17_58_00_0d_00_00_1b);
            set16('h0070, 128'h2e_c7_3b_cc_cf_00_00_00_00_00_00_00_01_00_00_00);
            set16('h0080, 128'h96_00_24_43_00_0d_00_01_1f_1a_fd_1d_81_79_00_00);
            set16('h0090, 128'h00_00_00_00_00_01_00_00_00_df_00_00_00_00_00_02);
            set16('h00a0, 128'h8c_56_4e_00_23_5e_b0_00_1f_45_00_0d_00_02_0d_73);
            set16('h00b0, 128'h25_a3_d0_68_00_00_00_00_00_00_00_01_00_00_00_16);
            set16('h00c0, 128'h00_00_00_00_00_00_46_27_00_24_41_00_0d_00_00_0d);
            set16('h00d0, 128'h18_c3_4e_77_da_00_00_00_00_00_00_00_02_42_00_00);
            set16('h00e0, 128'h00_10_41_41_50_4c_20_20_20_20_00_23_fa_28_00_23);
            set16('h00f0, 128'h55_00_0d_00_00_1e_96_69_a4_b6_a9_00_00_00_00_00);
            set16('h0100, 128'h00_00_02_00_00_00_00_00_00_00_03_00_00_00_64_00);
            set16('h0110, 128'h24_9f_00_00_13_44_00_0d_00_00_0d_20_7f_24_3f_50);
            set8 ('h0120, 64'h00_00_00_00_00_00_00_03);
        end
    endtask




    task automatic send_snapshot_frame;
        integer idx;
        integer b;
        reg [63:0] beat;
        begin
            drive_cycle(1'b0, IDLE_WORD, 1'b0);
            drive_cycle(1'b0, START_WORD, 1'b0);

            for (idx = 0; idx < FRAME_BYTES; idx = idx + 8) begin
                beat = IDLE_WORD;
                for (b = 0; b < 8; b = b + 1) begin
                    if ((idx + b) < FRAME_BYTES) begin
                        beat[63-(8*b) -: 8] = frame_bytes[idx+b];
                    end
                end
                drive_cycle(1'b1, beat, ((idx + 8) >= FRAME_BYTES));
            end

            drive_cycle(1'b0, IDLE_WORD, 1'b0);
            drive_cycle(1'b0, IDLE_WORD, 1'b0);
        end
    endtask

    always @(posedge i_clk_156) begin
        if (o_msg_valid) begin
            $display("[%0t] msg_valid type=%02h locate=%04h ts=%012h ref=%016h new_ref=%016h side=%02h shares=%08h price=%08h seq=%016h",
                     $time, o_msg_type, o_stock_locate, o_timestamp, o_order_ref_num,
                     o_new_order_ref_num, o_buy_sell, o_shares, o_price, o_seq_num);
        end
    end

    initial begin
        i_clk_156           = 1'b0;
        i_rst               = 1'b1;
        i_axi_rx_data       = IDLE_WORD;
        i_axi_rx_valid      = 1'b0;
        i_axi_rx_keep       = 8'hFF;
        i_axi_rx_last       = 1'b0;
        i_axi_rx_ingress_tick = 64'h1;

        load_snapshot_bytes();

        repeat (5) @(posedge i_clk_156);
        i_rst <= 1'b0;
        repeat (3) @(posedge i_clk_156);

        for (r = 0; r < NUM_ROUNDS; r = r + 1) begin
            $display("[%0t] replay round %0d/%0d", $time, r+1, NUM_ROUNDS);
            send_snapshot_frame();
            repeat (8) @(posedge i_clk_156);
        end

        repeat (40) @(posedge i_clk_156);
        $finish;
    end

    initial begin
        $dumpvars(0, tb_order_book_parser);
    end

endmodule
