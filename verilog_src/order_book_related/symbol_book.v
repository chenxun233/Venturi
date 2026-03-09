// order book per symbol.
module symbol_book #(
    parameter QTY_SHARE_BIT         = 32,           // number of bits to represent shares quantity at each price level
    parameter QTY_PRICE_LVL_BIT     = 12,           // trace 2^QTY_PRICE_LVL_BIT price levels
    parameter BOOK_LEVEL_BIT         = 12,           // trace 2^BOOK_LEVEL_BIT orders in the order table
    parameter PRICE_BASE            = 32'd0,        // 
    parameter STOCK_LOCATE          = 16'h000d      // stock locate for this symbol book, can be updated by control plane.
) (
    // order book parser interface
    input   wire                            i_clk_156,
    input   wire                            i_rst,               // active high
    input   wire                            i_msg_valid,
    input   wire [63:0]                     i_rx_ingress_tick,
    input   wire [7:0]                      i_msg_type,          // A, D, X, U, E, F
    input   wire [15:0]                     i_stock_locate,
    input   wire [63:0]                     i_order_ref_num,     // old order_ref for U
    input   wire [63:0]                     i_new_order_ref_num, // used for U
    input   wire [1:0]                      i_side,              // 'B' or 'S' for A/F
    input   wire [31:0]                     i_shares,
    input   wire [31:0]                     i_price,             // in 1/10000 dollars
    input   wire [47:0]                     i_timestamp,
    output  wire                            o_ask_best_valid_aligned,
    output  wire [QTY_PRICE_LVL_BIT-1:0]    o_ask_best_idx_aligned,
    output  wire [QTY_SHARE_BIT-1:0]        o_ask_best_shares,
    output  wire                            o_bid_best_valid_aligned,
    output  wire [QTY_PRICE_LVL_BIT-1:0]    o_bid_best_idx_aligned,
    output  wire [QTY_SHARE_BIT-1:0]        o_bid_best_shares
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


qty_level_wrapper #(
    .QTY_MSG_BIT            (QTY_MSG_BIT          ),
    .QTY_PRICE_LVL_BIT      (QTY_PRICE_LVL_BIT    ),
    .QTY_SHARE_BIT          (QTY_SHARE_BIT        ),
    .PRICE_BASE             (PRICE_BASE           ),
    .BID_OR_ASK             (2'b10) //  01 for bid, 10 for ask
)
ask_wrapper_inst (
    .i_clk_156              (i_clk_156          ),
    .i_rst                  (i_rst              ),
    .i_qty_msg              (qty_msg            ),
    .o_best_valid_aligned   (o_ask_best_valid_aligned),
    .o_best_idx_aligned     (o_ask_best_idx_aligned),
    .o_best_shares          (o_ask_best_shares    )
);


qty_level_wrapper #(
    .QTY_MSG_BIT            (QTY_MSG_BIT          ),
    .QTY_PRICE_LVL_BIT      (QTY_PRICE_LVL_BIT    ),
    .QTY_SHARE_BIT          (QTY_SHARE_BIT        ),
    .PRICE_BASE             (PRICE_BASE           ),
    .BID_OR_ASK             (2'b01) //  01 for bid, 10 for ask
)
bid_wrapper_inst (
    .i_clk_156              (i_clk_156          ),
    .i_rst                  (i_rst              ),
    .i_qty_msg              (qty_msg            ),
    .o_best_valid_aligned   (o_bid_best_valid_aligned),
    .o_best_idx_aligned     (o_bid_best_idx_aligned),
    .o_best_shares          (o_bid_best_shares    )
);



endmodule
