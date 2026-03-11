`timescale 1ns / 1ps

module tb_order_book_parser_builder;

    localparam DATA_WIDTH    = 64;
    localparam CTRL_WIDTH    = 8;
    localparam BAR0_SIZE     = 16;
    localparam CLK_PERIOD    = 10;
    localparam FIFO_RD_CLK_PERIOD = 6;
    localparam FRAME_BYTES   = 359;
    localparam NUM_ROUNDS    = 3;
    localparam PAYLOAD_W     = 274;
    localparam EVENT_CDC_DEPTH = 16;
    localparam AAPL_LOCATE   = 16'h000d;
    localparam HSBC_LOCATE   = 16'h0ee8;
    localparam ASK_VALID_MSB = PAYLOAD_W - 1;
    localparam BID_VALID_LSB = 16 + 64 + 32 + 32;

    localparam [63:0] IDLE_WORD  = 64'h0707_0707_0707_0707;
    localparam [63:0] START_WORD = 64'hFB55_5555_5555_55D5;

    reg                         i_clk_156;
    reg                         i_clk_250;
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
    wire                        builder_event_valid;
    wire [PAYLOAD_W-1:0]        builder_event_payload;
    wire                        event_cdc_wr_full;
    wire                        event_cdc_rd_empty;
    wire                        event_cdc_rd_valid;
    wire [PAYLOAD_W-1:0]        event_cdc_rd_data;

    reg  [47:0]                 i_ctl_dst_mac;
    reg  [31:0]                 i_ctl_dst_ip;
    reg  [15:0]                 i_ctl_dst_port;
    reg                         i_promiscuous;
    reg                         i_sync_fire;
    reg  [BAR0_SIZE-1:0]        i_ctl_reg;

    reg [7:0] frame_bytes [0:FRAME_BYTES-1];

    integer round_idx;
    integer total_parser_msgs;
    integer total_builder_payloads;
    integer total_payloads;
    integer aapl_payloads;
    integer hsbc_payloads;
    integer wr_domain_errors;
    integer rd_domain_errors;
    integer final_errors;
    integer total_errors;
    wire [15:0] payload_stock_locate = event_cdc_rd_data[15:0];
    wire [63:0] payload_bid_seq      = event_cdc_rd_data[79:16];
    wire [31:0] payload_bid_shares   = event_cdc_rd_data[111:80];
    wire [31:0] payload_bid_price    = event_cdc_rd_data[143:112];
    wire        payload_bid_valid    = event_cdc_rd_data[BID_VALID_LSB];
    wire [63:0] payload_ask_seq      = event_cdc_rd_data[208:145];
    wire [31:0] payload_ask_shares   = event_cdc_rd_data[240:209];
    wire [31:0] payload_ask_price    = event_cdc_rd_data[272:241];
    wire        payload_ask_valid    = event_cdc_rd_data[ASK_VALID_MSB];

    order_book_parser #(
        .DATA_WIDTH (DATA_WIDTH),
        .CTRL_WIDTH (CTRL_WIDTH),
        .BAR0_SIZE  (BAR0_SIZE)
    ) parser_dut (
        .i_clk_156            (i_clk_156),
        .i_rst                (i_rst),
        .i_axi_rx_data        (i_axi_rx_data),
        .i_axi_rx_valid       (i_axi_rx_valid),
        .i_axi_rx_keep        (i_axi_rx_keep),
        .i_axi_rx_last        (i_axi_rx_last),
        .i_axi_rx_ingress_tick(i_axi_rx_ingress_tick),
        .o_axi_rx_ready       (o_axi_rx_ready),
        .o_msg_valid          (o_msg_valid),
        .o_seq_num            (o_seq_num),
        .o_rx_ingress_tick    (o_rx_ingress_tick),
        .o_msg_type           (o_msg_type),
        .o_stock_locate       (o_stock_locate),
        .o_order_ref_num      (o_order_ref_num),
        .o_new_order_ref_num  (o_new_order_ref_num),
        .o_buy_sell           (o_buy_sell),
        .o_shares             (o_shares),
        .o_price              (o_price),
        .o_timestamp          (o_timestamp),
        .i_ctl_dst_mac        (i_ctl_dst_mac),
        .i_ctl_dst_ip         (i_ctl_dst_ip),
        .i_ctl_dst_port       (i_ctl_dst_port),
        .i_promiscuous        (i_promiscuous),
        .i_sync_fire          (i_sync_fire),
        .i_ctl_reg            (i_ctl_reg)
    );

    order_book_builder #(
        .PAYLOAD_W (PAYLOAD_W)
    ) builder_dut (
        .i_clk_156           (i_clk_156),
        .i_rst               (i_rst),
        .i_msg_valid         (o_msg_valid),
        .i_seq_num           (o_seq_num),
        .i_rx_ingress_tick   (o_rx_ingress_tick),
        .i_msg_type          (o_msg_type),
        .i_stock_locate      (o_stock_locate),
        .i_order_ref_num     (o_order_ref_num),
        .i_new_order_ref_num (o_new_order_ref_num),
        .i_buy_sell          (o_buy_sell),
        .i_shares            (o_shares),
        .i_price             (o_price),
        .i_timestamp         (o_timestamp),
        .o_valid             (builder_event_valid),
        .o_payload           (builder_event_payload)
    );

    async_fifo #(
        .DEPTH  (EVENT_CDC_DEPTH),
        .DATA_W (PAYLOAD_W)
    ) event_cdc_fifo_inst (
        .i_wr_clk   (i_clk_156),
        .i_wr_rst   (i_rst),
        .i_wr_en    (builder_event_valid),
        .i_wr_data  (builder_event_payload),
        .o_wr_full  (event_cdc_wr_full),
        .i_rd_clk   (i_clk_250),
        .i_rd_rst   (i_rst),
        .i_rd_en    (1'b1),
        .o_rd_empty (event_cdc_rd_empty),
        .o_rd_valid (event_cdc_rd_valid),
        .o_rd_data  (event_cdc_rd_data)
    );

    always #(CLK_PERIOD/2) i_clk_156 = ~i_clk_156;
    always #(FIFO_RD_CLK_PERIOD/2) i_clk_250 = ~i_clk_250;

    task automatic drive_cycle;
        input        vld;
        input [63:0] dat;
        input [7:0]  keep;
        input        lst;
        begin
            @(posedge i_clk_156);
            i_axi_rx_valid        <= vld;
            i_axi_rx_data         <= dat;
            i_axi_rx_last         <= lst;
            i_axi_rx_keep         <= keep;
            i_axi_rx_ingress_tick <= i_axi_rx_ingress_tick + 64'd1;
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
        integer i;
        begin
            for (i = 0; i < 2; i = i + 1) begin
                frame_bytes[off+i] = line[15-(i*8) -: 8];
            end
        end
    endtask

    task automatic clear_frame_bytes;
        integer i;
        begin
            for (i = 0; i < FRAME_BYTES; i = i + 1) begin
                frame_bytes[i] = 8'h00;
            end
        end
    endtask

    task automatic load_aapl_frame_bytes;
        begin
            clear_frame_bytes();
`include "../market_data/AAPL_13_B_payload_frames_hex.txt"
        end
    endtask

    task automatic load_hsbc_frame_bytes;
        begin
            clear_frame_bytes();
