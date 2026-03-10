// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtb_order_book_parser_builder_arbiter.h for the primary calling header

#include "Vtb_order_book_parser_builder_arbiter__pch.h"
#include "Vtb_order_book_parser_builder_arbiter__Syms.h"
#include "Vtb_order_book_parser_builder_arbiter___024root.h"

VL_ATTR_COLD void Vtb_order_book_parser_builder_arbiter___024root___eval_initial__TOP(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
VlCoroutine Vtb_order_book_parser_builder_arbiter___024root___eval_initial__TOP__Vtiming__0(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
VlCoroutine Vtb_order_book_parser_builder_arbiter___024root___eval_initial__TOP__Vtiming__1(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___eval_initial__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___eval_initial__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);

void Vtb_order_book_parser_builder_arbiter___024root___eval_initial(Vtb_order_book_parser_builder_arbiter___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtb_order_book_parser_builder_arbiter___024root___eval_initial\n"); );
    // Body
    Vtb_order_book_parser_builder_arbiter___024root___eval_initial__TOP(vlSelf);
    Vtb_order_book_parser_builder_arbiter___024root___eval_initial__TOP__Vtiming__0(vlSelf);
    Vtb_order_book_parser_builder_arbiter___024root___eval_initial__TOP__Vtiming__1(vlSelf);
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___eval_initial__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___eval_initial__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___eval_initial__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___eval_initial__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__0 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__i_clk_156__0 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_clk_156;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__i_rst__0 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_rst;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1 
        = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx;
}

VlCoroutine Vtb_order_book_parser_builder_arbiter___024root___eval_initial__TOP__Vtiming__0__2(Vtb_order_book_parser_builder_arbiter___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtb_order_book_parser_builder_arbiter___024root___eval_initial__TOP__Vtiming__0__2\n"); );
    // Body
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    co_await vlSelf->__VtrigSched_h3dc8676a__0.trigger(0U, 
                                                       nullptr, 
                                                       "@(posedge tb_order_book_parser_builder_arbiter.i_clk_156)", 
                                                       "verilator_src/tb_order_book_parser_builder_arbiter.sv", 
                                                       373);
    if (VL_UNLIKELY((0U == vlSelf->tb_order_book_parser_builder_arbiter__DOT__aapl_payloads))) {
        VL_WRITEF("[%0t] ERROR: no AAPL payloads were produced\n",
                  64,VL_TIME_UNITED_Q(1000),-9);
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors 
            = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors);
    }
    if (VL_UNLIKELY((0U == vlSelf->tb_order_book_parser_builder_arbiter__DOT__hsbc_payloads))) {
        VL_WRITEF("[%0t] ERROR: no HSBC payloads were produced\n",
                  64,VL_TIME_UNITED_Q(1000),-9);
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors 
            = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors);
    }
    VL_WRITEF("[%0t] summary: parser_msgs=%0d payloads=%0d aapl_payloads=%0d hsbc_payloads=%0d errors=%0d\n",
              64,VL_TIME_UNITED_Q(1000),-9,32,vlSelf->tb_order_book_parser_builder_arbiter__DOT__total_parser_msgs,
              32,vlSelf->tb_order_book_parser_builder_arbiter__DOT__total_payloads,
              32,vlSelf->tb_order_book_parser_builder_arbiter__DOT__aapl_payloads,
              32,vlSelf->tb_order_book_parser_builder_arbiter__DOT__hsbc_payloads,
              32,vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors);
    if (VL_UNLIKELY((0U != vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors))) {
        VL_WRITEF("[%0t] %%Fatal: tb_order_book_parser_builder_arbiter.sv:388: Assertion failed in %Ntb_order_book_parser_builder_arbiter: Verilator arbiter test failed with %0d errors\n",
                  64,VL_TIME_UNITED_Q(1000),-9,vlSymsp->name(),
                  32,vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors);
        VL_STOP_MT("verilator_src/tb_order_book_parser_builder_arbiter.sv", 388, "");
    }
    VL_WRITEF("[%0t] PASS: parser->builder arbiter test completed\n",
              64,VL_TIME_UNITED_Q(1000),-9);
    VL_FINISH_MT("verilator_src/tb_order_book_parser_builder_arbiter.sv", 392, "");
}

