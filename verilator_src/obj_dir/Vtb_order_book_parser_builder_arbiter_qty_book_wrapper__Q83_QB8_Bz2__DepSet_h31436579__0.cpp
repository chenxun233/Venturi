// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtb_order_book_parser_builder_arbiter.h for the primary calling header

#include "Vtb_order_book_parser_builder_arbiter__pch.h"
#include "Vtb_order_book_parser_builder_arbiter__Syms.h"
#include "Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2.h"

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__2(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__2\n"); );
    // Body
    if (vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__i_rst) {
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[1U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[2U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[3U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[4U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[5U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[6U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[7U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[8U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[9U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xaU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xbU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xcU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xdU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xeU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xfU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[1U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[2U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[3U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[4U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[5U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[6U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[7U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[8U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[9U] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xaU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xbU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xcU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xdU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xeU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xfU] = 0U;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__idx = 0U;
        while (VL_GTS_III(32, 0x100U, vlSelf->__PVT__bid_tree_builder_inst__DOT__idx)) {
            vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid[(0xffU 
                                                                         & vlSelf->__PVT__bid_tree_builder_inst__DOT__idx)] = 0U;
            vlSelf->__PVT__bid_tree_builder_inst__DOT__idx 
                = ((IData)(1U) + vlSelf->__PVT__bid_tree_builder_inst__DOT__idx);
        }
        vlSelf->__PVT__bid_tree_builder_inst__DOT__idx = 0x10U;
    } else {
        vlSelf->__PVT__bid_tree_builder_inst__DOT__idx = 0x10U;
    }
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [1U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [1U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [2U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [2U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [3U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [3U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [4U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [4U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [5U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [5U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [6U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [6U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [7U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [7U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [8U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [8U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [9U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [9U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xaU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0xaU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                     [0xbU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                     [0xbU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xcU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0xcU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                     [0xdU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                     [0xdU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xeU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0xeU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                     [0xfU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                     [0xfU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0U] ? 0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                        [1U] ? 1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [2U] ? 2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                        [3U] ? 3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [4U] ? 4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                        [5U] ? 5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [6U] ? 6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                        [7U] ? 7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [8U] ? 8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                        [9U] ? 9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaU] ? 0xaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                            [0xbU] ? 0xbU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcU] ? 0xcU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                            [0xdU] ? 0xdU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeU] ? 0xeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                            [0xfU] ? 0xfU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][8U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x10U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x11U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][8U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x10U] ? 0x10U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x11U] ? 0x11U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][9U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x12U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x13U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][9U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x12U] ? 0x12U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x13U] ? 0x13U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xaU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x14U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x15U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xaU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x14U] ? 0x14U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x15U] ? 0x15U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xbU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x16U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x17U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xbU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x16U] ? 0x16U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x17U] ? 0x17U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xcU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x18U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x19U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xcU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x18U] ? 0x18U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x19U] ? 0x19U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xdU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xdU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1aU] ? 0x1aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x1bU] ? 0x1bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xeU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xeU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1cU] ? 0x1cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x1dU] ? 0x1dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xfU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xfU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1eU] ? 0x1eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x1fU] ? 0x1fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x10U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x20U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x21U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x10U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x20U] ? 0x20U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x21U] ? 0x21U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x11U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x22U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x23U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x11U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x22U] ? 0x22U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x23U] ? 0x23U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x12U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x24U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x25U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x12U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x24U] ? 0x24U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x25U] ? 0x25U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x13U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x26U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x27U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x13U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x26U] ? 0x26U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x27U] ? 0x27U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x14U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x28U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x29U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x14U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x28U] ? 0x28U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x29U] ? 0x29U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x15U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x15U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2aU] ? 0x2aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x2bU] ? 0x2bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x16U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x16U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2cU] ? 0x2cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x2dU] ? 0x2dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x17U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x17U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2eU] ? 0x2eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x2fU] ? 0x2fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x18U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x30U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x31U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x18U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x30U] ? 0x30U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x31U] ? 0x31U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x19U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x32U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x33U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x19U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x32U] ? 0x32U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x33U] ? 0x33U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x34U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x35U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x34U] ? 0x34U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x35U] ? 0x35U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x36U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x37U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x36U] ? 0x36U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x37U] ? 0x37U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x38U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x39U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x38U] ? 0x38U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x39U] ? 0x39U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3aU] ? 0x3aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x3bU] ? 0x3bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3cU] ? 0x3cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x3dU] ? 0x3dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3eU] ? 0x3eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x3fU] ? 0x3fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x20U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x40U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x41U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x20U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x40U] ? 0x40U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x41U] ? 0x41U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x21U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x42U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x43U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x21U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x42U] ? 0x42U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x43U] ? 0x43U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x22U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x44U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x45U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x22U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x44U] ? 0x44U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x45U] ? 0x45U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x23U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x46U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x47U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x23U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x46U] ? 0x46U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x47U] ? 0x47U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x24U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x48U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x49U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x24U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x48U] ? 0x48U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x49U] ? 0x49U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x25U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x25U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4aU] ? 0x4aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x4bU] ? 0x4bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x26U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x26U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4cU] ? 0x4cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x4dU] ? 0x4dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x27U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x27U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4eU] ? 0x4eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x4fU] ? 0x4fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x28U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x50U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x51U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x28U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x50U] ? 0x50U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x51U] ? 0x51U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x29U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x52U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x53U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x29U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x52U] ? 0x52U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x53U] ? 0x53U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x54U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x55U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x54U] ? 0x54U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x55U] ? 0x55U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x56U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x57U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x56U] ? 0x56U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x57U] ? 0x57U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x58U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x59U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x58U] ? 0x58U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x59U] ? 0x59U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5aU] ? 0x5aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x5bU] ? 0x5bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5cU] ? 0x5cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x5dU] ? 0x5dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5eU] ? 0x5eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x5fU] ? 0x5fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x30U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x60U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x61U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x30U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x60U] ? 0x60U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x61U] ? 0x61U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x31U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x62U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x63U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x31U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x62U] ? 0x62U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x63U] ? 0x63U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x32U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x64U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x65U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x32U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x64U] ? 0x64U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x65U] ? 0x65U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x33U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x66U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x67U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x33U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x66U] ? 0x66U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x67U] ? 0x67U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x34U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x68U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x69U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x34U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x68U] ? 0x68U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x69U] ? 0x69U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x35U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x35U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6aU] ? 0x6aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x6bU] ? 0x6bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x36U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x36U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6cU] ? 0x6cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x6dU] ? 0x6dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x37U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x37U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6eU] ? 0x6eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x6fU] ? 0x6fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x38U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x70U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x71U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x38U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x70U] ? 0x70U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x71U] ? 0x71U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x39U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x72U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x73U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x39U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x72U] ? 0x72U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x73U] ? 0x73U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x74U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x75U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x74U] ? 0x74U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x75U] ? 0x75U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x76U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x77U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x76U] ? 0x76U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x77U] ? 0x77U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x78U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x79U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x78U] ? 0x78U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x79U] ? 0x79U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7aU] ? 0x7aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x7bU] ? 0x7bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7cU] ? 0x7cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x7dU] ? 0x7dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7eU] ? 0x7eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x7fU] ? 0x7fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x40U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x80U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x81U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x40U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x80U] ? 0x80U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x81U] ? 0x81U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x41U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x82U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x83U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x41U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x82U] ? 0x82U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x83U] ? 0x83U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x42U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x84U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x85U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x42U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x84U] ? 0x84U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x85U] ? 0x85U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x43U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x86U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x87U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x43U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x86U] ? 0x86U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x87U] ? 0x87U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x44U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x88U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x89U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x44U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x88U] ? 0x88U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x89U] ? 0x89U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x45U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x45U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8aU] ? 0x8aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x8bU] ? 0x8bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x46U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x46U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8cU] ? 0x8cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x8dU] ? 0x8dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x47U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x47U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8eU] ? 0x8eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x8fU] ? 0x8fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x48U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x90U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x91U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x48U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x90U] ? 0x90U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x91U] ? 0x91U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x49U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x92U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x93U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x49U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x92U] ? 0x92U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x93U] ? 0x93U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x94U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x95U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x94U] ? 0x94U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x95U] ? 0x95U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x96U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x97U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x96U] ? 0x96U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x97U] ? 0x97U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x98U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x99U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x98U] ? 0x98U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x99U] ? 0x99U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9aU] ? 0x9aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x9bU] ? 0x9bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9cU] ? 0x9cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x9dU] ? 0x9dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9eU] ? 0x9eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x9fU] ? 0x9fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x50U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x50U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa0U] ? 0xa0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xa1U] ? 0xa1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x51U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x51U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa2U] ? 0xa2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xa3U] ? 0xa3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x52U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x52U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa4U] ? 0xa4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xa5U] ? 0xa5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x53U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x53U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa6U] ? 0xa6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xa7U] ? 0xa7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x54U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x54U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa8U] ? 0xa8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xa9U] ? 0xa9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x55U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xabU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x55U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaaU] ? 0xaaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xabU] ? 0xabU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x56U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xacU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xadU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x56U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xacU] ? 0xacU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xadU] ? 0xadU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x57U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xafU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x57U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaeU] ? 0xaeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xafU] ? 0xafU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x58U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x58U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb0U] ? 0xb0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xb1U] ? 0xb1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x59U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x59U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb2U] ? 0xb2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xb3U] ? 0xb3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb4U] ? 0xb4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xb5U] ? 0xb5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb6U] ? 0xb6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xb7U] ? 0xb7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb8U] ? 0xb8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xb9U] ? 0xb9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbaU] ? 0xbaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xbbU] ? 0xbbU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbcU] ? 0xbcU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xbdU] ? 0xbdU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbeU] ? 0xbeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xbfU] ? 0xbfU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x60U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x60U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc0U] ? 0xc0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xc1U] ? 0xc1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x61U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x61U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc2U] ? 0xc2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xc3U] ? 0xc3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x62U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x62U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc4U] ? 0xc4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xc5U] ? 0xc5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x63U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x63U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc6U] ? 0xc6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xc7U] ? 0xc7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x64U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x64U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc8U] ? 0xc8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xc9U] ? 0xc9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x65U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x65U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcaU] ? 0xcaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xcbU] ? 0xcbU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x66U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xccU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x66U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xccU] ? 0xccU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xcdU] ? 0xcdU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x67U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xceU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x67U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xceU] ? 0xceU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xcfU] ? 0xcfU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x68U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x68U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd0U] ? 0xd0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xd1U] ? 0xd1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x69U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x69U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd2U] ? 0xd2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xd3U] ? 0xd3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd4U] ? 0xd4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xd5U] ? 0xd5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd6U] ? 0xd6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xd7U] ? 0xd7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd8U] ? 0xd8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xd9U] ? 0xd9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdaU] ? 0xdaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xdbU] ? 0xdbU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xddU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdcU] ? 0xdcU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xddU] ? 0xddU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdeU] ? 0xdeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xdfU] ? 0xdfU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x70U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x70U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe0U] ? 0xe0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xe1U] ? 0xe1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x71U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x71U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe2U] ? 0xe2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xe3U] ? 0xe3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x72U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x72U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe4U] ? 0xe4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xe5U] ? 0xe5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x73U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x73U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe6U] ? 0xe6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xe7U] ? 0xe7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x74U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x74U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe8U] ? 0xe8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xe9U] ? 0xe9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x75U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xebU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x75U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeaU] ? 0xeaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xebU] ? 0xebU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x76U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xecU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xedU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x76U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xecU] ? 0xecU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xedU] ? 0xedU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x77U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xefU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x77U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeeU] ? 0xeeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xefU] ? 0xefU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x78U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x78U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf0U] ? 0xf0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xf1U] ? 0xf1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x79U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x79U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf2U] ? 0xf2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xf3U] ? 0xf3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf4U] ? 0xf4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xf5U] ? 0xf5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf6U] ? 0xf6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xf7U] ? 0xf7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf8U] ? 0xf8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xf9U] ? 0xf9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfaU] ? 0xfaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xfbU] ? 0xfbU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfcU] ? 0xfcU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xfdU] ? 0xfdU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xffU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfeU] ? 0xfeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xffU] ? 0xffU : 0U));
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__0\n"); );
    // Init
    CData/*2:0*/ __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr;
    __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr = 0;
    CData/*2:0*/ __Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0;
    __Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0 = 0;
    VlWide<5>/*130:0*/ __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0;
    VL_ZERO_W(131, __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0);
    CData/*0:0*/ __Vdlyvset__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0;
    __Vdlyvset__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0 = 0;
    CData/*2:0*/ __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr;
    __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr = 0;
    // Body
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U] 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U];
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U] 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U];
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U] 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U];
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U] 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U];
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U] 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U];
    __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr;
    __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr;
    __Vdlyvset__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0 = 0U;
    vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v0 = 0U;
    vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v1 = 0U;
    if (vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__i_rst) {
        __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr = 0U;
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count = 0U;
        __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr = 0U;
    } else {
        if (vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_pop) {
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem
                [vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr][0U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem
                [vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr][1U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem
                [vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr][2U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem
                [vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr][3U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem
                [vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr][4U];
            __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr 
                = (7U & ((IData)(1U) + (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr)));
        } else {
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U];
        }
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count 
            = (0xfU & ((2U == (((IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__do_push) 
                                << 1U) | (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_pop)))
                        ? ((IData)(1U) + (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count))
                        : ((1U == (((IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__do_push) 
                                    << 1U) | (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_pop)))
                            ? ((IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count) 
                               - (IData)(1U)) : (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count))));
        if (vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__do_push) {
            __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[0U] 
                = vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__qty_msg[0U];
            __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[1U] 
                = vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__qty_msg[1U];
            __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[2U] 
                = vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__qty_msg[2U];
            __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[3U] 
                = vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__qty_msg[3U];
            __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[4U] 
                = vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__qty_msg[4U];
            __Vdlyvset__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0 = 1U;
            __Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr;
            __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr 
                = (7U & ((IData)(1U) + (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr)));
        }
    }
    if ((2U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op))) {
        vlSelf->__Vdlyvval__bid_bram_inst__DOT__bram__v0 
            = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_i_shares;
        vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v0 = 1U;
        vlSelf->__Vdlyvdim0__bid_bram_inst__DOT__bram__v0 
            = vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_prc_idx;
    }
    if ((2U == (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
                [0U][0U] ? 1U : 0U))) {
        vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v1 = 1U;
        vlSelf->__Vdlyvdim0__bid_bram_inst__DOT__bram__v1 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
            [0U][0U];
    }
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr 
        = __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr;
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr 
        = __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr;
    if (__Vdlyvset__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0) {
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem[__Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0][0U] 
            = __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[0U];
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem[__Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0][1U] 
            = __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[1U];
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem[__Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0][2U] 
            = __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[2U];
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem[__Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0][3U] 
            = __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[3U];
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem[__Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0][4U] 
            = __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[4U];
    }
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__1(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__1\n"); );
    // Init
    CData/*7:0*/ __Vdlyvdim0__bid_tree_builder_inst__DOT__last_bid_t_valid__v0;
    __Vdlyvdim0__bid_tree_builder_inst__DOT__last_bid_t_valid__v0 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__last_bid_t_valid__v0;
    __Vdlyvval__bid_tree_builder_inst__DOT__last_bid_t_valid__v0 = 0;
    CData/*0:0*/ __Vdlyvset__bid_tree_builder_inst__DOT__last_bid_t_valid__v0;
    __Vdlyvset__bid_tree_builder_inst__DOT__last_bid_t_valid__v0 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v0;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v0 = 0;
    CData/*0:0*/ __Vdlyvset__bid_tree_builder_inst__DOT__mid_bid_t_valid__v0;
    __Vdlyvset__bid_tree_builder_inst__DOT__mid_bid_t_valid__v0 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v1;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v1 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v2;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v2 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v3;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v3 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v4;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v4 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v5;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v5 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v6;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v6 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v7;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v7 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v8;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v8 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v9;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v9 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v10;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v10 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v11;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v11 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v12;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v12 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v13;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v13 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v14;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v14 = 0;
    CData/*0:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v15;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v15 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v0;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v0 = 0;
    CData/*0:0*/ __Vdlyvset__bid_tree_builder_inst__DOT__mid_bid_t_idx__v0;
    __Vdlyvset__bid_tree_builder_inst__DOT__mid_bid_t_idx__v0 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v1;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v1 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v2;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v2 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v3;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v3 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v4;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v4 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v5;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v5 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v6;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v6 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v7;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v7 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v8;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v8 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v9;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v9 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v10;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v10 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v11;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v11 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v12;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v12 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v13;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v13 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v14;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v14 = 0;
    CData/*7:0*/ __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v15;
    __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v15 = 0;
    // Body
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state;
    __Vdlyvset__bid_tree_builder_inst__DOT__mid_bid_t_idx__v0 = 0U;
    __Vdlyvset__bid_tree_builder_inst__DOT__mid_bid_t_valid__v0 = 0U;
    __Vdlyvset__bid_tree_builder_inst__DOT__last_bid_t_valid__v0 = 0U;
    if ((1U & (~ (IData)(vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__i_rst)))) {
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v0 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][0U];
        __Vdlyvset__bid_tree_builder_inst__DOT__mid_bid_t_idx__v0 = 1U;
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v1 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][1U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v2 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][2U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v3 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][3U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v4 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][4U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v5 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][5U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v6 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][6U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v7 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][7U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v8 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][8U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v9 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][9U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v10 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][0xaU];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v11 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][0xbU];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v12 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][0xcU];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v13 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][0xdU];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v14 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][0xeU];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v15 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
            [0U][0xfU];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v0 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][0U];
        __Vdlyvset__bid_tree_builder_inst__DOT__mid_bid_t_valid__v0 = 1U;
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v1 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][1U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v2 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][2U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v3 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][3U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v4 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][4U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v5 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][5U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v6 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][6U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v7 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][7U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v8 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][8U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v9 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][9U];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v10 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][0xaU];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v11 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][0xbU];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v12 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][0xcU];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v13 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][0xdU];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v14 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][0xeU];
        __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v15 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
            [0U][0xfU];
        if ((0U != (IData)(vlSelf->__PVT__tree_price_change))) {
            __Vdlyvval__bid_tree_builder_inst__DOT__last_bid_t_valid__v0 
                = (2U == (IData)(vlSelf->__PVT__tree_price_change));
            __Vdlyvset__bid_tree_builder_inst__DOT__last_bid_t_valid__v0 = 1U;
            __Vdlyvdim0__bid_tree_builder_inst__DOT__last_bid_t_valid__v0 
                = ((2U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state))
                    ? ((IData)(vlSelf->bid_qty_builder_inst__DOT____VdfgExtracted_hd5c3f456__0)
                        ? (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_prc_idx)
                        : ((IData)(vlSelf->bid_qty_builder_inst__DOT____VdfgExtracted_hfec9ef15__0)
                            ? (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_prc_idx)
                            : 0U)) : 0U);
        }
    }
    if (vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__i_rst) {
        vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num = 0ULL;
        vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_d_shares = 0U;
        vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_is_add = 0U;
        vlSelf->__PVT__aligner_inst__DOT__best_price_aligned = 0U;
        vlSelf->o_best_shares = 0U;
        vlSelf->__PVT__aligner_inst__DOT__best_price_d1 = 0U;
    } else {
        if (vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_valid) {
            vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num 
                = (((QData)((IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U])) 
                    << 0x20U) | (QData)((IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U])));
            vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_d_shares 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U];
            vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_is_add 
                = (1U & vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U]);
        }
        vlSelf->__PVT__aligner_inst__DOT__best_price_aligned 
            = vlSelf->__PVT__aligner_inst__DOT__best_price_d1;
        vlSelf->o_best_shares = vlSelf->__PVT__bram_o_data_b;
        vlSelf->__PVT__aligner_inst__DOT__best_price_d1 
            = VL_SHIFTL_III(32,32,32, vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
                            [0U][0U], 2U);
    }
    vlSelf->__PVT__aligner_inst__DOT__best_valid_aligned 
        = ((1U & (~ (IData)(vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__i_rst))) 
           && (IData)(vlSelf->__PVT__aligner_inst__DOT__best_valid_d1));
    if (__Vdlyvset__bid_tree_builder_inst__DOT__mid_bid_t_idx__v0) {
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v0;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[1U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v1;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[2U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v2;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[3U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v3;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[4U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v4;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[5U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v5;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[6U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v6;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[7U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v7;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[8U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v8;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[9U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v9;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xaU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v10;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xbU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v11;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xcU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v12;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xdU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v13;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xeU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v14;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx[0xfU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_idx__v15;
    }
    if (__Vdlyvset__bid_tree_builder_inst__DOT__mid_bid_t_valid__v0) {
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v0;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[1U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v1;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[2U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v2;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[3U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v3;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[4U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v4;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[5U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v5;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[6U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v6;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[7U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v7;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[8U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v8;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[9U] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v9;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xaU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v10;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xbU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v11;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xcU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v12;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xdU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v13;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xeU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v14;
        vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid[0xfU] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__mid_bid_t_valid__v15;
    }
    if (__Vdlyvset__bid_tree_builder_inst__DOT__last_bid_t_valid__v0) {
        vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid[__Vdlyvdim0__bid_tree_builder_inst__DOT__last_bid_t_valid__v0] 
            = __Vdlyvval__bid_tree_builder_inst__DOT__last_bid_t_valid__v0;
    }
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0U] ? 0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                        [1U] ? 1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [2U] ? 2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                        [3U] ? 3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [4U] ? 4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                        [5U] ? 5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [6U] ? 6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                        [7U] ? 7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [8U] ? 8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                        [9U] ? 9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaU] ? 0xaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                            [0xbU] ? 0xbU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcU] ? 0xcU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                            [0xdU] ? 0xdU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeU] ? 0xeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                            [0xfU] ? 0xfU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][8U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x10U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x11U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][8U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x10U] ? 0x10U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x11U] ? 0x11U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][9U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x12U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x13U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][9U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x12U] ? 0x12U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x13U] ? 0x13U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xaU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x14U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x15U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xaU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x14U] ? 0x14U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x15U] ? 0x15U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xbU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x16U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x17U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xbU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x16U] ? 0x16U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x17U] ? 0x17U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xcU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x18U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x19U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xcU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x18U] ? 0x18U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x19U] ? 0x19U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xdU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xdU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1aU] ? 0x1aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x1bU] ? 0x1bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xeU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xeU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1cU] ? 0x1cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x1dU] ? 0x1dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0xfU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0xfU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x1eU] ? 0x1eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x1fU] ? 0x1fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x10U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x20U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x21U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x10U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x20U] ? 0x20U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x21U] ? 0x21U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x11U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x22U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x23U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x11U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x22U] ? 0x22U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x23U] ? 0x23U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x12U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x24U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x25U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x12U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x24U] ? 0x24U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x25U] ? 0x25U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x13U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x26U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x27U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x13U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x26U] ? 0x26U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x27U] ? 0x27U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x14U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x28U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x29U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x14U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x28U] ? 0x28U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x29U] ? 0x29U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x15U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x15U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2aU] ? 0x2aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x2bU] ? 0x2bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x16U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x16U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2cU] ? 0x2cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x2dU] ? 0x2dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x17U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x17U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x2eU] ? 0x2eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x2fU] ? 0x2fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x18U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x30U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x31U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x18U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x30U] ? 0x30U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x31U] ? 0x31U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x19U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x32U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x33U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x19U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x32U] ? 0x32U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x33U] ? 0x33U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x34U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x35U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x34U] ? 0x34U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x35U] ? 0x35U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x36U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x37U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x36U] ? 0x36U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x37U] ? 0x37U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x38U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x39U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x38U] ? 0x38U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x39U] ? 0x39U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3aU] ? 0x3aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x3bU] ? 0x3bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3cU] ? 0x3cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x3dU] ? 0x3dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x1fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x1fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x3eU] ? 0x3eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x3fU] ? 0x3fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x20U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x40U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x41U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x20U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x40U] ? 0x40U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x41U] ? 0x41U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x21U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x42U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x43U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x21U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x42U] ? 0x42U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x43U] ? 0x43U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x22U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x44U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x45U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x22U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x44U] ? 0x44U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x45U] ? 0x45U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x23U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x46U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x47U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x23U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x46U] ? 0x46U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x47U] ? 0x47U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x24U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x48U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x49U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x24U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x48U] ? 0x48U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x49U] ? 0x49U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x25U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x25U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4aU] ? 0x4aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x4bU] ? 0x4bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x26U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x26U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4cU] ? 0x4cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x4dU] ? 0x4dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x27U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x27U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x4eU] ? 0x4eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x4fU] ? 0x4fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x28U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x50U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x51U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x28U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x50U] ? 0x50U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x51U] ? 0x51U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x29U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x52U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x53U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x29U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x52U] ? 0x52U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x53U] ? 0x53U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x54U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x55U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x54U] ? 0x54U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x55U] ? 0x55U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x56U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x57U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x56U] ? 0x56U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x57U] ? 0x57U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x58U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x59U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x58U] ? 0x58U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x59U] ? 0x59U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5aU] ? 0x5aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x5bU] ? 0x5bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5cU] ? 0x5cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x5dU] ? 0x5dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x2fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x2fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x5eU] ? 0x5eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x5fU] ? 0x5fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x30U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x60U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x61U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x30U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x60U] ? 0x60U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x61U] ? 0x61U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x31U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x62U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x63U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x31U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x62U] ? 0x62U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x63U] ? 0x63U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x32U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x64U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x65U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x32U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x64U] ? 0x64U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x65U] ? 0x65U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x33U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x66U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x67U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x33U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x66U] ? 0x66U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x67U] ? 0x67U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x34U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x68U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x69U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x34U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x68U] ? 0x68U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x69U] ? 0x69U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x35U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x35U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6aU] ? 0x6aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x6bU] ? 0x6bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x36U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x36U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6cU] ? 0x6cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x6dU] ? 0x6dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x37U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x37U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x6eU] ? 0x6eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x6fU] ? 0x6fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x38U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x70U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x71U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x38U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x70U] ? 0x70U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x71U] ? 0x71U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x39U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x72U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x73U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x39U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x72U] ? 0x72U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x73U] ? 0x73U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x74U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x75U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x74U] ? 0x74U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x75U] ? 0x75U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x76U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x77U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x76U] ? 0x76U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x77U] ? 0x77U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x78U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x79U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x78U] ? 0x78U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x79U] ? 0x79U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7aU] ? 0x7aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x7bU] ? 0x7bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7cU] ? 0x7cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x7dU] ? 0x7dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x3fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x3fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x7eU] ? 0x7eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x7fU] ? 0x7fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x40U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x80U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x81U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x40U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x80U] ? 0x80U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x81U] ? 0x81U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x41U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x82U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x83U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x41U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x82U] ? 0x82U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x83U] ? 0x83U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x42U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x84U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x85U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x42U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x84U] ? 0x84U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x85U] ? 0x85U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x43U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x86U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x87U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x43U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x86U] ? 0x86U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x87U] ? 0x87U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x44U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x88U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x89U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x44U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x88U] ? 0x88U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x89U] ? 0x89U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x45U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x45U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8aU] ? 0x8aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x8bU] ? 0x8bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x46U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x46U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8cU] ? 0x8cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x8dU] ? 0x8dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x47U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x47U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x8eU] ? 0x8eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x8fU] ? 0x8fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x48U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x90U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x91U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x48U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x90U] ? 0x90U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x91U] ? 0x91U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x49U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x92U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x93U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x49U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x92U] ? 0x92U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x93U] ? 0x93U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x94U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x95U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x94U] ? 0x94U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x95U] ? 0x95U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x96U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x97U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x96U] ? 0x96U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x97U] ? 0x97U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x98U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x99U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x98U] ? 0x98U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x99U] ? 0x99U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9aU] ? 0x9aU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x9bU] ? 0x9bU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9cU] ? 0x9cU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x9dU] ? 0x9dU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x4fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x4fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0x9eU] ? 0x9eU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0x9fU] ? 0x9fU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x50U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x50U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa0U] ? 0xa0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xa1U] ? 0xa1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x51U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x51U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa2U] ? 0xa2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xa3U] ? 0xa3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x52U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x52U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa4U] ? 0xa4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xa5U] ? 0xa5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x53U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x53U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa6U] ? 0xa6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xa7U] ? 0xa7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x54U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x54U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xa8U] ? 0xa8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xa9U] ? 0xa9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x55U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xabU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x55U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaaU] ? 0xaaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xabU] ? 0xabU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x56U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xacU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xadU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x56U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xacU] ? 0xacU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xadU] ? 0xadU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x57U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xafU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x57U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xaeU] ? 0xaeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xafU] ? 0xafU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x58U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x58U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb0U] ? 0xb0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xb1U] ? 0xb1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x59U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x59U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb2U] ? 0xb2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xb3U] ? 0xb3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb4U] ? 0xb4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xb5U] ? 0xb5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb6U] ? 0xb6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xb7U] ? 0xb7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xb8U] ? 0xb8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xb9U] ? 0xb9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbaU] ? 0xbaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xbbU] ? 0xbbU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbcU] ? 0xbcU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xbdU] ? 0xbdU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x5fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x5fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xbeU] ? 0xbeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xbfU] ? 0xbfU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x60U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x60U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc0U] ? 0xc0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xc1U] ? 0xc1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x61U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x61U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc2U] ? 0xc2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xc3U] ? 0xc3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x62U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x62U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc4U] ? 0xc4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xc5U] ? 0xc5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x63U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x63U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc6U] ? 0xc6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xc7U] ? 0xc7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x64U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x64U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xc8U] ? 0xc8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xc9U] ? 0xc9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x65U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x65U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcaU] ? 0xcaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xcbU] ? 0xcbU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x66U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xccU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x66U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xccU] ? 0xccU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xcdU] ? 0xcdU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x67U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xceU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xcfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x67U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xceU] ? 0xceU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xcfU] ? 0xcfU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x68U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x68U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd0U] ? 0xd0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xd1U] ? 0xd1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x69U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x69U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd2U] ? 0xd2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xd3U] ? 0xd3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd4U] ? 0xd4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xd5U] ? 0xd5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd6U] ? 0xd6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xd7U] ? 0xd7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xd8U] ? 0xd8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xd9U] ? 0xd9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdaU] ? 0xdaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xdbU] ? 0xdbU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xddU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdcU] ? 0xdcU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xddU] ? 0xddU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x6fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x6fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xdeU] ? 0xdeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xdfU] ? 0xdfU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x70U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x70U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe0U] ? 0xe0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xe1U] ? 0xe1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x71U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x71U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe2U] ? 0xe2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xe3U] ? 0xe3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x72U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x72U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe4U] ? 0xe4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xe5U] ? 0xe5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x73U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x73U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe6U] ? 0xe6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xe7U] ? 0xe7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x74U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x74U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xe8U] ? 0xe8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xe9U] ? 0xe9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x75U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xebU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x75U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeaU] ? 0xeaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xebU] ? 0xebU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x76U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xecU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xedU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x76U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xecU] ? 0xecU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xedU] ? 0xedU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x77U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xefU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x77U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xeeU] ? 0xeeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xefU] ? 0xefU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x78U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x78U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf0U] ? 0xf0U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xf1U] ? 0xf1U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x79U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x79U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf2U] ? 0xf2U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xf3U] ? 0xf3U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf4U] ? 0xf4U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xf5U] ? 0xf5U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf6U] ? 0xf6U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xf7U] ? 0xf7U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xf8U] ? 0xf8U : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xf9U] ? 0xf9U : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfaU] ? 0xfaU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xfbU] ? 0xfbU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfcU] ? 0xfcU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xfdU] ? 0xfdU : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[3U][0x7fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xffU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[3U][0x7fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
           [0xfeU] ? 0xfeU : (vlSelf->__PVT__bid_tree_builder_inst__DOT__last_bid_t_valid
                              [0xffU] ? 0xffU : 0U));
    vlSelf->__PVT__aligner_inst__DOT__best_valid_d1 
        = ((1U & (~ (IData)(vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__i_rst))) 
           && vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [0U][0U]);
    vlSelf->o_best_valid_aligned = ((IData)(vlSelf->__PVT__aligner_inst__DOT__best_valid_aligned) 
                                    & (0U < vlSelf->o_best_shares));
    if (vlSelf->o_best_valid_aligned) {
        vlSelf->o_seq_num = vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num;
        vlSelf->o_best_price_aligned = vlSelf->__PVT__aligner_inst__DOT__best_price_aligned;
    } else {
        vlSelf->o_seq_num = 0xffffffffffffffffULL;
        vlSelf->o_best_price_aligned = 0U;
    }
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__2(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__2\n"); );
    // Body
    if (vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__i_rst) {
        vlSelf->__PVT__bram_o_data_a = 0U;
        vlSelf->__PVT__bram_o_data_b = 0U;
    } else {
        vlSelf->__PVT__bram_o_data_a = ((1U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op))
                                         ? vlSelf->__PVT__bid_bram_inst__DOT__bram
                                        [vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_prc_idx]
                                         : 0U);
        vlSelf->__PVT__bram_o_data_b = ((1U == (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
                                                [0U]
                                                [0U]
                                                 ? 1U
                                                 : 0U))
                                         ? vlSelf->__PVT__bid_bram_inst__DOT__bram
                                        [vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
                                        [0U][0U]] : 0U);
    }
    if (vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v0) {
        vlSelf->__PVT__bid_bram_inst__DOT__bram[vlSelf->__Vdlyvdim0__bid_bram_inst__DOT__bram__v0] 
            = vlSelf->__Vdlyvval__bid_bram_inst__DOT__bram__v0;
    }
    if (vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v1) {
        vlSelf->__PVT__bid_bram_inst__DOT__bram[vlSelf->__Vdlyvdim0__bid_bram_inst__DOT__bram__v1] = 0U;
    }
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__0\n"); );
    // Body
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__do_push 
        = (IData)(((4U == (6U & vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__qty_msg[4U])) 
                   & (8U != (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count))));
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__3(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__3\n"); );
    // Init
    CData/*7:0*/ __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__0__Vfuncout;
    __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__0__Vfuncout = 0;
    IData/*31:0*/ __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__0__price;
    __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__0__price = 0;
    // Body
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [1U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [1U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [2U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [2U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [3U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [3U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [4U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [4U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [5U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [5U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [6U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [6U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [7U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [7U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [8U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [8U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [9U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [9U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xaU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0xaU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                     [0xbU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                     [0xbU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xcU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0xcU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                     [0xdU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                     [0xdU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xeU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0xeU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                     [0xfU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                     [0xfU] : 0U));
    if (vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__i_rst) {
        vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_prc_idx = 0U;
        vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 0U;
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op = 0U;
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_i_shares = 0U;
    } else {
        if (vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_valid) {
            __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__0__price 
                = ((vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U] 
                    << 0x1fU) | (vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U] 
                                 >> 1U));
            vlSelf->__PVT__bid_qty_builder_inst__DOT__cal_qty_book_addr__Vstatic__price_offset_u30 
                = (__Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__0__price 
                   >> 2U);
            __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__0__Vfuncout 
                = ((0x100U > vlSelf->__PVT__bid_qty_builder_inst__DOT__cal_qty_book_addr__Vstatic__price_offset_u30)
                    ? (0xffU & vlSelf->__PVT__bid_qty_builder_inst__DOT__cal_qty_book_addr__Vstatic__price_offset_u30)
                    : 0U);
            vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_prc_idx 
                = __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__0__Vfuncout;
        }
        if ((0U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state))) {
            if (vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_valid) {
                vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op = 1U;
                vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 1U;
            } else {
                vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op = 0U;
                vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 0U;
                vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_i_shares = 0U;
            }
        } else if ((1U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state))) {
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 2U;
        } else if ((2U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state))) {
            vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op = 2U;
            vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_i_shares 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_new;
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 0U;
        } else {
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 0U;
            vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_i_shares = 0U;
            vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op = 0U;
        }
    }
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state 
        = vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state;
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__4(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__4\n"); );
    // Body
    vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U] 
        = vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U];
    vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U] 
        = vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U];
    vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U] 
        = vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U];
    vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U] 
        = vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U];
    vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U] 
        = vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U];
    vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_valid 
        = ((1U & (~ (IData)(vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__i_rst))) 
           && (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_pop));
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__0\n"); );
    // Init
    CData/*2:0*/ __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr;
    __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr = 0;
    CData/*2:0*/ __Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0;
    __Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0 = 0;
    VlWide<5>/*130:0*/ __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0;
    VL_ZERO_W(131, __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0);
    CData/*0:0*/ __Vdlyvset__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0;
    __Vdlyvset__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0 = 0;
    CData/*2:0*/ __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr;
    __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr = 0;
    // Body
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U] 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U];
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U] 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U];
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U] 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U];
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U] 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U];
    vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U] 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U];
    __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr;
    __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr 
        = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr;
    __Vdlyvset__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0 = 0U;
    vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v0 = 0U;
    vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v1 = 0U;
    if (vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__i_rst) {
        __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr = 0U;
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count = 0U;
        __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr = 0U;
    } else {
        if (vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_pop) {
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem
                [vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr][0U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem
                [vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr][1U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem
                [vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr][2U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem
                [vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr][3U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem
                [vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr][4U];
            __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr 
                = (7U & ((IData)(1U) + (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr)));
        } else {
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[0U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[1U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[2U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U];
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U] 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U];
        }
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count 
            = (0xfU & ((2U == (((IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__do_push) 
                                << 1U) | (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_pop)))
                        ? ((IData)(1U) + (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count))
                        : ((1U == (((IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__do_push) 
                                    << 1U) | (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_pop)))
                            ? ((IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count) 
                               - (IData)(1U)) : (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count))));
        if (vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__do_push) {
            __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[0U] 
                = vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__qty_msg[0U];
            __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[1U] 
                = vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__qty_msg[1U];
            __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[2U] 
                = vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__qty_msg[2U];
            __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[3U] 
                = vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__qty_msg[3U];
            __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[4U] 
                = vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__qty_msg[4U];
            __Vdlyvset__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0 = 1U;
            __Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr;
            __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr 
                = (7U & ((IData)(1U) + (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr)));
        }
    }
    if ((2U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op))) {
        vlSelf->__Vdlyvval__bid_bram_inst__DOT__bram__v0 
            = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_i_shares;
        vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v0 = 1U;
        vlSelf->__Vdlyvdim0__bid_bram_inst__DOT__bram__v0 
            = vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_prc_idx;
    }
    if ((2U == (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
                [0U][0U] ? 1U : 0U))) {
        vlSelf->__Vdlyvset__bid_bram_inst__DOT__bram__v1 = 1U;
        vlSelf->__Vdlyvdim0__bid_bram_inst__DOT__bram__v1 
            = vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
            [0U][0U];
    }
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr 
        = __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr;
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr 
        = __Vdly__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr;
    if (__Vdlyvset__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0) {
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem[__Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0][0U] 
            = __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[0U];
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem[__Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0][1U] 
            = __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[1U];
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem[__Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0][2U] 
            = __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[2U];
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem[__Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0][3U] 
            = __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[3U];
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem[__Vdlyvdim0__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0][4U] 
            = __Vdlyvval__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem__v0[4U];
    }
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__0\n"); );
    // Body
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__do_push 
        = (IData)(((4U == (6U & vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__qty_msg[4U])) 
                   & (8U != (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count))));
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__3(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__3\n"); );
    // Init
    CData/*7:0*/ __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__1__Vfuncout;
    __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__1__Vfuncout = 0;
    IData/*31:0*/ __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__1__price;
    __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__1__price = 0;
    // Body
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[3U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [1U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [1U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [2U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [2U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [3U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [3U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [4U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [4U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [5U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [5U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [6U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [6U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [7U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [7U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [8U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [8U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                   [9U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                   [9U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xaU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0xaU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                     [0xbU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                     [0xbU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xcU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0xcU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                     [0xdU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                     [0xdU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[3U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
           [0xeU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
           [0xeU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid
                     [0xfU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx
                     [0xfU] : 0U));
    if (vlSymsp->TOP.tb_order_book_parser_builder_arbiter__DOT__i_rst) {
        vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_prc_idx = 0U;
        vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 0U;
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op = 0U;
        vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_i_shares = 0U;
    } else {
        if (vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_valid) {
            __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__1__price 
                = ((vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[4U] 
                    << 0x1fU) | (vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg[3U] 
                                 >> 1U));
            vlSelf->__PVT__bid_qty_builder_inst__DOT__cal_qty_book_addr__Vstatic__price_offset_u30 
                = (__Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__1__price 
                   >> 2U);
            __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__1__Vfuncout 
                = ((0x100U > vlSelf->__PVT__bid_qty_builder_inst__DOT__cal_qty_book_addr__Vstatic__price_offset_u30)
                    ? (0xffU & vlSelf->__PVT__bid_qty_builder_inst__DOT__cal_qty_book_addr__Vstatic__price_offset_u30)
                    : 0U);
            vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_prc_idx 
                = __Vfunc_bid_qty_builder_inst__DOT__cal_qty_book_addr__1__Vfuncout;
        }
        if ((0U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state))) {
            if (vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_valid) {
                vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op = 1U;
                vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 1U;
            } else {
                vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op = 0U;
                vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 0U;
                vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_i_shares = 0U;
            }
        } else if ((1U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state))) {
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 2U;
        } else if ((2U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state))) {
            vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op = 2U;
            vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_i_shares 
                = vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_new;
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 0U;
        } else {
            vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state = 0U;
            vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_i_shares = 0U;
            vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_op = 0U;
        }
    }
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state 
        = vlSelf->__Vdly__bid_qty_builder_inst__DOT__qty_upd_state;
}
