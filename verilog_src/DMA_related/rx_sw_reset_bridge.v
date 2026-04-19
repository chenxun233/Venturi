`timescale 1ps/1ps

module rx_sw_reset_bridge #(
    parameter HOLD_CYCLES = 1024,
    parameter COUNTER_W   = 12
) (
    input  wire i_user_clk,
    input  wire i_user_reset_p,
    input  wire i_reg_reset,
    input  wire i_rx_clk_156,
    output wire o_reset_250,
    output wire o_reset_156
);

reg [COUNTER_W-1:0] hold_ctr;
reg                 reset_250;
wire                reset_156_sync;

always @(posedge i_user_clk or posedge i_user_reset_p) begin
    if (i_user_reset_p) begin
        hold_ctr  <= HOLD_CYCLES[COUNTER_W-1:0];
        reset_250 <= 1'b1;
    end else if (i_reg_reset) begin
        hold_ctr  <= HOLD_CYCLES[COUNTER_W-1:0];
        reset_250 <= 1'b1;
    end else if (reset_250) begin
        if (hold_ctr == {COUNTER_W{1'b0}}) begin
            reset_250 <= 1'b0;
        end else begin
            hold_ctr <= hold_ctr - {{(COUNTER_W-1){1'b0}}, 1'b1};
        end
    end
end

rst_synchronizer #(
    .IN_ACTIVE_HIGH  (1),
    .OUT_ACTIVE_HIGH (1)
) rx_sw_reset_sync_inst (
    .i_clk   (i_rx_clk_156),
    .rst_in  (reset_250),
    .rst_out (reset_156_sync)
);

assign o_reset_250 = i_user_reset_p | reset_250;
assign o_reset_156 = reset_156_sync;

endmodule
