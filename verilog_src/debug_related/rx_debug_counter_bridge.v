`timescale 1ps/1ps

module rx_debug_counter_bridge #(
    parameter SYMBOL_NUM = 2
) (
    input  wire                         i_user_clk,
    input  wire [63:0]                  i_pcs_frame_count_gray,
    input  wire [63:0]                  i_frame_count_gray,
    input  wire [63:0]                  i_msg_count_gray,
    input  wire [SYMBOL_NUM*64-1:0]     i_event_count_gray,
    output wire [63:0]                  o_pcs_frame_count,
    output wire [63:0]                  o_frame_count,
    output wire [63:0]                  o_msg_count,
    output wire [SYMBOL_NUM*64-1:0]     o_event_count
);

wire [63:0] pcs_frame_count_gray_sync;
wire [63:0] frame_count_gray_sync;
wire [63:0] msg_count_gray_sync;
wire [SYMBOL_NUM*64-1:0] event_count_gray_sync;

genvar idx;

bit_synchronizer #(
    .BIT_WIDTH (64)
) pcs_frame_count_sync_inst (
    .i_clk  (i_user_clk),
    .i_in   (i_pcs_frame_count_gray),
    .o_out  (pcs_frame_count_gray_sync)
);

gray_to_binary #(
    .WIDTH (64)
) pcs_frame_count_decode_inst (
    .i_gray   (pcs_frame_count_gray_sync),
    .o_binary (o_pcs_frame_count)
);

bit_synchronizer #(
    .BIT_WIDTH (64)
) frame_count_sync_inst (
    .i_clk  (i_user_clk),
    .i_in   (i_frame_count_gray),
    .o_out  (frame_count_gray_sync)
);

gray_to_binary #(
    .WIDTH (64)
) frame_count_decode_inst (
    .i_gray   (frame_count_gray_sync),
    .o_binary (o_frame_count)
);

bit_synchronizer #(
    .BIT_WIDTH (64)
) msg_count_sync_inst (
    .i_clk  (i_user_clk),
    .i_in   (i_msg_count_gray),
    .o_out  (msg_count_gray_sync)
);

gray_to_binary #(
    .WIDTH (64)
) msg_count_decode_inst (
    .i_gray   (msg_count_gray_sync),
    .o_binary (o_msg_count)
);

generate
    for (idx = 0; idx < SYMBOL_NUM; idx = idx + 1) begin : g_event_count_bridge
        bit_synchronizer #(
            .BIT_WIDTH (64)
        ) event_count_sync_inst (
            .i_clk  (i_user_clk),
            .i_in   (i_event_count_gray[idx*64 +: 64]),
            .o_out  (event_count_gray_sync[idx*64 +: 64])
        );

        gray_to_binary #(
            .WIDTH (64)
        ) event_count_decode_inst (
            .i_gray   (event_count_gray_sync[idx*64 +: 64]),
            .o_binary (o_event_count[idx*64 +: 64])
        );
    end
endgenerate

endmodule
