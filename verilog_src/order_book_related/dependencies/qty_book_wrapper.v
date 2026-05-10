module qty_book_wrapper #(
    parameter QTY_MSG_BIT       = 2+32+1+32+1+48, // {bid_ask, price, is_add, d_shares, op_done, timestamp}
    parameter QTY_PRICE_LVL_BIT = 10,
    parameter BID_OR_ASK        = 2'b01 // 01 for ask, 10 for bid
) (
    input  wire                          i_clk_156,
    input  wire                          i_rst,
    input  wire [QTY_MSG_BIT-1:0]        i_qty_msg,
    input  wire [31:0]                   i_price_base,
    output wire [31:0]                   o_best_price_aligned,
    output wire [31:0]                   o_best_shares,
    output wire                          o_op_done_aligned,
    output wire [47:0]                   o_frame_ts_aligned

);

wire [QTY_PRICE_LVL_BIT-1:0]  best_idx;
wire [31:0]                   best_price = i_price_base + ({24'd0, best_idx} << 2);

localparam IDLE  = 2'b00;
localparam READ  = 2'b01;
localparam WRITE = 2'b10;


wire [QTY_PRICE_LVL_BIT-1:0] price_idx;
wire [1:0]                   price_change;
wire [QTY_PRICE_LVL_BIT-1:0] bram_addr_a;
wire [1:0]                   bram_op_a;
wire [31:0]     bram_i_data_a;
wire [31:0]     bram_o_data_a;
wire [QTY_PRICE_LVL_BIT-1:0] bram_addr_b = best_idx;
wire [1:0]                   bram_op_b = best_idx != 0 ? READ : IDLE;
wire [31:0]     bram_i_data_b = {32{1'b0}};
wire [31:0]     bram_o_data_b;
wire                         op_done;
wire                         t_op_done;
wire [47:0]                 qty_frame_ts;
wire [47:0]                 tree_frame_ts;
wire                        qty_mem_init_done;

qty_builder #(
    .QTY_MSG_BIT       (QTY_MSG_BIT),
    .QTY_PRICE_LVL_BIT (QTY_PRICE_LVL_BIT),
    .BID_OR_ASK        (BID_OR_ASK)
) qty_builder_inst (
    .i_clk_156          (i_clk_156      ),
    .i_rst              (i_rst          ),
    .i_mem_init_done    (qty_mem_init_done),
    .i_qty_msg          (i_qty_msg      ),
    .i_price_base       (i_price_base   ),
    .i_bram_o_data      (bram_o_data_a  ),
    .o_price_idx        (price_idx      ),
    .o_price_change     (price_change   ),
    .o_op_done          (op_done        ),
    .o_frame_ts         (qty_frame_ts ),  
    .o_bram_addr        (bram_addr_a    ),
    .o_bram_op          (bram_op_a      ),
    .o_bram_i_data      (bram_i_data_a  )
);

tree_builder #(
    .QTY_PRICE_LVL_BIT (QTY_PRICE_LVL_BIT),
    .BID_OR_ASK        (BID_OR_ASK)
) tree_builder_inst (
    .i_clk              (i_clk_156   ),
    .i_rst              (i_rst       ),
    .i_price_idx        (price_idx   ),
    .i_price_change     (price_change),
    .i_op_done          (op_done     ),
    .i_frame_ts         (qty_frame_ts ),
    .o_frame_ts         (tree_frame_ts ),
    .o_best_price_idx   (best_idx    ),
    .o_op_done          (t_op_done   )
);

aligner #(
) aligner_inst (
    .i_clk_156             (i_clk_156),
    .i_rst                 (i_rst),
    .i_best_price          (best_price),
    .i_t_op_done           (t_op_done),
    .i_best_shares         (bram_o_data_b),
    .i_frame_ts            (tree_frame_ts),
    .o_best_price_aligned  (o_best_price_aligned),
    .o_best_shares         (o_best_shares),
    .o_op_done_aligned     (o_op_done_aligned),
    .o_frame_ts_aligned    (o_frame_ts_aligned)                   
);

// wire [31:0] temp_bram_o_data_b;

// assign bram_o_data_b = best_valid ? temp_bram_o_data_b : {32{1'b0}};

bram_dp #(
    .ADDR_WIDTH (QTY_PRICE_LVL_BIT),
    .DATA_WIDTH (32)
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
    .o_data_b (bram_o_data_b),
    .o_init_done(qty_mem_init_done)
);



endmodule