`include "../market_data/HSBC_3816_S_payload_frames_hex.txt"
        end
    endtask

    task automatic send_loaded_frame;
        integer idx;
        integer b;
        integer bytes_this_beat;
        reg [63:0] beat;
        reg [7:0]  keep;
        begin
            drive_cycle(1'b0, IDLE_WORD, 8'h00, 1'b0);
            drive_cycle(1'b0, START_WORD, 8'h00, 1'b0);

            for (idx = 0; idx < FRAME_BYTES; idx = idx + 8) begin
                beat = IDLE_WORD;
                bytes_this_beat = FRAME_BYTES - idx;
                if (bytes_this_beat > 8) begin
                    bytes_this_beat = 8;
                end
                for (b = 0; b < 8; b = b + 1) begin
                    if ((idx + b) < FRAME_BYTES) begin
                        beat[63-(8*b) -: 8] = frame_bytes[idx+b];
                    end
                end
                if (bytes_this_beat == 8) begin
                    keep = 8'hFF;
                end else begin
                    keep = (8'hFF << (8 - bytes_this_beat));
                end
                drive_cycle(1'b1, beat, keep, ((idx + 8) >= FRAME_BYTES));
            end

            drive_cycle(1'b0, IDLE_WORD, 8'h00, 1'b0);
            drive_cycle(1'b0, IDLE_WORD, 8'h00, 1'b0);
        end
    endtask

    task automatic drain_builder_until_empty;
        integer timeout;
        begin
            timeout = 0;
            begin : drain_loop
                while (builder_dut.symbol_book_AAPL.o_not_empty ||
                       builder_dut.symbol_book_HSBC.o_not_empty ||
                       builder_event_valid ||
                       !event_cdc_rd_empty ||
                       event_cdc_rd_valid) begin
                    @(posedge i_clk_156 or posedge i_clk_250);
                    timeout = timeout + 1;
                    if (timeout > 2400) begin
                        $display("[%0t] ERROR: timed out draining async FIFO output", $time);
                        final_errors = final_errors + 1;
                        disable drain_loop;
                    end
                end
            end
        end
    endtask

    always @(posedge i_clk_156) begin
        if (o_msg_valid) begin
            total_parser_msgs <= total_parser_msgs + 1;
            $display("[%0t] parser msg[%0d] type=%02h locate=%04h ref=%016h new_ref=%016h side=%02h shares=%08h price=%08h seq=%016h",
                     $time, total_parser_msgs + 1, o_msg_type, o_stock_locate, o_order_ref_num,
                     o_new_order_ref_num, o_buy_sell, o_shares, o_price, o_seq_num);
        end

        if (builder_event_valid) begin
            total_builder_payloads <= total_builder_payloads + 1;
            $display("[%0t] builder payload[%0d] queued for async FIFO",
                     $time, total_builder_payloads + 1);
        end

        if (builder_event_valid && event_cdc_wr_full) begin
            $display("[%0t] ERROR: async FIFO overflow dropped a builder payload", $time);
            wr_domain_errors <= wr_domain_errors + 1;
        end
    end

    always @(posedge i_clk_250) begin
        if (event_cdc_rd_valid) begin
            total_payloads <= total_payloads + 1;
            $display("[%0t] async fifo payload[%0d] locate=%04h ask_valid=%0b ask_price=%08h ask_shares=%08h ask_seq=%016h bid_valid=%0b bid_price=%08h bid_shares=%08h bid_seq=%016h",
                     $time, total_payloads + 1, payload_stock_locate,
                     payload_ask_valid, payload_ask_price, payload_ask_shares, payload_ask_seq,
                     payload_bid_valid, payload_bid_price, payload_bid_shares, payload_bid_seq);

            case (payload_stock_locate)
                AAPL_LOCATE: begin
                    aapl_payloads <= aapl_payloads + 1;
                    if (payload_ask_valid !== 1'b0) begin
                        $display("[%0t] ERROR: AAPL payload has unexpected ask_valid=1", $time);
                        rd_domain_errors <= rd_domain_errors + 1;
                    end
                end
                HSBC_LOCATE: begin
                    hsbc_payloads <= hsbc_payloads + 1;
                    if (payload_bid_valid !== 1'b0) begin
                        $display("[%0t] ERROR: HSBC payload has unexpected bid_valid=1", $time);
                        rd_domain_errors <= rd_domain_errors + 1;
                    end
                end
                default: begin
                    $display("[%0t] ERROR: arbiter produced unknown stock locate %04h", $time, payload_stock_locate);
                    rd_domain_errors <= rd_domain_errors + 1;
                end
            endcase
        end
    end

    initial begin
        i_clk_156             = 1'b0;
        i_clk_250             = 1'b0;
        i_rst                 = 1'b1;
        i_axi_rx_data         = IDLE_WORD;
        i_axi_rx_valid        = 1'b0;
        i_axi_rx_keep         = 8'hFF;
        i_axi_rx_last         = 1'b0;
        i_axi_rx_ingress_tick = 64'd1;
        i_ctl_dst_mac         = 48'h0;
        i_ctl_dst_ip          = 32'h0;
        i_ctl_dst_port        = 16'h0;
        i_promiscuous         = 1'b1;
        i_sync_fire           = 1'b0;
        i_ctl_reg             = {BAR0_SIZE{1'b0}};

        total_parser_msgs     = 0;
        total_builder_payloads = 0;
        total_payloads        = 0;
        aapl_payloads         = 0;
        hsbc_payloads         = 0;
        wr_domain_errors      = 0;
        rd_domain_errors      = 0;
        final_errors          = 0;
        total_errors          = 0;

        repeat (5) @(posedge i_clk_156);
        i_rst <= 1'b0;
        repeat (3) @(posedge i_clk_156);

        for (round_idx = 0; round_idx < NUM_ROUNDS; round_idx = round_idx + 1) begin
            $display("[%0t] starting traffic round %0d/%0d", $time, round_idx + 1, NUM_ROUNDS);

            load_aapl_frame_bytes();
            send_loaded_frame();
            repeat (8) @(posedge i_clk_156);

            load_hsbc_frame_bytes();
            send_loaded_frame();
            repeat (8) @(posedge i_clk_156);

            drain_builder_until_empty();

            repeat (12) @(posedge i_clk_156);
        end

        repeat (30) @(posedge i_clk_156);

        if (aapl_payloads == 0) begin
            $display("[%0t] ERROR: no AAPL payloads were produced", $time);
            final_errors = final_errors + 1;
        end
        if (hsbc_payloads == 0) begin
            $display("[%0t] ERROR: no HSBC payloads were produced", $time);
            final_errors = final_errors + 1;
        end
        if (total_builder_payloads != total_payloads) begin
            $display("[%0t] ERROR: builder/FIFO payload count mismatch builder=%0d fifo=%0d",
                     $time, total_builder_payloads, total_payloads);
            final_errors = final_errors + 1;
        end
        total_errors = wr_domain_errors + rd_domain_errors + final_errors;

        $display("[%0t] summary: parser_msgs=%0d builder_payloads=%0d fifo_payloads=%0d aapl_payloads=%0d hsbc_payloads=%0d errors=%0d",
                 $time, total_parser_msgs, total_builder_payloads, total_payloads,
                 aapl_payloads, hsbc_payloads, total_errors);

        if (total_errors != 0) begin
            $fatal(1, "Parser/builder arbiter test failed with %0d errors", total_errors);
        end

        $display("[%0t] PASS: parser->builder arbiter test completed", $time);
        $finish;
    end

    initial begin
        $dumpvars(0, tb_order_book_parser_builder);
    end

endmodule
