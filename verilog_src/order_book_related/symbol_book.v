// order book per symbol.
module symbol_book #(
    parameter PRICE_ADDR_WIDTH = 10,    // trace 2^PRICE_ADDR_WIDTH price levels
    parameter BOOK_ADDR_WIDTH  = 12,     // trace 2^BOOK_ADDR_WIDTH orders in the order table
    parameter LOWEST_PRICE     = 32'd100_0000, // default lowest price (in 1/10000 dollars) for all stocks, can be updated by control plane.
    parameter STOCK_LOCATE     = 16'h000d      // stock locate for this symbol book, can be updated by control plane.
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

localparam                  DATA_WIDTH                  = 8+64+64+1+32+32;      // {msg_type, order_ref_num, new_order_ref_num, side, price, shares}
localparam                  PRICE_DEPTH                 = 1<<PRICE_ADDR_WIDTH;  // total number of price levels to track across all symbols
localparam                  BOOK_DEPTH                  = 1<<BOOK_ADDR_WIDTH;   // total number of orders to track in the order table, 2^BOOK_ADDR_WIDTH
localparam                  ORDER_BOOK_WIDTH            = 66;                   // {valid, side,shares, price } 1+1+32+32 = 66 bits

localparam                  TYPE_A                      = 8'h41;
localparam                  TYPE_X                      = 8'h58;
localparam                  TYPE_D                      = 8'h44;
localparam                  TYPE_U                      = 8'h55;
localparam                  TYPE_E                      = 8'h45;
localparam                  TYPE_F                      = 8'h46;
localparam                  TYPE_C                      = 8'h43;

// === FIFO. save it in FIFO first, in case order book updating may need multiple cycles.
wire [DATA_WIDTH-1:0]       fifo_i_data                 = {i_msg_type, i_order_ref_num, i_new_order_ref_num, i_side, i_shares, i_price};
wire [DATA_WIDTH-1:0]       fifo_o_data;
wire [7:0]                  fifo_o_msg_type             = fifo_o_data[DATA_WIDTH-1:DATA_WIDTH-8]      ;
wire [63:0]                 fifo_o_order_ref_num        = fifo_o_data[DATA_WIDTH-9:DATA_WIDTH-72]     ;
wire [63:0]                 fifo_o_new_order_ref_num    = fifo_o_data[DATA_WIDTH-73:DATA_WIDTH-136]   ;
wire                        fifo_o_side                 = fifo_o_data[DATA_WIDTH-137]                 ;
wire [31:0]                 fifo_o_shares               = fifo_o_data[DATA_WIDTH-138:DATA_WIDTH-169]  ;
wire [31:0]                 fifo_o_price                = fifo_o_data[DATA_WIDTH-170:DATA_WIDTH-201]  ;
reg  [BOOK_ADDR_WIDTH-1:0]  addr;            // address for order table BRAM read/write 
wire                        fifo_pop_valid;
wire                        do_push;                     
wire                        do_pop;                      
reg [ORDER_BOOK_WIDTH-1:0]  wdata; 
wire[ORDER_BOOK_WIDTH-1:0]  rdata;
          
wire                        rvalid            = rdata[ORDER_BOOK_WIDTH-1];
wire                        rside             = rdata[ORDER_BOOK_WIDTH-2];
wire [31:0]                 rshares           = rdata[63:32];
wire [31:0]                 rprice            = rdata[31:0];

wire                        wvalid            = wdata[ORDER_BOOK_WIDTH-1];
wire                        wside             = wdata[ORDER_BOOK_WIDTH-2];
wire [31:0]                 wshares           = wdata[63:32];
wire [31:0]                 wprice            = wdata[31:0];

reg  [1:0]                  state_order_book;
localparam                  IDLE                        = 2'b00 ;
localparam                  FIRST_CYCLE                 = 2'b01 ;
localparam                  SECOND_CYCLE                = 2'b10 ;

