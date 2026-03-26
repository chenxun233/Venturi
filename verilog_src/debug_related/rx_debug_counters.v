`timescale 1ps/1ps

module rx_debug_counters #(
    parameter SYMBOL_NUM = 2
) (
    input  wire                         i_clk_156,
    input  wire                         i_rst,
    input  wire                         i_pcs_frame_started,
    input  wire                         i_frame_started,
    input  wire                         i_msg_valid,
    input  wire [SYMBOL_NUM-1:0]        i_event_valid,
    output wire [63:0]                  o_pcs_frame_count_gray,
    output wire [63:0]                  o_frame_count_gray,
    output wire [63:0]                  o_msg_count_gray,
    output wire [SYMBOL_NUM*64-1:0]     o_event_count_gray
);
reg [63:0] pcs_frame_count;
reg [63:0] frame_count;
reg [63:0] msg_count;
reg [63:0] event_count [0:SYMBOL_NUM-1];

integer idx;
genvar event_idx;

assign o_pcs_frame_count_gray = (pcs_frame_count >> 1) ^ pcs_frame_count;
assign o_frame_count_gray = (frame_count >> 1) ^ frame_count;
assign o_msg_count_gray   = (msg_count >> 1) ^ msg_count;

generate
    for (event_idx = 0; event_idx < SYMBOL_NUM; event_idx = event_idx + 1) begin : g_event_count_gray
        assign o_event_count_gray[event_idx*64 +: 64] =
            (event_count[event_idx] >> 1) ^ event_count[event_idx];
    end
endgenerate

always @(posedge i_clk_156 or posedge i_rst) begin
    if (i_rst) begin
        pcs_frame_count <= 64'd0;
        frame_count <= 64'd0;
        msg_count   <= 64'd0;
        for (idx = 0; idx < SYMBOL_NUM; idx = idx + 1) begin
            event_count[idx] <= 64'd0;
        end
    end else begin
        if (i_pcs_frame_started) begin
            pcs_frame_count <= pcs_frame_count + 64'd1;
        end
        if (i_frame_started) begin
            frame_count <= frame_count + 64'd1;
        end
        if (i_msg_valid) begin
            msg_count <= msg_count + 64'd1;
        end
        for (idx = 0; idx < SYMBOL_NUM; idx = idx + 1) begin
            if (i_event_valid[idx]) begin
                event_count[idx] <= event_count[idx] + 64'd1;
            end
        end
    end
end

endmodule