#ifdef VL_DEBUG
VL_ATTR_COLD void Vtb_order_book_parser_builder_arbiter___024root___dump_triggers__act(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
#endif  // VL_DEBUG

void Vtb_order_book_parser_builder_arbiter___024root___eval_triggers__act(Vtb_order_book_parser_builder_arbiter___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtb_order_book_parser_builder_arbiter___024root___eval_triggers__act\n"); );
    // Body
    vlSelf->__VactTriggered.set(0U, (((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_clk_156) 
                                      & (~ (IData)(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__i_clk_156__0))) 
                                     | ((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_rst) 
                                        & (~ (IData)(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__i_rst__0)))));
    vlSelf->__VactTriggered.set(1U, vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1));
    vlSelf->__VactTriggered.set(2U, (vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1) 
                                     | vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1)));
    vlSelf->__VactTriggered.set(3U, vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1));
    vlSelf->__VactTriggered.set(4U, (vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1) 
                                     | vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1)));
    vlSelf->__VactTriggered.set(5U, vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1));
    vlSelf->__VactTriggered.set(6U, (vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1) 
                                     | vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1)));
    vlSelf->__VactTriggered.set(7U, vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1));
    vlSelf->__VactTriggered.set(8U, (vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1) 
                                     | vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1)));
    vlSelf->__VactTriggered.set(9U, vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1));
    vlSelf->__VactTriggered.set(0xaU, (vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1) 
                                       | vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1)));
    vlSelf->__VactTriggered.set(0xbU, vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1));
    vlSelf->__VactTriggered.set(0xcU, (vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1) 
                                       | vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1)));
    vlSelf->__VactTriggered.set(0xdU, vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1));
    vlSelf->__VactTriggered.set(0xeU, (vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1) 
                                       | vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1)));
    vlSelf->__VactTriggered.set(0xfU, vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1));
    vlSelf->__VactTriggered.set(0x10U, (vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1) 
                                        | vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx.neq(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1)));
    vlSelf->__VactTriggered.set(0x11U, ((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_clk_156) 
                                        & (~ (IData)(vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__i_clk_156__0))));
    vlSelf->__VactTriggered.set(0x12U, vlSelf->__VdlySched.awaitingCurrentTime());
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__i_clk_156__0 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_clk_156;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__i_rst__0 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_rst;
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid);
    vlSelf->__Vtrigprevexpr___TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst____PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx__1.assign(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx);
    if (VL_UNLIKELY((1U & (~ (IData)(vlSelf->__VactDidInit))))) {
        vlSelf->__VactDidInit = 1U;
        vlSelf->__VactTriggered.set(1U, 1U);
        vlSelf->__VactTriggered.set(2U, 1U);
        vlSelf->__VactTriggered.set(3U, 1U);
        vlSelf->__VactTriggered.set(4U, 1U);
        vlSelf->__VactTriggered.set(5U, 1U);
        vlSelf->__VactTriggered.set(6U, 1U);
        vlSelf->__VactTriggered.set(7U, 1U);
        vlSelf->__VactTriggered.set(8U, 1U);
        vlSelf->__VactTriggered.set(9U, 1U);
        vlSelf->__VactTriggered.set(0xaU, 1U);
        vlSelf->__VactTriggered.set(0xbU, 1U);
        vlSelf->__VactTriggered.set(0xcU, 1U);
        vlSelf->__VactTriggered.set(0xdU, 1U);
        vlSelf->__VactTriggered.set(0xeU, 1U);
        vlSelf->__VactTriggered.set(0xfU, 1U);
        vlSelf->__VactTriggered.set(0x10U, 1U);
    }
#ifdef VL_DEBUG
    if (VL_UNLIKELY(vlSymsp->_vm_contextp__->debug())) {
        Vtb_order_book_parser_builder_arbiter___024root___dump_triggers__act(vlSelf);
    }
#endif
}

