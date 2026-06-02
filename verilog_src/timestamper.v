`timescale 1ns / 1ps

module timestamper #(
    parameter COUNTER_WIDTH = 48
)(
    input  wire                         i_clk,
    input  wire                         i_rst,      // active high
    input  wire                         i_event,    // one-cycle pulse at frame boundary
    output reg  [COUNTER_WIDTH-1:0]     o_mac_timestamp,
    output reg  [COUNTER_WIDTH-1:0]     o_dma_timestamp_gray
);
    reg [COUNTER_WIDTH-1:0] tick_counter;

    // Export the live counter in Gray code for minimum-latency CDC into the DMA
    // domain. The DMA side restores it to binary and latches locally there.
    wire [COUNTER_WIDTH-1:0] dma_timestamp_gray = ((tick_counter-1) >> 1) ^ (tick_counter-1);

    always @(posedge i_clk) begin
        if (i_rst) begin
            tick_counter            <= {COUNTER_WIDTH{1'b0}};
            o_mac_timestamp         <= {COUNTER_WIDTH{1'b0}};
            o_dma_timestamp_gray    <= {COUNTER_WIDTH{1'b0}};
        end else begin
            tick_counter            <= tick_counter + {{(COUNTER_WIDTH-1){1'b0}}, 1'b1};
            o_dma_timestamp_gray    <= dma_timestamp_gray;
            if (i_event) begin
                o_mac_timestamp <= tick_counter;
            end
        end
    end
endmodule
