// order book per symbol.
module symbol_book #(
    parameter SHARE_PER_PRICE_BIT   = 32,     // number of bits to represent shares quantity at each price level
    parameter PRICE_ADDR_WIDTH      = 10,    // trace 2^PRICE_ADDR_WIDTH price levels
    parameter BOOK_ADDR_WIDTH       = 12,     // trace 2^BOOK_ADDR_WIDTH orders in the order table
    parameter PRICE_BASE            = 32'd2317700, // 231.77 in  dollars. In binary, the last two bits are always 00, can drop
    parameter STOCK_LOCATE          = 16'h000d      // stock locate for this symbol book, can be updated by control plane.
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
    input   wire                        i_side,               // 'B' or 'S' for A/F
    input   wire [31:0]                 i_shares,
    input   wire [31:0]                 i_price,            // in 1/10000 dollars
    input   wire [47:0]                 i_timestamp

);

localparam                  MSG_WIDTH                   = 8+64+64+1+32+32;      // {msg_type, order_ref_num, new_order_ref_num, side, price, shares}
localparam                  BOOK_DEPTH                  = 1<<BOOK_ADDR_WIDTH;   // total number of orders to track in the order table, 2^BOOK_ADDR_WIDTH
localparam                  ORDER_BOOK_WIDTH            = 66;                   // {valid, side,shares, price } 1+1+32+32 = 66 bits
localparam                  QTY_BOOK_WIDTH              = SHARE_PER_PRICE_BIT;
localparam                  PRICE_DEPTH                 = 1 << PRICE_ADDR_WIDTH;

localparam                  TYPE_A                      = 8'h41;
localparam                  TYPE_X                      = 8'h58;
localparam                  TYPE_D                      = 8'h44;
localparam                  TYPE_U                      = 8'h55;
localparam                  TYPE_E                      = 8'h45;
localparam                  TYPE_F                      = 8'h46;
localparam                  TYPE_C                      = 8'h43;

// === FIFO. save it in FIFO first, in case order book updating may need multiple cycles.
wire [MSG_WIDTH-1:0]        ff_i_msg                 = {i_msg_type, i_order_ref_num, i_new_order_ref_num, i_side, i_shares, i_price};
wire [MSG_WIDTH-1:0]        ff_o_msg;
wire [7:0]                  ff_o_msg_type             = ff_o_msg[MSG_WIDTH-1:MSG_WIDTH-8]      ;
wire [63:0]                 ff_o_order_ref_num        = ff_o_msg[MSG_WIDTH-9:MSG_WIDTH-72]     ;
wire [63:0]                 ff_o_new_order_ref_num    = ff_o_msg[MSG_WIDTH-73:MSG_WIDTH-136]   ;
wire                        ff_o_side                 = ff_o_msg[MSG_WIDTH-137]                 ;
wire [31:0]                 ff_o_shares               = ff_o_msg[MSG_WIDTH-138:MSG_WIDTH-169]  ;
wire [31:0]                 ff_o_price                = ff_o_msg[MSG_WIDTH-170:MSG_WIDTH-201]  ;
reg  [BOOK_ADDR_WIDTH-1:0]  order_addr;            // address for order table BRAM read/write 
wire                        ff_o_valid;
wire                        ff_push;                     
wire                        ff_pop;                      
reg [ORDER_BOOK_WIDTH-1:0]  order_w_data; 
wire[ORDER_BOOK_WIDTH-1:0]  order_r_data;
          
wire                        order_rvalid      = order_r_data[ORDER_BOOK_WIDTH-1];
wire                        order_rside       = order_r_data[ORDER_BOOK_WIDTH-2];
wire [31:0]                 order_rshares     = order_r_data[63:32];
wire [31:0]                 order_rprice      = order_r_data[31:0];

wire                        order_wvalid      = order_w_data[ORDER_BOOK_WIDTH-1];
wire                        order_wside       = order_w_data[ORDER_BOOK_WIDTH-2];
wire [31:0]                 order_wshares     = order_w_data[63:32];
wire [31:0]                 order_wprice      = order_w_data[31:0];


reg [1:0]                   order_upd_state;
reg [1:0]                   qty_upd_state;
localparam                  IDLE                        = 2'b00 ;
localparam                  FIRST_CYCLE                 = 2'b01 ;
localparam                  SECOND_CYCLE                = 2'b10 ;

reg [1:0]                   order_op; 
localparam                  READ                        = 2'b01 ;
localparam                  WRITE                       = 2'b10 ;



wire                        order_bram_busy       = ((order_upd_state != IDLE)  || (qty_upd_state != IDLE));
wire                        ff_not_empty; 

