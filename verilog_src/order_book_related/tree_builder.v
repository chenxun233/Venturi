module bid_tree #(
    parameter QTY_PRICE_LVL_BIT = 10   // LEVELS = QTY_PRICE_LVL_BIT, leaves = 2^LEVELS
)(
    input  wire                        i_clk,
    input  wire                        i_rst,        // active high
    input wire [QTY_PRICE_LVL_BIT-1:0] o_price_idx,
    input wire                         o_price_changed,
    input wire                         o_bid_ask,
    output reg                         o_busy,
    output reg                         o_done,       // pulse when update reaches root

    // Best bid output (root)
    output wire                         o_best_valid,
    output wire [QTY_PRICE_LVL_BIT-1:0] o_best_idx
);

    localparam LEVELS      = QTY_PRICE_LVL_BIT;
    localparam LEAF_COUNT  = (1 << LEVELS);

    // --------------------------------------------------------------------
    // Tree storage (over-allocated: each level has [0..LEAF_COUNT-1])
    // Only nodes [0..(1<<lv)-1] are meaningful at level lv.
    // level 0 = root, level LEVELS = leaves.
    // --------------------------------------------------------------------
    reg                          bid_t_valid [0:LEVELS][0:LEAF_COUNT-1];
    reg [QTY_PRICE_LVL_BIT-1:0]  bid_t_idx   [0:LEVELS][0:LEAF_COUNT-1];

    assign o_best_bid_valid = bid_t_valid[0][0];
    assign o_best_bid_idx   = bid_t_idx[0][0];

    // Ready when not busy
    assign o_upd_ready  = ~o_busy;

    // --------------------------------------------------------------------
    // Bid-side select: prefer RIGHT child (higher index) if valid
    // Returns packed {valid, idx} in (1 + QTY_PRICE_LVL_BIT) bits.
    // --------------------------------------------------------------------
    function [QTY_PRICE_LVL_BIT:0] pick_bid;
        input                        l_valid;
        input [QTY_PRICE_LVL_BIT-1:0] l_idx;
        input                        r_valid;
        input [QTY_PRICE_LVL_BIT-1:0] r_idx;
        begin
            if (r_valid)       pick_bid = {1'b1, r_idx};
            else if (l_valid)  pick_bid = {1'b1, l_idx};
            else               pick_bid = {1'b0, {QTY_PRICE_LVL_BIT{1'b0}}};
        end
    endfunction

    // --------------------------------------------------------------------
    // Update engine: one level per cycle
    // cycle 0: write leaf (LEVELS, leaf_idx)
    // cycle 1: update parent at level LEVELS-1
    // ...
    // cycle LEVELS: update root at level 0
    // --------------------------------------------------------------------
    reg [QTY_PRICE_LVL_BIT-1:0] cur_node;
    reg [3:0]                  cur_level; // enough for LEVELS up to 15; adjust if needed

    // internal wires for child selection
    reg                        l_valid_r, r_valid_r;
    reg [QTY_PRICE_LVL_BIT-1:0] l_idx_r,   r_idx_r;
    reg [QTY_PRICE_LVL_BIT:0]   sel;       // {valid, idx}

    integer lv, nd;

    always @(posedge i_clk or posedge i_rst) begin
        if (i_rst) begin
            o_busy <= 1'b0;
            o_done <= 1'b0;
            cur_node  <= {QTY_PRICE_LVL_BIT{1'b0}};
            cur_level <= 4'd0;

            // Optional: clear tree valid bits on reset (can be expensive).
            // If you don't want resettable large regs, remove this loop.
            for (lv = 0; lv <= LEVELS; lv = lv + 1) begin
                for (nd = 0; nd < LEAF_COUNT; nd = nd + 1) begin
                    t_valid[lv][nd] <= 1'b0;
                    t_idx[lv][nd]   <= {QTY_PRICE_LVL_BIT{1'b0}};
                end
            end
        end else begin
            o_done <= 1'b0;

            // Start a new update when idle
            if (~o_busy) begin
                if (i_upd_valid) begin
                    // Write leaf at bottom level
                    t_valid[LEVELS][i_leaf_idx] <= i_leaf_valid;
                    t_idx[LEVELS][i_leaf_idx]   <= i_leaf_idx;

                    // Begin propagation next cycle from parent of leaf
                    o_busy    <= 1'b1;
                    cur_node  <= (i_leaf_idx >> 1);
                    cur_level <= LEVELS - 1;
                end
            end else begin
                // Busy: update one parent node at (cur_level, cur_node)
                // Children are at (cur_level+1, 2*cur_node) and (cur_level+1, 2*cur_node+1)
                l_valid_r <= t_valid[cur_level+1][(cur_node << 1)];
                l_idx_r   <= t_idx  [cur_level+1][(cur_node << 1)];
                r_valid_r <= t_valid[cur_level+1][(cur_node << 1) + 1];
                r_idx_r   <= t_idx  [cur_level+1][(cur_node << 1) + 1];

                // Use previous sampled child regs to update current node
                // (This adds one extra cycle of latency inside the engine, but is safe and timing-friendly.)
                sel = pick_bid(l_valid_r, l_idx_r, r_valid_r, r_idx_r);

                t_valid[cur_level][cur_node] <= sel[QTY_PRICE_LVL_BIT];
                t_idx  [cur_level][cur_node] <= sel[QTY_PRICE_LVL_BIT-1:0];

                if (cur_level == 0) begin
                    // Root updated
                    o_busy <= 1'b0;
                    o_done <= 1'b1;
                end else begin
                    // Move up one level
                    cur_node  <= (cur_node >> 1);
                    cur_level <= cur_level - 1;
                end
            end
        end
    end

endmodule