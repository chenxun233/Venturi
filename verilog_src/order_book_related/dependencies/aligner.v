module aligner #(
    parameter QTY_SHARE_BIT = 32
) (
    input  wire                     i_clk_156,
    input  wire                     i_rst,
    input  wire                     i_best_valid,
    input  wire [31:0]              i_best_price,
    input  wire [QTY_SHARE_BIT-1:0] i_best_shares,
    output wire                     o_best_valid_aligned,
    output wire [31:0]              o_best_price_aligned,
    output reg  [QTY_SHARE_BIT-1:0] o_best_shares
);

reg        best_valid_d1;
reg        best_valid_aligned;
reg [31:0] best_price_d1;
reg [31:0] best_price_aligned;

assign o_best_valid_aligned = best_valid_aligned && (o_best_shares > 0);
assign o_best_price_aligned = o_best_valid_aligned ? best_price_aligned : 32'd0;

always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        best_valid_d1      <= 1'b0;
        best_valid_aligned <= 1'b0;
        best_price_d1      <= 32'd0;
        best_price_aligned <= 32'd0;
        o_best_shares      <= {QTY_SHARE_BIT{1'b0}};
    end else begin
        best_valid_aligned <= best_valid_d1;
        best_price_aligned <= best_price_d1;
        o_best_shares      <= i_best_shares;
        best_valid_d1      <= i_best_valid;
        best_price_d1      <= i_best_price;
    end
end

endmodule
