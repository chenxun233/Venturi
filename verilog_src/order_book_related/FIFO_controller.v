module FIFO_controller #(
    parameter STOCK_LOCATE = 16'h000d
)
(
    // parser side
    
    input   wire            i_msg_valid     ,
    input   wire [15:0]     i_stock_locate  ,
    // book side
    input  wire             i_not_empty     ,
    input  wire             i_state         ,
    // FIFO side
    output  wire            o_do_push       ,
    output  wire            o_do_pop
);
    
assign o_do_push = i_msg_valid && (i_stock_locate == STOCK_LOCATE); 
assign o_do_pop  = i_not_empty && (i_state == 2'b00); 


endmodule