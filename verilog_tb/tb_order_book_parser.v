`timescale 1ns / 1ps

module tb_order_book_parser;

    localparam DATA_WIDTH  = 64;
    localparam CTRL_WIDTH  = 8;
    localparam BAR0_SIZE   = 16;
    localparam CLK_PERIOD  = 10;
    localparam FRAME_BYTES = 258;
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

    reg  [47:0]                 i_ctl_dst_mac;
    reg  [31:0]                 i_ctl_dst_ip;
    reg  [15:0]                 i_ctl_dst_port;
    reg                         i_promiscuous;
    reg                         i_sync_fire;
    reg  [BAR0_SIZE-1:0]        i_ctl_reg;

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
        .o_timestamp        (o_timestamp),
        .i_ctl_dst_mac      (i_ctl_dst_mac),
        .i_ctl_dst_ip       (i_ctl_dst_ip),
        .i_ctl_dst_port     (i_ctl_dst_port),
        .i_promiscuous      (i_promiscuous),
        .i_sync_fire        (i_sync_fire),
        .i_ctl_reg          (i_ctl_reg)
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

    task automatic load_snapshot_bytes;
        integer i;
        begin
            for (i = 0; i < FRAME_BYTES; i = i + 1) begin
                frame_bytes[i] = 8'h00;
            end

            set16('h0000, 128'h01005e00000100112233445508004500);
            set16('h0010, 128'h00f4000100004011cd19c0a80132e901);
            set16('h0020, 128'h020315b304d200e0b9064e4153445154);
            set16('h0030, 128'h45535420000000000000000100060024);
            set16('h0040, 128'h4101f000000d18c2ed8da20000000000);
            set16('h0050, 128'h0022b142000007d04152475820202020);
            set16('h0060, 128'h001880a800284622c300000d18c6aa3b);
            set16('h0070, 128'hba0000000000003d7453000000645a56);
            set16('h0080, 128'h5a5a5420202000e975a04c45484d001f);
            set16('h0090, 128'h451b3600020d18d4f6b6900000000000);
            set16('h00a0, 128'h0099f300000003000000000000457f00);
            set16('h00b0, 128'h175814b500000d18c4dbafcc00000000);
            set16('h00c0, 128'h00002133000001f40023550ee800000d);
            set16('h00d0, 128'h18c4c103f30000000000000046000000);
            set16('h00e0, 128'h00000034be000001f40005f8e8001344);
            set16('h00f0, 128'h19f400000d18c4058fa2000000000000);
            set2 ('h0100, 16'h03d7);
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

        i_ctl_dst_mac       = 48'h0;
        i_ctl_dst_ip        = 32'h0;
        i_ctl_dst_port      = 16'h0;
        i_promiscuous       = 1'b1;
        i_sync_fire         = 1'b0;
        i_ctl_reg           = {BAR0_SIZE{1'b0}};

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
