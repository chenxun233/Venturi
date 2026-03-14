// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See VRC_gearbox256.h for the primary calling header

#include "VRC_gearbox256__pch.h"
#include "VRC_gearbox256___024root.h"

VL_ATTR_COLD void VRC_gearbox256___024root___eval_static(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_static\n"); );
}

VL_ATTR_COLD void VRC_gearbox256___024root___eval_initial__TOP(VRC_gearbox256___024root* vlSelf);

VL_ATTR_COLD void VRC_gearbox256___024root___eval_initial(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_initial\n"); );
    // Body
    VRC_gearbox256___024root___eval_initial__TOP(vlSelf);
    vlSelf->__Vtrigprevexpr___TOP__clk__0 = vlSelf->clk;
    vlSelf->__Vtrigprevexpr___TOP__rst_n__0 = vlSelf->rst_n;
}

VL_ATTR_COLD void VRC_gearbox256___024root___eval_initial__TOP(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_initial__TOP\n"); );
    // Body
    vlSelf->m_axis_rc_tready = 1U;
}

VL_ATTR_COLD void VRC_gearbox256___024root___eval_final(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_final\n"); );
}

#ifdef VL_DEBUG
VL_ATTR_COLD void VRC_gearbox256___024root___dump_triggers__stl(VRC_gearbox256___024root* vlSelf);
#endif  // VL_DEBUG
VL_ATTR_COLD bool VRC_gearbox256___024root___eval_phase__stl(VRC_gearbox256___024root* vlSelf);

VL_ATTR_COLD void VRC_gearbox256___024root___eval_settle(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_settle\n"); );
    // Init
    IData/*31:0*/ __VstlIterCount;
    CData/*0:0*/ __VstlContinue;
    // Body
    __VstlIterCount = 0U;
    vlSelf->__VstlFirstIteration = 1U;
    __VstlContinue = 1U;
    while (__VstlContinue) {
        if (VL_UNLIKELY((0x64U < __VstlIterCount))) {
#ifdef VL_DEBUG
            VRC_gearbox256___024root___dump_triggers__stl(vlSelf);
#endif
            VL_FATAL_MT("verilog_src/PCIe_related/dependencies/RC_gearbox256.v", 17, "", "Settle region did not converge.");
        }
        __VstlIterCount = ((IData)(1U) + __VstlIterCount);
        __VstlContinue = 0U;
        if (VRC_gearbox256___024root___eval_phase__stl(vlSelf)) {
            __VstlContinue = 1U;
        }
        vlSelf->__VstlFirstIteration = 0U;
    }
}

#ifdef VL_DEBUG
VL_ATTR_COLD void VRC_gearbox256___024root___dump_triggers__stl(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___dump_triggers__stl\n"); );
    // Body
    if ((1U & (~ (IData)(vlSelf->__VstlTriggered.any())))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelf->__VstlTriggered.word(0U))) {
        VL_DBG_MSGF("         'stl' region trigger index 0 is active: Internal 'stl' trigger - first iteration\n");
    }
}
#endif  // VL_DEBUG

void VRC_gearbox256___024root___ico_sequent__TOP__0(VRC_gearbox256___024root* vlSelf);

VL_ATTR_COLD void VRC_gearbox256___024root___eval_stl(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_stl\n"); );
    // Body
    if ((1ULL & vlSelf->__VstlTriggered.word(0U))) {
        VRC_gearbox256___024root___ico_sequent__TOP__0(vlSelf);
    }
}

VL_ATTR_COLD void VRC_gearbox256___024root___eval_triggers__stl(VRC_gearbox256___024root* vlSelf);

VL_ATTR_COLD bool VRC_gearbox256___024root___eval_phase__stl(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_phase__stl\n"); );
    // Init
    CData/*0:0*/ __VstlExecute;
    // Body
    VRC_gearbox256___024root___eval_triggers__stl(vlSelf);
    __VstlExecute = vlSelf->__VstlTriggered.any();
    if (__VstlExecute) {
        VRC_gearbox256___024root___eval_stl(vlSelf);
    }
    return (__VstlExecute);
}

#ifdef VL_DEBUG
VL_ATTR_COLD void VRC_gearbox256___024root___dump_triggers__ico(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___dump_triggers__ico\n"); );
    // Body
    if ((1U & (~ (IData)(vlSelf->__VicoTriggered.any())))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelf->__VicoTriggered.word(0U))) {
        VL_DBG_MSGF("         'ico' region trigger index 0 is active: Internal 'ico' trigger - first iteration\n");
    }
}
#endif  // VL_DEBUG

#ifdef VL_DEBUG
VL_ATTR_COLD void VRC_gearbox256___024root___dump_triggers__act(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___dump_triggers__act\n"); );
    // Body
    if ((1U & (~ (IData)(vlSelf->__VactTriggered.any())))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelf->__VactTriggered.word(0U))) {
        VL_DBG_MSGF("         'act' region trigger index 0 is active: @(posedge clk or negedge rst_n)\n");
    }
}
#endif  // VL_DEBUG

#ifdef VL_DEBUG
VL_ATTR_COLD void VRC_gearbox256___024root___dump_triggers__nba(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___dump_triggers__nba\n"); );
    // Body
    if ((1U & (~ (IData)(vlSelf->__VnbaTriggered.any())))) {
        VL_DBG_MSGF("         No triggers active\n");
    }
    if ((1ULL & vlSelf->__VnbaTriggered.word(0U))) {
        VL_DBG_MSGF("         'nba' region trigger index 0 is active: @(posedge clk or negedge rst_n)\n");
    }
}
#endif  // VL_DEBUG

VL_ATTR_COLD void VRC_gearbox256___024root___ctor_var_reset(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___ctor_var_reset\n"); );
    // Body
    vlSelf->clk = VL_RAND_RESET_I(1);
    vlSelf->rst_n = VL_RAND_RESET_I(1);
    VL_RAND_RESET_W(256, vlSelf->m_axis_rc_tdata);
    vlSelf->m_axis_rc_tvalid = VL_RAND_RESET_I(1);
    VL_RAND_RESET_W(75, vlSelf->m_axis_rc_tuser);
    vlSelf->m_axis_rc_tkeep = VL_RAND_RESET_I(8);
    vlSelf->m_axis_rc_tlast = VL_RAND_RESET_I(1);
    vlSelf->m_axis_rc_tready = VL_RAND_RESET_I(1);
    vlSelf->rc_valid = VL_RAND_RESET_I(1);
    vlSelf->rc_payload_last = VL_RAND_RESET_I(1);
    VL_RAND_RESET_W(256, vlSelf->rc_payload);
    vlSelf->rc_payload_dw_keep = VL_RAND_RESET_I(8);
    VL_RAND_RESET_W(96, vlSelf->rc_descriptor);
    vlSelf->RC_gearbox256__DOT__sop = VL_RAND_RESET_I(1);
    VL_RAND_RESET_W(128, vlSelf->RC_gearbox256__DOT__data_saver);
    vlSelf->RC_gearbox256__DOT__rc_last_keep = VL_RAND_RESET_I(8);
    vlSelf->__Vtrigprevexpr___TOP__clk__0 = VL_RAND_RESET_I(1);
    vlSelf->__Vtrigprevexpr___TOP__rst_n__0 = VL_RAND_RESET_I(1);
}