reg [1:0]                   read_or_write; // for debugging
localparam                  READ                        = 2'b01 ;
localparam                  WRITE                       = 2'b10 ;




wire                        not_empty; 

symbol_book_FIFO #(
    .DEPTH          (8              ),
    .DATA_W         (DATA_WIDTH     ) // {msg_type, order_ref_num, new_order_ref_num, side, shares, price}
) symbol_book_fifo (
    .i_clk          (i_clk_156      ),
    .i_rst          (i_rst          ),
    .i_do_push      (do_push        ), 
    .i_push_data    (fifo_i_data    ),  
    .o_push_ready   (               ),
    .i_do_pop       (do_pop         ),
    .o_not_empty    (not_empty      ),
    .o_pop_valid    (fifo_pop_valid ),
    .o_pop_data     (fifo_o_data    ) // remain the same if no new pop or pop invalid
);
// === data from/to fifo ===


FIFO_controller #(
    .STOCK_LOCATE    (STOCK_LOCATE   )
) fifo_controller (
    .i_msg_valid     (i_msg_valid        ),
    .i_stock_locate  (i_stock_locate     ),
    .i_not_empty     (not_empty          ),
    .i_state         (state_order_book   ),
    .o_do_push       (do_push            ),
    .o_do_pop        (do_pop             ) // only fire do_pop when not_empty and in IDLE state
);

symbol_book_BRAM #(
    .ADDR_WIDTH      (BOOK_ADDR_WIDTH    ),
    .DATA_WIDTH      (ORDER_BOOK_WIDTH   )
) order_tbl_bram (
    .i_clk           (i_clk_156          ),
    .i_rst           (i_rst              ),
    .i_addr          (addr               ),
    .i_r_or_w        (read_or_write      ),
    .i_data          (wdata              ),
    .o_data          (rdata              ) // remain the same if addr is does not change or no new read signal
);


// === price level quantity table ===
(* ram_style = "block" *) reg  [31:0]  bid_qty  [0:PRICE_DEPTH-1]   ;
(* ram_style = "block" *) reg  [31:0]  ask_qty  [0:PRICE_DEPTH-1]   ;






