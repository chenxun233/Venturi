module qty_book_wrapper #(
    parameter QTY_MSG_BIT       = 2+32+1+32,
    parameter QTY_PRICE_LVL_BIT = 10,
    parameter QTY_SHARE_BIT     = 32,
    parameter PRICE_BASE        = 32'd0,
    parameter BID_OR_ASK        = 2'b01 // 01 for ask, 10 for bid
) (
    input  wire                          i_clk_156,
    input  wire                          i_rst,
    input  wire [QTY_MSG_BIT-1:0]        i_qty_msg,
    output wire                          o_best_valid_aligned,
    output wire [31:0]                   o_best_price_aligned,
    output wire [QTY_SHARE_BIT-1:0]      o_best_shares,
    output wire [63:0]                   o_seq_num
);

wire [QTY_PRICE_LVL_BIT-1:0]  best_idx;
wire                          best_valid;
wire [31:0]                   best_price = PRICE_BASE + ({24'd0, best_idx} << 2);

localparam IDLE  = 2'b00;
localparam READ  = 2'b01;
localparam WRITE = 2'b10;

assign o_seq_num = (o_best_valid_aligned) ? seq_num : 64'hFFFF_FFFF_FFFF_FFFF; // output max seq_num when no valid best price, so that downstream can ignore it.

wire [QTY_PRICE_LVL_BIT-1:0] tree_price_idx;
wire [1:0]                   tree_price_change;
wire [QTY_PRICE_LVL_BIT-1:0] bram_addr_a;
wire [1:0]                   bram_op_a;
wire [QTY_SHARE_BIT-1:0]     bram_i_data_a;
wire [QTY_SHARE_BIT-1:0]     bram_o_data_a;
wire [QTY_PRICE_LVL_BIT-1:0] bram_addr_b = best_idx;
wire [1:0]                   bram_op_b = best_valid ? READ : IDLE;
wire [QTY_SHARE_BIT-1:0]     bram_i_data_b = {QTY_SHARE_BIT{1'b0}};
wire [QTY_SHARE_BIT-1:0]     bram_o_data_b;
wire [63:0]                  seq_num;

qty_builder #(
    .QTY_MSG_BIT       (QTY_MSG_BIT),
    .QTY_PRICE_LVL_BIT (QTY_PRICE_LVL_BIT),
    .QTY_SHARE_BIT     (QTY_SHARE_BIT),
    .PRICE_BASE        (PRICE_BASE),
    .BID_OR_ASK        (BID_OR_ASK)
) bid_qty_builder_inst (
    .i_clk_156          (i_clk_156),
    .i_rst              (i_rst),
    .i_qty_msg          (i_qty_msg),
    .i_bram_o_data      (bram_o_data_a),
    .o_tree_price_idx   (tree_price_idx),
    .o_tree_price_change(tree_price_change),
    .o_seq_num          (seq_num          ),
    .o_bram_addr        (bram_addr_a      ),
    .o_bram_op          (bram_op_a      ),
    .o_bram_i_data      (bram_i_data_a  )
);

tree_builder #(
    .QTY_PRICE_LVL_BIT (QTY_PRICE_LVL_BIT),
    .BID_OR_ASK        (BID_OR_ASK)
) bid_tree_builder_inst (
    .i_clk              (i_clk_156),
    .i_rst              (i_rst),
    .i_tree_price_idx   (tree_price_idx),
    .i_tree_price_change(tree_price_change),
    .o_tree_best_valid  (best_valid),
    .o_tree_best_idx    (best_idx)
);

aligner #(
    .QTY_SHARE_BIT (QTY_SHARE_BIT)
) aligner_inst (
    .i_clk_156             (i_clk_156),
    .i_rst                 (i_rst),
    .i_best_valid          (best_valid),
    .i_best_price          (best_price),
    .i_best_shares         (bram_o_data_b),
    .o_best_valid_aligned  (o_best_valid_aligned),
    .o_best_price_aligned  (o_best_price_aligned),
    .o_best_shares         (o_best_shares)
);

// wire [QTY_SHARE_BIT-1:0] temp_bram_o_data_b;

// assign bram_o_data_b = best_valid ? temp_bram_o_data_b : {QTY_SHARE_BIT{1'b0}};

bram_dp #(
    .ADDR_WIDTH (QTY_PRICE_LVL_BIT),
    .DATA_WIDTH (QTY_SHARE_BIT)
) bid_bram_inst (
    .i_clk    (i_clk_156),
    .i_rst    (i_rst),
    .i_addr_a (bram_addr_a),
    .i_op_a   (bram_op_a),
    .i_data_a (bram_i_data_a),
    .o_data_a (bram_o_data_a),
    .i_addr_b (bram_addr_b),
    .i_op_b   (bram_op_b),
    .i_data_b (bram_i_data_b),
    .o_data_b (bram_o_data_b)
);


endmodule
