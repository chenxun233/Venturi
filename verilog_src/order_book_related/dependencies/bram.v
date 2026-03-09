module bram #(
    parameter ADDR_WIDTH = 12,
    parameter DATA_WIDTH = 66
) (
    input   wire                        i_clk,
    input   wire                        i_rst,
    input   wire [ADDR_WIDTH-1:0]       i_addr,
    input   wire [1:0]                  i_op,
    input   wire [DATA_WIDTH-1:0]       i_data,
    output  reg  [DATA_WIDTH-1:0]       o_data
);

localparam IDLE  = 2'b00,
           READ  = 2'b01,
           WRITE = 2'b10;

wire do_read  = (i_op == READ);
wire do_write = (i_op == WRITE);

(* ram_style = "block" *) reg [DATA_WIDTH-1:0] bram [0:(1<<ADDR_WIDTH)-1];
integer init_i;

// Power-up initialization for simulation and FPGA BRAM init.
initial begin
    for (init_i = 0; init_i < (1<<ADDR_WIDTH); init_i = init_i + 1) begin
        bram[init_i] = {DATA_WIDTH{1'b0}};
    end
end

initial begin
    o_data = {DATA_WIDTH{1'b0}};
end

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
