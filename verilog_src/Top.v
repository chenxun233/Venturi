`timescale 1ns / 1ps

// =============================================================================
// Top.v - Venturi FPGA NIC Top Level Module
// =============================================================================
// Top-level wrapper using the modular PCIe interface architecture:
//   pcie_wrapper.v → CQ_parser, CC_formatter, RQ_formatter, RC_parser
//   rx_dma_config → Register handling for RX DMA configuration
// =============================================================================

module top #(
parameter DATA_WIDTH = 256,
parameter KEEP_WIDTH = DATA_WIDTH / 32,
parameter BAR0_SIZE  = 16
)(
// =========================================================================
// PCIe Interface
// =========================================================================
output wire [7:0]   pci_exp_txn         ,
output wire [7:0]   pci_exp_txp         ,
input  wire [7:0]   pci_exp_rxn         ,
input  wire [7:0]   pci_exp_rxp         ,
input  wire         i_host_clk_100_p    ,
input  wire         i_host_clk_100_n    ,
// =========================================================================
// system Clock (100MHz from the board)
// =========================================================================
input  wire         clk_100_p           ,
input  wire         clk_100_n           ,
// =========================================================================
// PCIe Reset (active-low from slot PERST#)
// =========================================================================
input  wire         i_pcie_rst_n        ,
//  =========================================================================
// SFP interface 0
//  =========================================================================
input  wire         i_gt_refclk_p       ,
input  wire         i_gt_refclk_n       ,
input  wire         i_gt_rx_p_0         ,
input  wire         i_gt_rx_n_0         ,
output wire         o_gt_tx_p_0         ,
output wire         o_gt_tx_n_0         ,
output wire         sfp_0_tx_disable    ,
output wire         sfp_0_rs            ,
// SFP diagnostic inputs
input  wire         sfp_0_los           ,   // SFP0 Loss-of-Signal (1 = no RX light/signal)
input  wire         sfp_0_npres             // SFP0 Not-Present    (0 = module inserted)
);


assign sfp_0_tx_disable = 1'b0;
assign sfp_0_rs = 1'b0;

// =========================================================================
// Clock Buffer for Reference Clock
// =========================================================================
wire pcie_clk_50;
wire pcie_clk_100;
wire sys_clk_100;

IBUFDS_GTE3 #(
    .REFCLK_EN_TX_PATH  (1'b0),
    .REFCLK_HROW_CK_SEL (2'b00),
    .REFCLK_ICNTL_RX    (2'b00)
) refclk_ibuf (
    .O     (pcie_clk_100),
    .ODIV2 (pcie_clk_50),
    .CEB   (1'b0),
    .I     (i_host_clk_100_p),
    .IB    (i_host_clk_100_n)
);

IBUFGDS #(
   .DIFF_TERM("FALSE"),
   .IBUF_LOW_PWR("FALSE")   
)
clk_100mhz_ibufg_inst (
   .O   (sys_clk_100),
   .I   (clk_100_p),
   .IB  (clk_100_n) 
);

// =========================================================================
// Internal Wires - User Clock and Reset
// =========================================================================
wire user_clk_250;
wire user_reset_p;

// =========================================================================
// Internal Wires - CQ Parser Outputs
// =========================================================================
wire                    cq_valid;
wire [3:0]              cq_type;
wire [BAR0_SIZE-1:0]    cq_reg_addr;
wire [63:0]             cq_payload;
wire [2:0]              cq_bar_id;
wire [15:0]             cq_requester_id;
wire [7:0]              cq_tag;
wire [2:0]              cq_tc;
wire [6:0]              cq_lower_addr;
wire [10:0]             cq_payload_dw_count;
wire                    cq_last;

// =========================================================================
// Internal Wires - CC Formatter Inputs
// =========================================================================
wire                    cc_ready;
wire                    cc_valid;
wire [15:0]             cc_requester_id;
wire [7:0]              cc_tag;
wire [2:0]              cc_tc;
wire [6:0]              cc_lower_addr;
wire [10:0]             cc_dword_count;
wire [2:0]              cc_status;
wire [DATA_WIDTH/2-1:0] cc_payload;
wire                    cc_last;

// =========================================================================
// Internal Wires - RQ Formatter Inputs
// =========================================================================
wire                    rq_ready;
wire                    rq_valid;
wire [3:0]              rq_type;
wire                    rq_payload_last;
wire [63:0]             rq_addr;
wire [10:0]             rq_payload_dw_count;
wire [7:0]              rq_tag;
wire [2:0]              rq_tc;
wire [DATA_WIDTH-1:0]   rq_payload;
wire                    dma_rq_ready;
wire                    dma_rq_valid;
wire [3:0]              dma_rq_type;
wire                    dma_rq_payload_last;
wire [63:0]             dma_rq_addr;
wire [10:0]             dma_rq_payload_dw_count;
wire [7:0]              dma_rq_tag;
wire [2:0]              dma_rq_tc;
wire [DATA_WIDTH-1:0]   dma_rq_payload;

// =========================================================================
// Internal Wires - RC Parser Outputs (realigned by gearbox)
// =========================================================================
wire                    rc_valid;
wire [11:0]             rc_lower_addr;
wire [3:0]              rc_err_code;
wire [7:0]              rc_tag;
wire [12:0]             rc_payload_byte_count;
wire                    rc_request_completed;
wire [15:0]             rc_requester_id;
wire                    rc_payload_last;
wire [DATA_WIDTH-1:0]   rc_payload;
wire [KEEP_WIDTH-1:0]   rc_payload_dw_keep;
wire                    rc_posioned;
wire [12:0]             rc_payload_dw_count;

// =========================================================================
// Internal Wires - Configuration
// =========================================================================
wire [2:0]              cfg_max_payload;
wire [2:0]              cfg_max_read_req;
wire [15:0]             pcie_requester_id;
localparam SYMBOL_NUM = 2;
localparam [SYMBOL_NUM*16-1:0] SYMBOL_STOCK_LOCATE_INIT = {
    16'h0000,
    16'h0000
    
};
localparam [SYMBOL_NUM*32-1:0] SYMBOL_PRICE_BASE_INIT = {
    32'd0,
    32'd0
};

// =========================================================================
// Internal Wires - Status
// =========================================================================





// =========================================================================
// PCIe Interface Instantiation
// =========================================================================

pcie_wrapper #(
    .DATA_WIDTH                 (DATA_WIDTH),
    .BAR0_SIZE                  (BAR0_SIZE)
) pcie_wrapper_inst (
    // PCIe Physical Pins
    .o_pci_exp_txn              (pci_exp_txn),
    .o_pci_exp_txp              (pci_exp_txp),
    .i_pci_exp_rxn              (pci_exp_rxn),
    .i_pci_exp_rxp              (pci_exp_rxp),

    // System Interface
    .pcie_clk_50                (pcie_clk_50),
    .pcie_clk_100               (pcie_clk_100),
    .i_pcie_rst_n               (i_pcie_rst_n),        // Active-low (IP configured for ACTIVE LOW)
    .o_user_clk_250             (user_clk_250),
    .o_user_reset_p             (user_reset_p),      // Active-high

    // CQ Parser Outputs
    .o_cq_valid                 (cq_valid           ),
    .o_cq_type                  (cq_type            ),
    .o_cq_reg_addr              (cq_reg_addr        ),
    .o_cq_payload               (cq_payload         ),
    .o_cq_bar_id                (cq_bar_id          ),
    .o_cq_requester_id          (cq_requester_id    ),
    .o_cq_tag                   (cq_tag             ),
    .o_cq_tc                    (cq_tc              ),
    .o_cq_lower_addr            (cq_lower_addr      ),
    .o_cq_payload_dw_count      (cq_payload_dw_count),
    .o_cq_last                  (cq_last            ),

    // CC Formatter Inputs
    .o_cc_ready                 (cc_ready           ),
    .i_cc_valid                 (cc_valid           ),
    .i_cc_requester_id          (cc_requester_id    ),
    .i_cc_tag                   (cc_tag             ),
    .i_cc_tc                    (cc_tc              ),
    .i_cc_lower_addr            (cc_lower_addr      ),
    .i_cc_dword_count           (cc_dword_count     ),
    .i_cc_status                (cc_status          ),
    .i_cc_payload               (cc_payload         ),
    .i_cc_last                  (cc_last            ),

    // RQ Formatter Inputs
    .o_rq_ready                 (rq_ready),
    .i_rq_valid                 (rq_valid),
    .i_rq_type                  (rq_type),
    .i_rq_payload_last          (rq_payload_last),
    .i_rq_addr                  (rq_addr),
    .i_rq_payload_dw_count      (rq_payload_dw_count),
    .i_rq_tag                   (rq_tag),
    .i_rq_tc                    (rq_tc),
    .i_rq_payload               (rq_payload),
    // RC Parser Outputs (realigned by gearbox)
    .o_rc_lower_addr            (rc_lower_addr),
    .o_rc_err_code              (rc_err_code),
    .o_rc_valid                 (rc_valid),
    .o_rc_tag                   (rc_tag),
    .o_rc_payload_byte_count    (rc_payload_byte_count),
    .o_rc_request_completed     (rc_request_completed),
    .o_rc_requester_id          (rc_requester_id),
    .o_rc_payload_last          (rc_payload_last),
    .o_rc_payload               (rc_payload),
    .o_rc_payload_dw_keep       (rc_payload_dw_keep),
    .o_rc_posioned              (rc_posioned),
    .o_rc_payload_dw_count      (rc_payload_dw_count),

    // Configuration Outputs
    .o_cfg_max_payload          (cfg_max_payload),
    .o_cfg_max_read_req         (cfg_max_read_req),
    .o_pcie_requester_id        (pcie_requester_id)
);

wire rx_dma_reg_reset;
wire       rx_sw_reset_156;
wire       rx_logic_reset_156;
wire       rx_logic_reset_250;

rx_dma_config #(
    .DATA_WIDTH                 (DATA_WIDTH),
    .BAR0_SIZE                  (BAR0_SIZE),
    .SYMBOL_NUM                 (SYMBOL_NUM),
    .SYMBOL_STOCK_LOCATE_INIT   (SYMBOL_STOCK_LOCATE_INIT),
    .SYMBOL_PRICE_BASE_INIT     (SYMBOL_PRICE_BASE_INIT)
) rx_dma_config_inst (
    .user_clk                (user_clk_250),
    .user_reset_p            (user_reset_p),
    .cq_valid                (cq_valid),
    .cq_type                 (cq_type),
    .cq_reg_addr             (cq_reg_addr),
    .cq_payload              (cq_payload),
    .cq_requester_id         (cq_requester_id),
    .cq_tag                  (cq_tag),
    .cq_tc                   (cq_tc),
    .cq_lower_addr           (cq_lower_addr),
    .cq_payload_dw_count     (cq_payload_dw_count),
    .cc_ready                (cc_ready),
    .cc_valid                (cc_valid),
    .cc_requester_id         (cc_requester_id),
    .cc_tag                  (cc_tag),
    .cc_tc                   (cc_tc),
    .cc_lower_addr           (cc_lower_addr),
    .cc_dword_count          (cc_dword_count),
    .cc_status               (cc_status),
    .cc_payload              (cc_payload),
    .cc_last                 (cc_last),
    .o_rx_que_iova_addr     (rx_dma_que_iova_addr),
    .o_rx_que_slot_num      (rx_dma_que_slot_num),
    .o_rx_que_enable        (rx_dma_que_enable),
    .o_rx_que_cons_ptr      (rx_dma_que_cons_ptr),
    .o_symbol_stock_locate  (symbol_stock_locate_cfg),
    .o_symbol_price_base    (symbol_price_base_cfg),
    .o_reg_reset            (rx_dma_reg_reset),
    .i_rx_que_prod_ptr      (rx_dma_que_prod_ptr),
    .i_rx_que_drop_count    (rx_dma_que_drop_count),
    .i_rx_que_status        (rx_dma_que_status),
    .i_rx_dbg_pcs_frame_count (rx_dbg_pcs_frame_count),
    .i_rx_dbg_frame_count   (rx_dbg_frame_count),
    .i_rx_dbg_msg_count     (rx_dbg_msg_count),
    .i_rx_dbg_event_count   (rx_dbg_event_count),
    .i_dma_timestamp        (w_dma_ts_bin)
);

rx_dma_stage #(
    .SYMBOL_NUM         (SYMBOL_NUM),
    .PAYLOAD_W          (EVENT_PAYLOAD_W)
) rx_dma_stage_inst (
    .i_clk              (user_clk_250),
    .i_rst              (rx_logic_reset_250),
    .i_dma_timestamp    (w_dma_ts_bin),
    .i_event_empty      (event_cdc_rd_empty),
    .i_event_valid      (event_cdc_rd_valid),
    .i_event_payload    (event_cdc_rd_data),
    .o_event_pop        (event_cdc_rd_en),
    .i_que_iova_addr    (rx_dma_que_iova_addr),
    .i_que_slot_num     (rx_dma_que_slot_num),
    .i_que_enable       (rx_dma_que_enable),
    .i_que_cons_ptr     (rx_dma_que_cons_ptr),
    .o_que_prod_ptr     (rx_dma_que_prod_ptr),
    .o_que_drop_count   (rx_dma_que_drop_count),
    .o_que_status       (rx_dma_que_status),
    .i_rq_ready         (dma_rq_ready),
    .i_reg_reset        (rx_dma_reg_reset),
    .o_rq_valid         (dma_rq_valid),
    .o_rq_type          (dma_rq_type),
    .o_rq_payload_last  (dma_rq_payload_last),
    .o_rq_addr          (dma_rq_addr),
    .o_rq_payload_dw_count(dma_rq_payload_dw_count),
    .o_rq_tag           (dma_rq_tag),
    .o_rq_tc            (dma_rq_tc),
    .o_rq_payload       (dma_rq_payload)
);

wire    w_xgmii_clk;
wire    w_sys_rst;
wire    w_xgmii_tx_rst;
wire    w_xgmii_rx_clk;
wire    w_xgmii_rx_rst;

wire [63:0]  w_xgmii_rxd      ;
wire [7:0]   w_xgmii_rxc      ;

// MAC RX: XGMII -> AXI Stream (frame delineation)
wire [63:0]  w_axi_rx_data    ;
wire         w_axi_rx_valid   ;
wire [7:0]   w_axi_rx_keep    ;
wire         w_axi_rx_last     ;
wire         w_axi_rx_ready    ;
wire [47:0]  w_frame_ts       ;
wire [47:0]  w_dma_ts_gray_rx ;
wire [47:0]  w_dma_ts_gray_dma;
wire [47:0]  w_dma_ts_bin     ;

// Control-plane outputs for parser settings
// wire [47:0]             o_ctl_mac_addr      = 48'h0100_5E00_0001;
// wire [31:0]             o_ctl_ip_addr       = 32'hE901_0203;
// wire [15:0]             o_ctl_port          = 16'h04D2;
// wire                    o_ctl_promiscuous   = 1'b1;
// wire [BAR0_SIZE-1:0]    o_ctl_reg           = {BAR0_SIZE{1'b0}};
// wire                    o_sync_fire         = 1'b0;

// Order book parser outputs
wire                    axi_rx_ready;
wire                    done;
wire                    msg_valid;
wire [63:0]             seq_num;
wire [47:0]             msg_timestamp;
wire [15:0]             msg_len;
wire [7:0]              msg_type;
wire [15:0]             stock_locate;
wire [63:0]             order_ref_num;
wire [63:0]             new_order_ref_num;
wire [7:0]              buy_sell;
wire [31:0]             shares;
wire [31:0]             price;
localparam EVENT_PAYLOAD_W = 192;
localparam EVENT_CDC_DEPTH = 4;

wire [SYMBOL_NUM-1:0]                  builder_event_valid     ;
wire [SYMBOL_NUM*EVENT_PAYLOAD_W-1:0]  builder_event_payload   ;
wire [SYMBOL_NUM-1:0]                  event_cdc_wr_full       ;
wire [SYMBOL_NUM-1:0]                  event_cdc_rd_en         ;
wire [SYMBOL_NUM-1:0]                  event_cdc_rd_empty      ;
wire [SYMBOL_NUM-1:0]                  event_cdc_rd_valid      ;
wire [SYMBOL_NUM*EVENT_PAYLOAD_W-1:0]  event_cdc_rd_data       ;
wire [SYMBOL_NUM*64-1:0]               rx_dma_que_iova_addr    ;
wire [SYMBOL_NUM*64-1:0]               rx_dma_que_slot_num   ;
wire [SYMBOL_NUM*64-1:0]               rx_dma_que_enable         ;
wire [SYMBOL_NUM*64-1:0]               rx_dma_que_cons_ptr     ;
wire [SYMBOL_NUM*64-1:0]               rx_dma_que_prod_ptr     ;
wire [SYMBOL_NUM*64-1:0]               rx_dma_que_drop_count   ;
wire [SYMBOL_NUM*64-1:0]               rx_dma_que_status       ;
wire [SYMBOL_NUM*16-1:0]                symbol_stock_locate_cfg ;
wire [SYMBOL_NUM*32-1:0]                symbol_price_base_cfg   ;
wire [SYMBOL_NUM*16-1:0]                symbol_stock_locate_cfg_rx;
wire [SYMBOL_NUM*32-1:0]                symbol_price_base_cfg_rx;
wire                                    pcs_frame_started       ;
wire                                    mac_frame_started       ;
wire [63:0]                             rx_dbg_pcs_frame_count_gray ;
wire [63:0]                             rx_dbg_frame_count_gray ;
wire [63:0]                             rx_dbg_msg_count_gray   ;
wire [SYMBOL_NUM*64-1:0]                rx_dbg_event_count_gray ;
wire [63:0]                             rx_dbg_pcs_frame_count  ;
wire [63:0]                             rx_dbg_frame_count      ;
wire [63:0]                             rx_dbg_msg_count        ;
wire [SYMBOL_NUM*64-1:0]                rx_dbg_event_count      ;

assign w_axi_rx_ready = axi_rx_ready;
assign dma_rq_ready        = rq_ready;
assign rq_valid            = dma_rq_valid;
assign rq_type             = dma_rq_type;
assign rq_payload_last     = dma_rq_payload_last;
assign rq_addr             = dma_rq_addr;
assign rq_payload_dw_count = dma_rq_payload_dw_count;
assign rq_tag              = dma_rq_tag;
assign rq_tc               = dma_rq_tc;
assign rq_payload          = dma_rq_payload;

wire         ask_best_valid ;
wire [31:0]  ask_best_price ;
wire [31:0]  ask_best_shares;
wire         bid_best_valid ;
wire [31:0]  bid_best_price ;
wire [31:0]  bid_best_shares;

rx_sw_reset_bridge #(
    .HOLD_CYCLES (1024),
    .COUNTER_W   (12)
) rx_sw_reset_bridge_inst (
    .i_user_clk      (user_clk_250),
    .i_user_reset_p  (user_reset_p),
    .i_reg_reset     (rx_dma_reg_reset),
    .i_rx_clk_156    (w_xgmii_rx_clk),
    .o_reset_250     (rx_logic_reset_250),
    .o_reset_156     (rx_sw_reset_156)
);

assign rx_logic_reset_156 = w_xgmii_rx_rst | rx_sw_reset_156;


MAC_layer_rx #(
    .DATA_WIDTH (64),
    .CTRL_WIDTH (8)
) MAC_layer_rx_inst (
    .i_xgmii_rx_clk   (w_xgmii_rx_clk   ),
    .i_xgmii_rx_rst   (rx_logic_reset_156),
    .i_xgmii_rxd      (w_xgmii_rxd      ),
    .i_xgmii_rxc      (w_xgmii_rxc      ),
    .i_rx_status      (w_sfp_rx_status  ),
    .o_axi_rx_data    (w_axi_rx_data    ),
    .o_axi_rx_valid   (w_axi_rx_valid   ),
    .o_axi_rx_keep    (w_axi_rx_keep    ),
    .o_axi_rx_last    (w_axi_rx_last    ),
    .o_frame_started  (mac_frame_started),
    .i_axi_rx_ready   (w_axi_rx_ready   )
);


timestamper #(
    .COUNTER_WIDTH (48)
) timestamp_inst (
    .i_clk               (w_xgmii_rx_clk    ),
    .i_rst               (rx_logic_reset_156),
    .i_event             (mac_frame_started ),
    .o_mac_timestamp     (w_frame_ts        ),
    .o_dma_timestamp_gray(w_dma_ts_gray_rx  )
);

order_book_parser order_book_parser_inst (
    .i_clk_156              (w_xgmii_rx_clk     ),
    .i_rst                  (rx_logic_reset_156 ),
    .i_axi_rx_data          (w_axi_rx_data      ),
    .i_axi_rx_valid         (w_axi_rx_valid     ),
    .i_axi_rx_keep          (w_axi_rx_keep      ),
    .i_axi_rx_last          (w_axi_rx_last      ),
    .i_frame_ts             (w_frame_ts         ),
    .o_axi_rx_ready         (axi_rx_ready       ),
    .o_msg_valid            (msg_valid          ),
    .o_seq_num              (seq_num            ),
    .o_frame_ts             (msg_timestamp      ),
    .o_msg_type             (msg_type           ),
    .o_stock_locate         (stock_locate       ),
    .o_order_ref_num        (order_ref_num      ),
    .o_new_order_ref_num    (new_order_ref_num  ),
    .o_buy_sell             (buy_sell           ), 
    .o_shares               (shares             ),
    .o_price                (price              )
);

order_book_builder #(
    .SYMBOL_NUM         (SYMBOL_NUM)
) order_book_builder_inst (
    .i_clk_156          (w_xgmii_rx_clk        ),
    .i_rst              (rx_logic_reset_156    ),
    .i_msg_valid        (msg_valid             ),
    .i_msg_type         (msg_type              ),
    .i_stock_locate     (stock_locate          ),
    .i_order_ref_num    (order_ref_num         ),
    .i_new_order_ref_num(new_order_ref_num     ),
    .i_buy_sell         (buy_sell              ),
    .i_shares           (shares                ),
    .i_price            (price                 ),
    .i_frame_ts         (msg_timestamp         ),
    .i_symbol_stock_locate_cfg (symbol_stock_locate_cfg_rx),
    .i_symbol_price_base_cfg   (symbol_price_base_cfg_rx),
    .o_event_valid      (builder_event_valid   ),
    .o_event_payload    (builder_event_payload )
);

rx_debug_counters #(
    .SYMBOL_NUM (SYMBOL_NUM)
) rx_debug_counters_inst    (
    .i_clk_156              (w_xgmii_rx_clk),
    .i_rst                  (rx_logic_reset_156),
    .i_pcs_frame_started    (pcs_frame_started),
    .i_frame_started        (mac_frame_started),
    .i_msg_valid            (msg_valid),
    .i_event_valid          (builder_event_valid),
    .o_pcs_frame_count_gray (rx_dbg_pcs_frame_count_gray),
    .o_frame_count_gray     (rx_dbg_frame_count_gray),
    .o_msg_count_gray       (rx_dbg_msg_count_gray),
    .o_event_count_gray     (rx_dbg_event_count_gray)
);

rx_debug_counter_bridge #(
    .SYMBOL_NUM (SYMBOL_NUM)
) rx_debug_counter_bridge_inst (
    .i_user_clk             (user_clk_250),
    .i_pcs_frame_count_gray (rx_dbg_pcs_frame_count_gray),
    .i_frame_count_gray     (rx_dbg_frame_count_gray),
    .i_msg_count_gray       (rx_dbg_msg_count_gray),
    .i_event_count_gray     (rx_dbg_event_count_gray),
    .o_pcs_frame_count      (rx_dbg_pcs_frame_count),
    .o_frame_count          (rx_dbg_frame_count),
    .o_msg_count            (rx_dbg_msg_count),
    .o_event_count          (rx_dbg_event_count)
);

bit_synchronizer #(
    .BIT_WIDTH (48)
) dma_timestamp_sync_inst (
    .i_clk  (user_clk_250       ),
    .i_in   (w_dma_ts_gray_rx   ),
    .o_out  (w_dma_ts_gray_dma  )
);

gray_to_binary #(
    .WIDTH (48)
) dma_timestamp_decode_inst (
    .i_gray   (w_dma_ts_gray_dma),
    .o_binary (w_dma_ts_bin     )
);

bit_synchronizer #(
    .BIT_WIDTH (SYMBOL_NUM*16)
) symbol_stock_locate_cfg_sync_inst (
    .i_clk  (w_xgmii_rx_clk),
    .i_in   (symbol_stock_locate_cfg),
    .o_out  (symbol_stock_locate_cfg_rx)
);

bit_synchronizer #(
    .BIT_WIDTH (SYMBOL_NUM*32)
) symbol_price_base_cfg_sync_inst (
    .i_clk  (w_xgmii_rx_clk),
    .i_in   (symbol_price_base_cfg),
    .o_out  (symbol_price_base_cfg_rx)
);

generate
    genvar cdc_idx;
    for (cdc_idx = 0; cdc_idx < SYMBOL_NUM; cdc_idx = cdc_idx + 1) begin : g_event_cdc
        async_fifo #(
            .DEPTH  (EVENT_CDC_DEPTH),
            .DATA_W (EVENT_PAYLOAD_W)
        ) event_cdc_inst (
            .i_wr_clk    (w_xgmii_rx_clk                                                    ),
            .i_wr_rst    (rx_logic_reset_156                                                ),
            .i_wr_en     (builder_event_valid[cdc_idx]                                      ),
            .i_wr_data   (builder_event_payload[cdc_idx*EVENT_PAYLOAD_W +: EVENT_PAYLOAD_W] ),
            .o_wr_full   (event_cdc_wr_full[cdc_idx]                                        ),
            .i_rd_clk    (user_clk_250                                                      ),
            .i_rd_rst    (rx_logic_reset_250                                                ),
            .i_rd_en     (event_cdc_rd_en[cdc_idx]                                          ),
            .o_rd_empty  (event_cdc_rd_empty[cdc_idx]                                       ),
            .o_rd_valid  (event_cdc_rd_valid[cdc_idx]                                       ),
            .o_rd_data   (event_cdc_rd_data[cdc_idx*EVENT_PAYLOAD_W +: EVENT_PAYLOAD_W]     )
        );
    end
endgenerate






// control_plane #()
// control_plane_inst
// (
// // pcie_wrapper interface
// .i_user_clk_250         (user_clk_250       ),
// .i_user_reset_p         (user_reset_p       ), //active hig
// .i_cq_valid             (cq_valid           ),
// .i_cq_type              (cq_type            ),
// .i_cq_reg_addr          (cq_reg_addr        ),
// .i_cq_payload           (cq_payload         ),
// .i_cq_bar_id            (cq_bar_id          ),
// .i_cq_requester_id      (cq_requester_id    ),
// .i_cq_tag               (cq_tag             ),
// .i_cq_tc                (cq_tc              ),
// .i_cq_lower_addr        (cq_lower_addr      ),
// .i_cq_payload_dw_count  (cq_payload_dw_count),
// .i_cq_last              (cq_last            ),
// .i_cc_ready             (cc_ready           ),
// .o_cc_valid             (cc_valid           ),
// .o_cc_requester_id      (cc_requester_id    ),
// .o_cc_tag               (cc_tag             ),
// .o_cc_tc                (cc_tc              ),
// .o_cc_lower_addr        (cc_lower_addr      ),
// .o_cc_dword_count       (cc_dword_count     ),
// .o_cc_status            (cc_status          ),
// .o_cc_payload           (cc_payload         ),
// .o_cc_last              (cc_last            ),
// .i_clk_156              (w_xgmii_rx_clk     ),
// .o_sync_fire            (o_sync_fire        ),
// .o_ctl_mac_addr         (o_ctl_mac_addr     ),
// .o_ctl_ip_addr          (o_ctl_ip_addr      ),
// .o_ctl_port             (o_ctl_port         ),
// .o_ctl_promiscuous      (o_ctl_promiscuous  ),
// .o_ctl_reg              (o_ctl_reg          )
// );





wire w_sfp_rx_status;



pcs_pma_wrapper #(
    .DATA_WIDTH(64)
)
pcs_pma_wrapper_inst (
    .i_drp_clk                      (sys_clk_100        ),
    .i_rst_n                        (i_pcie_rst_n       ),
    .i_sw_reset_p                   (rx_logic_reset_250 ),
    .i_gt_refclk_p                  (i_gt_refclk_p      ),
    .i_gt_refclk_n                  (i_gt_refclk_n      ),
    .i_gt_rx_p_0                    (i_gt_rx_p_0        ),
    .i_gt_rx_n_0                    (i_gt_rx_n_0        ),
    .o_gt_tx_p_0                    (o_gt_tx_p_0        ),
    .o_gt_tx_n_0                    (o_gt_tx_n_0        ),
    .i_xgmii_txd                    ({8{8'h07}}         ),  // TX idle
    .i_xgmii_txc                    (8'hFF              ),  // TX idle
    .o_xgmii_rxd                    (w_xgmii_rxd        ),
    .o_xgmii_rxc                    (w_xgmii_rxc        ),
    .o_xgmii_tx_clk                 (w_xgmii_clk        ),
    .o_xgmii_tx_rst                 (w_xgmii_tx_rst     ),
    .o_xgmii_rx_clk                 (w_xgmii_rx_clk     ),
    .o_xgmii_rx_rst                 (w_xgmii_rx_rst     ),
    .o_rx_frame_start               (pcs_frame_started  ),
    .o_rx_status                    (w_sfp_rx_status    )
);



endmodule
