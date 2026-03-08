module bid_tree_builder #(
    parameter QTY_PRICE_LVL_BIT = 10   // LEVELS = QTY_PRICE_LVL_BIT, leaves = 2^LEVELS
)(
    input  wire                         i_clk,
    input  wire                         i_rst,        // active high
    input  wire [QTY_PRICE_LVL_BIT-1:0] i_bid_price_idx,
    input  wire [1:0]                   i_bid_price_change,

    output wire                         o_bid_best_valid,
    output wire [QTY_PRICE_LVL_BIT-1:0] o_bid_best_idx
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


    reg                          last_bid_t_valid [0:LAST_COUNT-1];
    reg [QTY_PRICE_LVL_BIT-1:0]  mid_bid_t_idx    [0:MID_COUNT-1];
    reg                          mid_bid_t_valid  [0:MID_COUNT-1];

    wire [QTY_PRICE_LVL_BIT-1:0] mid_to_last_bid_t_idx   [MID_LEVEL:LEVELS-1][0:UPPER_MAX_COUNT-1];
    wire                         mid_to_last_bid_t_valid [MID_LEVEL:LEVELS-1][0:UPPER_MAX_COUNT-1];
    wire [QTY_PRICE_LVL_BIT-1:0] root_to_midbid_t_idx    [0:LOWER_LEVELS-1][0:LOWER_MAX_COUNT-1];
    wire                         root_to_midbid_t_valid  [0:LOWER_LEVELS-1][0:LOWER_MAX_COUNT-1];

    integer idx;


    // The leaf level stores only valid bits. The index is implied by the array subscript.
    // mid_bid_t_* is the pipeline register stage at MID_LEVEL.
    always @(posedge i_clk or posedge i_rst) begin
        if (i_rst) begin
            for (idx = 0; idx < LAST_COUNT; idx = idx + 1) begin
                last_bid_t_valid[idx] <= 1'b0;
            end
            for (idx = 0; idx < MID_COUNT; idx = idx + 1) begin
                mid_bid_t_valid[idx] <= 1'b0;
                mid_bid_t_idx[idx]   <= {QTY_PRICE_LVL_BIT{1'b0}};
            end
        end else begin
            if (i_bid_price_change != IDLE) begin
                last_bid_t_valid[i_bid_price_idx] <= (i_bid_price_change == NON_EMPTY);
            end

            for (idx = 0; idx < MID_COUNT; idx = idx + 1) begin
                mid_bid_t_valid[idx] <= mid_to_last_bid_t_valid[MID_LEVEL][idx];
                mid_bid_t_idx[idx]   <= mid_to_last_bid_t_idx[MID_LEVEL][idx];
            end
        end
    end

    genvar level;
    genvar node;
    generate
        // Reduce the leaf valid bits up to MID_LEVEL combinationally.
        for (level = MID_LEVEL; level < LEVELS; level = level + 1) begin : gen_last_to_mid_levels
            localparam integer NODE_COUNT = (1 << level);
            for (node = 0; node < NODE_COUNT; node = node + 1) begin : gen_last_to_mid_nodes
                localparam integer LEFT_CHILD  = (node << 1);
                localparam integer RIGHT_CHILD = LEFT_CHILD + 1;
                localparam [QTY_PRICE_LVL_BIT-1:0] LEFT_LEAF_IDX  = LEFT_CHILD;
                localparam [QTY_PRICE_LVL_BIT-1:0] RIGHT_LEAF_IDX = RIGHT_CHILD;

                if (level == LEVELS - 1) begin : gen_from_leaf
                    assign mid_to_last_bid_t_valid[level][node] =
                        last_bid_t_valid[LEFT_CHILD] | last_bid_t_valid[RIGHT_CHILD];

                    assign mid_to_last_bid_t_idx[level][node] =
                        last_bid_t_valid[RIGHT_CHILD] ? RIGHT_LEAF_IDX :
                        last_bid_t_valid[LEFT_CHILD]  ? LEFT_LEAF_IDX  :
                                                        {QTY_PRICE_LVL_BIT{1'b0}};
                end else begin : gen_from_mid_to_last_level
                    assign mid_to_last_bid_t_valid[level][node] =
                        mid_to_last_bid_t_valid[level+1][LEFT_CHILD] | mid_to_last_bid_t_valid[level+1][RIGHT_CHILD];

                    assign mid_to_last_bid_t_idx[level][node] =
                        mid_to_last_bid_t_valid[level+1][RIGHT_CHILD] ? mid_to_last_bid_t_idx[level+1][RIGHT_CHILD] :
                        mid_to_last_bid_t_valid[level+1][LEFT_CHILD]  ? mid_to_last_bid_t_idx[level+1][LEFT_CHILD]  :
                                                                  {QTY_PRICE_LVL_BIT{1'b0}};
                end
            end
        end

        // Use the registered MID_LEVEL results as the source for the upper half of the tree.
        for (level = 0; level < MID_LEVEL; level = level + 1) begin : gen_mid_to_root_levels
            localparam integer NODE_COUNT = (1 << level);

            for (node = 0; node < NODE_COUNT; node = node + 1) begin : gen_mid_to_root_nodes
                localparam integer LEFT_CHILD  = (node << 1);
                localparam integer RIGHT_CHILD = LEFT_CHILD + 1;

                if (level == MID_LEVEL - 1) begin : gen_from_mid_regs
                    assign root_to_midbid_t_valid[level][node] =
                        mid_bid_t_valid[LEFT_CHILD] | mid_bid_t_valid[RIGHT_CHILD];

                    assign root_to_midbid_t_idx[level][node] =
                        mid_bid_t_valid[RIGHT_CHILD] ? mid_bid_t_idx[RIGHT_CHILD] :
                        mid_bid_t_valid[LEFT_CHILD]  ? mid_bid_t_idx[LEFT_CHILD]  :
                                                       {QTY_PRICE_LVL_BIT{1'b0}};
                end else begin : gen_from_root_to_midlevel
                    assign root_to_midbid_t_valid[level][node] =
                        root_to_midbid_t_valid[level+1][LEFT_CHILD] | root_to_midbid_t_valid[level+1][RIGHT_CHILD];

                    assign root_to_midbid_t_idx[level][node] =
                        root_to_midbid_t_valid[level+1][RIGHT_CHILD] ? root_to_midbid_t_idx[level+1][RIGHT_CHILD] :
                        root_to_midbid_t_valid[level+1][LEFT_CHILD]  ? root_to_midbid_t_idx[level+1][LEFT_CHILD]  :
                                                                  {QTY_PRICE_LVL_BIT{1'b0}};
                end
            end
        end

        if (MID_LEVEL == 0) begin : gen_root_from_mid
            assign o_bid_best_valid = mid_bid_t_valid[0];
            assign o_bid_best_idx   = mid_bid_t_idx[0];
        end else begin : gen_root_from_lower
            assign o_bid_best_valid = root_to_midbid_t_valid[0][0];
            assign o_bid_best_idx   = root_to_midbid_t_idx[0][0];
        end
    endgenerate




endmodule
