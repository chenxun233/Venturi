// order book per symbol.
module symbol_book #(
    parameter QTY_SHARE_BIT         = 32,           // number of bits to represent shares quantity at each price level
    parameter QTY_PRICE_LVL_BIT     = 12,           // trace 2^QTY_PRICE_LVL_BIT price levels
    parameter BOOK_LEVEL_BIT         = 12,           // trace 2^BOOK_LEVEL_BIT orders in the order table
    parameter PRICE_BASE            = 32'd0,        // 
    parameter STOCK_LOCATE          = 16'h000d      // stock locate for this symbol book, can be updated by control plane.
) (
    // order book parser interface
    input   wire                        i_clk_156,
    input   wire                        i_rst,               // active high
    input   wire                        i_msg_valid,
    input   wire [63:0]                 i_rx_ingress_tick,
    input   wire [7:0]                  i_msg_type,          // A, D, X, U, E, F
    input   wire [15:0]                 i_stock_locate,
    input   wire [63:0]                 i_order_ref_num,     // old order_ref for U
    input   wire [63:0]                 i_new_order_ref_num, // used for U
    input   wire [1:0]                  i_side,              // 'B' or 'S' for A/F
    input   wire [31:0]                 i_shares,
    input   wire [31:0]                 i_price,             // in 1/10000 dollars
    input   wire [47:0]                 i_timestamp
);

localparam                  PARSER_MSG_BIT              = 1+64+8+16+64+64+2+32+32+48;
localparam                  QTY_MSG_BIT                 = 2+32+1+32; // {bid_ask, price, is_add, d_shares} 

localparam                  IDLE                        = 2'b00;
localparam                  FIRST_CYCLE                 = 2'b01;
localparam                  SECOND_CYCLE                = 2'b10;
localparam                  READ                        = 2'b01;
localparam                  WRITE                       = 2'b10;
localparam                  BID                         = 2'b01;
localparam                  ASK                         = 2'b10;

wire [PARSER_MSG_BIT-1:0]       parser_msg               = {i_msg_valid, i_rx_ingress_tick, i_msg_type, i_stock_locate, i_order_ref_num, i_new_order_ref_num, i_side, i_shares, i_price, i_timestamp};

wire [QTY_MSG_BIT-1:0]          qty_msg;
wire [QTY_PRICE_LVL_BIT-1:0]    ask_price_idx;
wire [1:0]                      ask_price_change;
wire                            ask_best_valid;
wire [QTY_PRICE_LVL_BIT-1:0]    ask_best_idx;
wire [QTY_PRICE_LVL_BIT-1:0]    bid_price_idx;
wire [1:0]                      bid_price_change;
wire                            bid_best_valid;
wire [QTY_PRICE_LVL_BIT-1:0]    bid_best_idx;

wire stock_valid = (i_stock_locate == STOCK_LOCATE);

book_builder #(
    .BOOK_LEVEL_BIT     (BOOK_LEVEL_BIT         ),
    .PARSER_MSG_BIT     (PARSER_MSG_BIT         ),
    .QTY_MSG_BIT        (QTY_MSG_BIT            )
) book_builder_inst (
    .i_clk_156          (i_clk_156              ),
    .i_rst              (i_rst                  ),
    .i_parser_msg       (parser_msg             ),
    .i_stock_valid      (stock_valid            ),
    .o_qty_msg          (qty_msg                )
);


ask_qty_builder #(
    .QTY_MSG_BIT        (QTY_MSG_BIT          ),
    .QTY_PRICE_LVL_BIT  (QTY_PRICE_LVL_BIT    ),
    .QTY_SHARE_BIT      (QTY_SHARE_BIT        ),
    .PRICE_BASE         (PRICE_BASE           )
)
ask_qty_builder_inst (
    .i_clk_156          (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_qty_msg          (qty_msg            ),
    .o_ask_price_idx     (ask_price_idx      ),
    .o_ask_price_change  (ask_price_change  )
);


bid_qty_builder #(
    .QTY_MSG_BIT        (QTY_MSG_BIT          ),
    .QTY_PRICE_LVL_BIT  (QTY_PRICE_LVL_BIT    ),
    .QTY_SHARE_BIT      (QTY_SHARE_BIT        ),
    .PRICE_BASE         (PRICE_BASE           )
)
bid_qty_builder_inst (
    .i_clk_156          (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_qty_msg          (qty_msg            ),
    .o_bid_price_idx     (bid_price_idx         ),
    .o_bid_price_change  (bid_price_change     )
);

ask_tree_builder #(
    .QTY_PRICE_LVL_BIT  (QTY_PRICE_LVL_BIT    )
) ask_tree_builder_inst (
    .i_clk              (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_ask_price_idx    (ask_price_idx      ),
    .i_ask_price_change (ask_price_change  ),
    .o_ask_best_valid   (ask_best_valid     ),
    .o_ask_best_idx     (ask_best_idx       )
);

bid_tree_builder #(
    .QTY_PRICE_LVL_BIT  (QTY_PRICE_LVL_BIT    )
) bid_tree_builder_inst (
    .i_clk              (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_bid_price_idx    (bid_price_idx      ),
    .i_bid_price_change (bid_price_change  ),
    .o_bid_best_valid   (bid_best_valid     ),
    .o_bid_best_idx     (bid_best_idx       )
);




endmodule
