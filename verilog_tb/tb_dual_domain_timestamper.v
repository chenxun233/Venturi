`timescale 1ps / 1ps

module tb_dual_domain_timestamper;

    localparam TS_WIDTH   = 48;
    localparam PAYLOAD_W  = 192;
    localparam CLK156_PS  = 6400;
    localparam CLK250_PS  = 4000;

    reg                  clk_156;
    reg                  clk_250;
    reg                  rst;
    reg                  mac_event;

    reg                  event_empty;
    reg                  event_valid;
    reg  [PAYLOAD_W-1:0] event_payload;
    reg  [63:0]          que_iova_addr;
    reg  [63:0]          que_slot_num;
    reg  [63:0]          que_enable;
    reg  [63:0]          que_cons_ptr;
    reg                  reg_reset;
    reg                  rq_ready;

    wire [TS_WIDTH-1:0]  mac_timestamp;
    wire [TS_WIDTH-1:0]  dma_timestamp_gray;
    wire [TS_WIDTH-1:0]  dma_timestamp_gray_sync;
    wire [TS_WIDTH-1:0]  dma_timestamp_bin;
    wire                 event_pop;
    wire [63:0]          que_prod_ptr;
    wire [63:0]          que_drop_count;
    wire [63:0]          que_status;
    wire                 rq_valid;
    wire [3:0]           rq_type;
    wire                 rq_payload_last;
    wire [63:0]          rq_addr;
    wire [10:0]          rq_payload_dw_count;
    wire [7:0]           rq_tag;
    wire [2:0]           rq_tc;
    wire [255:0]         rq_payload;

    reg  [TS_WIDTH-1:0]  frame_ts_0;
    reg  [TS_WIDTH-1:0]  frame_ts_1;
    reg  [47:0]          last_event_tk;
    reg  [47:0]          event_tk_0;
    reg  [47:0]          event_tk_1;
    reg  [47:0]          event_tk_2;

    timestamper #(
        .COUNTER_WIDTH (TS_WIDTH)
    ) dut_timestamper (
        .i_clk               (clk_156),
        .i_rst               (rst),
        .i_event             (mac_event),
        .o_mac_timestamp     (mac_timestamp),
        .o_dma_timestamp_gray(dma_timestamp_gray)
    );

    bit_synchronizer #(
        .BIT_WIDTH (TS_WIDTH)
    ) dut_sync (
        .i_clk  (clk_250),
        .i_in   (dma_timestamp_gray),
        .o_out  (dma_timestamp_gray_sync)
    );

    gray_to_binary #(
        .WIDTH (TS_WIDTH)
    ) dut_decode (
        .i_gray   (dma_timestamp_gray_sync),
        .o_binary (dma_timestamp_bin)
    );

    rx_dma_stage #(
        .SYMBOL_NUM (1),
        .PAYLOAD_W  (PAYLOAD_W)
    ) dut_rx_dma_stage (
        .i_clk               (clk_250),
        .i_rst               (rst),
        .i_dma_timestamp     (dma_timestamp_bin),
        .i_event_empty       (event_empty),
        .i_event_valid       (event_valid),
        .i_event_payload     (event_payload),
        .o_event_pop         (event_pop),
        .i_que_iova_addr     (que_iova_addr),
        .i_que_slot_num      (que_slot_num),
        .i_que_enable        (que_enable),
        .i_que_cons_ptr      (que_cons_ptr),
        .i_reg_reset         (reg_reset),
        .o_que_prod_ptr      (que_prod_ptr),
        .o_que_drop_count    (que_drop_count),
        .o_que_status        (que_status),
        .i_rq_ready          (rq_ready),
        .o_rq_valid          (rq_valid),
        .o_rq_type           (rq_type),
        .o_rq_payload_last   (rq_payload_last),
        .o_rq_addr           (rq_addr),
        .o_rq_payload_dw_count(rq_payload_dw_count),
        .o_rq_tag            (rq_tag),
        .o_rq_tc             (rq_tc),
        .o_rq_payload        (rq_payload)
    );

    always #(CLK156_PS/2) clk_156 = ~clk_156;
    always #(CLK250_PS/2) clk_250 = ~clk_250;

    task automatic pulse_mac_event;
        begin
            @(posedge clk_156);
            mac_event = 1'b1;
            @(posedge clk_156);
            mac_event = 1'b0;
        end
    endtask

    task automatic issue_dma_record(
        input [31:0] ask_price,
        input [31:0] ask_shares,
        input [31:0] bid_price,
        input [31:0] bid_shares,
        input [47:0] frame_ts,
        input [15:0] stock_locate
    );
        begin
            @(posedge clk_250);
            event_empty   = 1'b0;
            event_payload = {ask_price, ask_shares, bid_price, bid_shares, frame_ts, stock_locate};

            wait (event_pop == 1'b1);
            @(posedge clk_250);
            event_valid = 1'b1;

            @(posedge clk_250);
            event_valid = 1'b0;
            event_empty = 1'b1;
            event_payload = {PAYLOAD_W{1'b0}};
        end
    endtask

    task automatic check_rq_record(
        input [63:0] expected_addr,
        input [7:0]  expected_first_flag,
        input [47:0] expected_frame_ts,
        input [15:0] expected_stock_locate,
        input [31:0] expected_ask_price,
        input [31:0] expected_ask_shares,
        input [31:0] expected_bid_price,
        input [31:0] expected_bid_shares,
        input [63:0] expected_prod_ptr,
        input [47:0] prev_event_tk,
        output [47:0] observed_event_tk
    );
        reg [7:0]  first_flag;
        reg [31:0] ask_price;
        reg [31:0] ask_shares;
        reg [31:0] bid_price;
        reg [31:0] bid_shares;
        reg [47:0] frame_ts;
        reg [47:0] event_tk;
        reg [15:0] stock_locate;
        begin
            wait (rq_valid == 1'b1);

            first_flag   = rq_payload[247:240];
            ask_price    = rq_payload[239:208];
            ask_shares   = rq_payload[207:176];
            bid_price    = rq_payload[175:144];
            bid_shares   = rq_payload[143:112];
            frame_ts     = rq_payload[111:64];
            event_tk     = rq_payload[63:16];
            stock_locate = rq_payload[15:0];

            if (rq_type != 4'b0001) begin
                $fatal(1, "RQ type mismatch expected write got %0h", rq_type);
            end
            if (!rq_payload_last) begin
                $fatal(1, "RQ payload_last should be asserted");
            end
            if (rq_addr != expected_addr) begin
                $fatal(1, "RQ addr mismatch expected %0h actual %0h", expected_addr, rq_addr);
            end
            if (rq_payload_dw_count != 11'd8) begin
                $fatal(1, "RQ dword count mismatch expected 8 actual %0d", rq_payload_dw_count);
            end
            if (rq_tag != 8'h40) begin
                $fatal(1, "RQ tag mismatch expected 0x40 actual %0h", rq_tag);
            end
            if (rq_tc != 3'd0) begin
                $fatal(1, "RQ traffic class mismatch expected 0 actual %0d", rq_tc);
            end
            if (first_flag != expected_first_flag) begin
                $fatal(1, "first_event mismatch expected %0h actual %0h", expected_first_flag, first_flag);
            end
            if (frame_ts != expected_frame_ts) begin
                $fatal(1, "frame_ts mismatch expected %0h actual %0h", expected_frame_ts, frame_ts);
            end
            if (stock_locate != expected_stock_locate) begin
                $fatal(1, "stock_locate mismatch expected %0h actual %0h", expected_stock_locate, stock_locate);
            end
            if (ask_price != expected_ask_price || ask_shares != expected_ask_shares ||
                bid_price != expected_bid_price || bid_shares != expected_bid_shares) begin
                $fatal(1, "price/share payload mismatch");
            end
            if (event_tk < expected_frame_ts) begin
                $fatal(1, "event_tk %0d should not be earlier than frame_ts %0d", event_tk, expected_frame_ts);
            end
            if (event_tk <= prev_event_tk) begin
                $fatal(1, "event_tk %0d should be later than previous event_tk %0d", event_tk, prev_event_tk);
            end

            wait (que_prod_ptr == expected_prod_ptr);
            if (que_drop_count != 64'd0) begin
                $fatal(1, "Queue drop count should remain zero, got %0d", que_drop_count);
            end
            if (que_status[0] != 1'b1) begin
                $fatal(1, "Queue enabled bit should stay asserted");
            end

            observed_event_tk = event_tk;
        end
    endtask

    initial begin
        clk_156      = 1'b0;
        clk_250      = 1'b0;
        rst          = 1'b1;
        mac_event    = 1'b0;
        event_empty  = 1'b1;
        event_valid  = 1'b0;
        event_payload= {PAYLOAD_W{1'b0}};
        que_iova_addr= 64'h0000_0000_1000_0000;
        que_slot_num = 64'd16;
        que_enable   = 64'd1;
        que_cons_ptr = 64'd0;
        reg_reset    = 1'b0;
        rq_ready     = 1'b1;
        frame_ts_0   = {TS_WIDTH{1'b0}};
        frame_ts_1   = {TS_WIDTH{1'b0}};
        last_event_tk= 48'd0;
        event_tk_0   = 48'd0;
        event_tk_1   = 48'd0;
        event_tk_2   = 48'd0;

        repeat (4) @(posedge clk_156);
        rst = 1'b0;

        repeat (8) @(posedge clk_156);
        pulse_mac_event();
        @(posedge clk_156);
        frame_ts_0 = mac_timestamp;
        if (frame_ts_0 == {TS_WIDTH{1'b0}}) begin
            $fatal(1, "first MAC frame timestamp was not captured");
        end

        repeat (10) @(posedge clk_250);
        issue_dma_record(32'h1111_2222, 32'h0000_0011, 32'h3333_4444, 32'h0000_0022, frame_ts_0, 16'h000d);
        check_rq_record(64'h0000_0000_1000_0000,
                        8'h01,
                        frame_ts_0,
                        16'h000d,
                        32'h1111_2222,
                        32'h0000_0011,
                        32'h3333_4444,
                        32'h0000_0022,
                        64'd1,
                        last_event_tk,
                        event_tk_0);
        last_event_tk = event_tk_0;

        issue_dma_record(32'haaaa_bbbb, 32'h0000_0033, 32'hcccc_dddd, 32'h0000_0044, frame_ts_0, 16'h000d);
        check_rq_record(64'h0000_0000_1000_0020,
                        8'h00,
                        frame_ts_0,
                        16'h000d,
                        32'haaaa_bbbb,
                        32'h0000_0033,
                        32'hcccc_dddd,
                        32'h0000_0044,
                        64'd2,
                        last_event_tk,
                        event_tk_1);
        last_event_tk = event_tk_1;

        repeat (20) @(posedge clk_156);
        pulse_mac_event();
        @(posedge clk_156);
        frame_ts_1 = mac_timestamp;
        if (frame_ts_1 <= frame_ts_0) begin
            $fatal(1, "second MAC frame timestamp %0d should be later than first %0d", frame_ts_1, frame_ts_0);
        end

        repeat (10) @(posedge clk_250);
        issue_dma_record(32'h0102_0304, 32'h0000_0055, 32'h0506_0708, 32'h0000_0066, frame_ts_1, 16'h0ee8);
        check_rq_record(64'h0000_0000_1000_0040,
                        8'h01,
                        frame_ts_1,
                        16'h0ee8,
                        32'h0102_0304,
                        32'h0000_0055,
                        32'h0506_0708,
                        32'h0000_0066,
                        64'd3,
                        last_event_tk,
                        event_tk_2);

        $display("[%0t] PASS: rx_dma_stage timestamps and first_event tagging validated", $time);
        $finish;
    end

endmodule
