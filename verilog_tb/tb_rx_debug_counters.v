`timescale 1ps / 1ps

module tb_rx_debug_counters;

    localparam SYMBOL_NUM = 2;
    localparam BAR0_SIZE  = 16;
    localparam DATA_WIDTH = 256;
    localparam CLK156_PS  = 6400;
    localparam CLK250_PS  = 4000;
    localparam TYPE_READ  = 4'b0000;

    localparam [BAR0_SIZE-1:0] REG_RX_DBG_PCS_FRAME_COUNT  = 16'h0F8;
    localparam [BAR0_SIZE-1:0] REG_RX_DBG_FRAME_COUNT      = 16'h100;
    localparam [BAR0_SIZE-1:0] REG_RX_DBG_MSG_COUNT        = 16'h108;
    localparam [BAR0_SIZE-1:0] REG_RX_DBG_EVENT_COUNT_Q0   = 16'h110;
    localparam [BAR0_SIZE-1:0] REG_RX_DBG_EVENT_COUNT_Q1   = 16'h118;

    reg                      clk_156;
    reg                      clk_250;
    reg                      rst;
    reg                      pcs_frame_started;
    reg                      frame_started;
    reg                      msg_valid;
    reg  [SYMBOL_NUM-1:0]    event_valid;

    reg                      cq_valid;
    reg  [3:0]               cq_type;
    reg  [BAR0_SIZE-1:0]     cq_reg_addr;
    reg  [63:0]              cq_payload;
    reg  [15:0]              cq_requester_id;
    reg  [7:0]               cq_tag;
    reg  [2:0]               cq_tc;
    reg  [6:0]               cq_lower_addr;
    reg  [10:0]              cq_payload_dw_count;
    reg                      cc_ready;

    wire                     cc_valid;
    wire [15:0]              cc_requester_id;
    wire [7:0]               cc_tag;
    wire [2:0]               cc_tc;
    wire [6:0]               cc_lower_addr;
    wire [10:0]              cc_dword_count;
    wire [2:0]               cc_status;
    wire [DATA_WIDTH/2-1:0]  cc_payload;
    wire                     cc_last;

    wire [63:0]              pcs_frame_count_gray;
    wire [63:0]              frame_count_gray;
    wire [63:0]              msg_count_gray;
    wire [SYMBOL_NUM*64-1:0] event_count_gray;
    wire [63:0]              pcs_frame_count;
    wire [63:0]              frame_count;
    wire [63:0]              msg_count;
    wire [SYMBOL_NUM*64-1:0] event_count;

    wire [SYMBOL_NUM*64-1:0] rx_que_iova_addr;
    wire [SYMBOL_NUM*64-1:0] rx_que_slot_num;
    wire [SYMBOL_NUM*64-1:0] rx_que_enable;
    wire [SYMBOL_NUM*64-1:0] rx_que_cons_ptr;
    wire [SYMBOL_NUM*16-1:0] symbol_stock_locate;
    wire [SYMBOL_NUM*32-1:0] symbol_price_base;
    wire                     reg_reset;

    rx_debug_counters #(
        .SYMBOL_NUM (SYMBOL_NUM)
    ) dut_counters (
        .i_clk_156          (clk_156),
        .i_rst              (rst),
        .i_pcs_frame_started(pcs_frame_started),
        .i_frame_started    (frame_started),
        .i_msg_valid        (msg_valid),
        .i_event_valid      (event_valid),
        .o_pcs_frame_count_gray (pcs_frame_count_gray),
        .o_frame_count_gray (frame_count_gray),
        .o_msg_count_gray   (msg_count_gray),
        .o_event_count_gray (event_count_gray)
    );

    rx_debug_counter_bridge #(
        .SYMBOL_NUM (SYMBOL_NUM)
    ) dut_bridge (
        .i_user_clk         (clk_250),
        .i_pcs_frame_count_gray (pcs_frame_count_gray),
        .i_frame_count_gray (frame_count_gray),
        .i_msg_count_gray   (msg_count_gray),
        .i_event_count_gray (event_count_gray),
        .o_pcs_frame_count  (pcs_frame_count),
        .o_frame_count      (frame_count),
        .o_msg_count        (msg_count),
        .o_event_count      (event_count)
    );

    rx_dma_config #(
        .DATA_WIDTH (DATA_WIDTH),
        .BAR0_SIZE  (BAR0_SIZE),
        .SYMBOL_NUM (SYMBOL_NUM)
    ) dut_cfg (
        .user_clk              (clk_250),
        .user_reset_p          (rst),
        .cq_valid              (cq_valid),
        .cq_type               (cq_type),
        .cq_reg_addr           (cq_reg_addr),
        .cq_payload            (cq_payload),
        .cq_requester_id       (cq_requester_id),
        .cq_tag                (cq_tag),
        .cq_tc                 (cq_tc),
        .cq_lower_addr         (cq_lower_addr),
        .cq_payload_dw_count   (cq_payload_dw_count),
        .cc_ready              (cc_ready),
        .cc_valid              (cc_valid),
        .cc_requester_id       (cc_requester_id),
        .cc_tag                (cc_tag),
        .cc_tc                 (cc_tc),
        .cc_lower_addr         (cc_lower_addr),
        .cc_dword_count        (cc_dword_count),
        .cc_status             (cc_status),
        .cc_payload            (cc_payload),
        .cc_last               (cc_last),
        .o_rx_que_iova_addr    (rx_que_iova_addr),
        .o_rx_que_slot_num     (rx_que_slot_num),
        .o_rx_que_enable       (rx_que_enable),
        .o_rx_que_cons_ptr     (rx_que_cons_ptr),
        .o_symbol_stock_locate (symbol_stock_locate),
        .o_symbol_price_base   (symbol_price_base),
        .o_reg_reset           (reg_reset),
        .i_rx_que_prod_ptr     ({(SYMBOL_NUM*64){1'b0}}),
        .i_rx_que_drop_count   ({(SYMBOL_NUM*64){1'b0}}),
        .i_rx_que_status       ({(SYMBOL_NUM*64){1'b0}}),
        .i_rx_dbg_pcs_frame_count (pcs_frame_count),
        .i_rx_dbg_frame_count  (frame_count),
        .i_rx_dbg_msg_count    (msg_count),
        .i_rx_dbg_event_count  (event_count),
        .i_dma_timestamp       (48'd0)
    );

    always #(CLK156_PS/2) clk_156 = ~clk_156;
    always #(CLK250_PS/2) clk_250 = ~clk_250;

    task automatic pulse_pcs_frame;
        begin
            @(posedge clk_156);
            pcs_frame_started <= 1'b1;
            @(posedge clk_156);
            pcs_frame_started <= 1'b0;
        end
    endtask

    task automatic pulse_frame;
        begin
            @(posedge clk_156);
            frame_started <= 1'b1;
            @(posedge clk_156);
            frame_started <= 1'b0;
        end
    endtask

    task automatic pulse_msg;
        begin
            @(posedge clk_156);
            msg_valid <= 1'b1;
            @(posedge clk_156);
            msg_valid <= 1'b0;
        end
    endtask

    task automatic pulse_event(input [SYMBOL_NUM-1:0] event_mask);
        begin
            @(posedge clk_156);
            event_valid <= event_mask;
            @(posedge clk_156);
            event_valid <= {SYMBOL_NUM{1'b0}};
        end
    endtask

    task automatic read_reg(
        input [BAR0_SIZE-1:0] reg_addr,
        input [63:0]          expected_value
    );
        begin
            wait (cc_valid == 1'b0);
            @(posedge clk_250);
            cq_valid            <= 1'b1;
            cq_type             <= TYPE_READ;
            cq_reg_addr         <= reg_addr;
            cq_payload          <= 64'd0;
            cq_requester_id     <= 16'h1234;
            cq_tag              <= 8'h5a;
            cq_tc               <= 3'd0;
            cq_lower_addr       <= 7'd0;
            cq_payload_dw_count <= 11'd1;

            @(posedge clk_250);
            cq_valid <= 1'b0;

            wait (cc_valid == 1'b1);
            $display("DBG read addr=%0h value=%0d", reg_addr, cc_payload[63:0]);
            if (cc_payload[63:0] != expected_value) begin
                $fatal(1, "Read mismatch addr=%0h expected=%0d actual=%0d",
                       reg_addr,
                       expected_value,
                       cc_payload[63:0]);
            end
            if (!cc_last || cc_status != 3'b000) begin
                $fatal(1, "Completion metadata mismatch addr=%0h", reg_addr);
            end
            @(posedge clk_250);
        end
    endtask

    initial begin
        clk_156            = 1'b0;
        clk_250            = 1'b0;
        rst                = 1'b1;
        pcs_frame_started  = 1'b0;
        frame_started      = 1'b0;
        msg_valid          = 1'b0;
        event_valid        = {SYMBOL_NUM{1'b0}};
        cq_valid           = 1'b0;
        cq_type            = 4'd0;
        cq_reg_addr        = {BAR0_SIZE{1'b0}};
        cq_payload         = 64'd0;
        cq_requester_id    = 16'd0;
        cq_tag             = 8'd0;
        cq_tc              = 3'd0;
        cq_lower_addr      = 7'd0;
        cq_payload_dw_count= 11'd0;
        cc_ready           = 1'b1;

        repeat (4) @(posedge clk_156);
        repeat (4) @(posedge clk_250);
        rst = 1'b0;

        pulse_pcs_frame();
        pulse_pcs_frame();
        pulse_pcs_frame();
        pulse_pcs_frame();
        pulse_frame();
        pulse_frame();
        pulse_frame();

        pulse_msg();
        pulse_msg();
        pulse_msg();
        pulse_msg();
        pulse_msg();

        pulse_event(2'b01);
        pulse_event(2'b10);
        pulse_event(2'b11);
        pulse_event(2'b10);

        repeat (16) @(posedge clk_250);

        if (pcs_frame_count != 64'd4) begin
            $fatal(1, "pcs_frame_count mismatch expected 4 actual %0d", pcs_frame_count);
        end
        if (frame_count != 64'd3) begin
            $fatal(1, "frame_count mismatch expected 3 actual %0d", frame_count);
        end
        if (msg_count != 64'd5) begin
            $fatal(1, "msg_count mismatch expected 5 actual %0d", msg_count);
        end
        if (event_count[63:0] != 64'd2) begin
            $fatal(1, "event_count q0 mismatch expected 2 actual %0d", event_count[63:0]);
        end
        if (event_count[127:64] != 64'd3) begin
            $fatal(1, "event_count q1 mismatch expected 3 actual %0d", event_count[127:64]);
        end
        if (dut_cfg.i_rx_dbg_event_count[63:0] != 64'd2) begin
            $fatal(1, "cfg input event_count q0 mismatch expected 2 actual %0d", dut_cfg.i_rx_dbg_event_count[63:0]);
        end
        if (dut_cfg.i_rx_dbg_event_count[127:64] != 64'd3) begin
            $fatal(1, "cfg input event_count q1 mismatch expected 3 actual %0d", dut_cfg.i_rx_dbg_event_count[127:64]);
        end

        read_reg(REG_RX_DBG_PCS_FRAME_COUNT, 64'd4);
        read_reg(REG_RX_DBG_FRAME_COUNT, 64'd3);
        read_reg(REG_RX_DBG_MSG_COUNT, 64'd5);
        read_reg(REG_RX_DBG_EVENT_COUNT_Q0, 64'd2);
        read_reg(REG_RX_DBG_EVENT_COUNT_Q1, 64'd3);

        $display("PASS: rx debug counters and BAR readout validated");
        $finish;
    end

endmodule
