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
    input   wire [SYMBOL_NUM*16-1:0]        i_symbol_stock_locate_cfg,
    input   wire [SYMBOL_NUM*32-1:0]        i_symbol_price_base_cfg,
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

wire [PARSER_MSG_BIT-1:0]       parser_msg   = {i_msg_valid, i_msg_type, i_stock_locate, i_order_ref_num, i_new_order_ref_num, side, i_shares, i_price,i_frame_ts};

generate
    genvar symbol_idx;
    for (symbol_idx = 0; symbol_idx < SYMBOL_NUM; symbol_idx = symbol_idx + 1) begin : g_symbol_books
        wire [15:0] symbol_stock_locate_cfg;
        wire [31:0] symbol_price_base_cfg;

        assign symbol_stock_locate_cfg = i_symbol_stock_locate_cfg[symbol_idx*16 +: 16];
        assign symbol_price_base_cfg   = i_symbol_price_base_cfg[symbol_idx*32 +: 32];

        symbol_book #(
            .QTY_PRICE_LVL_BIT  (8),
            .BOOK_LEVEL_BIT     (12 ),
            .EVENT_FIFO_DEPTH   (EVENT_FIFO_DEPTH),
            .PARSER_MSG_BIT     (PARSER_MSG_BIT),
            .PAYLOAD_W          (PAYLOAD_W          )
        ) symbol_book_inst (
            .i_clk_156          (i_clk_156          ),
            .i_rst              (i_rst              ),
            .i_stock_locate_cfg (symbol_stock_locate_cfg),
            .i_price_base_cfg   (symbol_price_base_cfg),
            .i_parser_msg       (parser_msg         ),
            .o_event_found      (o_event_valid[symbol_idx]),
            .o_payload          (o_event_payload[symbol_idx*PAYLOAD_W +: PAYLOAD_W])
        );
    end
endgenerate

endmodule