msg_fifo #(
    .DEPTH          (8              ),
    .DATA_W         (MSG_WIDTH      ) // {msg_type, order_ref_num, new_order_ref_num, side, shares, price}
) msg_fifo_inst (
    .i_clk          (i_clk_156      ),
    .i_rst          (i_rst          ),
    .i_do_push      (ff_push        ), 
    .i_data         (ff_i_msg       ),  
    .o_push_ready   (               ),
    .i_do_pop       (ff_pop         ),
    .o_not_empty    (ff_not_empty   ),
    .o_valid        (ff_o_valid     ),
    .o_data         (ff_o_msg       ) // remain the same if no new pop or pop invalid
);
// === data from/to fifo ===


fifo_controller #(
    .STOCK_LOCATE    (STOCK_LOCATE      )
) fifo_controller_inst (
    .i_msg_valid     (i_msg_valid       ),
    .i_stock_locate  (i_stock_locate    ),
    .i_not_empty     (ff_not_empty      ),
    .i_is_busy       (order_bram_busy   ),
    .o_do_push       (ff_push           ),
    .o_do_pop        (ff_pop            ) // only fire ff_pop when ff_not_empty and in IDLE state
);

bram #(
    .ADDR_WIDTH      (BOOK_ADDR_WIDTH    ),
    .DATA_WIDTH      (ORDER_BOOK_WIDTH   )
) order_bram_inst (
    .i_clk           (i_clk_156          ),
    .i_rst           (i_rst              ),
    .i_addr          (order_addr         ),
    .i_op            (order_op           ),
    .i_data          (order_w_data       ),
    .o_data          (order_r_data       ) // remain the same if order_addr is does not change or no new read signal
);



