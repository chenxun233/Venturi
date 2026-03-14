module order_book_builder #(
    parameter STOCK_BITS       = 6,     // number of tracked stocks = 2^STOCK_BITS
    parameter PRICE_BITS       = 10,    // number of price levels per stock = 2^PRICE_BITS
    parameter SYMBOL_NUM       = 2,
    parameter PARSER_MSG_BIT   = 1+64+8+16+64+64+2+32+32, // {msg_valid, seq_num, msg_type, stock_locate, order_ref_num, new_order_ref_num, side, shares, price}
    parameter EVENT_FIFO_DEPTH = 2,
    parameter PAYLOAD_W        = 2*(1+32+32+64)+16
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
    input   wire [47:0]                 i_timestamp,
    output  wire                        o_valid,
    output  wire [PAYLOAD_W-1:0]        o_payload
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

wire                         aapl_ff_not_empty;
wire                         aapl_ff_valid;
wire [PAYLOAD_W-1:0]         aapl_payload;
wire                         hsbc_ff_not_empty;
wire                         hsbc_ff_valid;
wire [PAYLOAD_W-1:0]         hsbc_payload;
wire [SYMBOL_NUM-1:0]        arb_src_pop;
wire [SYMBOL_NUM-1:0]        arb_src_not_empty;
wire [SYMBOL_NUM-1:0]        arb_src_valid;
wire [SYMBOL_NUM*PAYLOAD_W-1:0] arb_src_payload;


wire [PARSER_MSG_BIT-1:0]       parser_msg   = {i_msg_valid, i_seq_num, i_msg_type, i_stock_locate, i_order_ref_num, i_new_order_ref_num, side, i_shares, i_price};

symbol_book #(
    .QTY_SHARE_BIT      (32), // number of bits to represent shares quantity at each price level
    .QTY_PRICE_LVL_BIT  (8),
    .BOOK_LEVEL_BIT     (12 ),
    .PRICE_BASE         (32'd0000_0000  ), // default lowest price (in 1/10000 dollars) for all stocks, can be updated by control plane.
    .STOCK_LOCATE       (16'h000d       ), // stock locate for this symbol book, assigned by control plane
    .EVENT_FIFO_DEPTH   (EVENT_FIFO_DEPTH)
) symbol_book_AAPL (
    .i_clk_156          (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_parser_msg       (parser_msg          ),
    .i_ff_pop           (arb_src_pop[0]     ),
    .o_not_empty        (aapl_ff_not_empty  ),
    .o_valid            (aapl_ff_valid      ),
    .o_payload          (aapl_payload       )
);

symbol_book #(
    .QTY_SHARE_BIT      (32), // number of bits to represent shares quantity at each price level
    .QTY_PRICE_LVL_BIT  (8),
    .BOOK_LEVEL_BIT     (12 ),
    .PRICE_BASE         (32'd0000_0000  ), // default lowest price (in 1/10000 dollars) for all stocks, can be updated by control plane.
    .STOCK_LOCATE       (16'h0ee8       ), // stock locate for this symbol book, assigned by control plane
    .EVENT_FIFO_DEPTH   (EVENT_FIFO_DEPTH)
) symbol_book_HSBC (
    .i_clk_156          (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_parser_msg       (parser_msg         ),
    .i_ff_pop           (arb_src_pop[1]     ),
    .o_not_empty        (hsbc_ff_not_empty ),
    .o_valid            (hsbc_ff_valid     ),
    .o_payload          (hsbc_payload      )
);

assign arb_src_not_empty = {hsbc_ff_not_empty, aapl_ff_not_empty};
assign arb_src_valid     = {hsbc_ff_valid,     aapl_ff_valid};
assign arb_src_payload   = {hsbc_payload,      aapl_payload};

arbiter #(
    .SYMBOL_NUM (SYMBOL_NUM),
    .PAYLOAD_W  (PAYLOAD_W)
) event_fifo_arbiter_inst (
    .i_clk_156      (i_clk_156),
    .i_rst          (i_rst),
    .i_src_not_empty(arb_src_not_empty),
    .i_src_valid    (arb_src_valid),
    .i_src_payload  (arb_src_payload),
    .o_src_pop      (arb_src_pop),
    .o_valid        (o_valid),
    .o_payload      (o_payload)
);

wire            o_ask_best_valid    = o_payload[273];
wire [31:0]     o_ask_best_price    = o_payload[272:241];
wire [31:0]     o_ask_best_shares   = o_payload[240:209];
wire [63:0]     o_ask_seq_num       = o_payload[208:145];
wire            o_bid_best_valid    = o_payload[144];
wire [31:0]     o_bid_best_price    = o_payload[143:112];
wire [31:0]     o_bid_best_shares   = o_payload[111:80];
wire [63:0]     o_bid_seq_num       = o_payload[79:16];
wire [15:0]     o_stock_locate      = o_payload[15:0];

endmodule