void Vtb_order_book_parser_builder_arbiter___024root___act_sequent__TOP__0(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__1(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__2(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__2(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__3(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__4(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__3(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__4(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);

void Vtb_order_book_parser_builder_arbiter___024root___eval_act(Vtb_order_book_parser_builder_arbiter___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtb_order_book_parser_builder_arbiter___024root___eval_act\n"); );
    // Body
    if ((0x20000ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter___024root___act_sequent__TOP__0(vlSelf);
    }
    if ((8ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
    }
    if ((0x10ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
    }
    if ((0x80ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
    }
    if ((0x100ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
    }
    if ((0x800ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
    }
    if ((0x1000ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
    }
    if ((0x8000ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    }
    if ((0x10000ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    }
    if ((1ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__2((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__2((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__2((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__2((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    }
    if ((2ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__3((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
    }
    if ((4ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__4((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
    }
    if ((0x20ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__3((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
    }
    if ((0x40ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__4((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
    }
    if ((0x200ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__3((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
    }
    if ((0x400ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__4((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
    }
    if ((0x2000ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__3((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    }
    if ((0x4000ULL & vlSelf->__VactTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___act_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__4((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    }
}

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter___024root___nba_sequent__TOP__0(Vtb_order_book_parser_builder_arbiter___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtb_order_book_parser_builder_arbiter___024root___nba_sequent__TOP__0\n"); );
    // Init
    CData/*0:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__wr_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__wr_ptr = 0;
    CData/*0:0*/ __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0;
    __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0 = 0;
    VlWide<9>/*273:0*/ __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0;
    VL_ZERO_W(274, __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0);
    CData/*0:0*/ __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0;
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0 = 0;
    CData/*0:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr = 0;
    VlWide<9>/*273:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload;
    VL_ZERO_W(274, __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload);
    CData/*2:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr = 0;
    CData/*2:0*/ __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0;
    __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0 = 0;
    VlWide<9>/*282:0*/ __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0;
    VL_ZERO_W(283, __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0);
    CData/*0:0*/ __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0;
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0 = 0;
    CData/*2:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr = 0;
    SData/*11:0*/ __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0;
    __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0 = 0;
    VlWide<3>/*66:0*/ __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0;
    VL_ZERO_W(67, __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0);
    CData/*0:0*/ __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0;
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0 = 0;
    CData/*0:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__wr_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__wr_ptr = 0;
    CData/*0:0*/ __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0;
    __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0 = 0;
    VlWide<9>/*273:0*/ __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0;
    VL_ZERO_W(274, __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0);
    CData/*0:0*/ __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0;
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0 = 0;
    CData/*0:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr = 0;
    VlWide<9>/*273:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload;
    VL_ZERO_W(274, __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload);
    CData/*2:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr = 0;
    CData/*2:0*/ __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0;
    __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0 = 0;
    VlWide<9>/*282:0*/ __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0;
    VL_ZERO_W(283, __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0);
    CData/*0:0*/ __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0;
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0 = 0;
    CData/*2:0*/ __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr = 0;
    SData/*11:0*/ __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0;
    __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0 = 0;
    VlWide<3>/*66:0*/ __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0;
    VL_ZERO_W(67, __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0);
    CData/*0:0*/ __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0;
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0 = 0;
    // Body
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[0U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[0U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[1U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[1U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[2U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[2U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[3U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[3U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[4U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[4U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[5U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[5U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[6U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[6U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[7U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[7U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[8U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[8U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr;
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[0U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[0U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[1U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[1U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[2U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[2U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[3U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[3U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[4U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[4U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[5U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[5U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[6U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[6U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[7U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[7U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[8U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[8U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[0U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[0U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[1U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[1U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[2U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[2U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[3U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[3U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[4U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[4U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[5U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[5U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[6U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[6U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[7U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[7U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[8U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[8U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[0U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[0U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[1U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[1U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[2U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[2U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[3U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[3U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[4U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[4U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[5U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[5U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[6U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[6U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[7U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[7U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[8U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[8U];
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr;
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0 = 0U;
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0 = 0U;
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[0U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[0U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[1U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[1U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[2U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[2U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[0U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[0U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[1U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[1U];
    vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[2U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[2U];
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0 = 0U;
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0 = 0U;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__wr_ptr 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__wr_ptr;
    __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__wr_ptr 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__wr_ptr;
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0 = 0U;
    __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0 = 0U;
    if (VL_UNLIKELY(vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_msg_valid)) {
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__total_parser_msgs 
            = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__total_parser_msgs);
        VL_WRITEF("[%0t] parser msg[%0d] type=%02x locate=%04x ref=%016x new_ref=%016x side=%02x shares=%08x price=%08x seq=%016x\n",
                  64,VL_TIME_UNITED_Q(1000),-9,32,vlSelf->tb_order_book_parser_builder_arbiter__DOT__total_parser_msgs,
                  8,(IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_msg_type),
                  16,vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_stock_locate,
                  64,vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_order_ref_num,
                  64,vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_new_order_ref_num,
                  8,(IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_buy_sell),
                  32,vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_shares,
                  32,vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_price,
                  64,vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_seq_num);
    }
    if (VL_UNLIKELY((1U & ((((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_ff_valid) 
                             << 1U) | (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_ff_valid)) 
                           >> (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))) {
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__total_payloads 
            = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__total_payloads);
        if (vlSelf->tb_order_book_parser_builder_arbiter__DOT__batch_checks_en) {
            vlSelf->tb_order_book_parser_builder_arbiter__DOT__batch_payloads 
                = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__batch_payloads);
        }
        VL_WRITEF("[%0t] arbiter payload[%0d] locate=%04x ask_valid=%0b ask_price=%08x ask_shares=%08x ask_seq=%016x bid_valid=%0b bid_price=%08x bid_shares=%08x bid_seq=%016x\n",
                  64,VL_TIME_UNITED_Q(1000),-9,32,vlSelf->tb_order_book_parser_builder_arbiter__DOT__total_payloads,
                  16,(0xffffU & vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[0U]),
                  1,(1U & (vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[8U] 
                           >> 0x11U)),32,((vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[8U] 
                                           << 0xfU) 
                                          | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[7U] 
                                             >> 0x11U)),
                  32,((vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[7U] 
                       << 0xfU) | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[6U] 
                                   >> 0x11U)),64,(((QData)((IData)(
                                                                   vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[6U])) 
                                                   << 0x2fU) 
                                                  | (((QData)((IData)(
                                                                      vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[5U])) 
                                                      << 0xfU) 
                                                     | ((QData)((IData)(
                                                                        vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[4U])) 
                                                        >> 0x11U))),
                  1,(1U & (vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[4U] 
                           >> 0x10U)),32,((vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[4U] 
                                           << 0x10U) 
                                          | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[3U] 
                                             >> 0x10U)),
                  32,((vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[3U] 
                       << 0x10U) | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[2U] 
                                    >> 0x10U)),64,(
                                                   ((QData)((IData)(
                                                                    vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[2U])) 
                                                    << 0x30U) 
                                                   | (((QData)((IData)(
                                                                       vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[1U])) 
                                                       << 0x10U) 
                                                      | ((QData)((IData)(
                                                                         vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[0U])) 
                                                         >> 0x10U))));
        if ((0xdU == (0xffffU & vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[0U]))) {
            if (VL_UNLIKELY((0x20000U & vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[8U]))) {
                VL_WRITEF("[%0t] ERROR: AAPL payload has unexpected ask_valid=1\n",
                          64,VL_TIME_UNITED_Q(1000),
                          -9);
                vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors 
                    = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors);
            }
            vlSelf->tb_order_book_parser_builder_arbiter__DOT__aapl_payloads 
                = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__aapl_payloads);
        } else if (VL_LIKELY((0xee8U == (0xffffU & 
                                         vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[0U])))) {
            if (VL_UNLIKELY((0x10000U & vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[4U]))) {
                VL_WRITEF("[%0t] ERROR: HSBC payload has unexpected bid_valid=1\n",
                          64,VL_TIME_UNITED_Q(1000),
                          -9);
                vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors 
                    = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors);
            }
            vlSelf->tb_order_book_parser_builder_arbiter__DOT__hsbc_payloads 
                = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__hsbc_payloads);
        } else {
            VL_WRITEF("[%0t] ERROR: arbiter produced unknown stock locate %04x\n",
                      64,VL_TIME_UNITED_Q(1000),-9,
                      16,(0xffffU & vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[0U]));
            vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors 
                = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors);
        }
        if (VL_UNLIKELY((((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__batch_checks_en) 
                          & (1U == vlSelf->tb_order_book_parser_builder_arbiter__DOT__batch_payloads)) 
                         & (0xdU != (0xffffU & vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[0U]))))) {
            VL_WRITEF("[%0t] ERROR: first arbitrated payload in batch was not AAPL\n",
                      64,VL_TIME_UNITED_Q(1000),-9);
            vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors 
                = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors);
        }
        if (VL_UNLIKELY((((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__batch_checks_en) 
                          & (2U == vlSelf->tb_order_book_parser_builder_arbiter__DOT__batch_payloads)) 
                         & (0xee8U != (0xffffU & vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[0U]))))) {
            VL_WRITEF("[%0t] ERROR: second arbitrated payload in batch was not HSBC\n",
                      64,VL_TIME_UNITED_Q(1000),-9);
            vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors 
                = ((IData)(1U) + vlSelf->tb_order_book_parser_builder_arbiter__DOT__errors);
        }
    }
    if (vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_rst) {
        __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr = 0U;
        __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr = 0U;
        __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr = 0U;
        __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr = 0U;
        __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr = 0U;
        __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__count = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__count = 0U;
        vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[0U] = 0U;
        vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[1U] = 0U;
        vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[2U] = 0U;
        vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[0U] = 0U;
        vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[1U] = 0U;
        vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[2U] = 0U;
        __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__wr_ptr = 0U;
        __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__wr_ptr = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_bid_best_price = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_ask_best_price = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_bid_best_price = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_ask_best_price = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_ask_best_shares = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_ask_best_shares = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_bid_best_shares = 0U;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_bid_best_shares = 0U;
    } else {
        if (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_pop) {
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][0U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][1U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][2U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[3U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][3U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[4U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][4U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[5U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][5U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[6U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][6U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[7U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][7U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[8U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][8U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr 
                = (7U & ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr)));
        } else {
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[0U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[1U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[2U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[3U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[3U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[4U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[4U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[5U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[5U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[6U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[6U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[7U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[7U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[8U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_msg[8U];
        }
        if (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_pop) {
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][0U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][1U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][2U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[3U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][3U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[4U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][4U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[5U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][5U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[6U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][6U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[7U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][7U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[8U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr][8U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr 
                = (7U & ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr)));
        } else {
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[0U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[1U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[2U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[3U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[3U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[4U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[4U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[5U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[5U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[6U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[6U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[7U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[7U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[8U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_msg[8U];
        }
        if (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_HSBC__i_ff_pop) {
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr][0U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr][1U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr][2U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[3U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr][3U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[4U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr][4U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[5U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr][5U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[6U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr][6U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[7U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr][7U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[8U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr][8U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr 
                = (1U & ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr)));
        } else {
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[0U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[1U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[2U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[3U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[3U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[4U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[4U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[5U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[5U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[6U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[6U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[7U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[7U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[8U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[8U];
        }
        if (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_AAPL__i_ff_pop) {
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr][0U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr][1U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr][2U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[3U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr][3U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[4U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr][4U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[5U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr][5U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[6U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr][6U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[7U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr][7U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[8U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr][8U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr 
                = (1U & ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr)));
        } else {
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[0U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[1U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[2U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[3U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[3U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[4U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[4U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[5U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[5U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[6U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[6U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[7U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[7U];
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[8U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[8U];
        }
        if (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__do_push) {
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[0U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[1U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[2U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[3U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[3U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[4U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[4U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[5U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[5U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[6U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[6U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[7U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[7U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[8U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[8U];
            __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0 = 1U;
            __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr;
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr 
                = (7U & ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr)));
        }
        if (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__do_push) {
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[0U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[1U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[2U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[3U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[3U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[4U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[4U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[5U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[5U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[6U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[6U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[7U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[7U];
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[8U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__parser_msg[8U];
            __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0 = 1U;
            __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr;
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr 
                = (7U & ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr)));
        }
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count 
            = (0xfU & ((2U == (((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__do_push) 
                                << 1U) | (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_pop)))
                        ? ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count))
                        : ((1U == (((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__do_push) 
                                    << 1U) | (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_pop)))
                            ? ((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count) 
                               - (IData)(1U)) : (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count))));
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count 
            = (0xfU & ((2U == (((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__do_push) 
                                << 1U) | (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_pop)))
                        ? ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count))
                        : ((1U == (((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__do_push) 
                                    << 1U) | (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_pop)))
                            ? ((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count) 
                               - (IData)(1U)) : (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count))));
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__count 
            = (3U & ((2U == (((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__do_push) 
                              << 1U) | (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_HSBC__i_ff_pop)))
                      ? ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__count))
                      : ((1U == (((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__do_push) 
                                  << 1U) | (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_HSBC__i_ff_pop)))
                          ? ((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__count) 
                             - (IData)(1U)) : (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__count))));
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__count 
            = (3U & ((2U == (((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__do_push) 
                              << 1U) | (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_AAPL__i_ff_pop)))
                      ? ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__count))
                      : ((1U == (((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__do_push) 
                                  << 1U) | (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_AAPL__i_ff_pop)))
                          ? ((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__count) 
                             - (IData)(1U)) : (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__count))));
        if ((1U == (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_op))) {
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_addr][0U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_addr][1U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_o_data[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_addr][2U];
        }
        if ((1U == (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_op))) {
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[0U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_addr][0U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[1U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_addr][1U];
            vlSelf->__Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_o_data[2U] 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram
                [vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_addr][2U];
        }
        if (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__do_push) {
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[0U] 
                = (0xee8U | ((IData)(((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_valid_aligned)
                                       ? vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num
                                       : 0xffffffffffffffffULL)) 
                             << 0x10U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[1U] 
                = (((IData)(((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_valid_aligned)
                              ? vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num
                              : 0xffffffffffffffffULL)) 
                    >> 0x10U) | ((IData)((((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_valid_aligned)
                                            ? vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num
                                            : 0xffffffffffffffffULL) 
                                          >> 0x20U)) 
                                 << 0x10U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[2U] 
                = (((IData)((((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_valid_aligned)
                               ? vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num
                               : 0xffffffffffffffffULL) 
                             >> 0x20U)) >> 0x10U) | 
                   ((IData)((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_price_aligned)) 
                              << 0x20U) | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_shares)))) 
                    << 0x10U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[3U] 
                = (((IData)((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_price_aligned)) 
                              << 0x20U) | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_shares)))) 
                    >> 0x10U) | ((IData)(((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_price_aligned)) 
                                            << 0x20U) 
                                           | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_shares))) 
                                          >> 0x20U)) 
                                 << 0x10U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[4U] 
                = (((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_seq_num) 
                    << 0x11U) | (((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_valid_aligned) 
                                  << 0x10U) | ((IData)(
                                                       ((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_price_aligned)) 
                                                          << 0x20U) 
                                                         | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_shares))) 
                                                        >> 0x20U)) 
                                               >> 0x10U)));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[5U] 
                = (((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_seq_num) 
                    >> 0xfU) | ((IData)((vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_seq_num 
                                         >> 0x20U)) 
                                << 0x11U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[6U] 
                = (((IData)((vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_seq_num 
                             >> 0x20U)) >> 0xfU) | 
                   ((IData)((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_price_aligned)) 
                              << 0x20U) | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_shares)))) 
                    << 0x11U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[7U] 
                = (((IData)((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_price_aligned)) 
                              << 0x20U) | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_shares)))) 
                    >> 0xfU) | ((IData)(((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_price_aligned)) 
                                           << 0x20U) 
                                          | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_shares))) 
                                         >> 0x20U)) 
                                << 0x11U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[8U] 
                = (((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_valid_aligned) 
                    << 0x11U) | ((IData)(((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_price_aligned)) 
                                            << 0x20U) 
                                           | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_shares))) 
                                          >> 0x20U)) 
                                 >> 0xfU));
            __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0 = 1U;
            __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__wr_ptr;
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__wr_ptr 
                = (1U & ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__wr_ptr)));
        }
        if (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__do_push) {
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[0U] 
                = (0xdU | ((IData)(((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_valid_aligned)
                                     ? vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num
                                     : 0xffffffffffffffffULL)) 
                           << 0x10U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[1U] 
                = (((IData)(((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_valid_aligned)
                              ? vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num
                              : 0xffffffffffffffffULL)) 
                    >> 0x10U) | ((IData)((((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_valid_aligned)
                                            ? vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num
                                            : 0xffffffffffffffffULL) 
                                          >> 0x20U)) 
                                 << 0x10U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[2U] 
                = (((IData)((((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_valid_aligned)
                               ? vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.__PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num
                               : 0xffffffffffffffffULL) 
                             >> 0x20U)) >> 0x10U) | 
                   ((IData)((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_price_aligned)) 
                              << 0x20U) | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_shares)))) 
                    << 0x10U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[3U] 
                = (((IData)((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_price_aligned)) 
                              << 0x20U) | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_shares)))) 
                    >> 0x10U) | ((IData)(((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_price_aligned)) 
                                            << 0x20U) 
                                           | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_shares))) 
                                          >> 0x20U)) 
                                 << 0x10U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[4U] 
                = (((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_seq_num) 
                    << 0x11U) | (((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_valid_aligned) 
                                  << 0x10U) | ((IData)(
                                                       ((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_price_aligned)) 
                                                          << 0x20U) 
                                                         | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_shares))) 
                                                        >> 0x20U)) 
                                               >> 0x10U)));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[5U] 
                = (((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_seq_num) 
                    >> 0xfU) | ((IData)((vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_seq_num 
                                         >> 0x20U)) 
                                << 0x11U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[6U] 
                = (((IData)((vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_seq_num 
                             >> 0x20U)) >> 0xfU) | 
                   ((IData)((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_price_aligned)) 
                              << 0x20U) | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_shares)))) 
                    << 0x11U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[7U] 
                = (((IData)((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_price_aligned)) 
                              << 0x20U) | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_shares)))) 
                    >> 0xfU) | ((IData)(((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_price_aligned)) 
                                           << 0x20U) 
                                          | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_shares))) 
                                         >> 0x20U)) 
                                << 0x11U));
            __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[8U] 
                = (((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_valid_aligned) 
                    << 0x11U) | ((IData)(((((QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_price_aligned)) 
                                            << 0x20U) 
                                           | (QData)((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_shares))) 
                                          >> 0x20U)) 
                                 >> 0xfU));
            __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0 = 1U;
            __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0 
                = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__wr_ptr;
            __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__wr_ptr 
                = (1U & ((IData)(1U) + (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__wr_ptr)));
        }
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_bid_best_price 
            = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_price_aligned;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_ask_best_price 
            = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_price_aligned;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_bid_best_price 
            = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_price_aligned;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_ask_best_price 
            = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_price_aligned;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_ask_best_shares 
            = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_shares;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_ask_best_shares 
            = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_shares;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_bid_best_shares 
            = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_shares;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_bid_best_shares 
            = vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_shares;
    }
    if ((2U == (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_op))) {
        __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[0U] 
            = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_i_data[0U];
        __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[1U] 
            = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_i_data[1U];
        __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[2U] 
            = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_i_data[2U];
        __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0 = 1U;
        __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0 
            = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_addr;
    }
    if ((2U == (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_op))) {
        __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[0U] 
            = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_i_data[0U];
        __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[1U] 
            = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_i_data[1U];
        __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[2U] 
            = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_i_data[2U];
        __Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0 = 1U;
        __Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0 
            = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_addr;
    }
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_bid_best_valid 
        = ((1U & (~ (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_rst))) 
           && (IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_valid_aligned));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_bid_best_valid 
        = ((1U & (~ (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_rst))) 
           && (IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_valid_aligned));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr;
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__rd_ptr;
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__rd_ptr;
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[0U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[0U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[1U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[1U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[2U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[2U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[3U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[3U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[4U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[4U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[5U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[5U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[6U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[6U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[7U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[7U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[8U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[8U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__rd_ptr;
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[0U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[0U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[1U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[1U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[2U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[2U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[3U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[3U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[4U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[4U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[5U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[5U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[6U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[6U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[7U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[7U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[8U] 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[8U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr;
    if (__Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0) {
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][0U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[0U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][1U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[1U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][2U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[2U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][3U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[3U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][4U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[4U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][5U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[5U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][6U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[6U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][7U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[7U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][8U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[8U];
    }
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__wr_ptr;
    if (__Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0) {
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][0U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[0U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][1U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[1U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][2U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[2U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][3U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[3U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][4U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[4U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][5U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[5U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][6U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[6U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][7U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[7U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0][8U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__mem__v0[8U];
    }
    if (__Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0) {
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0][0U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[0U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0][1U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[1U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0][2U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[2U];
    }
    if (__Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0) {
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0][0U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[0U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0][1U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[1U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0][2U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__order_book_inst__DOT__bram__v0[2U];
    }
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__wr_ptr 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__wr_ptr;
    if (__Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0) {
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0][0U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[0U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0][1U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[1U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0][2U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[2U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0][3U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[3U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0][4U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[4U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0][5U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[5U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0][6U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[6U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0][7U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[7U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0][8U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__mem__v0[8U];
    }
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__wr_ptr 
        = __Vdly__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__wr_ptr;
    if (__Vdlyvset__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0) {
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0][0U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[0U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0][1U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[1U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0][2U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[2U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0][3U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[3U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0][4U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[4U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0][5U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[5U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0][6U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[6U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0][7U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[7U];
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem[__Vdlyvdim0__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0][8U] 
            = __Vdlyvval__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__mem__v0[8U];
    }
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_ff_valid 
        = ((1U & (~ (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_rst))) 
           && (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_HSBC__i_ff_pop));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_ff_valid 
        = ((1U & (~ (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_rst))) 
           && (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_AAPL__i_ff_pop));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__arb_src_not_empty 
        = (((0U != (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__count)) 
            << 1U) | (0U != (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__count)));
}

extern const VlWide<9>/*287:0*/ Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_h52991b67_0;
extern const VlWide<9>/*287:0*/ Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_hed984950_0;

VL_INLINE_OPT void Vtb_order_book_parser_builder_arbiter___024root___nba_comb__TOP__0(Vtb_order_book_parser_builder_arbiter___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtb_order_book_parser_builder_arbiter___024root___nba_comb__TOP__0\n"); );
    // Init
    IData/*31:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp;
    tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp = 0;
    CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__scan_idx_c;
    tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__scan_idx_c = 0;
    CData/*0:0*/ tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT____VdfgTmp_h62692f1d__0;
    tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT____VdfgTmp_h62692f1d__0 = 0;
    CData/*0:0*/ __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__Vfuncout;
    __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__Vfuncout = 0;
    IData/*31:0*/ __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__base;
    __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__base = 0;
    VlWide<18>/*575:0*/ __Vtemp_3;
    // Body
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_c 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__rr_ptr_q;
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__have_grant_c = 0U;
    __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__base 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__rr_ptr_q;
    tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp 
        = __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__base;
    if (VL_LTES_III(32, 2U, tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp)) {
        tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp 
            = (tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp 
               - (IData)(2U));
    }
    __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__Vfuncout 
        = (1U & tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp);
    tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__scan_idx_c 
        = __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__Vfuncout;
    if ((1U & ((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__arb_src_not_empty) 
               >> (IData)(tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__scan_idx_c)))) {
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_c 
            = tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__scan_idx_c;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__have_grant_c = 1U;
    }
    __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__base 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__rr_ptr_q;
    tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp 
        = ((IData)(1U) + __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__base);
    if (VL_LTES_III(32, 2U, tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp)) {
        tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp 
            = (tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp 
               - (IData)(2U));
    }
    __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__Vfuncout 
        = (1U & tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__Vstatic__tmp);
    tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__scan_idx_c 
        = __Vfunc_tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__wrap_idx__119__Vfuncout;
    if ((1U & ((~ (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__have_grant_c)) 
               & ((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__arb_src_not_empty) 
                  >> (IData)(tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__scan_idx_c))))) {
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_c 
            = tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__scan_idx_c;
        vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__have_grant_c = 1U;
    }
    __Vtemp_3[0U] = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[0U];
    __Vtemp_3[1U] = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[1U];
    __Vtemp_3[2U] = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[2U];
    __Vtemp_3[3U] = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[3U];
    __Vtemp_3[4U] = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[4U];
    __Vtemp_3[5U] = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[5U];
    __Vtemp_3[6U] = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[6U];
    __Vtemp_3[7U] = vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[7U];
    __Vtemp_3[8U] = ((vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[0U] 
                      << 0x12U) | vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__aapl_payload[8U]);
    __Vtemp_3[9U] = ((vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[0U] 
                      >> 0xeU) | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[1U] 
                                  << 0x12U));
    __Vtemp_3[0xaU] = ((vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[1U] 
                        >> 0xeU) | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[2U] 
                                    << 0x12U));
    __Vtemp_3[0xbU] = ((vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[2U] 
                        >> 0xeU) | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[3U] 
                                    << 0x12U));
    __Vtemp_3[0xcU] = ((vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[3U] 
                        >> 0xeU) | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[4U] 
                                    << 0x12U));
    __Vtemp_3[0xdU] = ((vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[4U] 
                        >> 0xeU) | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[5U] 
                                    << 0x12U));
    __Vtemp_3[0xeU] = ((vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[5U] 
                        >> 0xeU) | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[6U] 
                                    << 0x12U));
    __Vtemp_3[0xfU] = ((vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[6U] 
                        >> 0xeU) | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[7U] 
                                    << 0x12U));
    __Vtemp_3[0x10U] = ((vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[7U] 
                         >> 0xeU) | (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[8U] 
                                     << 0x12U));
    __Vtemp_3[0x11U] = (vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__hsbc_payload[8U] 
                        >> 0xeU);
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[0U] 
        = (Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_h52991b67_0[0U] 
           & ((0x223U >= (0x3ffU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
               ? (((0U == (0x1fU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
                    ? 0U : (__Vtemp_3[((IData)(1U) 
                                       + (0x1fU & (
                                                   ((IData)(0x112U) 
                                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                   >> 5U)))] 
                            << ((IData)(0x20U) - (0x1fU 
                                                  & ((IData)(0x112U) 
                                                     * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))) 
                  | (__Vtemp_3[(0x1fU & (((IData)(0x112U) 
                                          * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                         >> 5U))] >> 
                     (0x1fU & ((IData)(0x112U) * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))
               : Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_hed984950_0[0U]));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[1U] 
        = (Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_h52991b67_0[1U] 
           & ((0x223U >= (0x3ffU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
               ? (((0U == (0x1fU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
                    ? 0U : (__Vtemp_3[((IData)(2U) 
                                       + (0x1fU & (
                                                   ((IData)(0x112U) 
                                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                   >> 5U)))] 
                            << ((IData)(0x20U) - (0x1fU 
                                                  & ((IData)(0x112U) 
                                                     * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))) 
                  | (__Vtemp_3[((IData)(1U) + (0x1fU 
                                               & (((IData)(0x112U) 
                                                   * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                  >> 5U)))] 
                     >> (0x1fU & ((IData)(0x112U) * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))
               : Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_hed984950_0[1U]));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[2U] 
        = (Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_h52991b67_0[2U] 
           & ((0x223U >= (0x3ffU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
               ? (((0U == (0x1fU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
                    ? 0U : (__Vtemp_3[((IData)(3U) 
                                       + (0x1fU & (
                                                   ((IData)(0x112U) 
                                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                   >> 5U)))] 
                            << ((IData)(0x20U) - (0x1fU 
                                                  & ((IData)(0x112U) 
                                                     * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))) 
                  | (__Vtemp_3[((IData)(2U) + (0x1fU 
                                               & (((IData)(0x112U) 
                                                   * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                  >> 5U)))] 
                     >> (0x1fU & ((IData)(0x112U) * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))
               : Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_hed984950_0[2U]));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[3U] 
        = (Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_h52991b67_0[3U] 
           & ((0x223U >= (0x3ffU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
               ? (((0U == (0x1fU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
                    ? 0U : (__Vtemp_3[((IData)(4U) 
                                       + (0x1fU & (
                                                   ((IData)(0x112U) 
                                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                   >> 5U)))] 
                            << ((IData)(0x20U) - (0x1fU 
                                                  & ((IData)(0x112U) 
                                                     * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))) 
                  | (__Vtemp_3[((IData)(3U) + (0x1fU 
                                               & (((IData)(0x112U) 
                                                   * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                  >> 5U)))] 
                     >> (0x1fU & ((IData)(0x112U) * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))
               : Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_hed984950_0[3U]));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[4U] 
        = (Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_h52991b67_0[4U] 
           & ((0x223U >= (0x3ffU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
               ? (((0U == (0x1fU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
                    ? 0U : (__Vtemp_3[((IData)(5U) 
                                       + (0x1fU & (
                                                   ((IData)(0x112U) 
                                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                   >> 5U)))] 
                            << ((IData)(0x20U) - (0x1fU 
                                                  & ((IData)(0x112U) 
                                                     * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))) 
                  | (__Vtemp_3[((IData)(4U) + (0x1fU 
                                               & (((IData)(0x112U) 
                                                   * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                  >> 5U)))] 
                     >> (0x1fU & ((IData)(0x112U) * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))
               : Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_hed984950_0[4U]));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[5U] 
        = (Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_h52991b67_0[5U] 
           & ((0x223U >= (0x3ffU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
               ? (((0U == (0x1fU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
                    ? 0U : (__Vtemp_3[((IData)(6U) 
                                       + (0x1fU & (
                                                   ((IData)(0x112U) 
                                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                   >> 5U)))] 
                            << ((IData)(0x20U) - (0x1fU 
                                                  & ((IData)(0x112U) 
                                                     * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))) 
                  | (__Vtemp_3[((IData)(5U) + (0x1fU 
                                               & (((IData)(0x112U) 
                                                   * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                  >> 5U)))] 
                     >> (0x1fU & ((IData)(0x112U) * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))
               : Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_hed984950_0[5U]));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[6U] 
        = (Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_h52991b67_0[6U] 
           & ((0x223U >= (0x3ffU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
               ? (((0U == (0x1fU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
                    ? 0U : (__Vtemp_3[((IData)(7U) 
                                       + (0x1fU & (
                                                   ((IData)(0x112U) 
                                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                   >> 5U)))] 
                            << ((IData)(0x20U) - (0x1fU 
                                                  & ((IData)(0x112U) 
                                                     * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))) 
                  | (__Vtemp_3[((IData)(6U) + (0x1fU 
                                               & (((IData)(0x112U) 
                                                   * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                  >> 5U)))] 
                     >> (0x1fU & ((IData)(0x112U) * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))
               : Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_hed984950_0[6U]));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[7U] 
        = (Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_h52991b67_0[7U] 
           & ((0x223U >= (0x3ffU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
               ? (((0U == (0x1fU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
                    ? 0U : (__Vtemp_3[((IData)(8U) 
                                       + (0x1fU & (
                                                   ((IData)(0x112U) 
                                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                   >> 5U)))] 
                            << ((IData)(0x20U) - (0x1fU 
                                                  & ((IData)(0x112U) 
                                                     * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))) 
                  | (__Vtemp_3[((IData)(7U) + (0x1fU 
                                               & (((IData)(0x112U) 
                                                   * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                  >> 5U)))] 
                     >> (0x1fU & ((IData)(0x112U) * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))
               : Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_hed984950_0[7U]));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_builder_payload[8U] 
        = (Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_h52991b67_0[8U] 
           & ((0x223U >= (0x3ffU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
               ? (((0U == (0x1fU & ((IData)(0x112U) 
                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q))))
                    ? 0U : (__Vtemp_3[((IData)(9U) 
                                       + (0x1fU & (
                                                   ((IData)(0x112U) 
                                                    * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                   >> 5U)))] 
                            << ((IData)(0x20U) - (0x1fU 
                                                  & ((IData)(0x112U) 
                                                     * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))) 
                  | (__Vtemp_3[((IData)(8U) + (0x1fU 
                                               & (((IData)(0x112U) 
                                                   * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)) 
                                                  >> 5U)))] 
                     >> (0x1fU & ((IData)(0x112U) * (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_q)))))
               : Vtb_order_book_parser_builder_arbiter__ConstPool__CONST_hed984950_0[8U]));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[0U] 
        = (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_axi_rx_data);
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[1U] 
        = (IData)((vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_axi_rx_data 
                   >> 0x20U));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[2U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[0U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[3U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[1U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[4U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[2U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[5U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[3U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[6U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[4U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[7U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[5U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[8U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[6U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[9U] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[7U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[0xaU] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[8U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[0xbU] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[9U];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[0xcU] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[0xaU];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[0xdU] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[0xbU];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[0xeU] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[0xcU];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__cur_buff[0xfU] 
        = vlSelf->tb_order_book_parser_builder_arbiter__DOT__parser_dut__DOT__prev_buff[0xdU];
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__do_push 
        = ((8U != (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count)) 
           & ((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_msg_valid) 
              & (0xdU == (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_stock_locate))));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__do_push 
        = ((8U != (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count)) 
           & ((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_msg_valid) 
              & (0xee8U == (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__o_stock_locate))));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__do_push 
        = ((2U != (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__event_fifo_inst__DOT__count)) 
           & ((vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_price_aligned 
               != vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_ask_best_price) 
              | ((vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst.o_best_shares 
                  != vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_ask_best_shares) 
                 | (((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_valid_aligned) 
                     != (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_bid_best_valid)) 
                    | ((vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_price_aligned 
                        != vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_bid_best_price) 
                       | (vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst.o_best_shares 
                          != vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__prev_bid_best_shares))))));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__do_push 
        = ((2U != (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__event_fifo_inst__DOT__count)) 
           & ((vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_price_aligned 
               != vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_ask_best_price) 
              | ((vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst.o_best_shares 
                  != vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_ask_best_shares) 
                 | (((IData)(vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_valid_aligned) 
                     != (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_bid_best_valid)) 
                    | ((vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_price_aligned 
                        != vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_bid_best_price) 
                       | (vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst.o_best_shares 
                          != vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__prev_bid_best_shares))))));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_pop 
        = ((0U != (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count)) 
           & ((~ (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__ff_o_valid)) 
              & (0U == (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__book_builder_inst__DOT__book_upd_state))));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_pop 
        = ((0U != (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__msg_fifo_inst__DOT__count)) 
           & ((~ (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__ff_o_valid)) 
              & (0U == (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__book_builder_inst__DOT__book_upd_state))));
    tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT____VdfgTmp_h62692f1d__0 
        = ((IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__have_grant_c) 
           & (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__i_builder_ff_pop));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_AAPL__i_ff_pop 
        = ((~ (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_c)) 
           & (IData)(tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT____VdfgTmp_h62692f1d__0));
    vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT____Vcellinp__symbol_book_HSBC__i_ff_pop 
        = ((IData)(tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT____VdfgTmp_h62692f1d__0) 
           & (IData)(vlSelf->tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__event_fifo_arbiter_inst__DOT__grant_idx_c));
}

void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter___024root___nba_sequent__TOP__1(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
void Vtb_order_book_parser_builder_arbiter___024root___nba_sequent__TOP__2(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__1(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter___024root___nba_sequent__TOP__3(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__2(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__2(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__0(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__3(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__3(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__3(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst__3(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__4(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__4(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__1(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2* vlSelf);
void Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8* vlSelf);

void Vtb_order_book_parser_builder_arbiter___024root___eval_nba(Vtb_order_book_parser_builder_arbiter___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    Vtb_order_book_parser_builder_arbiter__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtb_order_book_parser_builder_arbiter___024root___eval_nba\n"); );
    // Body
    if ((0x20000ULL & vlSelf->__VnbaTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter___024root___nba_sequent__TOP__0(vlSelf);
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    }
    if ((1ULL & vlSelf->__VnbaTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter___024root___nba_sequent__TOP__1(vlSelf);
        Vtb_order_book_parser_builder_arbiter___024root___nba_sequent__TOP__2(vlSelf);
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    }
    if ((0x20000ULL & vlSelf->__VnbaTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter___024root___nba_sequent__TOP__3(vlSelf);
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__2((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__2((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__2((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__2((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    }
    if ((0x20001ULL & vlSelf->__VnbaTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter___024root___nba_comb__TOP__0(vlSelf);
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__0((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
    }
    if ((1ULL & vlSelf->__VnbaTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__3((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst__3((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__3((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst__3((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    }
    if ((0x20000ULL & vlSelf->__VnbaTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__4((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__4((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__4((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_sequent__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__4((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    }
    if ((0x20001ULL & vlSelf->__VnbaTriggered.word(0U))) {
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst));
        Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8___nba_comb__TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst__1((&vlSymsp->TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst));
    }
}
