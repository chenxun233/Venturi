// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtb_order_book_parser_builder_arbiter.h for the primary calling header

#include "Vtb_order_book_parser_builder_arbiter__pch.h"
#include "Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2.h"

VL_ATTR_COLD void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___ctor_var_reset(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___ctor_var_reset\n"); );
    // Body
    vlSelf->i_clk_156 = VL_RAND_RESET_I(1);
    vlSelf->i_rst = VL_RAND_RESET_I(1);
    VL_RAND_RESET_W(131, vlSelf->i_qty_msg);
    vlSelf->o_best_valid_aligned = VL_RAND_RESET_I(1);
    vlSelf->o_best_price_aligned = VL_RAND_RESET_I(32);
    vlSelf->o_best_shares = VL_RAND_RESET_I(32);
    vlSelf->o_seq_num = VL_RAND_RESET_Q(64);
    vlSelf->__PVT__tree_price_change = VL_RAND_RESET_I(2);
    vlSelf->__PVT__bram_o_data_a = VL_RAND_RESET_I(32);
    vlSelf->__PVT__bram_o_data_b = VL_RAND_RESET_I(32);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state = VL_RAND_RESET_I(2);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op = VL_RAND_RESET_I(2);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_i_shares = VL_RAND_RESET_I(32);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_pop = VL_RAND_RESET_I(1);
    VL_RAND_RESET_W(131, vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_valid = VL_RAND_RESET_I(1);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_prc_idx = VL_RAND_RESET_I(8);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_is_add = VL_RAND_RESET_I(1);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_d_shares = VL_RAND_RESET_I(32);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num = VL_RAND_RESET_Q(64);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_new = VL_RAND_RESET_I(32);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__cal_qty_book_addr__Vstatic__price_offset_u30 = VL_RAND_RESET_I(30);
    vlSelf->bid_qty_builder_inst__DOT____VdfgExtracted_hd5c3f456__0 = VL_RAND_RESET_I(1);
    vlSelf->bid_qty_builder_inst__DOT____VdfgExtracted_hfec9ef15__0 = VL_RAND_RESET_I(1);
    for (int __Vi0 = 0; __Vi0 < 8; ++__Vi0) {
        VL_RAND_RESET_W(131, vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem[__Vi0]);
    }
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr = VL_RAND_RESET_I(3);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr = VL_RAND_RESET_I(3);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count = VL_RAND_RESET_I(4);
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__do_push = VL_RAND_RESET_I(1);
    for (int __Vi0 = 0; __Vi0 < 256; ++__Vi0) {
        vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid[__Vi0] = VL_RAND_RESET_I(1);
    }
    for (int __Vi0 = 0; __Vi0 < 16; ++__Vi0) {
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[__Vi0] = VL_RAND_RESET_I(8);
    }
    for (int __Vi0 = 0; __Vi0 < 16; ++__Vi0) {
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[__Vi0] = VL_RAND_RESET_I(1);
    }
    for (int __Vi0 = 0; __Vi0 < 4; ++__Vi0) {
        for (int __Vi1 = 0; __Vi1 < 128; ++__Vi1) {
            vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[__Vi0][__Vi1] = VL_RAND_RESET_I(8);
        }
    }
    for (int __Vi0 = 0; __Vi0 < 4; ++__Vi0) {
        for (int __Vi1 = 0; __Vi1 < 128; ++__Vi1) {
            vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[__Vi0][__Vi1] = VL_RAND_RESET_I(1);
        }
    }
    for (int __Vi0 = 0; __Vi0 < 4; ++__Vi0) {
        for (int __Vi1 = 0; __Vi1 < 8; ++__Vi1) {
            vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[__Vi0][__Vi1] = VL_RAND_RESET_I(8);
        }
    }
    for (int __Vi0 = 0; __Vi0 < 4; ++__Vi0) {
        for (int __Vi1 = 0; __Vi1 < 8; ++__Vi1) {
            vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[__Vi0][__Vi1] = VL_RAND_RESET_I(1);
        }
    }
    vlSelf->__PVT__bid_tree_builder_inst__DOT__idx = VL_RAND_RESET_I(32);
    vlSelf->__PVT__aligner_inst__DOT__best_valid_d1 = VL_RAND_RESET_I(1);
    vlSelf->__PVT__aligner_inst__DOT__best_valid_aligned = VL_RAND_RESET_I(1);
    vlSelf->__PVT__aligner_inst__DOT__best_price_d1 = VL_RAND_RESET_I(32);
    vlSelf->__PVT__aligner_inst__DOT__best_price_aligned = VL_RAND_RESET_I(32);
    for (int __Vi0 = 0; __Vi0 < 256; ++__Vi0) {
        vlSelf->__PVT__bid_bram_inst__DOT__bram[__Vi0] = VL_RAND_RESET_I(32);
    }
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = VL_RAND_RESET_I(2);
    VL_RAND_RESET_W(131, vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg);
    vlSelf->__Vdlyvdim0__bid_bram_inst__DOT__bram__v0 = 0;
    vlSelf->__Vdlyvval__bid_bram_inst__DOT__bram__v0 = VL_RAND_RESET_I(32);
    vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v0 = 0;
    vlSelf->__Vdlyvdim0__bid_bram_inst__DOT__bram__v1 = 0;
    vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v1 = 0;
}
