// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table internal header
//
// Internal details; most calling programs do not need this header,
// unless using verilator public meta comments.

#ifndef VERILATED_VTB_ORDER_BOOK_PARSER_BUILDER_ARBITER__SYMS_H_
#define VERILATED_VTB_ORDER_BOOK_PARSER_BUILDER_ARBITER__SYMS_H_  // guard

#include "verilated.h"

// INCLUDE MODEL CLASS

#include "Vtb_order_book_parser_builder_arbiter.h"

// INCLUDE MODULE CLASSES
#include "Vtb_order_book_parser_builder_arbiter___024root.h"
#include "Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2.h"
#include "Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8.h"

// SYMS CLASS (contains all model state)
class alignas(VL_CACHE_LINE_BYTES)Vtb_order_book_parser_builder_arbiter__Syms final : public VerilatedSyms {
  public:
    // INTERNAL STATE
    Vtb_order_book_parser_builder_arbiter* const __Vm_modelp;
    VlDeleter __Vm_deleter;
    bool __Vm_didInit = false;

    // MODULE INSTANCE STATE
    Vtb_order_book_parser_builder_arbiter___024root TOP;
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2 TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__ask_wrapper_inst;
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8 TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_AAPL__DOT__bid_wrapper_inst;
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2 TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__ask_wrapper_inst;
    Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8 TOP__tb_order_book_parser_builder_arbiter__DOT__builder_dut__DOT__symbol_book_HSBC__DOT__bid_wrapper_inst;

    // SCOPE NAMES
    VerilatedScope __Vscope_tb_order_book_parser_builder_arbiter;

    // CONSTRUCTORS
    Vtb_order_book_parser_builder_arbiter__Syms(VerilatedContext* contextp, const char* namep, Vtb_order_book_parser_builder_arbiter* modelp);
    ~Vtb_order_book_parser_builder_arbiter__Syms();

    // METHODS
    const char* name() { return TOP.name(); }
};

#endif  // guard
