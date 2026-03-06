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

// ==== qty bram RMW controller ====

reg [1:0]                   latch_qty_bid_ask;
reg [PRICE_ADDR_WIDTH-1:0]  latch_qty_prc_idx; // price index for qty bram access, calculated from qty_price. Latch it because the calculation also takes one cycle.
reg                         latch_qty_is_add;
reg [QTY_BOOK_WIDTH-1:0]    latch_qty_d_shares;

reg [1:0]                   qty_upd_state;
reg [1:0]                   qty_op;
reg [QTY_BOOK_WIDTH-1:0]    qty_i_data;
wire [QTY_BOOK_WIDTH-1:0]   qty_bid_o_data;
wire [QTY_BOOK_WIDTH-1:0]   qty_ask_o_data;

wire [1:0]                  qty_bid_op        = (latch_qty_bid_ask == BID) ? qty_op : IDLE;
wire [1:0]                  qty_ask_op        = (latch_qty_bid_ask == ASK) ? qty_op : IDLE;
wire [QTY_BOOK_WIDTH-1:0]   qty_cur           = (latch_qty_bid_ask == ASK) ? qty_ask_o_data : qty_bid_o_data;
wire [QTY_BOOK_WIDTH-1:0]   qty_new           = latch_qty_is_add ?
                                                (qty_cur + latch_qty_d_shares) :
                                                ((qty_cur > latch_qty_d_shares) ? (qty_cur - latch_qty_d_shares) : {QTY_BOOK_WIDTH{1'b0}});


wire [QTY_MSG_WIDTH-1:0]    qty_msg;
wire [1:0]                  qty_bid_ask     = qty_msg[QTY_MSG_WIDTH-1:QTY_MSG_WIDTH-2];
wire [31:0]                 qty_price       = qty_msg[QTY_MSG_WIDTH-3:QTY_MSG_WIDTH-34];
wire                        qty_is_add      = qty_msg[QTY_MSG_WIDTH-35];
wire [31:0]                 qty_d_shares    = qty_msg[QTY_MSG_WIDTH-36:QTY_MSG_WIDTH-67];

book_builder #(
    .STOCK_LOCATE     (STOCK_LOCATE      ),
    .BOOK_ADDR_WIDTH  (BOOK_ADDR_WIDTH  ),
    .PARSER_MSG_WIDTH (PARSER_MSG_WIDTH ),
    .ORDER_MSG_WIDTH  (ORDER_MSG_WIDTH  ),
    .QTY_MSG_WIDTH    (QTY_MSG_WIDTH    )
) book_builder_inst (
    .i_clk_156          (i_clk_156              ),
    .i_rst              (i_rst                  ),
    .i_parser_msg       (parser_msg             ),
    .o_qty_msg          (qty_msg                )
);



// ==== latch signals for qty update ====
always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        latch_qty_bid_ask    <= IDLE;
        latch_qty_prc_idx    <= {PRICE_ADDR_WIDTH{1'b0}};
        latch_qty_is_add     <= 1'b0;
        latch_qty_d_shares   <= {QTY_BOOK_WIDTH{1'b0}};
    end else begin
        if (qty_bid_ask != IDLE)  begin
            latch_qty_bid_ask    <= qty_bid_ask;
            latch_qty_prc_idx    <= cal_qty_book_addr(qty_price);
            latch_qty_is_add     <= qty_is_add;
            latch_qty_d_shares   <= qty_d_shares;
        end
    end
end


always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        qty_upd_state           <= IDLE;
        qty_op                  <= IDLE;
    end else begin
        case (qty_upd_state)
            IDLE: begin
                if (qty_bid_ask != IDLE) begin
                    qty_op          <= READ;
                    qty_upd_state   <= FIRST_CYCLE; // wait for read data
                end else begin
                    qty_op          <= IDLE;
                    qty_upd_state   <= IDLE;
                    qty_i_data      <= {QTY_BOOK_WIDTH{1'b0}};
                end
            end
            FIRST_CYCLE: begin
                qty_upd_state       <= SECOND_CYCLE;
            end
            SECOND_CYCLE: begin
                qty_op          <= WRITE;
                qty_i_data      <= qty_new;
                qty_upd_state   <= IDLE;
            end
            default: begin
                qty_upd_state <= IDLE;
                qty_i_data    <= {QTY_BOOK_WIDTH{1'b0}};
                qty_op        <= IDLE;
            end
        endcase
    end
end

bram #(
    .ADDR_WIDTH         (PRICE_ADDR_WIDTH    ),
    .DATA_WIDTH         (QTY_BOOK_WIDTH      )
)
bid_qty_bram_inst (
    .i_clk              (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_addr             (latch_qty_prc_idx  ),
    .i_op               (qty_bid_op         ),
    .i_data             (qty_i_data         ),
    .o_data             (qty_bid_o_data     )
);

bram #(
    .ADDR_WIDTH         (PRICE_ADDR_WIDTH    ),
    .DATA_WIDTH         (QTY_BOOK_WIDTH      )
)
ask_qty_bram_inst (
    .i_clk              (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_addr             (latch_qty_prc_idx  ),
    .i_op               (qty_ask_op         ),
    .i_data             (qty_i_data         ),
    .o_data             (qty_ask_o_data      )
);

function [PRICE_ADDR_WIDTH-1:0] cal_qty_book_addr (input [31:0] price);
    reg [29:0] price_offset_u30;
begin
    if (price >= PRICE_BASE) begin
        price_offset_u30 = (price - PRICE_BASE) >> 2;
        if (price_offset_u30 < PRICE_DEPTH) begin
            cal_qty_book_addr = price_offset_u30[PRICE_ADDR_WIDTH-1:0];
        end else begin
            cal_qty_book_addr = {PRICE_ADDR_WIDTH{1'b0}};
        end
    end else begin
        cal_qty_book_addr = {PRICE_ADDR_WIDTH{1'b0}};
    end
end
endfunction

endmodule
