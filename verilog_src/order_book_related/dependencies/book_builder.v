module book_builder #(
    parameter BOOK_LEVEL_BIT      = 12,
    parameter PARSER_MSG_BIT      = 1+64+8+16+64+64+2+32+32+48,
    parameter QTY_MSG_BIT         = 2+32+1+32+64 // {bid_ask, price, is_add, d_shares}
) (
    input   wire                        i_clk_156,
    input   wire                        i_rst,               // active high
    input   wire                        i_stock_valid, // indicating if the input stock is the right one.
    input   wire [PARSER_MSG_BIT-1:0]   i_parser_msg,
    output  wire [QTY_MSG_BIT-1:0]      o_qty_msg
);


reg [1:0]                               qty_bid_ask;
reg  [31:0]                             qty_price;
reg                                     qty_is_add;
reg  [31:0]                             qty_d_shares;
reg  [63:0]                             qty_seq_num;


assign o_qty_msg = { qty_bid_ask, qty_price, qty_is_add, qty_d_shares, qty_seq_num};

localparam ORDER_BOOK_WIDTH            = 67; // {valid, side, shares, price}
localparam TYPE_A                      = 8'h41;
localparam TYPE_X                      = 8'h58;
localparam TYPE_D                      = 8'h44;
localparam TYPE_U                      = 8'h55;
localparam TYPE_E                      = 8'h45;
localparam TYPE_F                      = 8'h46;
localparam TYPE_C                      = 8'h43;
localparam IDLE                        = 2'b00;
localparam FIRST_CYCLE                 = 2'b01;
localparam SECOND_CYCLE                = 2'b10;
localparam THIRD_CYCLE                 = 2'b11;
localparam READ                        = 2'b01;
localparam WRITE                       = 2'b10;
localparam BID                         = 2'b01;
localparam ASK                         = 2'b10;

wire [PARSER_MSG_BIT-1:0] ff_o_msg;
wire                        ff_o_valid;
wire                        ff_push;
wire                        ff_pop;
wire                        ff_not_empty;
wire [63:0]   msg_seq_num       = ff_o_msg[281:218] ;
wire [7:0]    msg_type          = ff_o_msg[217:210] ;
wire [63:0]   msg_order_ref_num = ff_o_msg[193:130] ;
wire [63:0]   msg_new_order_ref = ff_o_msg[129:66]  ;
wire [1:0]    msg_side          = ff_o_msg[65:64]   ;
wire [31:0]   msg_shares        = ff_o_msg[63:32]   ;
wire [31:0]   msg_price         = ff_o_msg[31:0]    ;

assign ff_push = i_parser_msg[282] && i_stock_valid;
assign ff_pop  = ff_not_empty && (book_upd_state == IDLE) && !ff_o_valid;

fifo #(
    .DEPTH          (8              ),
    .DATA_W         (PARSER_MSG_BIT)
) msg_fifo_inst (
    .i_clk          (i_clk_156      ),
    .i_rst          (i_rst          ),
    .i_do_push      (ff_push        ),
    .i_data         (i_parser_msg   ),
    .o_push_ready   (               ),
    .i_do_pop       (ff_pop         ),
    .o_not_empty    (ff_not_empty   ),
    .o_valid        (ff_o_valid     ),
    .o_data         (ff_o_msg       )
);

reg [1:0]                   book_upd_state;
reg [1:0]                   book_op;
reg [BOOK_LEVEL_BIT-1:0]   book_addr;
reg [ORDER_BOOK_WIDTH-1:0]  book_i_data;
wire [ORDER_BOOK_WIDTH-1:0] book_o_data;

wire                        book_i_valid      = book_i_data[ORDER_BOOK_WIDTH-1];
wire [1:0]                  book_i_side       = book_i_data[ORDER_BOOK_WIDTH-2:ORDER_BOOK_WIDTH-3];
wire [31:0]                 book_i_shares     = book_i_data[63:32];
wire [31:0]                 book_i_price      = book_i_data[31:0];

