// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design internal header
// See VRC_gearbox256.h for the primary calling header

#ifndef VERILATED_VRC_GEARBOX256___024ROOT_H_
#define VERILATED_VRC_GEARBOX256___024ROOT_H_  // guard

#include "verilated.h"


class VRC_gearbox256__Syms;

class alignas(VL_CACHE_LINE_BYTES) VRC_gearbox256___024root final : public VerilatedModule {
  public:

    // DESIGN SPECIFIC STATE
    VL_IN8(clk,0,0);
    VL_IN8(rst_n,0,0);
    VL_IN8(m_axis_rc_tvalid,0,0);
    VL_IN8(m_axis_rc_tkeep,7,0);
    VL_IN8(m_axis_rc_tlast,0,0);
    VL_OUT8(m_axis_rc_tready,0,0);
    VL_OUT8(rc_valid,0,0);
    VL_OUT8(rc_payload_last,0,0);
    VL_OUT8(rc_payload_dw_keep,7,0);
    CData/*0:0*/ RC_gearbox256__DOT__sop;
    CData/*7:0*/ RC_gearbox256__DOT__rc_last_keep;
    CData/*0:0*/ __VstlFirstIteration;
    CData/*0:0*/ __VicoFirstIteration;
    CData/*0:0*/ __Vtrigprevexpr___TOP__clk__0;
    CData/*0:0*/ __Vtrigprevexpr___TOP__rst_n__0;
    CData/*0:0*/ __VactContinue;
    VL_INW(m_axis_rc_tdata,255,0,8);
    VL_INW(m_axis_rc_tuser,74,0,3);
    VL_OUTW(rc_payload,255,0,8);
    VL_OUTW(rc_descriptor,95,0,3);
    VlWide<4>/*127:0*/ RC_gearbox256__DOT__data_saver;
    IData/*31:0*/ __VactIterCount;
    VlTriggerVec<1> __VstlTriggered;
    VlTriggerVec<1> __VicoTriggered;
    VlTriggerVec<1> __VactTriggered;
    VlTriggerVec<1> __VnbaTriggered;

    // INTERNAL VARIABLES
    VRC_gearbox256__Syms* const vlSymsp;

    // CONSTRUCTORS
    VRC_gearbox256___024root(VRC_gearbox256__Syms* symsp, const char* v__name);
    ~VRC_gearbox256___024root();
    VL_UNCOPYABLE(VRC_gearbox256___024root);

    // INTERNAL METHODS
    void __Vconfigure(bool first);
};


#endif  // guard