// ==== main logic ====
always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        order_w_data        <= {ORDER_BOOK_WIDTH{1'b0}};
        order_upd_state     <= IDLE;
        order_op            <= IDLE;
        order_addr          <= {BOOK_ADDR_WIDTH{1'b0}};
        qty_bid_ask          <= IDLE;
        qty_price           <= 32'd0;
        qty_is_add          <= 1'b0;
        qty_d_shares        <= 32'd0;
    end else if (ff_o_valid || (order_upd_state != IDLE)) begin
        case (order_upd_state)
            IDLE: begin
                case (ff_o_msg_type) 
                TYPE_A,TYPE_F: begin // create new order
                    order_op        <= WRITE;
                    order_w_data        <= {1'b1, ff_o_side, ff_o_shares, ff_o_price};
                    order_addr          <= cal_order_book_addr(ff_o_order_ref_num);
                    order_upd_state     <= IDLE;
                    qty_bid_ask          <= (ff_o_side == 1'b0) ? BID : ASK;
                    qty_price           <= ff_o_price;
                    qty_is_add          <= 1'b1; 
                    qty_d_shares        <= ff_o_shares;
                end
                TYPE_D: begin // read for updating bid/ask qty, then delete order
                    order_op            <= READ;
                    order_addr          <= cal_order_book_addr(ff_o_order_ref_num);
                    order_upd_state     <= FIRST_CYCLE;
                    qty_bid_ask          <= IDLE;
                end
                TYPE_E,TYPE_C,TYPE_X: begin // first, read data
                    order_op             <= READ;
                    order_addr           <= cal_order_book_addr(ff_o_order_ref_num);
                    order_upd_state      <= FIRST_CYCLE;
                    qty_bid_ask          <= IDLE;
                end
                TYPE_U: begin   // first, read
                    order_op             <= READ;
                    order_addr           <= cal_order_book_addr(ff_o_order_ref_num);
                    order_upd_state      <= FIRST_CYCLE;
                    qty_bid_ask          <= IDLE;
                end
                default: begin
                    order_op        <= IDLE;
                    order_w_data        <= {ORDER_BOOK_WIDTH{1'b0}};
                    order_addr          <= {BOOK_ADDR_WIDTH{1'b0}};
                    order_upd_state     <= IDLE;
                    qty_bid_ask          <= IDLE;
                end
                endcase
            end
            FIRST_CYCLE: begin
                case (ff_o_msg_type)
                TYPE_U: begin //: insert new order (also waiting for read)
                    order_op       <= WRITE;
                    order_w_data       <= {1'b1, ff_o_side, ff_o_shares, ff_o_price};
                    order_addr         <= cal_order_book_addr(ff_o_new_order_ref_num);
                    order_upd_state    <= SECOND_CYCLE;
                    qty_bid_ask         <= (ff_o_side == 1'b0) ? BID : ASK;
                    qty_price          <= ff_o_price;
                    qty_is_add         <= 1'b1; // add order = increase qty at this price level
                    qty_d_shares       <= ff_o_shares;
                end
                TYPE_E,TYPE_C,TYPE_X,TYPE_D: begin 
                    order_upd_state    <= SECOND_CYCLE; // wait for the data to be read
                    qty_bid_ask         <= IDLE;
                end
                default: begin
                    order_op       <= IDLE;
                    order_w_data       <= {ORDER_BOOK_WIDTH{1'b0}};
                    order_addr         <= {BOOK_ADDR_WIDTH{1'b0}};
                    order_upd_state    <= IDLE;
                    qty_bid_ask         <= IDLE;
                end
                endcase
            end
            SECOND_CYCLE: begin
                case (ff_o_msg_type)
                    TYPE_D,TYPE_U: begin // delete order
                        if (order_rvalid) begin
                            order_op        <= WRITE;
                            order_w_data        <= {ORDER_BOOK_WIDTH{1'b0}};
                            order_addr          <= cal_order_book_addr(ff_o_order_ref_num);
                            qty_bid_ask          <= (order_rside == 1'b0) ? BID : ASK;
                            qty_price           <= order_rprice;
                            qty_is_add          <= 1'b0; // delete order = reduce qty at this price level
                            qty_d_shares        <= order_rshares;
                        end else begin
                            order_op        <= IDLE;
                            order_w_data        <= {ORDER_BOOK_WIDTH{1'b0}};
                            order_addr          <= {BOOK_ADDR_WIDTH{1'b0}};
                            qty_bid_ask          <= IDLE;
                        end
                        order_upd_state   <= IDLE; 
                    end
                    TYPE_E,TYPE_C,TYPE_X: begin // second , write new shares back
                        if (order_rvalid && ( order_rshares >= ff_o_shares)) begin
                            order_op          <= WRITE;
                            order_addr             <= cal_order_book_addr(ff_o_order_ref_num);
                            order_w_data <= {1'b1, order_rside, (order_rshares - ff_o_shares), order_rprice};
                            qty_bid_ask             <= (order_rside == 1'b0) ? BID : ASK;
                            qty_price              <= order_rprice;
                            qty_is_add             <= 1'b0; // sub shares
                            qty_d_shares           <= ff_o_shares;
                        end else begin
                            order_op           <= IDLE;
                            order_w_data           <= {ORDER_BOOK_WIDTH{1'b0}};
                            order_addr             <= {BOOK_ADDR_WIDTH{1'b0}};
                            qty_bid_ask             <= IDLE;
                        end
                        order_upd_state            <= IDLE;
                    end
                    default: begin
                        order_op               <= IDLE;
                        order_w_data               <= {ORDER_BOOK_WIDTH{1'b0}};
                        order_addr                 <= {BOOK_ADDR_WIDTH{1'b0}};
                        order_upd_state            <= IDLE;
                        qty_bid_ask                 <= IDLE;
                    end
                endcase
            end
        endcase
    end else begin
        order_op        <= IDLE;
        order_w_data        <= {ORDER_BOOK_WIDTH{1'b0}};
        order_addr          <= {BOOK_ADDR_WIDTH{1'b0}};
        order_upd_state     <= order_upd_state;
        qty_bid_ask          <= IDLE;
    end
end





// ==== qty bram RMW controller (moved from qty_bram.v) ====



localparam                  BID                         = 2'b01;
localparam                  ASK                         = 2'b10;
reg [1:0]                   qty_bid_ask;
reg [31:0]                  qty_price;
reg                         qty_is_add;
reg [31:0]                  qty_d_shares; // _d means delta.

reg [1:0]                   latch_qty_bid_ask;
reg [31:0]                  latch_qty_prc_idx; // price index for qty bram access, calculated from qty_price. Latch it because the calculation also takes one cycle.
reg                         latch_qty_is_add;
reg [QTY_BOOK_WIDTH-1:0]    latch_qty_d_shares;


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
wire                        qty_upd_valid     = (qty_bid_ask != IDLE) && (qty_upd_state == IDLE);


// ==== latch signals for qty update ====
always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        latch_qty_bid_ask     <= IDLE;
        latch_qty_prc_idx    <= 32'd0;
        latch_qty_is_add     <= 1'b0;
        latch_qty_d_shares   <= {QTY_BOOK_WIDTH{1'b0}};
    end else begin
        if (qty_upd_valid)  begin
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
                if (qty_upd_valid) begin
                    qty_op          <= READ;
                    qty_upd_state   <= FIRST_CYCLE;
                end else begin
                    qty_op          <= IDLE;
                    qty_upd_state   <= IDLE;
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


// ==== hash function for order reference number ===
function [BOOK_ADDR_WIDTH-1:0] cal_order_book_addr (input [63:0] order_ref_num) ;
    integer i;
    integer j;
begin
    cal_order_book_addr = 0;
    for (i=0; i<BOOK_ADDR_WIDTH; i=i+1) begin
        for (j=0; j<64; j=j+BOOK_ADDR_WIDTH) begin
            if (i+j < 64) begin
            cal_order_book_addr[i] = cal_order_book_addr[i] ^ order_ref_num[i+j];
            end 
        end
    end
end
endfunction



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