wire                        book_o_valid      = book_o_data[ORDER_BOOK_WIDTH-1];
wire [1:0]                  book_o_side       = book_o_data[ORDER_BOOK_WIDTH-2:ORDER_BOOK_WIDTH-3];
wire [31:0]                 book_o_shares     = book_o_data[63:32];
wire [31:0]                 book_o_price      = book_o_data[31:0];

bram #(
    .ADDR_WIDTH      (BOOK_LEVEL_BIT    ),
    .DATA_WIDTH      (ORDER_BOOK_WIDTH   )
) order_book_inst (
    .i_clk           (i_clk_156         ),
    .i_rst           (i_rst             ),
    .i_addr          (book_addr         ),
    .i_op            (book_op           ),
    .i_data          (book_i_data       ),
    .o_data          (book_o_data       )
);

always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        book_i_data      <= {ORDER_BOOK_WIDTH{1'b0}};
        book_upd_state   <= IDLE;
        book_op          <= IDLE;
        book_addr        <= {BOOK_LEVEL_BIT{1'b0}};
        qty_bid_ask      <= IDLE;
        qty_price        <= 32'd0;
        qty_is_add       <= 1'b0;
        qty_d_shares     <= 32'd0;
        qty_seq_num     <= 64'd0;
    end else if (ff_o_valid || (book_upd_state != IDLE)) begin
        case (book_upd_state)
            IDLE: begin
                case (msg_type)
                    TYPE_A,TYPE_F: begin
                        book_op          <= WRITE;
                        book_i_data      <= {1'b1, msg_side, msg_shares, msg_price};
                        book_addr        <= cal_order_book_addr(msg_order_ref_num);
                        book_upd_state   <= IDLE;
                        qty_bid_ask     <= msg_side;
                        qty_price       <= msg_price;
                        qty_is_add      <= 1'b1;
                        qty_d_shares    <= msg_shares;
                        qty_seq_num     <= msg_seq_num;
                    end
                    TYPE_E,TYPE_C,TYPE_X,TYPE_U,TYPE_D: begin
                        book_op          <= READ;
                        book_addr        <= cal_order_book_addr(msg_order_ref_num);
                        book_upd_state   <= FIRST_CYCLE;
                        qty_bid_ask      <= IDLE;
                    end
                    default: begin
                        book_op          <= IDLE;
                        book_i_data      <= {ORDER_BOOK_WIDTH{1'b0}};
                        book_addr        <= {BOOK_LEVEL_BIT{1'b0}};
                        book_upd_state   <= IDLE;
                        qty_bid_ask      <= IDLE;   
                    end
                endcase
            end
            FIRST_CYCLE: begin
                case (msg_type)
                    TYPE_E,TYPE_C,TYPE_X,TYPE_D,TYPE_U: begin
                        book_op          <= READ;
                        book_addr        <= cal_order_book_addr(msg_order_ref_num);
                        book_upd_state   <= SECOND_CYCLE;
                        qty_bid_ask      <= IDLE;
                    end
                    default: begin
                        book_op          <= IDLE;
                        book_i_data      <= {ORDER_BOOK_WIDTH{1'b0}};
                        book_addr        <= {BOOK_LEVEL_BIT{1'b0}};
                        book_upd_state   <= IDLE;
                        qty_bid_ask      <= IDLE;
                    end
                endcase
            end
            SECOND_CYCLE: begin
                case (msg_type)
                    TYPE_D: begin
                        if (book_o_valid) begin
                            book_op          <= WRITE;
                            book_i_data      <= {ORDER_BOOK_WIDTH{1'b0}};
                            book_addr        <= cal_order_book_addr(msg_order_ref_num);
                            qty_bid_ask      <= book_o_side;
                            qty_price        <= book_o_price;
                            qty_is_add       <= 1'b0;
                            qty_d_shares     <= book_o_shares;
                            qty_seq_num     <= msg_seq_num;
                        end else begin
                            book_op          <= IDLE;
                            book_i_data      <= {ORDER_BOOK_WIDTH{1'b0}};
                            book_addr        <= {BOOK_LEVEL_BIT{1'b0}};
                            qty_bid_ask      <= IDLE;
                        end
                        book_upd_state <= IDLE;
                    end
                    TYPE_U:begin
                        if (book_o_valid) begin // remove the old order
                            book_op          <= WRITE;
                            book_i_data      <= {ORDER_BOOK_WIDTH{1'b0}};
                            book_addr        <= cal_order_book_addr(msg_order_ref_num);
                            qty_bid_ask      <= book_o_side;
                            qty_price        <= book_o_price;
                            qty_is_add       <= 1'b0;
                            qty_d_shares     <= book_o_shares;
                            book_upd_state   <= THIRD_CYCLE;
                            qty_seq_num     <= msg_seq_num;
                        end else begin
                            book_op          <= IDLE;
                            book_i_data      <= {ORDER_BOOK_WIDTH{1'b0}};
                            book_addr        <= {BOOK_LEVEL_BIT{1'b0}};
                            book_upd_state  <= IDLE;
                        end                        
                    end
                    TYPE_E,TYPE_C,TYPE_X: begin
                        if (book_o_valid && (book_o_shares >= msg_shares)) begin
                            book_op          <= WRITE;
                            book_addr        <= cal_order_book_addr(msg_order_ref_num);
                            book_i_data      <= {(book_o_shares - msg_shares==0) ? 1'b0 : 1'b1, book_o_side, (book_o_shares - msg_shares), book_o_price};
                            qty_bid_ask     <= book_o_side;
                            qty_price       <= book_o_price;
                            qty_is_add      <= 1'b0;
                            qty_d_shares    <= msg_shares;
                            qty_seq_num     <= msg_seq_num;
                        end else begin
                            book_op          <= IDLE;
                            book_i_data      <= {ORDER_BOOK_WIDTH{1'b0}};
                            book_addr        <= {BOOK_LEVEL_BIT{1'b0}};
                            qty_bid_ask      <= IDLE;
                        end
                        book_upd_state <= IDLE;
                    end
                    default: begin
                        book_op          <= IDLE;
                        book_i_data      <= {ORDER_BOOK_WIDTH{1'b0}};
                        book_addr        <= {BOOK_LEVEL_BIT{1'b0}};
                        qty_bid_ask      <= IDLE;
                        book_upd_state  <= IDLE;
                    end
                endcase
            end
            THIRD_CYCLE: begin
                if (msg_type == TYPE_U) begin
                    book_op          <= WRITE;
                    book_i_data      <= {1'b1, book_o_side, msg_shares, msg_price}; // old side, new shares and new price
                    book_addr        <= cal_order_book_addr(msg_new_order_ref);
                    qty_bid_ask      <= book_o_side; // the old order side.
                    qty_price        <= msg_price;  // new price
                    qty_is_add       <= 1'b1;
                    qty_d_shares     <= msg_shares; // new shares
                    qty_seq_num      <= msg_seq_num; // new sequence number
                end else begin
                    book_op          <= IDLE;
                    book_i_data      <= {ORDER_BOOK_WIDTH{1'b0}};
                    book_addr        <= {BOOK_LEVEL_BIT{1'b0}};
                    qty_bid_ask      <= IDLE;
                end
                book_upd_state   <= IDLE;
            end
            default: begin
                book_upd_state <= IDLE;
                book_op        <= IDLE;
                book_i_data    <= {ORDER_BOOK_WIDTH{1'b0}};
                book_addr      <= {BOOK_LEVEL_BIT{1'b0}};
                qty_bid_ask   <= IDLE;
            end
        endcase
    end else begin
        book_op        <= IDLE;
        book_i_data    <= {ORDER_BOOK_WIDTH{1'b0}};
        book_addr      <= {BOOK_LEVEL_BIT{1'b0}};
        qty_bid_ask   <= IDLE;
    end
end

function [BOOK_LEVEL_BIT-1:0] cal_order_book_addr (input [63:0] order_ref_num);
    integer i;
    integer j;
begin
    cal_order_book_addr = 0;
    for (i = 0; i < BOOK_LEVEL_BIT; i = i + 1) begin
        for (j = 0; j < 64; j = j + BOOK_LEVEL_BIT) begin
            if (i + j < 64) begin
                cal_order_book_addr[i] = cal_order_book_addr[i] ^ order_ref_num[i+j];
            end
        end
    end
end
endfunction

endmodule