always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        wdata               <= {ORDER_BOOK_WIDTH{1'b0}};
        state_order_book    <= IDLE;
        read_or_write       <= IDLE;
        addr                <= {BOOK_ADDR_WIDTH{1'b0}};
    end else if (fifo_pop_valid || (state_order_book != IDLE)) begin
        case (state_order_book)
            IDLE: begin
                case (fifo_o_msg_type)
                TYPE_A,TYPE_F: begin // add order
                    read_or_write       <= WRITE;
                    wdata               <= {1'b1, fifo_o_side, fifo_o_shares, fifo_o_price};
                    addr                <= cal_order_book_addr(fifo_o_order_ref_num);
                    state_order_book    <= IDLE;
                end
                TYPE_D: begin // delete order
                    read_or_write      <= WRITE;
                    wdata              <= {ORDER_BOOK_WIDTH{1'b0}};
                    addr               <= cal_order_book_addr(fifo_o_order_ref_num);
                    state_order_book   <= IDLE;
                end
                TYPE_E,TYPE_C,TYPE_X: begin // first, read data
                    read_or_write      <= READ;
                    addr               <= cal_order_book_addr(fifo_o_order_ref_num);
                    state_order_book   <= FIRST_CYCLE;
                end
                TYPE_U: begin   // first, clean old order
                    read_or_write      <= WRITE;
                    wdata              <= {ORDER_BOOK_WIDTH{1'b0}};
                    addr               <= cal_order_book_addr(fifo_o_order_ref_num);
                    state_order_book   <= FIRST_CYCLE;
                end
                default: begin
                    read_or_write    <= IDLE;
                    wdata            <= {ORDER_BOOK_WIDTH{1'b0}};
                    addr             <= {BOOK_ADDR_WIDTH{1'b0}};
                    state_order_book <= IDLE;
                end
                endcase
            end
            FIRST_CYCLE: begin
                case (fifo_o_msg_type)
                TYPE_U: begin // second cycle for U: insert new order
                    read_or_write      <= WRITE;
                    wdata              <= {1'b1, fifo_o_side, fifo_o_shares, fifo_o_price};
                    addr               <= cal_order_book_addr(fifo_o_new_order_ref_num);
                    state_order_book   <= IDLE;
                end
                TYPE_E,TYPE_C,TYPE_X: begin 
                    state_order_book   <= SECOND_CYCLE; // wait for the data to be read
                end
                default: begin
                    read_or_write    <= IDLE;
                    wdata            <= {ORDER_BOOK_WIDTH{1'b0}};
                    addr             <= {BOOK_ADDR_WIDTH{1'b0}};
                    state_order_book <= IDLE;
                end
                endcase
            end
            SECOND_CYCLE: begin
                case (fifo_o_msg_type)
                    TYPE_E,TYPE_C,TYPE_X: begin // second , write new shares back
                        read_or_write          <= WRITE;
                        addr                   <= cal_order_book_addr(fifo_o_order_ref_num);
                        if (!rvalid || (fifo_o_shares >= rshares)) begin
                            wdata <= {ORDER_BOOK_WIDTH{1'b0}};
                        end else begin
                            wdata <= {1'b1, rside, (rshares - fifo_o_shares), rprice};
                        end
                        state_order_book       <= IDLE;
                    end
                    default: begin
                        read_or_write    <= IDLE;
                        wdata            <= {ORDER_BOOK_WIDTH{1'b0}};
                        addr             <= {BOOK_ADDR_WIDTH{1'b0}};
                        state_order_book <= IDLE;
                    end
                endcase
            end
        endcase
    end else begin
        read_or_write    <= IDLE;
        wdata            <= {ORDER_BOOK_WIDTH{1'b0}};
        addr             <= {BOOK_ADDR_WIDTH{1'b0}};
        state_order_book <= state_order_book;
    end
end

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

     







// for debugging
reg                         do_push_d;
reg [7:0]                   fifo_o_msg_type_d;
reg                         hold_on_d;
reg [1:0]                   state_order_book_d;
reg [1:0]                   read_or_write_d;
reg [7:0]                   rshares_d;
reg [7:0]                   wshares_d;
reg [7:0]                   fifo_o_shares_d;
reg [BOOK_ADDR_WIDTH-1:0]   order_tbl_addr_d;
reg                         i_msg_valid_d;
reg                         do_pop_d;
reg                         fifo_pop_valid_d;
reg [7:0]                   i_msg_type_d;   


// Registered bridge stage for stable ILA probing.
always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        do_push_d           <= 1'b0;
        fifo_o_msg_type_d   <= 8'd0;
        wshares_d <= 8'd0;
        read_or_write_d     <= 2'd0;
        order_tbl_addr_d    <= {BOOK_ADDR_WIDTH{1'b0}};
        do_pop_d            <= 1'b0;
        fifo_pop_valid_d    <= 1'b0;

    end else begin
        do_push_d           <= do_push;
        fifo_o_msg_type_d   <= fifo_o_msg_type;
        wshares_d           <= wdata[39:32];
        read_or_write_d     <= read_or_write;
        order_tbl_addr_d    <= addr;  
        do_pop_d            <= do_pop;
        fifo_pop_valid_d    <= fifo_pop_valid;
    end
end




ila_order_book_0 order_book_ila_inst (
    .clk    (i_clk_156           ),
    .probe0 (do_push_d          ),
    .probe1 (fifo_o_msg_type_d  ),
    .probe2 (wshares_d          ),
    .probe3 (read_or_write_d    ),
    .probe4 (order_tbl_addr_d[3:0]   ),
    .probe5 (do_pop_d           ),
    .probe6 (fifo_pop_valid_d   )
);


    
endmodule
