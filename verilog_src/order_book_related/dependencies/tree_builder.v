module tree_builder #(
    parameter QTY_PRICE_LVL_BIT = 10,   // LEVELS = QTY_PRICE_LVL_BIT, leaves = 2^LEVELS
    parameter BID_OR_ASK = 2'b01             // 01 for bid tree, 10 for ask tree
)(
    input  wire                         i_clk,
    input  wire                         i_rst,        // active high
    input  wire [QTY_PRICE_LVL_BIT-1:0] i_price_idx,
    input  wire [1:0]                   i_price_change,
    input  wire                         i_op_done,
    output wire [QTY_PRICE_LVL_BIT-1:0] o_best_price_idx,
    output wire                         o_op_done
);
    localparam IDLE       = 2'b00;
    localparam EMPTY      = 2'b01;
    localparam NON_EMPTY  = 2'b10;
    localparam LEVELS     = QTY_PRICE_LVL_BIT;
    localparam LAST_COUNT = (1 << LEVELS);
    localparam MID_LEVEL  = (LEVELS >> 1);
    localparam MID_COUNT  = (1 << MID_LEVEL);
    localparam FIFO_DATA_W      = QTY_PRICE_LVL_BIT + 2; // bits for price index and change type
    localparam UPPER_MAX_COUNT  = LAST_COUNT >> 1;
    localparam LOWER_LEVELS     = (MID_LEVEL > 0) ? MID_LEVEL : 1;
    localparam LOWER_MAX_COUNT  = (MID_COUNT  > 1) ? (MID_COUNT >> 1) : 1;


    reg                          btm_valid [0:LAST_COUNT-1];
    reg [QTY_PRICE_LVL_BIT-1:0]  mid_idx    [0:MID_COUNT-1];
    reg                          mid_valid  [0:MID_COUNT-1];

    wire [QTY_PRICE_LVL_BIT-1:0] mid_to_btm_idx   [MID_LEVEL:LEVELS-1][0:UPPER_MAX_COUNT-1];
    wire                         mid_to_btm_valid [MID_LEVEL:LEVELS-1][0:UPPER_MAX_COUNT-1];
    wire [QTY_PRICE_LVL_BIT-1:0] top_to_mid_idx    [0:LOWER_LEVELS-1][0:LOWER_MAX_COUNT-1];
    wire                         top_to_mid_valid  [0:LOWER_LEVELS-1][0:LOWER_MAX_COUNT-1];

    integer idx;
    reg     latch_op_done;

    // The leaf level stores only valid bits. The index is implied by the array subscript.
    // mid_* is the pipeline register stage at MID_LEVEL.
    always @(posedge i_clk or posedge i_rst) begin
        if (i_rst) begin
            latch_op_done <= 1'b0;
            for (idx = 0; idx < LAST_COUNT; idx = idx + 1) begin
                btm_valid[idx] = 1'b0;
            end
            for (idx = 0; idx < MID_COUNT; idx = idx + 1) begin
                mid_valid[idx] = 1'b0;
                mid_idx[idx]   = {QTY_PRICE_LVL_BIT{1'b0}};
            end
        end else begin
            if (i_price_change != IDLE) begin
                btm_valid[i_price_idx] <= (i_price_change == NON_EMPTY);
                latch_op_done <= i_op_done;
            end

            for (idx = 0; idx < MID_COUNT; idx = idx + 1) begin
                mid_valid[idx] <= mid_to_btm_valid[MID_LEVEL][idx];
                mid_idx[idx]   <= mid_to_btm_idx[MID_LEVEL][idx];
            end
        end
    end

    genvar level;
    genvar node;
    generate
        // Reduce the leaf valid bits up to MID_LEVEL combinationally.
        for (level = MID_LEVEL; level < LEVELS; level = level + 1) begin : gen_btm_to_mid_levels
                localparam integer NODE_COUNT = (1 << level);
            for (node = 0; node < NODE_COUNT; node = node + 1) begin : gen_btm_to_mid_nodes
                localparam integer LEFT_CHILD  = (node << 1);
                localparam integer RIGHT_CHILD = LEFT_CHILD + 1;
                localparam [QTY_PRICE_LVL_BIT-1:0] LEFT_LEAF_IDX  = LEFT_CHILD[QTY_PRICE_LVL_BIT-1:0];
                localparam [QTY_PRICE_LVL_BIT-1:0] RIGHT_LEAF_IDX = RIGHT_CHILD[QTY_PRICE_LVL_BIT-1:0];

                if (level == LEVELS - 1) begin : gen_from_leaf
                    assign mid_to_btm_valid[level][node] =
                        btm_valid[LEFT_CHILD] | btm_valid[RIGHT_CHILD];
                    if (BID_OR_ASK == 2'b01) begin
                        // For bid tree, if both children are valid, take the right one (higher price).
                        assign mid_to_btm_idx[level][node] =
                            btm_valid[RIGHT_CHILD] ? RIGHT_LEAF_IDX :
                            btm_valid[LEFT_CHILD]  ? LEFT_LEAF_IDX  :
                                                            {QTY_PRICE_LVL_BIT{1'b0}};
                    end else if (BID_OR_ASK == 2'b10) begin
                        // For ask tree, if both children are valid, take the left one (lower price).
                        assign mid_to_btm_idx[level][node] =
                            btm_valid[LEFT_CHILD]  ? LEFT_LEAF_IDX  :
                            btm_valid[RIGHT_CHILD] ? RIGHT_LEAF_IDX :
                                                            {QTY_PRICE_LVL_BIT{1'b0}};
                    end
                end else begin : gen_from_mid_to_btm_level
                    assign mid_to_btm_valid[level][node] =
                        mid_to_btm_valid[level+1][LEFT_CHILD] | mid_to_btm_valid[level+1][RIGHT_CHILD];
                    if (BID_OR_ASK == 2'b01) begin
                        // For bid tree, if both children are valid, take the right one (higher price)
                        assign mid_to_btm_idx[level][node] =
                        mid_to_btm_valid[level+1][RIGHT_CHILD] ? mid_to_btm_idx[level+1][RIGHT_CHILD] :
                        mid_to_btm_valid[level+1][LEFT_CHILD]  ? mid_to_btm_idx[level+1][LEFT_CHILD]  :
                                                                  {QTY_PRICE_LVL_BIT{1'b0}};
                    end else if (BID_OR_ASK == 2'b10) begin
                        // For ask tree, if both children are valid, take the left one (lower price).
                    assign mid_to_btm_idx[level][node] =
                        mid_to_btm_valid[level+1][LEFT_CHILD]  ? mid_to_btm_idx[level+1][LEFT_CHILD]  :
                        mid_to_btm_valid[level+1][RIGHT_CHILD] ? mid_to_btm_idx[level+1][RIGHT_CHILD] :
                                                                  {QTY_PRICE_LVL_BIT{1'b0}};
                    end
                end
            end
        end

        // Use the registered MID_LEVEL results as the source for the upper half of the tree.
        for (level = 0; level < MID_LEVEL; level = level + 1) begin : gen_mid_to_top_levels
            localparam integer NODE_COUNT = (1 << level);

            for (node = 0; node < NODE_COUNT; node = node + 1) begin : gen_mid_to_top_nodes
                localparam integer LEFT_CHILD  = (node << 1);
                localparam integer RIGHT_CHILD = LEFT_CHILD + 1;

                if (level == MID_LEVEL - 1) begin : gen_from_mid_regs
                    assign top_to_mid_valid[level][node] =
                        mid_valid[LEFT_CHILD] | mid_valid[RIGHT_CHILD];
                    if (BID_OR_ASK == 2'b01) begin
                        // For bid tree, if both children are valid, take the right one (higher price
                        assign top_to_mid_idx[level][node] =
                            mid_valid[RIGHT_CHILD] ? mid_idx[RIGHT_CHILD] :
                            mid_valid[LEFT_CHILD]  ? mid_idx[LEFT_CHILD]  :
                                                        {QTY_PRICE_LVL_BIT{1'b0}};
                    end else if (BID_OR_ASK == 2'b10) begin
                        assign top_to_mid_idx[level][node] =
                            mid_valid[LEFT_CHILD]  ? mid_idx[LEFT_CHILD]  :
                            mid_valid[RIGHT_CHILD] ? mid_idx[RIGHT_CHILD] :
                                                        {QTY_PRICE_LVL_BIT{1'b0}};
                    end
                end else begin : gen_from_top_to_midlevel
                    assign top_to_mid_valid[level][node] =
                        top_to_mid_valid[level+1][LEFT_CHILD] | top_to_mid_valid[level+1][RIGHT_CHILD];
                    if (BID_OR_ASK == 2'b01) begin
                        assign top_to_mid_idx[level][node] =
                            top_to_mid_valid[level+1][RIGHT_CHILD] ? top_to_mid_idx[level+1][RIGHT_CHILD] :
                            top_to_mid_valid[level+1][LEFT_CHILD]  ? top_to_mid_idx[level+1][LEFT_CHILD]  :
                                                                    {QTY_PRICE_LVL_BIT{1'b0}};
                    end else if (BID_OR_ASK == 2'b10) begin
                        assign top_to_mid_idx[level][node] =
                            top_to_mid_valid[level+1][LEFT_CHILD]  ? top_to_mid_idx[level+1][LEFT_CHILD]  :
                            top_to_mid_valid[level+1][RIGHT_CHILD] ? top_to_mid_idx[level+1][RIGHT_CHILD] :
                                                                        {QTY_PRICE_LVL_BIT{1'b0}};
                    end
                end
            end
        end

        if (MID_LEVEL == 0) begin : gen_top_from_mid
            assign o_best_price_idx   = mid_idx[0];
            assign o_op_done = latch_op_done;
        end else begin : gen_top_from_lower
            assign o_best_price_idx   = top_to_mid_idx[0][0];
            assign o_op_done = latch_op_done;
        end
    endgenerate




endmodule
