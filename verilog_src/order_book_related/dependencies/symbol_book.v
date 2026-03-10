// order book per symbol.
module symbol_book #(
    parameter QTY_SHARE_BIT         = 32,           // number of bits to represent shares quantity at each price level
    parameter QTY_PRICE_LVL_BIT     = 12,           // trace 2^QTY_PRICE_LVL_BIT price levels
    parameter BOOK_LEVEL_BIT         = 12,           // trace 2^BOOK_LEVEL_BIT orders in the order table
    parameter PRICE_BASE            = 32'd0,        // 
    parameter STOCK_LOCATE          = 16'h000d,      // stock locate for this symbol book, can be updated by control plane.
    parameter PARSER_MSG_BIT        = 1+64+8+16+64+64+2+32+32
) (
    // order book parser interface
    input   wire                            i_clk_156,
    input   wire                            i_rst,               // active high
    input   wire [PARSER_MSG_BIT-1:0]       i_parser_msg,
    output  wire                            o_event             ,
    output  wire [2*(1+32+QTY_SHARE_BIT+64)+16-1:0] o_payload      
             // pass through seq
);
      
localparam                  QTY_MSG_BIT                 = 2+32+1+32+64; // {bid_ask, price, is_add, d_shares, seq_num}
wire                           o_ask_best_valid    ;
wire [31:0]                    o_ask_best_price    ;
wire [QTY_SHARE_BIT-1:0]       o_ask_best_shares   ;
wire [63:0]                    o_ask_seq_num       ;
wire                           o_bid_best_valid    ;
wire [31:0]                    o_bid_best_price    ;
wire [QTY_SHARE_BIT-1:0]       o_bid_best_shares   ;
wire [63:0]                    o_bid_seq_num       ;
reg [31:0]                     prev_ask_best_price    ;
reg [QTY_SHARE_BIT-1:0]        prev_ask_best_shares   ;
reg [63:0]                     prev_ask_seq_num       ;
reg                            prev_bid_best_valid    ;
reg [31:0]                     prev_bid_best_price    ;
reg [QTY_SHARE_BIT-1:0]        prev_bid_best_shares   ;


always @(posedge i_clk_156) begin
    if (i_rst) begin
        prev_ask_best_price  <= 32'd0;
        prev_ask_best_shares <= {QTY_SHARE_BIT{1'b0}};
        prev_ask_seq_num     <= 64'd0;
        prev_bid_best_valid  <= 1'b0;
        prev_bid_best_price  <= 32'd0;
        prev_bid_best_shares <= {QTY_SHARE_BIT{1'b0}};
    end else begin
        prev_ask_best_price  <= o_ask_best_price;
        prev_ask_best_shares <= o_ask_best_shares;
        prev_ask_seq_num     <= o_ask_seq_num;
        prev_bid_best_valid  <= o_bid_best_valid;
        prev_bid_best_price  <= o_bid_best_price;
        prev_bid_best_shares <= o_bid_best_shares;
    end
end

assign o_event = (o_ask_best_price != prev_ask_best_price)  ||
                 (o_ask_best_shares != prev_ask_best_shares) ||
                 (o_bid_best_valid != prev_bid_best_valid) ||
                 (o_bid_best_price != prev_bid_best_price) ||
                 (o_bid_best_shares != prev_bid_best_shares);

assign o_payload = {
    o_ask_best_valid,
    o_ask_best_price,
    o_ask_best_shares,
    o_ask_seq_num,
    o_bid_best_valid,
    o_bid_best_price,
    o_bid_best_shares,
    o_bid_seq_num,
    stock_locate
};



wire [QTY_MSG_BIT-1:0] qty_msg;
wire [15:0]            stock_locate = i_parser_msg[PARSER_MSG_BIT-74:PARSER_MSG_BIT-89]; // stock locate is at bit 210-225 in parser_msg, assigned by control plane.

wire stock_valid = (stock_locate == STOCK_LOCATE);

book_builder #(
    .BOOK_LEVEL_BIT     (BOOK_LEVEL_BIT         ),
    .PARSER_MSG_BIT     (PARSER_MSG_BIT         ),
    .QTY_MSG_BIT        (QTY_MSG_BIT            )
) book_builder_inst (
    .i_clk_156          (i_clk_156              ),
    .i_rst              (i_rst                  ),
    .i_parser_msg       (i_parser_msg           ),
    .i_stock_valid      (stock_valid            ),
    .o_qty_msg          (qty_msg                )
);


qty_book_wrapper #(
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
    .o_best_valid_aligned   (o_ask_best_valid   ),
    .o_best_price_aligned   (o_ask_best_price   ),
    .o_best_shares          (o_ask_best_shares   ),
    .o_seq_num              (o_ask_seq_num) 
);


qty_book_wrapper #(
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
    .o_best_valid_aligned   (o_bid_best_valid   ),
    .o_best_price_aligned   (o_bid_best_price   ),
    .o_best_shares          (o_bid_best_shares  ),
    .o_seq_num              (o_bid_seq_num) 
);



endmodule
