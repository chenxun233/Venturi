`timescale 1ns / 1ps

module tb_mac_order_book_builder;

    localparam DATA_WIDTH  = 64;
    localparam CTRL_WIDTH  = 8;
    localparam BAR0_SIZE   = 16;
    localparam CLK156_PERIOD = 10;
    localparam CLK250_PERIOD = 4;
    localparam FRAME_BYTES = 359;
    localparam PAYLOAD_W   = 192;
    localparam EVENT_CDC_DEPTH = 4;
    localparam SYMBOL_NUM  = 2;

    localparam SYMBOL_NONE = 2'd0;
    localparam SYMBOL_AAPL = 2'd1;
    localparam SYMBOL_HSBC = 2'd2;

    localparam AAPL_LOCATE = 16'h000d;
    localparam HSBC_LOCATE = 16'h0ee8;
    localparam [31:0] AAPL_PRICE_BASE = 32'd0;
    localparam [31:0] HSBC_PRICE_BASE = 32'd0;
    localparam [SYMBOL_NUM*16-1:0] SYMBOL_STOCK_LOCATE_CFG = {HSBC_LOCATE, AAPL_LOCATE};
    localparam [SYMBOL_NUM*32-1:0] SYMBOL_PRICE_BASE_CFG   = {HSBC_PRICE_BASE, AAPL_PRICE_BASE};
    localparam [63:0] AAPL_DMA_BASE = 64'h0000_0000_1000_0000;
    localparam [63:0] HSBC_DMA_BASE = 64'h0000_0000_1000_1000;

    localparam [63:0] IDLE_WORD    = 64'h0707_0707_0707_0707;
    localparam [63:0] START_L0_WORD = 64'hD555_5555_5555_55FB;

    reg                     i_clk_156;
    reg                     i_clk_250;
    reg                     i_rst;
    reg  [DATA_WIDTH-1:0]   i_xgmii_rxd;
    reg  [CTRL_WIDTH-1:0]   i_xgmii_rxc;
    reg                     i_rx_status;

    wire [DATA_WIDTH-1:0]   mac_axi_data;
    wire                    mac_axi_valid;
    wire [CTRL_WIDTH-1:0]   mac_axi_keep;
    wire                    mac_axi_last;
    wire                    mac_frame_started;
    wire [47:0]             mac_frame_ts;
    wire [47:0]             w_dma_ts_gray_rx;
    wire                    mac_axi_ready;

    wire                    parser_msg_valid;
    wire [63:0]             parser_seq_num;
    wire [7:0]              parser_msg_type;
    wire [15:0]             parser_stock_locate;
    wire [63:0]             parser_order_ref_num;
    wire [63:0]             parser_new_order_ref_num;
    wire [7:0]              parser_buy_sell;
    wire [31:0]             parser_shares;
    wire [31:0]             parser_price;
    wire [47:0]             parser_frame_ts;

    wire [1:0]              builder_event_valid;
    wire [2*PAYLOAD_W-1:0]  builder_event_payload;
    wire [SYMBOL_NUM-1:0]   event_cdc_wr_full;
    wire [SYMBOL_NUM-1:0]   event_cdc_rd_en;
    wire [SYMBOL_NUM-1:0]   event_cdc_rd_empty;
    wire [SYMBOL_NUM-1:0]   event_cdc_rd_valid;
    wire [SYMBOL_NUM*PAYLOAD_W-1:0] event_cdc_rd_data;
    wire [SYMBOL_NUM*64-1:0] event_cdc_wr_drop_count;
    wire [47:0]             w_dma_ts_gray_dma;
    wire [47:0]             w_dma_ts_bin;
    reg  [SYMBOL_NUM*64-1:0] rx_dma_que_iova_addr;
    reg  [SYMBOL_NUM*64-1:0] rx_dma_que_slot_num;
    reg  [SYMBOL_NUM*64-1:0] rx_dma_que_enable;
    reg  [SYMBOL_NUM*64-1:0] rx_dma_que_cons_ptr;
    reg                      rx_dma_reg_reset;
    wire [SYMBOL_NUM*64-1:0] rx_dma_que_prod_ptr;
    wire [SYMBOL_NUM*64-1:0] rx_dma_que_drop_count;
    wire [SYMBOL_NUM*64-1:0] rx_dma_que_status;
    wire [SYMBOL_NUM*64-1:0] rx_dma_event_pop_count;
    wire [SYMBOL_NUM*64-1:0] rx_dma_event_send_count;
    wire                     dma_rq_valid;
    wire [3:0]               dma_rq_type;
    wire                     dma_rq_payload_last;
    wire [63:0]              dma_rq_addr;
    wire [10:0]              dma_rq_payload_dw_count;
    wire [7:0]               dma_rq_tag;
    wire [2:0]               dma_rq_tc;
    wire [255:0]             dma_rq_payload;

    reg [7:0] frame_bytes [0:FRAME_BYTES-1];

    reg [1:0]  pending_symbol;
    reg        last_mac_frame_started;
    reg [47:0] last_mac_frame_ts;
    reg [47:0] last_w_dma_ts_gray_rx;
    reg [47:0] expected_aapl_frame_ts;
    reg [47:0] expected_hsbc_frame_ts;
    integer    aapl_frame_starts;
    integer    hsbc_frame_starts;
    integer    aapl_parser_msgs;
    integer    hsbc_parser_msgs;
    integer    aapl_builder_events;
    integer    hsbc_builder_events;
    integer    aapl_dma_records;
    integer    hsbc_dma_records;
    reg [47:0] last_dma_timestamp;
    reg        dma_timestamp_seen;
    reg [7:0]           dma_payload_first_event;
    reg [15:0]          dma_payload_locate;
    reg [47:0]          dma_payload_ts;
    reg [47:0]          dma_payload_event_tk;
    reg                 aapl_first_dma_seen;
    reg                 hsbc_first_dma_seen;
    integer    total_errors_mac;
    integer    total_errors_dma;
    integer    total_errors;

    wire [PAYLOAD_W-1:0] aapl_payload = builder_event_payload[0 +: PAYLOAD_W];
    wire [PAYLOAD_W-1:0] hsbc_payload = builder_event_payload[PAYLOAD_W +: PAYLOAD_W];

    wire [15:0] aapl_payload_locate = aapl_payload[15:0];
    wire [47:0] aapl_payload_ts     = aapl_payload[63:16];
    wire [15:0] hsbc_payload_locate = hsbc_payload[15:0];
    wire [47:0] hsbc_payload_ts     = hsbc_payload[63:16];
    wire [7:0]  dma_record_first_event = dma_rq_payload[247:240];
    wire [47:0] dma_record_frame_ts    = dma_rq_payload[111:64];
    wire [47:0] dma_record_event_tk    = dma_rq_payload[63:16];
    wire [15:0] dma_record_locate      = dma_rq_payload[15:0];

    MAC_layer_rx #(
        .DATA_WIDTH (DATA_WIDTH),
        .CTRL_WIDTH (CTRL_WIDTH)
    ) mac_dut (
        .i_xgmii_rx_clk   (i_clk_156),
        .i_xgmii_rx_rst   (i_rst),
        .i_xgmii_rxd      (i_xgmii_rxd),
        .i_xgmii_rxc      (i_xgmii_rxc),
        .i_rx_status      (i_rx_status),
        .o_axi_rx_data    (mac_axi_data),
        .o_axi_rx_valid   (mac_axi_valid),
        .o_axi_rx_keep    (mac_axi_keep),
        .o_axi_rx_last    (mac_axi_last),
        .o_frame_started  (mac_frame_started),
        .o_frame_ts       (mac_frame_ts),
        .o_dma_ts_gray    (w_dma_ts_gray_rx),
        .i_axi_rx_ready   (mac_axi_ready)
    );

    order_book_parser #(
        .DATA_WIDTH (DATA_WIDTH),
        .CTRL_WIDTH (CTRL_WIDTH),
        .BAR0_SIZE  (BAR0_SIZE)
    ) parser_dut (
        .i_clk_156            (i_clk_156),
        .i_rst                (i_rst),
        .i_axi_rx_data        (mac_axi_data),
        .i_axi_rx_valid       (mac_axi_valid),
        .i_axi_rx_keep        (mac_axi_keep),
        .i_axi_rx_last        (mac_axi_last),
        .i_frame_ts           (mac_frame_ts),
        .o_axi_rx_ready       (mac_axi_ready),
        .o_msg_valid          (parser_msg_valid),
        .o_seq_num            (parser_seq_num),
        .o_msg_type           (parser_msg_type),
        .o_stock_locate       (parser_stock_locate),
        .o_order_ref_num      (parser_order_ref_num),
        .o_new_order_ref_num  (parser_new_order_ref_num),
        .o_buy_sell           (parser_buy_sell),
        .o_shares             (parser_shares),
        .o_price              (parser_price),
        .o_frame_ts           (parser_frame_ts)
    );

    order_book_builder #(
        .PAYLOAD_W (PAYLOAD_W)
    ) builder_dut (
        .i_clk_156           (i_clk_156),
        .i_rst               (i_rst),
        .i_msg_valid         (parser_msg_valid),
        .i_msg_type          (parser_msg_type),
        .i_stock_locate      (parser_stock_locate),
        .i_order_ref_num     (parser_order_ref_num),
        .i_new_order_ref_num (parser_new_order_ref_num),
        .i_buy_sell          (parser_buy_sell),
        .i_shares            (parser_shares),
        .i_price             (parser_price),
        .i_frame_ts          (parser_frame_ts),
        .i_symbol_stock_locate_cfg (SYMBOL_STOCK_LOCATE_CFG),
        .i_symbol_price_base_cfg   (SYMBOL_PRICE_BASE_CFG),
        .o_event_valid       (builder_event_valid),
        .o_event_payload     (builder_event_payload)
    );

    bit_synchronizer #(
        .BIT_WIDTH (48)
    ) dma_timestamp_sync_inst (
        .i_clk  (i_clk_250),
        .i_in   (w_dma_ts_gray_rx),
        .o_out  (w_dma_ts_gray_dma)
    );

    gray_to_binary #(
        .WIDTH (48)
    ) dma_timestamp_decode_inst (
        .i_gray   (w_dma_ts_gray_dma),
        .o_binary (w_dma_ts_bin)
    );

    generate
        genvar cdc_idx;
        for (cdc_idx = 0; cdc_idx < SYMBOL_NUM; cdc_idx = cdc_idx + 1) begin : g_event_cdc
            async_fifo #(
                .DEPTH  (EVENT_CDC_DEPTH),
                .DATA_W (PAYLOAD_W)
            ) event_cdc_inst (
                .i_wr_clk    (i_clk_156),
                .i_wr_rst    (i_rst),
                .i_wr_en     (builder_event_valid[cdc_idx]),
                .i_wr_data   (builder_event_payload[cdc_idx*PAYLOAD_W +: PAYLOAD_W]),
                .o_wr_full   (event_cdc_wr_full[cdc_idx]),
                .o_wr_drop_count(event_cdc_wr_drop_count[cdc_idx*64 +: 64]),
                .i_rd_clk    (i_clk_250),
                .i_rd_rst    (i_rst),
                .i_rd_en     (event_cdc_rd_en[cdc_idx]),
                .o_rd_empty  (event_cdc_rd_empty[cdc_idx]),
                .o_rd_valid  (event_cdc_rd_valid[cdc_idx]),
                .o_rd_data   (event_cdc_rd_data[cdc_idx*PAYLOAD_W +: PAYLOAD_W])
            );
        end
    endgenerate

    rx_dma_stage #(
        .SYMBOL_NUM (SYMBOL_NUM),
        .PAYLOAD_W  (PAYLOAD_W)
    ) dma_stage_dut (
        .i_clk              (i_clk_250),
        .i_rst              (i_rst),
        .i_dma_timestamp    (w_dma_ts_bin),
        .i_event_empty      (event_cdc_rd_empty),
        .i_event_valid      (event_cdc_rd_valid),
        .i_event_payload    (event_cdc_rd_data),
        .o_event_pop        (event_cdc_rd_en),
        .i_que_iova_addr    (rx_dma_que_iova_addr),
        .i_que_slot_num     (rx_dma_que_slot_num),
        .i_que_enable       (rx_dma_que_enable),
        .i_que_cons_ptr     (rx_dma_que_cons_ptr),
        .i_reg_reset        (rx_dma_reg_reset),
        .o_que_prod_ptr     (rx_dma_que_prod_ptr),
        .o_que_drop_count   (rx_dma_que_drop_count),
        .o_que_status       (rx_dma_que_status),
        .o_event_pop_count  (rx_dma_event_pop_count),
        .o_event_send_count (rx_dma_event_send_count),
        .i_rq_ready         (1'b1),
        .o_rq_valid         (dma_rq_valid),
        .o_rq_type          (dma_rq_type),
        .o_rq_payload_last  (dma_rq_payload_last),
        .o_rq_addr          (dma_rq_addr),
        .o_rq_payload_dw_count(dma_rq_payload_dw_count),
        .o_rq_tag           (dma_rq_tag),
        .o_rq_tc            (dma_rq_tc),
        .o_rq_payload       (dma_rq_payload)
    );

    always #(CLK156_PERIOD/2) i_clk_156 = ~i_clk_156;
    always #(CLK250_PERIOD/2) i_clk_250 = ~i_clk_250;

    task automatic drive_xgmii_word (
        input [63:0] d,
        input [7:0]  c
    );
        begin
            @(posedge i_clk_156);
            i_xgmii_rxd <= d;
            i_xgmii_rxc <= c;
        end
    endtask

    task automatic send_idle (
        input integer cycles
    );
        integer idx;
        begin
            for (idx = 0; idx < cycles; idx = idx + 1) begin
                drive_xgmii_word(IDLE_WORD, 8'hFF);
            end
        end
    endtask

    task automatic set16 (
        input integer off,
        input [127:0] line
    );
        integer i;
        begin
            for (i = 0; i < 16; i = i + 1) begin
                frame_bytes[off+i] = line[127-(i*8) -: 8];
            end
        end
    endtask

    task automatic set2 (
        input integer off,
        input [15:0] line
    );
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
            set16('h0000, 128'h01_00_5e_00_00_01_00_11_22_33_44_55_08_00_45_00);
            set16('h0010, 128'h01_59_00_01_00_00_40_11_cc_b4_c0_a8_01_32_e9_01);
            set16('h0020, 128'h02_03_15_b3_04_d2_01_45_a9_a8_4e_41_53_44_51_54);
            set16('h0030, 128'h45_53_54_20_00_00_00_00_00_00_00_01_00_09_00_24);
            set16('h0040, 128'h41_00_0d_00_00_0d_18_c3_4e_77_da_00_00_00_00_00);
            set16('h0050, 128'h00_00_01_42_00_00_00_0a_41_41_50_4c_20_20_20_20);
            set16('h0060, 128'h00_00_00_64_00_28_46_00_0d_00_00_1b_d6_53_cf_f0);
            set16('h0070, 128'hbf_00_00_00_00_00_00_00_02_42_00_00_01_f4_41_41);
            set16('h0080, 128'h50_4c_20_20_20_20_00_00_00_c8_54_53_53_4d_00_17);
            set16('h0090, 128'h58_00_0d_00_00_1b_2e_c7_3b_cc_cf_00_00_00_00_00);
            set16('h00a0, 128'h00_00_01_00_00_00_0a_00_17_58_00_0d_00_00_1b_2e);
            set16('h00b0, 128'hc7_3b_cc_cf_00_00_00_00_00_00_00_02_00_00_00_64);
            set16('h00c0, 128'h00_24_43_00_0d_00_01_1f_1a_fd_1d_81_79_00_00_00);
            set16('h00d0, 128'h00_00_00_00_02_00_00_00_c8_00_00_00_00_00_02_8c);
            set16('h00e0, 128'h56_4e_00_00_00_c8_00_1f_45_00_0d_00_02_0d_73_25);
            set16('h00f0, 128'ha3_d0_68_00_00_00_00_00_00_00_02_00_00_00_c8_00);
            set16('h0100, 128'h00_00_00_00_00_46_27_00_24_41_00_0d_00_00_0d_18);
            set16('h0110, 128'hc3_4e_77_da_00_00_00_00_00_00_00_03_42_00_00_00);
            set16('h0120, 128'h14_41_41_50_4c_20_20_20_20_00_00_01_2c_00_23_55);
            set16('h0130, 128'h00_0d_00_00_1e_96_69_a4_b6_a9_00_00_00_00_00_00);
            set16('h0140, 128'h00_03_00_00_00_00_00_00_00_04_00_00_00_64_00_00);
            set16('h0150, 128'h01_90_00_13_44_00_0d_00_00_0d_20_7f_24_3f_50_00);
            frame_bytes['h0160] = 8'h00;
            frame_bytes['h0161] = 8'h00;
            frame_bytes['h0162] = 8'h00;
            frame_bytes['h0163] = 8'h00;
            frame_bytes['h0164] = 8'h00;
            frame_bytes['h0165] = 8'h00;
            frame_bytes['h0166] = 8'h04;
        end
    endtask

    task automatic load_hsbc_frame_bytes;
        begin
            clear_frame_bytes();
            set16('h0000, 128'h01_00_5e_00_00_01_00_11_22_33_44_55_08_00_45_00);
            set16('h0010, 128'h01_59_00_01_00_00_40_11_cc_b4_c0_a8_01_32_e9_01);
            set16('h0020, 128'h02_03_15_b3_04_d2_01_45_4e_91_4e_41_53_44_51_54);
            set16('h0030, 128'h45_53_54_20_00_00_00_00_00_00_00_01_00_09_00_24);
            set16('h0040, 128'h41_0e_e8_00_00_0d_18_c3_4e_77_da_00_00_00_00_00);
            set16('h0050, 128'h00_00_01_53_00_00_00_0a_48_53_42_43_20_20_20_20);
            set16('h0060, 128'h00_00_00_64_00_28_46_0e_e8_00_00_1b_d6_53_cf_f0);
            set16('h0070, 128'hbf_00_00_00_00_00_00_00_02_53_00_00_01_f4_48_53);
            set16('h0080, 128'h42_43_20_20_20_20_00_00_00_c8_54_53_53_4d_00_17);
            set16('h0090, 128'h58_0e_e8_00_00_1b_2e_c7_3b_cc_cf_00_00_00_00_00);
            set16('h00a0, 128'h00_00_01_00_00_00_0a_00_17_58_0e_e8_00_00_1b_2e);
            set16('h00b0, 128'hc7_3b_cc_cf_00_00_00_00_00_00_00_02_00_00_00_64);
            set16('h00c0, 128'h00_24_43_0e_e8_00_01_1f_1a_fd_1d_81_79_00_00_00);
            set16('h00d0, 128'h00_00_00_00_02_00_00_00_c8_00_00_00_00_00_02_8c);
            set16('h00e0, 128'h56_4e_00_00_00_c8_00_1f_45_0e_e8_00_02_0d_73_25);
            set16('h00f0, 128'ha3_d0_68_00_00_00_00_00_00_00_02_00_00_00_c8_00);
            set16('h0100, 128'h00_00_00_00_00_46_27_00_24_41_0e_e8_00_00_0d_18);
            set16('h0110, 128'hc3_4e_77_da_00_00_00_00_00_00_00_03_53_00_00_00);
            set16('h0120, 128'h14_48_53_42_43_20_20_20_20_00_00_01_2c_00_23_55);
            set16('h0130, 128'h0e_e8_00_00_1e_96_69_a4_b6_a9_00_00_00_00_00_00);
            set16('h0140, 128'h00_03_00_00_00_00_00_00_00_04_00_00_00_64_00_00);
            set16('h0150, 128'h01_90_00_13_44_0e_e8_00_00_0d_20_7f_24_3f_50_00);
            frame_bytes['h0160] = 8'h00;
            frame_bytes['h0161] = 8'h00;
            frame_bytes['h0162] = 8'h00;
            frame_bytes['h0163] = 8'h00;
            frame_bytes['h0164] = 8'h00;
            frame_bytes['h0165] = 8'h00;
            frame_bytes['h0166] = 8'h04;
        end
    endtask

    function [63:0] pack_data_word (input integer base_idx, input integer valid_bytes);
        integer lane;
        reg [63:0] tmp;
        begin
            tmp = 64'd0;
            for (lane = 0; lane < 8; lane = lane + 1) begin
                if (lane < valid_bytes) begin
                    tmp[lane*8 +: 8] = frame_bytes[base_idx + lane];
                end else begin
                    tmp[lane*8 +: 8] = 8'h07;
                end
            end
            pack_data_word = tmp;
        end
    endfunction

    function [63:0] pack_term_word (input integer base_idx, input integer valid_bytes);
        integer lane;
        reg [63:0] tmp;
        begin
            tmp = 64'd0;
            for (lane = 0; lane < 8; lane = lane + 1) begin
                if (lane < valid_bytes) begin
                    tmp[lane*8 +: 8] = frame_bytes[base_idx + lane];
                end else if (lane == valid_bytes) begin
                    tmp[lane*8 +: 8] = 8'hFD;
                end else begin
                    tmp[lane*8 +: 8] = 8'h07;
                end
            end
            pack_term_word = tmp;
        end
    endfunction

    function [7:0] pack_term_ctrl (input integer valid_bytes);
        begin
            pack_term_ctrl = (8'hFF << valid_bytes);
        end
    endfunction

    task automatic send_loaded_frame (
        input [1:0] symbol_sel
    );
        integer idx;
        integer bytes_left;
        integer valid_bytes;
        begin
            pending_symbol = symbol_sel;

            send_idle(2);
            drive_xgmii_word(START_L0_WORD, 8'b0000_0001);

            idx = 0;
            while ((FRAME_BYTES - idx) > 8) begin
                drive_xgmii_word(pack_data_word(idx, 8), 8'h00);
                idx = idx + 8;
            end

            bytes_left = FRAME_BYTES - idx;
            if (bytes_left == 8) begin
                drive_xgmii_word(pack_data_word(idx, 8), 8'h00);
                drive_xgmii_word(64'h0707_0707_0707_07FD, 8'hFF);
            end else begin
                valid_bytes = bytes_left;
                drive_xgmii_word(pack_term_word(idx, valid_bytes), pack_term_ctrl(valid_bytes));
            end

            send_idle(4);
        end
    endtask

    task automatic wait_for_pipeline_quiet;
        integer quiet_cycles;
        begin
            quiet_cycles = 0;
            while (quiet_cycles < 32) begin
                @(posedge i_clk_156);
                if (parser_msg_valid || (builder_event_valid != 2'b00)) begin
                    quiet_cycles = 0;
                end else begin
                    quiet_cycles = quiet_cycles + 1;
                end
            end
        end
    endtask

    task automatic wait_for_dma_quiet;
        integer quiet_cycles;
        begin
            quiet_cycles = 0;
            while (quiet_cycles < 32) begin
                @(posedge i_clk_250);
                if ((event_cdc_rd_empty != {SYMBOL_NUM{1'b1}}) || dma_rq_valid) begin
                    quiet_cycles = 0;
                end else begin
                    quiet_cycles = quiet_cycles + 1;
                end
            end
        end
    endtask

    always @(posedge i_clk_156) begin
        if (i_rst) begin
            last_mac_frame_started <= 1'b0;
            last_mac_frame_ts <= 48'd0;
            last_w_dma_ts_gray_rx <= 48'd0;
        end else begin
            last_mac_frame_started <= mac_frame_started;
            last_w_dma_ts_gray_rx <= w_dma_ts_gray_rx;

            if (mac_frame_ts != last_mac_frame_ts) begin
                if (!last_mac_frame_started) begin
                    $display("[%0t] ERROR: MAC frame timestamp changed without a prior frame_started pulse",
                             $time);
                    total_errors_mac <= total_errors_mac + 1;
                end
                if (w_dma_ts_gray_rx == last_w_dma_ts_gray_rx) begin
                    $display("[%0t] ERROR: DMA Gray timestamp did not advance across a frame start",
                             $time);
                    total_errors_mac <= total_errors_mac + 1;
                end
                last_mac_frame_ts <= mac_frame_ts;
                case (pending_symbol)
                    SYMBOL_AAPL: begin
                        expected_aapl_frame_ts <= mac_frame_ts;
                        aapl_frame_starts <= aapl_frame_starts + 1;
                        $display("[%0t] MAC frame start captured for AAPL frame_ts=%012h",
                                 $time, mac_frame_ts);
                    end
                    SYMBOL_HSBC: begin
                        expected_hsbc_frame_ts <= mac_frame_ts;
                        hsbc_frame_starts <= hsbc_frame_starts + 1;
                        $display("[%0t] MAC frame start captured for HSBC frame_ts=%012h",
                                 $time, mac_frame_ts);
                    end
                    default: begin
                        $display("[%0t] ERROR: observed MAC frame timestamp %012h with no pending symbol",
                                 $time, mac_frame_ts);
                        total_errors_mac <= total_errors_mac + 1;
                    end
                endcase
                pending_symbol <= SYMBOL_NONE;
            end

            if (parser_msg_valid) begin
                $display("[%0t] parser msg type=%02h locate=%04h seq=%016h frame_ts=%012h",
                         $time, parser_msg_type, parser_stock_locate, parser_seq_num, parser_frame_ts);

                case (parser_stock_locate)
                    AAPL_LOCATE: begin
                        aapl_parser_msgs <= aapl_parser_msgs + 1;
                        if (parser_frame_ts != expected_aapl_frame_ts) begin
                            $display("[%0t] ERROR: parser AAPL frame_ts mismatch expected=%012h actual=%012h",
                                     $time, expected_aapl_frame_ts, parser_frame_ts);
                            total_errors_mac <= total_errors_mac + 1;
                        end
                    end
                    HSBC_LOCATE: begin
                        hsbc_parser_msgs <= hsbc_parser_msgs + 1;
                        if (parser_frame_ts != expected_hsbc_frame_ts) begin
                            $display("[%0t] ERROR: parser HSBC frame_ts mismatch expected=%012h actual=%012h",
                                     $time, expected_hsbc_frame_ts, parser_frame_ts);
                            total_errors_mac <= total_errors_mac + 1;
                        end
                    end
                    default: begin
                    end
                endcase
            end

            if (builder_event_valid[0]) begin
                aapl_builder_events <= aapl_builder_events + 1;
                $display("[%0t] builder AAPL payload locate=%04h frame_ts=%012h",
                         $time, aapl_payload_locate, aapl_payload_ts);
                if (aapl_payload_locate != AAPL_LOCATE) begin
                    $display("[%0t] ERROR: AAPL builder payload locate mismatch %04h",
                             $time, aapl_payload_locate);
                    total_errors_mac <= total_errors_mac + 1;
                end
                if (aapl_payload_ts != expected_aapl_frame_ts) begin
                    $display("[%0t] ERROR: AAPL builder payload frame_ts mismatch expected=%012h actual=%012h",
                             $time, expected_aapl_frame_ts, aapl_payload_ts);
                    total_errors_mac <= total_errors_mac + 1;
                end
            end

            if (builder_event_valid[1]) begin
                hsbc_builder_events <= hsbc_builder_events + 1;
                $display("[%0t] builder HSBC payload locate=%04h frame_ts=%012h",
                         $time, hsbc_payload_locate, hsbc_payload_ts);
                if (hsbc_payload_locate != HSBC_LOCATE) begin
                    $display("[%0t] ERROR: HSBC builder payload locate mismatch %04h",
                             $time, hsbc_payload_locate);
                    total_errors_mac <= total_errors_mac + 1;
                end
                if (hsbc_payload_ts != expected_hsbc_frame_ts) begin
                    $display("[%0t] ERROR: HSBC builder payload frame_ts mismatch expected=%012h actual=%012h",
                             $time, expected_hsbc_frame_ts, hsbc_payload_ts);
                    total_errors_mac <= total_errors_mac + 1;
                end
            end
        end
    end

    always @(posedge i_clk_250) begin
        if (i_rst) begin
            dma_payload_first_event <= 8'd0;
            dma_payload_locate      <= 16'd0;
            dma_payload_ts          <= 48'd0;
            dma_payload_event_tk    <= 48'd0;
            aapl_first_dma_seen     <= 1'b0;
            hsbc_first_dma_seen     <= 1'b0;
        end else if (dma_rq_valid) begin
            dma_payload_first_event <= dma_record_first_event;
            dma_payload_locate      <= dma_record_locate;
            dma_payload_ts          <= dma_record_frame_ts;
            dma_payload_event_tk    <= dma_record_event_tk;

            if (dma_rq_type != 4'b0001) begin
                $display("[%0t] ERROR: DMA rq_type mismatch expected=1 actual=%0h",
                         $time, dma_rq_type);
                total_errors_dma <= total_errors_dma + 1;
            end
            if (!dma_rq_payload_last) begin
                $display("[%0t] ERROR: DMA payload_last should be asserted", $time);
                total_errors_dma <= total_errors_dma + 1;
            end
            if (dma_rq_payload_dw_count != 11'd8) begin
                $display("[%0t] ERROR: DMA payload dw count mismatch expected=8 actual=%0d",
                         $time, dma_rq_payload_dw_count);
                total_errors_dma <= total_errors_dma + 1;
            end
            if (dma_rq_tc != 3'd0) begin
                $display("[%0t] ERROR: DMA traffic class mismatch expected=0 actual=%0d",
                         $time, dma_rq_tc);
                total_errors_dma <= total_errors_dma + 1;
            end

            dma_timestamp_seen <= 1'b1;
            if (dma_record_event_tk <= last_dma_timestamp) begin
                $display("[%0t] ERROR: DMA event_tk did not advance expected > %0d actual=%0d",
                         $time, last_dma_timestamp, dma_record_event_tk);
                total_errors_dma <= total_errors_dma + 1;
            end
            last_dma_timestamp <= dma_record_event_tk;

            case (dma_record_locate)
                AAPL_LOCATE: begin
                    if (dma_rq_addr != (AAPL_DMA_BASE + (aapl_dma_records * 32))) begin
                        $display("[%0t] ERROR: AAPL DMA addr mismatch expected=%h actual=%h",
                                 $time, (AAPL_DMA_BASE + (aapl_dma_records * 32)), dma_rq_addr);
                        total_errors_dma <= total_errors_dma + 1;
                    end
                    if (dma_rq_tag != 8'h40) begin
                        $display("[%0t] ERROR: AAPL DMA tag mismatch expected=40 actual=%02h",
                                 $time, dma_rq_tag);
                        total_errors_dma <= total_errors_dma + 1;
                    end
                    if (dma_record_frame_ts != expected_aapl_frame_ts) begin
                        $display("[%0t] ERROR: AAPL DMA frame_ts mismatch expected=%012h actual=%012h",
                                 $time, expected_aapl_frame_ts, dma_record_frame_ts);
                        total_errors_dma <= total_errors_dma + 1;
                    end
                    if (!aapl_first_dma_seen) begin
                        if (dma_record_first_event != 8'h01) begin
                            $display("[%0t] ERROR: first AAPL DMA record should have first_event=1 actual=%02h",
                                     $time, dma_record_first_event);
                            total_errors_dma <= total_errors_dma + 1;
                        end
                        aapl_first_dma_seen <= 1'b1;
                    end else if (dma_record_first_event != 8'h00) begin
                        $display("[%0t] ERROR: subsequent AAPL DMA record should have first_event=0 actual=%02h",
                                 $time, dma_record_first_event);
                        total_errors_dma <= total_errors_dma + 1;
                    end
                    aapl_dma_records <= aapl_dma_records + 1;
                end
                HSBC_LOCATE: begin
                    if (dma_rq_addr != (HSBC_DMA_BASE + (hsbc_dma_records * 32))) begin
                        $display("[%0t] ERROR: HSBC DMA addr mismatch expected=%h actual=%h",
                                 $time, (HSBC_DMA_BASE + (hsbc_dma_records * 32)), dma_rq_addr);
                        total_errors_dma <= total_errors_dma + 1;
                    end
                    if (dma_rq_tag != 8'h41) begin
                        $display("[%0t] ERROR: HSBC DMA tag mismatch expected=41 actual=%02h",
                                 $time, dma_rq_tag);
                        total_errors_dma <= total_errors_dma + 1;
                    end
                    if (dma_record_frame_ts != expected_hsbc_frame_ts) begin
                        $display("[%0t] ERROR: HSBC DMA frame_ts mismatch expected=%012h actual=%012h",
                                 $time, expected_hsbc_frame_ts, dma_record_frame_ts);
                        total_errors_dma <= total_errors_dma + 1;
                    end
                    if (!hsbc_first_dma_seen) begin
                        if (dma_record_first_event != 8'h01) begin
                            $display("[%0t] ERROR: first HSBC DMA record should have first_event=1 actual=%02h",
                                     $time, dma_record_first_event);
                            total_errors_dma <= total_errors_dma + 1;
                        end
                        hsbc_first_dma_seen <= 1'b1;
                    end else if (dma_record_first_event != 8'h00) begin
                        $display("[%0t] ERROR: subsequent HSBC DMA record should have first_event=0 actual=%02h",
                                 $time, dma_record_first_event);
                        total_errors_dma <= total_errors_dma + 1;
                    end
                    hsbc_dma_records <= hsbc_dma_records + 1;
                end
                default: begin
                    $display("[%0t] ERROR: unexpected DMA stock_locate=%04h",
                             $time, dma_record_locate);
                    total_errors_dma <= total_errors_dma + 1;
                end
            endcase
        end
    end

    

    initial begin
        i_clk_156            = 1'b0;
        i_clk_250            = 1'b0;
        i_rst                = 1'b1;
        i_xgmii_rxd          = IDLE_WORD;
        i_xgmii_rxc          = 8'hFF;
        i_rx_status          = 1'b1;
        pending_symbol       = SYMBOL_NONE;
        last_mac_frame_started = 1'b0;
        last_mac_frame_ts    = 48'd0;
        last_w_dma_ts_gray_rx = 48'd0;
        expected_aapl_frame_ts = 48'd0;
        expected_hsbc_frame_ts = 48'd0;
        rx_dma_que_iova_addr = {HSBC_DMA_BASE, AAPL_DMA_BASE};
        rx_dma_que_slot_num  = {64'd16, 64'd16};
        rx_dma_que_enable    = {64'd1, 64'd1};
        rx_dma_que_cons_ptr  = {64'd0, 64'd0};
        rx_dma_reg_reset     = 1'b0;
        aapl_frame_starts    = 0;
        hsbc_frame_starts    = 0;
        aapl_parser_msgs     = 0;
        hsbc_parser_msgs     = 0;
        aapl_builder_events  = 0;
        hsbc_builder_events  = 0;
        aapl_dma_records     = 0;
        hsbc_dma_records     = 0;
        last_dma_timestamp   = 48'd0;
        dma_timestamp_seen   = 1'b0;
        dma_payload_first_event = 8'd0;
        dma_payload_locate   = 16'd0;
        dma_payload_ts       = 48'd0;
        dma_payload_event_tk = 48'd0;
        aapl_first_dma_seen  = 1'b0;
        hsbc_first_dma_seen  = 1'b0;
        total_errors_mac     = 0;
        total_errors_dma     = 0;
        total_errors         = 0;

        repeat (5) @(posedge i_clk_156);
        i_rst <= 1'b0;
        repeat (4) @(posedge i_clk_156);

        load_aapl_frame_bytes();
        send_loaded_frame(SYMBOL_AAPL);
        wait_for_pipeline_quiet();
        wait_for_dma_quiet();

        load_hsbc_frame_bytes();
        send_loaded_frame(SYMBOL_HSBC);
        wait_for_pipeline_quiet();
        wait_for_dma_quiet();

        repeat (20) @(posedge i_clk_156);

        if (aapl_frame_starts != 1) begin
            $display("[%0t] ERROR: expected exactly one AAPL frame start, got %0d",
                     $time, aapl_frame_starts);
            total_errors = total_errors + 1;
        end
        if (hsbc_frame_starts != 1) begin
            $display("[%0t] ERROR: expected exactly one HSBC frame start, got %0d",
                     $time, hsbc_frame_starts);
            total_errors = total_errors + 1;
        end
        if (aapl_parser_msgs == 0) begin
            $display("[%0t] ERROR: no AAPL parser messages observed", $time);
            total_errors = total_errors + 1;
        end
        if (hsbc_parser_msgs == 0) begin
            $display("[%0t] ERROR: no HSBC parser messages observed", $time);
            total_errors = total_errors + 1;
        end
        if (aapl_builder_events == 0) begin
            $display("[%0t] ERROR: no AAPL builder events observed", $time);
            total_errors = total_errors + 1;
        end
        if (hsbc_builder_events == 0) begin
            $display("[%0t] ERROR: no HSBC builder events observed", $time);
            total_errors = total_errors + 1;
        end
        if (aapl_dma_records == 0) begin
            $display("[%0t] ERROR: no AAPL DMA records observed", $time);
            total_errors = total_errors + 1;
        end
        if (hsbc_dma_records == 0) begin
            $display("[%0t] ERROR: no HSBC DMA records observed", $time);
            total_errors = total_errors + 1;
        end
        if (expected_aapl_frame_ts == expected_hsbc_frame_ts) begin
            $display("[%0t] ERROR: frame timestamps for AAPL and HSBC should differ", $time);
            total_errors = total_errors + 1;
        end
        if (rx_dma_que_drop_count != {(SYMBOL_NUM*64){1'b0}}) begin
            $display("[%0t] ERROR: DMA queue drop count should remain zero", $time);
            total_errors = total_errors + 1;
        end
        if (!dma_timestamp_seen) begin
            $display("[%0t] ERROR: no DMA timestamp updates observed", $time);
            total_errors = total_errors + 1;
        end
        if (aapl_dma_records != aapl_builder_events) begin
            $display("[%0t] ERROR: AAPL DMA record count mismatch expected builder_events=%0d actual dma_records=%0d",
                     $time, aapl_builder_events, aapl_dma_records);
            total_errors = total_errors + 1;
        end
        if (hsbc_dma_records != hsbc_builder_events) begin
            $display("[%0t] ERROR: HSBC DMA record count mismatch expected builder_events=%0d actual dma_records=%0d",
                     $time, hsbc_builder_events, hsbc_dma_records);
            total_errors = total_errors + 1;
        end
        if (rx_dma_que_prod_ptr[63:0] != aapl_dma_records) begin
            $display("[%0t] ERROR: AAPL DMA prod ptr mismatch expected=%0d actual=%0d",
                     $time, aapl_dma_records, rx_dma_que_prod_ptr[63:0]);
            total_errors = total_errors + 1;
        end
        if (rx_dma_que_prod_ptr[127:64] != hsbc_dma_records) begin
            $display("[%0t] ERROR: HSBC DMA prod ptr mismatch expected=%0d actual=%0d",
                     $time, hsbc_dma_records, rx_dma_que_prod_ptr[127:64]);
            total_errors = total_errors + 1;
        end
        if (rx_dma_que_status[2:0] != 3'b001) begin
            $display("[%0t] ERROR: AAPL DMA queue status mismatch expected busy/full/en=001 actual=%03b",
                     $time, rx_dma_que_status[2:0]);
            total_errors = total_errors + 1;
        end
        if (rx_dma_que_status[66:64] != 3'b001) begin
            $display("[%0t] ERROR: HSBC DMA queue status mismatch expected busy/full/en=001 actual=%03b",
                     $time, rx_dma_que_status[66:64]);
            total_errors = total_errors + 1;
        end
        total_errors = total_errors + total_errors_mac + total_errors_dma;

        $display("[%0t] summary: aapl_parser_msgs=%0d hsbc_parser_msgs=%0d aapl_builder_events=%0d hsbc_builder_events=%0d aapl_dma_records=%0d hsbc_dma_records=%0d mac_errors=%0d dma_errors=%0d errors=%0d",
                 $time, aapl_parser_msgs, hsbc_parser_msgs, aapl_builder_events, hsbc_builder_events,
                 aapl_dma_records, hsbc_dma_records, total_errors_mac, total_errors_dma, total_errors);

        if (total_errors != 0) begin
            $fatal(1, "MAC->parser->builder->dma timestamp TB failed with %0d errors", total_errors);
        end

        $display("[%0t] PASS: MAC->parser->builder->dma timestamp flow validated", $time);
        $finish;
    end

    initial begin
        $dumpvars(0, tb_mac_order_book_builder);
    end

endmodule
