module qty_builder #(
    parameter QTY_MSG_WIDTH     = 2+32+1+32,
    parameter PRICE_ADDR_WIDTH  = 10,
    parameter QTY_BOOK_WIDTH    = 32,
    parameter PRICE_BASE       =  32'd0 // 
)(
    input   wire                        i_clk_156,
    input   wire                        i_rst,               // active high
    input   wire [QTY_MSG_WIDTH-1:0]    i_qty_msg
    
);



reg [1:0]                              qty_upd_state;
reg [1:0]                              qty_op;
reg [QTY_BOOK_WIDTH-1:0]                qty_i_shares;
localparam IDLE                        = 2'b00;
localparam FIRST_CYCLE                 = 2'b01;
localparam SECOND_CYCLE                = 2'b10;
localparam READ                        = 2'b01;
localparam WRITE                       = 2'b10;
localparam BID                         = 2'b01;
localparam ASK                         = 2'b10;
localparam PRICE_DEPTH                 = 1 << PRICE_ADDR_WIDTH;

wire [1:0]                  i_qty_bid_ask   = i_qty_msg[QTY_MSG_WIDTH-1:QTY_MSG_WIDTH-2];
wire                        ff_not_empty;

assign                      ff_push         = i_qty_bid_ask != IDLE;
assign                      ff_pop          = ff_not_empty && (qty_upd_state == IDLE) && !ff_o_valid;

wire [QTY_MSG_WIDTH-1:0]    ff_o_qty_msg;

wire                        ff_o_valid;    
wire [1:0]                  qty_bid_ask     = ff_o_qty_msg[QTY_MSG_WIDTH-1:QTY_MSG_WIDTH-2];
wire [31:0]                 qty_price       = ff_o_qty_msg[QTY_MSG_WIDTH-3:QTY_MSG_WIDTH-34];
wire                        qty_is_add      = ff_o_qty_msg[QTY_MSG_WIDTH-35];
wire [31:0]                 qty_d_shares    = ff_o_qty_msg[QTY_MSG_WIDTH-36:QTY_MSG_WIDTH-67];

fifo #(
    .DEPTH              (8                  ),
    .DATA_W             (QTY_MSG_WIDTH      )
)qty_msg_fifo_inst (
    .i_clk              (i_clk_156          ),
    .i_rst              (i_rst              ),
    .i_do_push          (ff_push            ), 
    .o_push_ready       (                   ),
    .i_data             (i_qty_msg          ),
    .i_do_pop           (ff_pop             ),
    .o_not_empty        (ff_not_empty       ),
    .o_valid            (ff_o_valid         ),
    .o_data             (ff_o_qty_msg        )
);




reg [1:0]                   latch_qty_bid_ask;
reg [PRICE_ADDR_WIDTH-1:0]  latch_qty_prc_idx; // price index for qty bram access, calculated from qty_price. Latch it because the calculation also takes one cycle.
reg                         latch_qty_is_add;
reg [QTY_BOOK_WIDTH-1:0]    latch_qty_d_shares;

wire [31:0]                 qty_bid_o_shares;
wire [31:0]                 qty_ask_o_shares;
wire [1:0]                  qty_bid_op        = (latch_qty_bid_ask == BID) ? qty_op : IDLE;
wire [1:0]                  qty_ask_op        = (latch_qty_bid_ask == ASK) ? qty_op : IDLE;
wire [QTY_BOOK_WIDTH-1:0]   qty_cur           = (latch_qty_bid_ask == ASK) ? qty_ask_o_shares : qty_bid_o_shares;
wire [QTY_BOOK_WIDTH-1:0]   qty_new           = latch_qty_is_add ?
                                                (qty_cur + latch_qty_d_shares) :
                                                ((qty_cur > latch_qty_d_shares) ? (qty_cur - latch_qty_d_shares) : {QTY_BOOK_WIDTH{1'b0}});


// ==== latch signals for qty update ====
always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        latch_qty_bid_ask    <= IDLE;
        latch_qty_prc_idx    <= {PRICE_ADDR_WIDTH{1'b0}};
        latch_qty_is_add     <= 1'b0;
        latch_qty_d_shares   <= {QTY_BOOK_WIDTH{1'b0}};
    end else begin
        if (ff_o_valid)  begin
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
                if (ff_o_valid) begin
                    qty_op          <= READ;
                    qty_upd_state   <= FIRST_CYCLE; // wait for read data
                end else begin
                    qty_op          <= IDLE;
                    qty_upd_state   <= IDLE;
                    qty_i_shares      <= {QTY_BOOK_WIDTH{1'b0}};
                end
            end
            FIRST_CYCLE: begin
                qty_upd_state       <= SECOND_CYCLE;
            end
            SECOND_CYCLE: begin
                qty_op          <= WRITE;
                qty_i_shares    <= qty_new;
                qty_upd_state   <= IDLE;
            end
            default: begin
                qty_upd_state <= IDLE;
                qty_i_shares  <= {QTY_BOOK_WIDTH{1'b0}};
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
    .i_data             (qty_i_shares       ),
    .o_data             (qty_bid_o_shares     )
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
    .i_data             (qty_i_shares         ),
    .o_data             (qty_ask_o_shares      )
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