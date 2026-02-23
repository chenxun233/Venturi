`timescale 1ns / 1ps

module frame_timestamp #(
    parameter COUNTER_WIDTH = 64
)(
    input  wire                         i_clk,
    input  wire                         i_rst,      // active high
    input  wire                         i_event,    // one-cycle pulse at frame boundary
    output reg  [COUNTER_WIDTH-1:0]     o_event_timestamp
);
    reg [COUNTER_WIDTH-1:0] tick_counter;

    always @(posedge i_clk) begin
        if (i_rst) begin
            tick_counter       <= {COUNTER_WIDTH{1'b0}};
            o_event_timestamp  <= {COUNTER_WIDTH{1'b0}};
        end else begin
            tick_counter <= tick_counter + {{(COUNTER_WIDTH-1){1'b0}}, 1'b1};
            if (i_event) begin
                o_event_timestamp <= tick_counter;
            end
        end
    end
endmodule
