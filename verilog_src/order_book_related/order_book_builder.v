module order_book_builder #(
    parameter STOCK_BITS       = 6,     // number of tracked stocks = 2^STOCK_BITS
    parameter PRICE_BITS       = 10,    // number of price levels per stock = 2^PRICE_BITS
    parameter SYMBOL_NUM       = 2,
    parameter PARSER_MSG_BIT   = 1+8+16+64+64+2+32+32+48, // {msg_valid, msg_type, stock_locate, order_ref_num, new_order_ref_num, side, shares, price, frame_ts}
    parameter EVENT_FIFO_DEPTH = 2,
    parameter PAYLOAD_W        = 2*(32+32)+48+16
) (
    // order book parser interface
    input   wire                            i_clk_156,
    input   wire                            i_rst,               // active high
    input   wire                            i_msg_valid,
    input   wire [7:0]                      i_msg_type,          // A, D, X, U, E, F
    input   wire [15:0]                     i_stock_locate,
    input   wire [63:0]                     i_order_ref_num,     // old order_ref for U
    input   wire [63:0]                     i_new_order_ref_num, // used for U
    input   wire [7:0]                      i_buy_sell,          // 'B' or 'S' for A/F
    input   wire [31:0]                     i_shares,
    input   wire [31:0]                     i_price,
    input   wire [47:0]                     i_frame_ts,          
    output  wire [SYMBOL_NUM-1:0]           o_event_valid,
    output  wire [SYMBOL_NUM*PAYLOAD_W-1:0] o_event_payload
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

wire                         aapl_event_valid;
wire [PAYLOAD_W-1:0]         aapl_payload;
wire                         hsbc_event_valid;
wire [PAYLOAD_W-1:0]         hsbc_payload;


wire [PARSER_MSG_BIT-1:0]       parser_msg   = {i_msg_valid, i_msg_type, i_stock_locate, i_order_ref_num, i_new_order_ref_num, side, i_shares, i_price,i_frame_ts};

symbol_book #(
    .QTY_PRICE_LVL_BIT  (8),
    .BOOK_LEVEL_BIT     (12 ),
    .PRICE_BASE         (32'd0000_0000  ), // default lowest price (in 1/10000 dollars) for all stocks, can be updated by control plane.
    .STOCK_LOCATE       (16'h000d       ), // stock locate for this symbol book, assigned by control plane
    .EVENT_FIFO_DEPTH   (EVENT_FIFO_DEPTH),
    .PARSER_MSG_BIT     (PARSER_MSG_BIT),
    .PAYLOAD_W          (PAYLOAD_W          )
) symbol_book_AAPL (
    .i_clk_156          (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_parser_msg       (parser_msg         ),
    .o_event_found      (aapl_event_valid   ),
    .o_payload          (aapl_payload       )
);

symbol_book #(
    .QTY_PRICE_LVL_BIT  (8),
    .BOOK_LEVEL_BIT     (12 ),
    .PRICE_BASE         (32'd0000_0000  ), // default lowest price (in 1/10000 dollars) for all stocks, can be updated by control plane.
    .STOCK_LOCATE       (16'h0ee8       ), // stock locate for this symbol book, assigned by control plane
    .EVENT_FIFO_DEPTH   (EVENT_FIFO_DEPTH),
    .PARSER_MSG_BIT     (PARSER_MSG_BIT),
    .PAYLOAD_W          (PAYLOAD_W          )
) symbol_book_HSBC (
    .i_clk_156          (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_parser_msg       (parser_msg         ),
    .o_event_found      (hsbc_event_valid     ),
    .o_payload          (hsbc_payload      )
);

assign o_event_valid   = {hsbc_event_valid, aapl_event_valid};
assign o_event_payload = {hsbc_payload, aapl_payload};

endmodule
