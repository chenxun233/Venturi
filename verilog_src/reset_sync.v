//===============================================================
// Synchronize reset deassertion into i_sync_clk domain.
// Supports all input/output reset polarity combinations via parameters.
//===============================================================

module reset_sync #(
    parameter   I_ACTIVE_HIGH = 0,
    parameter   O_ACTIVE_HIGH = 1
)(
    input  wire i_sync_clk,
    input  wire i_async_reset,
    output wire o_sync_reset
);

reg [1:0] sync_reg;
wire i_async_reset_int;

// Internal reset is always active-high asserted.
assign i_async_reset_int = I_ACTIVE_HIGH ? i_async_reset : ~i_async_reset;

always @(posedge i_sync_clk or posedge i_async_reset_int) begin
    if (i_async_reset_int) begin
        // Asynchronous assert
        sync_reg <= 2'b11;
    end else begin
        // Synchronous deassert
        sync_reg <= {sync_reg[0], 1'b0};
    end
end

assign o_sync_reset = O_ACTIVE_HIGH ? sync_reg[1] : ~sync_reg[1];

endmodule
