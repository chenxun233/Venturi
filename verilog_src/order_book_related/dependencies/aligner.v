module aligner #(
) (
    input  wire                     i_clk_156,
    input  wire                     i_rst,
    input  wire [31:0]              i_best_price,
    input  wire [31:0]              i_best_shares,
    input wire                      i_t_op_done,
    input wire [47:0]               i_timestamp,
    output reg [31:0]               o_best_price_aligned,
    output reg  [31:0]              o_best_shares,
    output reg                      o_op_done_aligned,
    output wire [47:0]              o_timestamp_aligned
);

reg [31:0] best_price_d1;

reg op_done_d1;
reg op_done_d2;

reg [47:0] timestamp_d1;
reg [47:0] timestamp_d2;
reg [47:0] timestamp_aligned;

assign o_timestamp_aligned = o_op_done_aligned ? timestamp_aligned : {48{1'b0}};
always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        best_price_d1      <= 32'd0;
        o_best_price_aligned <= 32'd0;
        o_best_shares      <= {32{1'b0}};
        op_done_d1          <= 1'b0;
        op_done_d2          <= 1'b0;
        timestamp_d1        <= {48{1'b0}};
        timestamp_d2        <= {48{1'b0}};
        o_op_done_aligned   <= 1'b0;
        timestamp_aligned   <= {48{1'b0}};
    end else begin
        op_done_d1          <= i_t_op_done;
        op_done_d2          <= op_done_d1;
        o_op_done_aligned   <= op_done_d2;
        timestamp_d1        <= i_timestamp;
        timestamp_d2        <= timestamp_d1;
        timestamp_aligned  <= timestamp_d2;
        best_price_d1      <= i_best_price;
        o_best_price_aligned <= best_price_d1;
        o_best_shares      <= i_best_shares;
    end
end


endmodule
