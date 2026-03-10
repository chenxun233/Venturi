// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtb_order_book_parser_builder_arbiter.h for the primary calling header

#include "Vtb_order_book_parser_builder_arbiter__pch.h"
#include "Vtb_order_book_parser_builder_arbiter__Syms.h"
#include "Vtb_order_book_parser_builder_arbiter___024root.h"

void Vtb_order_book_parser_builder_arbiter___024root___ctor_var_reset(Vtb_order_book_parser_builder_arbiter___024root* vlSelf);

Vtb_order_book_parser_builder_arbiter___024root::Vtb_order_book_parser_builder_arbiter___024root(Vtb_order_book_parser_builder_arbiter__Syms* symsp, const char* v__name)
    : VerilatedModule{v__name}
    , __VdlySched{*symsp->_vm_contextp__}
    , vlSymsp{symsp}
 {
    // Reset structure values
    Vtb_order_book_parser_builder_arbiter___024root___ctor_var_reset(this);
}

void Vtb_order_book_parser_builder_arbiter___024root::__Vconfigure(bool first) {
    if (false && first) {}  // Prevent unused
}

Vtb_order_book_parser_builder_arbiter___024root::~Vtb_order_book_parser_builder_arbiter___024root() {
}
