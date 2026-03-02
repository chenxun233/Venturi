module symbol_book_BRAM #(
    parameter ADDR_WIDTH = 12,
    parameter DATA_WIDTH = 66
) (
    input   wire                        i_clk,
    input   wire                        i_rst,
    input   wire [ADDR_WIDTH-1:0]       i_addr,
    input   wire [1:0]                  i_r_or_w,
    input   wire [DATA_WIDTH-1:0]       i_data,
    output  reg  [DATA_WIDTH-1:0]       o_data
);

localparam IDLE  = 2'b00,
           READ  = 2'b01,
           WRITE = 2'b10;

wire do_read  = (i_r_or_w == READ);
wire do_write = (i_r_or_w == WRITE);

(* ram_style = "block" *) reg [DATA_WIDTH-1:0] bram [0:(1<<ADDR_WIDTH)-1];

always @(posedge i_clk) begin
    if (do_write) begin
        bram[i_addr] <= i_data;
    end

    if (i_rst) begin
        o_data <= {DATA_WIDTH{1'b0}};
    end else if (do_read) begin
        o_data <= bram[i_addr];
    end
end

endmodule
