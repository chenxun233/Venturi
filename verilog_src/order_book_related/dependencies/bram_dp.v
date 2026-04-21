module bram_dp #(
    parameter ADDR_WIDTH = 12,
    parameter DATA_WIDTH = 66
) (
    input   wire                        i_clk,
    input   wire                        i_rst,
    input   wire [ADDR_WIDTH-1:0]       i_addr_a,
    input   wire [1:0]                  i_op_a,
    input   wire [DATA_WIDTH-1:0]       i_data_a,
    output  reg  [DATA_WIDTH-1:0]       o_data_a,

    input   wire [ADDR_WIDTH-1:0]       i_addr_b,
    input   wire [1:0]                  i_op_b,
    input   wire [DATA_WIDTH-1:0]       i_data_b,
    output  reg  [DATA_WIDTH-1:0]       o_data_b,
    output  reg                         o_init_done
);

localparam IDLE  = 2'b00,
           READ  = 2'b01,
           WRITE = 2'b10;

wire do_read_a  = (i_op_a == READ);
wire do_write_a = (i_op_a == WRITE);
wire do_read_b  = (i_op_b == READ);
wire do_write_b = (i_op_b == WRITE);

(* ram_style = "block" *) reg [DATA_WIDTH-1:0] bram [0:(1<<ADDR_WIDTH)-1];
integer init_i;
reg [ADDR_WIDTH-1:0] scrub_addr;

initial begin
    for (init_i = 0; init_i < (1<<ADDR_WIDTH); init_i = init_i + 1) begin
        bram[init_i] = {DATA_WIDTH{1'b0}};
    end
end

initial begin
    o_data_a = {DATA_WIDTH{1'b0}};
    o_data_b = {DATA_WIDTH{1'b0}};
    o_init_done = 1'b1;
    scrub_addr = {ADDR_WIDTH{1'b0}};
end

always @(posedge i_clk) begin
    if (i_rst) begin
        o_data_a <= {DATA_WIDTH{1'b0}};
        o_data_b <= {DATA_WIDTH{1'b0}};
        o_init_done <= 1'b0;
        scrub_addr <= {ADDR_WIDTH{1'b0}};
    end else if (!o_init_done) begin
        bram[scrub_addr] <= {DATA_WIDTH{1'b0}};
        o_data_a <= {DATA_WIDTH{1'b0}};
        o_data_b <= {DATA_WIDTH{1'b0}};
        if (scrub_addr == {ADDR_WIDTH{1'b1}}) begin
            o_init_done <= 1'b1;
        end else begin
            scrub_addr <= scrub_addr + {{(ADDR_WIDTH-1){1'b0}}, 1'b1};
        end
    end else begin
        if (do_write_a) begin
            bram[i_addr_a] <= i_data_a;
        end
        if (do_write_b) begin
            bram[i_addr_b] <= i_data_b;
        end
        if (do_read_a) begin
            o_data_a <= bram[i_addr_a];
        end else begin
            o_data_a <= {DATA_WIDTH{1'b0}};
        end
        if (do_read_b) begin
            o_data_b <= bram[i_addr_b];
        end else begin
            o_data_b <= {DATA_WIDTH{1'b0}};
        end
    end
end

endmodule
