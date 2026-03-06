// order book per symbol.
module symbol_book #(
    parameter SHARE_PER_PRICE_BIT   = 32,      // number of bits to represent shares quantity at each price level
    parameter PRICE_ADDR_WIDTH      = 10,      // trace 2^PRICE_ADDR_WIDTH price levels
    parameter BOOK_ADDR_WIDTH       = 12,      // trace 2^BOOK_ADDR_WIDTH orders in the order table
    parameter PRICE_BASE            = 32'd2317700, // 231.77 in dollars. In binary, the last two bits are always 00, can drop
    parameter STOCK_LOCATE          = 16'h000d  // stock locate for this symbol book, can be updated by control plane.
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

localparam                  ORDER_MSG_WIDTH             = 8+64+64+2+32+32; // {msg_type, order_ref_num, new_order_ref_num, side, price, shares}
localparam                  PARSER_MSG_WIDTH            = 1+64+8+16+64+64+2+32+32+48;
localparam                  QTY_MSG_WIDTH               = 2+32+1+32; // {bid_ask, price, is_add, d_shares}
localparam                  QTY_BOOK_WIDTH              = SHARE_PER_PRICE_BIT;
localparam                  PRICE_DEPTH                 = 1 << PRICE_ADDR_WIDTH;

localparam                  IDLE                        = 2'b00;
localparam                  FIRST_CYCLE                 = 2'b01;
localparam                  SECOND_CYCLE                = 2'b10;
localparam                  READ                        = 2'b01;
localparam                  WRITE                       = 2'b10;
localparam                  BID                         = 2'b01;
localparam                  ASK                         = 2'b10;

wire [PARSER_MSG_WIDTH-1:0]       parser_msg               = {i_msg_valid, i_rx_ingress_tick, i_msg_type, i_stock_locate, i_order_ref_num, i_new_order_ref_num, i_side, i_shares, i_price, i_timestamp};

wire [QTY_MSG_WIDTH-1:0]          qty_msg;


book_builder #(
    .STOCK_LOCATE     (STOCK_LOCATE             ),
    .BOOK_ADDR_WIDTH  (BOOK_ADDR_WIDTH          ),
    .PARSER_MSG_WIDTH (PARSER_MSG_WIDTH         ),
    .ORDER_MSG_WIDTH  (ORDER_MSG_WIDTH          ),
    .QTY_MSG_WIDTH    (QTY_MSG_WIDTH            )
) book_builder_inst (
    .i_clk_156          (i_clk_156              ),
    .i_rst              (i_rst                  ),
    .i_parser_msg       (parser_msg             ),
    .o_qty_msg          (qty_msg                )
);


qty_builder qty_builder_inst (
    .i_clk_156          (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_qty_msg          (qty_msg            )
);



endmodule
