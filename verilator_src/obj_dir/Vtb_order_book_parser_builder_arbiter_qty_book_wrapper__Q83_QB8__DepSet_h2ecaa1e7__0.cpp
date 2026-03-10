// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtb_order_book_parser_builder_arbiter.h for the primary calling header

#include "Vtb_order_book_parser_builder_arbiter__pch.h"
#include "Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8.h"

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___eval_initial__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___eval_initial__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst\n"); );
    // Init
    IData/*31:0*/ __PVT__bid_bram_inst__DOT__init_i;
    __PVT__bid_bram_inst__DOT__init_i = 0;
    // Body
    __PVT__bid_bram_inst__DOT__init_i = 0U;
    while (VL_GTS_III(32, 0x100U, __PVT__bid_bram_inst__DOT__init_i)) {
        vlSelf->__PVT__bid_bram_inst__DOT__bram[(0xffU 
                                                 & __PVT__bid_bram_inst__DOT__init_i)] = 0U;
        __PVT__bid_bram_inst__DOT__init_i = ((IData)(1U) 
                                             + __PVT__bid_bram_inst__DOT__init_i);
    }
    vlSelf->__PVT__bram_o_data_a = 0U;
    vlSelf->__PVT__bram_o_data_b = 0U;
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__0\n"); );
    // Body
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[0U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [1U][0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [1U][1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[1U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [2U][0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [2U][1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[1U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [2U][2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [2U][3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[2U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[2U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[2U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid[2U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][7U]);
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1\n"); );
    // Body
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[0U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [1U][1U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
           [1U][1U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
                       [1U][0U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
                       [1U][0U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[1U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [2U][1U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
           [2U][1U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
                       [2U][0U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
                       [2U][0U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[1U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [2U][3U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
           [2U][3U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
                       [2U][2U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
                       [2U][2U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[2U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][1U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
           [3U][1U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
                       [3U][0U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
                       [3U][0U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[2U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][3U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
           [3U][3U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
                       [3U][2U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
                       [3U][2U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[2U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][5U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
           [3U][5U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
                       [3U][4U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
                       [3U][4U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx[2U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
           [3U][7U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
           [3U][7U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid
                       [3U][6U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx
                       [3U][6U] : 0U));
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__3(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__3\n"); );
    // Body
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0xaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0xbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0xcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0xdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0xeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0xfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][8U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x10U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x11U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][9U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x12U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x13U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][0xaU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x14U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x15U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][0xbU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x16U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x17U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][0xcU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x18U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x19U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][0xdU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x1aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x1bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][0xeU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x1cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x1dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[0U][0xfU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x1eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x1fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0xaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0xbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0xcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0xdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0xeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0xfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][8U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x10U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x11U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][9U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x12U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x13U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0xaU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x14U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x15U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0xbU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x16U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x17U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0xcU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x18U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x19U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0xdU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x1aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x1bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0xeU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x1cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x1dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0xfU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x1eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x1fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x10U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x20U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x21U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x11U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x22U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x23U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x12U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x24U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x25U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x13U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x26U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x27U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x14U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x28U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x29U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x15U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x2aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x2bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x16U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x2cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x2dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x17U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x2eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x2fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x18U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x30U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x31U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x19U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x32U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x33U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x1aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x34U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x35U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x1bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x36U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x37U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x1cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x38U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x39U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x1dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x3aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x3bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x1eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x3cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x3dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[1U][0x1fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x3eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x3fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][1U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][2U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][3U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][4U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][5U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][6U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][7U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][8U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][9U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0xaU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0xbU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0xcU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0xdU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0xeU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0xfU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][8U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x10U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x11U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][9U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x12U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x13U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0xaU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x14U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x15U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0xbU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x16U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x17U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0xcU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x18U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x19U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0xdU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x1aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x1bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0xeU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x1cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x1dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0xfU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x1eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x1fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x10U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x20U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x21U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x11U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x22U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x23U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x12U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x24U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x25U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x13U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x26U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x27U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x14U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x28U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x29U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x15U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x2aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x2bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x16U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x2cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x2dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x17U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x2eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x2fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x18U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x30U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x31U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x19U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x32U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x33U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x1aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x34U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x35U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x1bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x36U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x37U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x1cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x38U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x39U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x1dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x3aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x3bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x1eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x3cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x3dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x1fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x3eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x3fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x20U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x40U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x41U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x21U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x42U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x43U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x22U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x44U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x45U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x23U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x46U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x47U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x24U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x48U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x49U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x25U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x4aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x4bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x26U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x4cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x4dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x27U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x4eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x4fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x28U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x50U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x51U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x29U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x52U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x53U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x2aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x54U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x55U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x2bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x56U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x57U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x2cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x58U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x59U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x2dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x5aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x5bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x2eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x5cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x5dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x2fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x5eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x5fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x30U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x60U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x61U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x31U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x62U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x63U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x32U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x64U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x65U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x33U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x66U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x67U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x34U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x68U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x69U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x35U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x6aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x6bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x36U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x6cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x6dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x37U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x6eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x6fU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x38U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x70U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x71U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x39U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x72U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x73U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x3aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x74U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x75U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x3bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x76U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x77U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x3cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x78U] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x79U]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x3dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x7aU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x7bU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x3eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x7cU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x7dU]);
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid[2U][0x3fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x7eU] | vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x7fU]);
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__4(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__4\n"); );
    // Body
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][1U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][1U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [1U][0U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [1U][0U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][3U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][3U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [1U][2U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [1U][2U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][5U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][5U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [1U][4U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [1U][4U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][7U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][7U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [1U][6U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [1U][6U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][9U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][9U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [1U][8U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [1U][8U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0xbU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][0xbU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                         [1U][0xaU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                         [1U][0xaU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0xdU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][0xdU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                         [1U][0xcU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                         [1U][0xcU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0xfU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][0xfU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                         [1U][0xeU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                         [1U][0xeU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][8U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x11U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][0x11U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [1U][0x10U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [1U][0x10U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][9U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x13U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][0x13U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [1U][0x12U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [1U][0x12U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][0xaU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x15U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][0x15U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [1U][0x14U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [1U][0x14U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][0xbU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x17U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][0x17U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [1U][0x16U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [1U][0x16U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][0xcU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x19U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][0x19U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [1U][0x18U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [1U][0x18U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][0xdU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x1bU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][0x1bU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [1U][0x1aU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [1U][0x1aU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][0xeU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x1dU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][0x1dU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [1U][0x1cU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [1U][0x1cU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[0U][0xfU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [1U][0x1fU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [1U][0x1fU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [1U][0x1eU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [1U][0x1eU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][1U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][1U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [2U][0U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [2U][0U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][3U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][3U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [2U][2U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [2U][2U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][5U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][5U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [2U][4U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [2U][4U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][7U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][7U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [2U][6U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [2U][6U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][9U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][9U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [2U][8U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [2U][8U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0xbU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0xbU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                         [2U][0xaU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                         [2U][0xaU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0xdU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0xdU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                         [2U][0xcU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                         [2U][0xcU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0xfU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0xfU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                         [2U][0xeU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                         [2U][0xeU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][8U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x11U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x11U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x10U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x10U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][9U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x13U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x13U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x12U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x12U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0xaU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x15U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x15U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x14U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x14U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0xbU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x17U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x17U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x16U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x16U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0xcU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x19U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x19U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x18U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x18U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0xdU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x1bU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x1bU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x1aU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x1aU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0xeU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x1dU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x1dU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x1cU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x1cU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0xfU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x1fU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x1fU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x1eU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x1eU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x10U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x21U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x21U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x20U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x20U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x11U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x23U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x23U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x22U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x22U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x12U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x25U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x25U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x24U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x24U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x13U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x27U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x27U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x26U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x26U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x14U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x29U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x29U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x28U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x28U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x15U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x2bU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x2bU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x2aU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x2aU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x16U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x2dU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x2dU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x2cU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x2cU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x17U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x2fU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x2fU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x2eU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x2eU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x18U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x31U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x31U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x30U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x30U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x19U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x33U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x33U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x32U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x32U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x1aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x35U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x35U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x34U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x34U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x1bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x37U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x37U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x36U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x36U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x1cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x39U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x39U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x38U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x38U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x1dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x3bU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x3bU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x3aU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x3aU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x1eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x3dU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x3dU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x3cU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x3cU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[1U][0x1fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [2U][0x3fU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [2U][0x3fU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [2U][0x3eU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [2U][0x3eU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][1U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][1U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [3U][0U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [3U][0U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][1U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][3U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][3U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [3U][2U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [3U][2U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][2U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][5U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][5U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [3U][4U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [3U][4U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][3U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][7U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][7U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [3U][6U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [3U][6U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][4U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][9U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][9U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                       [3U][8U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                       [3U][8U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][5U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0xbU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0xbU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                         [3U][0xaU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                         [3U][0xaU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][6U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0xdU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0xdU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                         [3U][0xcU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                         [3U][0xcU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][7U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0xfU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0xfU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                         [3U][0xeU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                         [3U][0xeU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][8U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x11U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x11U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x10U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x10U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][9U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x13U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x13U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x12U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x12U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0xaU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x15U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x15U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x14U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x14U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0xbU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x17U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x17U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x16U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x16U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0xcU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x19U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x19U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x18U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x18U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0xdU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x1bU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x1bU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x1aU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x1aU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0xeU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x1dU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x1dU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x1cU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x1cU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0xfU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x1fU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x1fU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x1eU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x1eU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x10U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x21U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x21U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x20U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x20U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x11U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x23U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x23U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x22U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x22U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x12U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x25U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x25U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x24U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x24U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x13U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x27U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x27U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x26U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x26U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x14U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x29U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x29U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x28U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x28U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x15U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x2bU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x2bU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x2aU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x2aU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x16U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x2dU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x2dU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x2cU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x2cU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x17U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x2fU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x2fU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x2eU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x2eU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x18U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x31U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x31U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x30U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x30U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x19U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x33U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x33U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x32U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x32U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x1aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x35U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x35U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x34U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x34U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x1bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x37U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x37U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x36U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x36U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x1cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x39U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x39U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x38U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x38U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x1dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x3bU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x3bU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x3aU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x3aU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x1eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x3dU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x3dU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x3cU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x3cU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x1fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x3fU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x3fU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x3eU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x3eU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x20U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x41U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x41U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x40U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x40U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x21U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x43U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x43U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x42U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x42U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x22U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x45U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x45U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x44U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x44U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x23U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x47U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x47U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x46U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x46U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x24U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x49U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x49U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x48U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x48U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x25U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x4bU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x4bU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x4aU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x4aU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x26U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x4dU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x4dU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x4cU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x4cU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x27U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x4fU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x4fU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x4eU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x4eU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x28U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x51U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x51U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x50U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x50U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x29U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x53U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x53U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x52U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x52U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x2aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x55U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x55U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x54U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x54U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x2bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x57U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x57U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x56U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x56U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x2cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x59U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x59U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x58U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x58U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x2dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x5bU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x5bU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x5aU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x5aU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x2eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x5dU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x5dU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x5cU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x5cU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x2fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x5fU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x5fU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x5eU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x5eU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x30U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x61U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x61U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x60U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x60U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x31U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x63U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x63U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x62U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x62U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x32U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x65U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x65U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x64U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x64U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x33U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x67U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x67U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x66U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x66U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x34U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x69U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x69U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x68U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x68U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x35U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x6bU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x6bU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x6aU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x6aU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x36U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x6dU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x6dU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x6cU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x6cU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x37U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x6fU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x6fU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x6eU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x6eU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x38U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x71U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x71U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x70U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x70U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x39U] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x73U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x73U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x72U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x72U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x3aU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x75U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x75U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x74U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x74U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x3bU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x77U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x77U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x76U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x76U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x3cU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x79U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x79U] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x78U] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x78U] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x3dU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x7bU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x7bU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x7aU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x7aU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x3eU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x7dU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x7dU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x7cU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x7cU] : 0U));
    vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx[2U][0x3fU] 
        = (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
           [3U][0x7fU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
           [3U][0x7fU] : (vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid
                          [3U][0x7eU] ? vlSelf->__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx
                          [3U][0x7eU] : 0U));
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+            Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1\n"); );
    // Body
    vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_new 
        = ((IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_is_add)
            ? (vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_d_shares 
               + vlSelf->__PVT__bram_o_data_a) : ((vlSelf->__PVT__bram_o_data_a 
                                                   > vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_d_shares)
                                                   ? 
                                                  (vlSelf->__PVT__bram_o_data_a 
                                                   - vlSelf->__PVT__bid_qty_builder_inst__DOT__latch_qty_d_shares)
                                                   : 0U));
    vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_pop 
        = ((0U != (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count)) 
           & ((~ (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__ff_o_valid)) 
              & (0U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state))));
    vlSelf->bid_qty_builder_inst__DOT____VdfgExtracted_hd5c3f456__1 
        = ((0U == vlSelf->__PVT__bram_o_data_a) & (0U 
                                                   < vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_new));
    vlSelf->bid_qty_builder_inst__DOT____VdfgExtracted_hfec9ef15__1 
        = ((0U < vlSelf->__PVT__bram_o_data_a) & (0U 
                                                  == vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_new));
    vlSelf->__PVT__tree_price_change = ((2U == (IData)(vlSelf->__PVT__bid_qty_builder_inst__DOT__qty_upd_state))
                                         ? ((IData)(vlSelf->bid_qty_builder_inst__DOT____VdfgExtracted_hd5c3f456__1)
                                             ? 2U : 
                                            ((IData)(vlSelf->bid_qty_builder_inst__DOT____VdfgExtracted_hfec9ef15__1)
                                              ? 1U : 0U))
                                         : 0U);
}
