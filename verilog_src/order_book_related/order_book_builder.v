module order_book_builder #(
    parameter STOCK_BITS       = 6,     // number of tracked stocks = 2^STOCK_BITS
    parameter PRICE_BITS       = 10,    // number of price levels per stock = 2^PRICE_BITS
    parameter SYMBOL_NUM       = 1,
    parameter PARSER_MSG_BIT   = 1+64+8+16+64+64+2+32+32 // {msg_valid, seq_num, msg_type, stock_locate, order_ref_num, new_order_ref_num, side, shares, price}
) (
    // order book parser interface
    input   wire                        i_clk_156,
    input   wire                        i_rst,               // active high
    input   wire                        i_msg_valid,
    input   wire [63:0]                 i_seq_num,
    input   wire [63:0]                 i_rx_ingress_tick,
    input   wire [7:0]                  i_msg_type,          // A, D, X, U, E, F
    input   wire [15:0]                 i_stock_locate,
    input   wire [63:0]                 i_order_ref_num,     // old order_ref for U
    input   wire [63:0]                 i_new_order_ref_num, // used for U
    input   wire [7:0]                  i_buy_sell,          // 'B' or 'S' for A/F
    input   wire [31:0]                 i_shares,
    input   wire [31:0]                 i_price,
    input   wire [47:0]                 i_timestamp     ,
    output wire                         o_ask_best_valid  ,
    output wire [31:0]                  o_ask_best_price  ,
    output wire [31:0]                  o_ask_best_shares ,
    output wire                         o_bid_best_valid  ,
    output wire [31:0]                  o_bid_best_price  ,
    output wire [31:0]                  o_bid_best_shares 
);

localparam NULL = 2'd0;
localparam BUY  = 2'd1;
localparam SELL = 2'd2;

reg [1:0] side;

always @(*) begin
    case (i_buy_sell)
        8'h42:      side = BUY;  // 'B'
        8'h53:      side = SELL; // 'S'
        default:    side = NULL;
    endcase
end

wire [63:0] o_ask_seq_num;
wire [63:0] o_bid_seq_num;


wire [PARSER_MSG_BIT-1:0]       parser_msg   = {i_msg_valid, i_seq_num, i_msg_type, i_stock_locate, i_order_ref_num, i_new_order_ref_num, side, i_shares, i_price};

symbol_book #(
    .QTY_SHARE_BIT      (32), // number of bits to represent shares quantity at each price level
    .QTY_PRICE_LVL_BIT  (8),
    .BOOK_LEVEL_BIT     (12 ),
    .PRICE_BASE         (32'd0000_0000  ), // default lowest price (in 1/10000 dollars) for all stocks, can be updated by control plane.
    .STOCK_LOCATE       (16'h000d       )  // stock locate for this symbol book, assigned by control plane
) symbol_book_AAPL (
    .i_clk_156          (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_parser_msg       (parser_msg          ),
    .o_event            (                   ),
    .o_payload          (                   )
);

symbol_book #(
    .QTY_SHARE_BIT      (32), // number of bits to represent shares quantity at each price level
    .QTY_PRICE_LVL_BIT  (8),
    .BOOK_LEVEL_BIT     (12 ),
    .PRICE_BASE         (32'd0000_0000  ), // default lowest price (in 1/10000 dollars) for all stocks, can be updated by control plane.
    .STOCK_LOCATE       (16'h0ee8       )  // stock locate for this symbol book, assigned by control plane
) symbol_book_HSBC (
    .i_clk_156          (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_parser_msg       (parser_msg         ),
    .o_event            (                   ),
    .o_payload          (                   )
);


endmodule
