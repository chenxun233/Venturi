module ask_wrapper #(
    parameter QTY_MSG_BIT       = 2+32+1+32,
    parameter QTY_PRICE_LVL_BIT = 10,
    parameter QTY_SHARE_BIT     = 32,
    parameter PRICE_BASE        = 32'd0
) (
    input  wire                          i_clk_156,
    input  wire                          i_rst,
    input  wire [QTY_MSG_BIT-1:0]        i_qty_msg,
    output wire                          o_best_valid,
    output wire [QTY_PRICE_LVL_BIT-1:0]  o_best_idx,
    output reg                           o_best_valid_aligned,
    output reg  [QTY_PRICE_LVL_BIT-1:0]  o_best_idx_aligned,
    output reg  [QTY_SHARE_BIT-1:0]      o_best_shares
);

localparam IDLE  = 2'b00;
localparam READ  = 2'b01;
localparam WRITE = 2'b10;

wire [QTY_PRICE_LVL_BIT-1:0] tree_price_idx;
wire [1:0]                   tree_price_change;
wire [QTY_PRICE_LVL_BIT-1:0] bram_addr_a;
wire [1:0]                   bram_op_a;
wire [QTY_SHARE_BIT-1:0]     bram_i_data_a;
wire [QTY_SHARE_BIT-1:0]     bram_o_data_a;
wire [QTY_PRICE_LVL_BIT-1:0] bram_addr_b = o_best_idx;
wire [1:0]                   bram_op_b = o_best_valid ? READ : IDLE;
wire [QTY_SHARE_BIT-1:0]     bram_i_data_b = {QTY_SHARE_BIT{1'b0}};
wire [QTY_SHARE_BIT-1:0]     bram_o_data_b;
reg                          best_valid_d1;
reg [QTY_PRICE_LVL_BIT-1:0]  best_idx_d1;

ask_qty_builder #(
    .QTY_MSG_BIT       (QTY_MSG_BIT),
    .QTY_PRICE_LVL_BIT (QTY_PRICE_LVL_BIT),
    .QTY_SHARE_BIT     (QTY_SHARE_BIT),
    .PRICE_BASE        (PRICE_BASE)
) ask_qty_builder_inst (
    .i_clk_156          (i_clk_156),
    .i_rst              (i_rst),
    .i_qty_msg          (i_qty_msg),
    .i_bram_o_data      (bram_o_data_a),
    .o_tree_price_idx   (tree_price_idx),
    .o_tree_price_change(tree_price_change),
    .o_bram_addr        (bram_addr_a),
    .o_bram_op          (bram_op_a),
    .o_bram_i_data      (bram_i_data_a)
);

ask_tree_builder #(
    .QTY_PRICE_LVL_BIT (QTY_PRICE_LVL_BIT)
) ask_tree_builder_inst (
    .i_clk              (i_clk_156),
    .i_rst              (i_rst),
    .i_tree_price_idx   (tree_price_idx),
    .i_tree_price_change(tree_price_change),
    .o_tree_best_valid  (o_best_valid),
    .o_tree_best_idx    (o_best_idx)
);

always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        best_valid_d1        <= 1'b0;
        best_idx_d1          <= {QTY_PRICE_LVL_BIT{1'b0}};
        o_best_valid_aligned <= 1'b0;
        o_best_idx_aligned   <= {QTY_PRICE_LVL_BIT{1'b0}};
        o_best_shares        <= {QTY_SHARE_BIT{1'b0}};
    end else begin
        o_best_valid_aligned <= best_valid_d1;
        o_best_idx_aligned   <= best_idx_d1;
        o_best_shares        <= bram_o_data_b;
        best_valid_d1        <= o_best_valid;
        best_idx_d1          <= o_best_idx;
    end
end

bram_dp #(
    .ADDR_WIDTH (QTY_PRICE_LVL_BIT),
    .DATA_WIDTH (QTY_SHARE_BIT)
) ask_bram_inst (
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
