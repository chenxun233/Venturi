// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design internal header
// See Vtb_order_book_parser_builder_arbiter.h for the primary calling header

#ifndef VERILATED_VTB_ORDER_BOOK_PARSER_BUILDER_ARBITER_QTY_BOOK_WRAPPER__Q83_QB8_H_
#define VERILATED_VTB_ORDER_BOOK_PARSER_BUILDER_ARBITER_QTY_BOOK_WRAPPER__Q83_QB8_H_  // guard

#include "verilated.h"
#include "verilated_timing.h"


class Vtb_order_book_parser_builder_arbiter__Syms;

class alignas(VL_CACHE_LINE_BYTES) Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8 final : public VerilatedModule {
  public:

    // DESIGN SPECIFIC STATE
    VL_IN8(i_clk_156,0,0);
    VL_IN8(i_rst,0,0);
    VL_OUT8(o_best_valid_aligned,0,0);
    CData/*1:0*/ __PVT__tree_price_change;
    CData/*1:0*/ __PVT__bid_qty_builder_inst__DOT__qty_upd_state;
    CData/*1:0*/ __PVT__bid_qty_builder_inst__DOT__qty_upd_op;
    CData/*0:0*/ __PVT__bid_qty_builder_inst__DOT__ff_pop;
    CData/*0:0*/ __PVT__bid_qty_builder_inst__DOT__ff_o_valid;
    CData/*7:0*/ __PVT__bid_qty_builder_inst__DOT__latch_qty_prc_idx;
    CData/*0:0*/ __PVT__bid_qty_builder_inst__DOT__latch_qty_is_add;
    CData/*0:0*/ bid_qty_builder_inst__DOT____VdfgExtracted_hd5c3f456__1;
    CData/*0:0*/ bid_qty_builder_inst__DOT____VdfgExtracted_hfec9ef15__1;
    CData/*2:0*/ __PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__wr_ptr;
    CData/*2:0*/ __PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__rd_ptr;
    CData/*3:0*/ __PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__count;
    CData/*0:0*/ __PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__do_push;
    CData/*0:0*/ __PVT__aligner_inst__DOT__best_valid_d1;
    CData/*0:0*/ __PVT__aligner_inst__DOT__best_valid_aligned;
    CData/*1:0*/ __Vdly__bid_qty_builder_inst__DOT__qty_upd_state;
    CData/*7:0*/ __Vdlyvdim0__bid_bram_inst__DOT__bram__v0;
    CData/*0:0*/ __Vdlyvset__bid_bram_inst__DOT__bram__v0;
    CData/*7:0*/ __Vdlyvdim0__bid_bram_inst__DOT__bram__v1;
    CData/*0:0*/ __Vdlyvset__bid_bram_inst__DOT__bram__v1;
    VL_INW(i_qty_msg,130,0,5);
    VL_OUT(o_best_price_aligned,31,0);
    VL_OUT(o_best_shares,31,0);
    IData/*31:0*/ __PVT__bram_o_data_a;
    IData/*31:0*/ __PVT__bram_o_data_b;
    IData/*31:0*/ __PVT__bid_qty_builder_inst__DOT__qty_i_shares;
    VlWide<5>/*130:0*/ __PVT__bid_qty_builder_inst__DOT__ff_o_qty_msg;
    IData/*31:0*/ __PVT__bid_qty_builder_inst__DOT__latch_qty_d_shares;
    IData/*31:0*/ __PVT__bid_qty_builder_inst__DOT__qty_new;
    IData/*29:0*/ __PVT__bid_qty_builder_inst__DOT__cal_qty_book_addr__Vstatic__price_offset_u30;
    IData/*31:0*/ __PVT__bid_tree_builder_inst__DOT__idx;
    IData/*31:0*/ __PVT__aligner_inst__DOT__best_price_d1;
    IData/*31:0*/ __PVT__aligner_inst__DOT__best_price_aligned;
    VlWide<5>/*130:0*/ __Vdly__bid_qty_builder_inst__DOT__ff_o_qty_msg;
    IData/*31:0*/ __Vdlyvval__bid_bram_inst__DOT__bram__v0;
    VL_OUT64(o_seq_num,63,0);
    QData/*63:0*/ __PVT__bid_qty_builder_inst__DOT__latch_qty_seq_num;
    VlUnpacked<VlWide<5>/*130:0*/, 8> __PVT__bid_qty_builder_inst__DOT__qty_msg_fifo_inst__DOT__mem;
    VlUnpacked<CData/*0:0*/, 256> __PVT__bid_tree_builder_inst__DOT__last_bid_t_valid;
    VlUnpacked<CData/*7:0*/, 16> __PVT__bid_tree_builder_inst__DOT__mid_bid_t_idx;
    VlUnpacked<CData/*0:0*/, 16> __PVT__bid_tree_builder_inst__DOT__mid_bid_t_valid;
    VlUnpacked<VlUnpacked<CData/*7:0*/, 128>, 4> __PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_idx;
    VlUnpacked<VlUnpacked<CData/*0:0*/, 128>, 4> __PVT__bid_tree_builder_inst__DOT__mid_to_last_bid_t_valid;
    VlUnpacked<VlUnpacked<CData/*7:0*/, 8>, 4> __PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_idx;
    VlUnpacked<VlUnpacked<CData/*0:0*/, 8>, 4> __PVT__bid_tree_builder_inst__DOT__root_to_midbid_t_valid;
    VlUnpacked<IData/*31:0*/, 256> __PVT__bid_bram_inst__DOT__bram;

    // INTERNAL VARIABLES
    Vtb_order_book_parser_builder_arbiter__Syms* const vlSymsp;

    // CONSTRUCTORS
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8(Vtb_order_book_parser_builder_arbiter__Syms* symsp, const char* v__name);
    ~Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8();
    VL_UNCOPYABLE(Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8);

    // INTERNAL METHODS
    void __Vconfigure(bool first);
};


#endif  // guard
