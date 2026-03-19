// order book per symbol.
module symbol_book #(
    parameter QTY_PRICE_LVL_BIT     = 12,           // trace 2^QTY_PRICE_LVL_BIT price levels
    parameter BOOK_LEVEL_BIT         = 12,           // trace 2^BOOK_LEVEL_BIT orders in the order table
    parameter PRICE_BASE            = 32'd0,        // 
    parameter STOCK_LOCATE          = 16'h000d,      // stock locate for this symbol book, can be updated by control plane.
    parameter PARSER_MSG_BIT        = 1+8+16+64+64+2+32+32+48,
    parameter EVENT_FIFO_DEPTH      = 2,
    parameter PAYLOAD_W             = 2*(32+32)+48+16
) (
    // order book parser interface
    input   wire                                    i_clk_156,
    input   wire                                    i_rst,               // active high
    input   wire [PARSER_MSG_BIT-1:0]               i_parser_msg,
    output  wire                                    o_event_found,
    output  wire [PAYLOAD_W-1:0]                    o_payload
);
      
localparam                     QTY_MSG_BIT                 = 2+32+1+32+1+48; // {bid_ask, price, is_add, d_shares, op_done, timestamp}
wire [31:0]                    o_ask_best_price       ;
wire [31:0]                    o_ask_best_shares      ;
wire [31:0]                    o_bid_best_price       ;
wire [31:0]                    o_bid_best_shares      ;
reg [31:0]                     prev_ask_best_price    ;
reg [31:0]                     prev_ask_best_shares   ;
reg [31:0]                     prev_bid_best_price    ;
reg [31:0]                     prev_bid_best_shares   ;
wire                           ff_push;
wire [PAYLOAD_W-1:0]           payload;
wire                           ask_changed;
wire                           bid_changed;
wire [47:0]                     event_timestamp;
wire [47:0]                     ask_time_stamp;
wire [47:0]                     bid_time_stamp;
assign event_timestamp = ask_time_stamp | bid_time_stamp;


always @(posedge i_clk_156) begin
    if (i_rst) begin
        prev_ask_best_price  <= 32'd0;
        prev_ask_best_shares <= {32{1'b0}};
        prev_bid_best_price  <= 32'd0;
        prev_bid_best_shares <= {32{1'b0}};
    end else begin
        prev_ask_best_price  <= o_ask_best_price;
        prev_ask_best_shares <= o_ask_best_shares;
        prev_bid_best_price  <= o_bid_best_price;
        prev_bid_best_shares <= o_bid_best_shares;
    end
end



assign ask_changed = (o_ask_best_price != prev_ask_best_price)  ||
                     (o_ask_best_shares != prev_ask_best_shares) ;
assign bid_changed = (o_bid_best_price != prev_bid_best_price) ||
                     (o_bid_best_shares != prev_bid_best_shares);
assign o_event_found = (ask_changed || bid_changed) && op_done;




assign payload =  {
    o_ask_best_price,
    o_ask_best_shares,
    o_bid_best_price,
    o_bid_best_shares,
    event_timestamp,
    STOCK_LOCATE
};

assign o_payload = payload;



wire [QTY_MSG_BIT-1:0] qty_msg;
wire [15:0]            stock_locate = i_parser_msg[257:242];

wire stock_valid = (stock_locate == STOCK_LOCATE);
wire ask_op_done;
wire bid_op_done;
wire op_done = ask_op_done | bid_op_done;

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
    .PRICE_BASE             (PRICE_BASE           ),
    .BID_OR_ASK             (2'b10) //  01 for bid, 10 for ask
)
ask_wrapper_inst (
    .i_clk_156              (i_clk_156          ),
    .i_rst                  (i_rst              ),
    .i_qty_msg              (qty_msg            ),
    .o_best_price_aligned   (o_ask_best_price   ),
    .o_best_shares          (o_ask_best_shares   ),
    .o_op_done_aligned      (ask_op_done        ),
    .o_timestamp_aligned    (ask_time_stamp     )          

);


qty_book_wrapper #(
    .QTY_MSG_BIT            (QTY_MSG_BIT          ),
    .QTY_PRICE_LVL_BIT      (QTY_PRICE_LVL_BIT    ),
    .PRICE_BASE             (PRICE_BASE           ),
    .BID_OR_ASK             (2'b01) //  01 for bid, 10 for ask
)
bid_wrapper_inst (
    .i_clk_156              (i_clk_156          ),
    .i_rst                  (i_rst              ),
    .i_qty_msg              (qty_msg            ),
    .o_best_price_aligned   (o_bid_best_price   ),
    .o_best_shares          (o_bid_best_shares  ),
    .o_op_done_aligned      (bid_op_done        ),
    .o_timestamp_aligned    (bid_time_stamp     )
);




endmodule
