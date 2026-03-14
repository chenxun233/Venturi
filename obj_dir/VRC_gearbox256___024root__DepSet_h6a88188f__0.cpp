// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See VRC_gearbox256.h for the primary calling header

#include "VRC_gearbox256__pch.h"
#include "VRC_gearbox256___024root.h"

VL_INLINE_OPT void VRC_gearbox256___024root___ico_sequent__TOP__0(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___ico_sequent__TOP__0\n"); );
    // Body
    vlSelf->rc_payload[0U] = vlSelf->RC_gearbox256__DOT__data_saver[0U];
    vlSelf->rc_payload[1U] = vlSelf->RC_gearbox256__DOT__data_saver[1U];
    vlSelf->rc_payload[2U] = vlSelf->RC_gearbox256__DOT__data_saver[2U];
    vlSelf->rc_payload[3U] = vlSelf->RC_gearbox256__DOT__data_saver[3U];
    vlSelf->rc_payload[4U] = vlSelf->m_axis_rc_tdata[4U];
    vlSelf->rc_payload[5U] = vlSelf->m_axis_rc_tdata[5U];
    vlSelf->rc_payload[6U] = vlSelf->m_axis_rc_tdata[6U];
    vlSelf->rc_payload[7U] = vlSelf->m_axis_rc_tdata[7U];
    vlSelf->RC_gearbox256__DOT__sop = ((IData)(vlSelf->m_axis_rc_tvalid) 
                                       & vlSelf->m_axis_rc_tuser[1U]);
}

void VRC_gearbox256___024root___eval_ico(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_ico\n"); );
    // Body
    if ((1ULL & vlSelf->__VicoTriggered.word(0U))) {
        VRC_gearbox256___024root___ico_sequent__TOP__0(vlSelf);
    }
}

void VRC_gearbox256___024root___eval_triggers__ico(VRC_gearbox256___024root* vlSelf);

bool VRC_gearbox256___024root___eval_phase__ico(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_phase__ico\n"); );
    // Init
    CData/*0:0*/ __VicoExecute;
    // Body
    VRC_gearbox256___024root___eval_triggers__ico(vlSelf);
    __VicoExecute = vlSelf->__VicoTriggered.any();
    if (__VicoExecute) {
        VRC_gearbox256___024root___eval_ico(vlSelf);
    }
    return (__VicoExecute);
}

void VRC_gearbox256___024root___eval_act(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_act\n"); );
}

VL_INLINE_OPT void VRC_gearbox256___024root___nba_sequent__TOP__0(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___nba_sequent__TOP__0\n"); );
    // Init
    CData/*7:0*/ __Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__Vfuncout;
    __Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__Vfuncout = 0;
    SData/*10:0*/ __Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__dw_count;
    __Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__dw_count = 0;
    CData/*7:0*/ __Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__Vfuncout;
    __Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__Vfuncout = 0;
    SData/*10:0*/ __Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__dw_count;
    __Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__dw_count = 0;
    // Body
    vlSelf->rc_valid = ((IData)(vlSelf->rst_n) && (IData)(vlSelf->m_axis_rc_tvalid));
    if (vlSelf->rst_n) {
        if (vlSelf->m_axis_rc_tvalid) {
            if ((8U > (0x7ffU & vlSelf->m_axis_rc_tdata[1U]))) {
                vlSelf->rc_payload_last = 1U;
                __Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__dw_count 
                    = (0x7ffU & vlSelf->m_axis_rc_tdata[1U]);
                __Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__Vfuncout 
                    = ((4U & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__dw_count))
                        ? ((2U & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__dw_count))
                            ? ((1U & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__dw_count))
                                ? 0x7fU : 0x3fU) : 
                           ((1U & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__dw_count))
                             ? 0x1fU : 0xfU)) : ((2U 
                                                  & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__dw_count))
                                                  ? 
                                                 ((1U 
                                                   & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__dw_count))
                                                   ? 7U
                                                   : 3U)
                                                  : 
                                                 ((1U 
                                                   & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__dw_count))
                                                   ? 1U
                                                   : 0xffU)));
                vlSelf->rc_payload_dw_keep = __Vfunc_RC_gearbox256__DOT__calc_tail_keep__1__Vfuncout;
            } else if (vlSelf->m_axis_rc_tlast) {
                if (vlSelf->m_axis_rc_tlast) {
                    vlSelf->rc_payload_last = 1U;
                    vlSelf->rc_payload_dw_keep = vlSelf->RC_gearbox256__DOT__rc_last_keep;
                }
            } else {
                vlSelf->rc_payload_last = 0U;
                vlSelf->rc_payload_dw_keep = 0xffU;
            }
            if (vlSelf->RC_gearbox256__DOT__sop) {
                vlSelf->rc_descriptor[0U] = vlSelf->m_axis_rc_tdata[0U];
                vlSelf->rc_descriptor[1U] = vlSelf->m_axis_rc_tdata[1U];
                vlSelf->rc_descriptor[2U] = vlSelf->m_axis_rc_tdata[2U];
                __Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__dw_count 
                    = (0x7ffU & vlSelf->m_axis_rc_tdata[1U]);
                __Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__Vfuncout 
                    = ((4U & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__dw_count))
                        ? ((2U & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__dw_count))
                            ? ((1U & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__dw_count))
                                ? 0x7fU : 0x3fU) : 
                           ((1U & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__dw_count))
                             ? 0x1fU : 0xfU)) : ((2U 
                                                  & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__dw_count))
                                                  ? 
                                                 ((1U 
                                                   & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__dw_count))
                                                   ? 7U
                                                   : 3U)
                                                  : 
                                                 ((1U 
                                                   & (IData)(__Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__dw_count))
                                                   ? 1U
                                                   : 0xffU)));
                vlSelf->RC_gearbox256__DOT__rc_last_keep 
                    = __Vfunc_RC_gearbox256__DOT__calc_tail_keep__0__Vfuncout;
            }
            vlSelf->RC_gearbox256__DOT__data_saver[0U] 
                = vlSelf->m_axis_rc_tdata[4U];
            vlSelf->RC_gearbox256__DOT__data_saver[1U] 
                = vlSelf->m_axis_rc_tdata[5U];
            vlSelf->RC_gearbox256__DOT__data_saver[2U] 
                = vlSelf->m_axis_rc_tdata[6U];
            vlSelf->RC_gearbox256__DOT__data_saver[3U] 
                = vlSelf->m_axis_rc_tdata[7U];
        } else {
            vlSelf->rc_payload_last = 0U;
            vlSelf->rc_descriptor[0U] = 0U;
            vlSelf->rc_descriptor[1U] = 0U;
            vlSelf->rc_descriptor[2U] = 0U;
            vlSelf->RC_gearbox256__DOT__data_saver[0U] = 0U;
            vlSelf->RC_gearbox256__DOT__data_saver[1U] = 0U;
            vlSelf->RC_gearbox256__DOT__data_saver[2U] = 0U;
            vlSelf->RC_gearbox256__DOT__data_saver[3U] = 0U;
            vlSelf->rc_payload_dw_keep = 0U;
            vlSelf->RC_gearbox256__DOT__rc_last_keep = 0U;
        }
    } else {
        vlSelf->rc_payload_last = 0U;
        vlSelf->rc_descriptor[0U] = 0U;
        vlSelf->rc_descriptor[1U] = 0U;
        vlSelf->rc_descriptor[2U] = 0U;
        vlSelf->RC_gearbox256__DOT__data_saver[0U] = 0U;
        vlSelf->RC_gearbox256__DOT__data_saver[1U] = 0U;
        vlSelf->RC_gearbox256__DOT__data_saver[2U] = 0U;
        vlSelf->RC_gearbox256__DOT__data_saver[3U] = 0U;
        vlSelf->rc_payload_dw_keep = 0U;
        vlSelf->RC_gearbox256__DOT__rc_last_keep = 0U;
    }
    vlSelf->rc_payload[0U] = vlSelf->RC_gearbox256__DOT__data_saver[0U];
    vlSelf->rc_payload[1U] = vlSelf->RC_gearbox256__DOT__data_saver[1U];
    vlSelf->rc_payload[2U] = vlSelf->RC_gearbox256__DOT__data_saver[2U];
    vlSelf->rc_payload[3U] = vlSelf->RC_gearbox256__DOT__data_saver[3U];
    vlSelf->rc_payload[4U] = vlSelf->m_axis_rc_tdata[4U];
    vlSelf->rc_payload[5U] = vlSelf->m_axis_rc_tdata[5U];
    vlSelf->rc_payload[6U] = vlSelf->m_axis_rc_tdata[6U];
    vlSelf->rc_payload[7U] = vlSelf->m_axis_rc_tdata[7U];
}

void VRC_gearbox256___024root___eval_nba(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_nba\n"); );
    // Body
    if ((1ULL & vlSelf->__VnbaTriggered.word(0U))) {
        VRC_gearbox256___024root___nba_sequent__TOP__0(vlSelf);
    }
}

void VRC_gearbox256___024root___eval_triggers__act(VRC_gearbox256___024root* vlSelf);

bool VRC_gearbox256___024root___eval_phase__act(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_phase__act\n"); );
    // Init
    VlTriggerVec<1> __VpreTriggered;
    CData/*0:0*/ __VactExecute;
    // Body
    VRC_gearbox256___024root___eval_triggers__act(vlSelf);
    __VactExecute = vlSelf->__VactTriggered.any();
    if (__VactExecute) {
        __VpreTriggered.andNot(vlSelf->__VactTriggered, vlSelf->__VnbaTriggered);
        vlSelf->__VnbaTriggered.thisOr(vlSelf->__VactTriggered);
        VRC_gearbox256___024root___eval_act(vlSelf);
    }
    return (__VactExecute);
}

bool VRC_gearbox256___024root___eval_phase__nba(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_phase__nba\n"); );
    // Init
    CData/*0:0*/ __VnbaExecute;
    // Body
    __VnbaExecute = vlSelf->__VnbaTriggered.any();
    if (__VnbaExecute) {
        VRC_gearbox256___024root___eval_nba(vlSelf);
        vlSelf->__VnbaTriggered.clear();
    }
    return (__VnbaExecute);
}

#ifdef VL_DEBUG
VL_ATTR_COLD void VRC_gearbox256___024root___dump_triggers__ico(VRC_gearbox256___024root* vlSelf);
#endif  // VL_DEBUG
#ifdef VL_DEBUG
VL_ATTR_COLD void VRC_gearbox256___024root___dump_triggers__nba(VRC_gearbox256___024root* vlSelf);
#endif  // VL_DEBUG
#ifdef VL_DEBUG
VL_ATTR_COLD void VRC_gearbox256___024root___dump_triggers__act(VRC_gearbox256___024root* vlSelf);
#endif  // VL_DEBUG

void VRC_gearbox256___024root___eval(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval\n"); );
    // Init
    IData/*31:0*/ __VicoIterCount;
    CData/*0:0*/ __VicoContinue;
    IData/*31:0*/ __VnbaIterCount;
    CData/*0:0*/ __VnbaContinue;
    // Body
    __VicoIterCount = 0U;
    vlSelf->__VicoFirstIteration = 1U;
    __VicoContinue = 1U;
    while (__VicoContinue) {
        if (VL_UNLIKELY((0x64U < __VicoIterCount))) {
#ifdef VL_DEBUG
            VRC_gearbox256___024root___dump_triggers__ico(vlSelf);
#endif
            VL_FATAL_MT("verilog_src/PCIe_related/dependencies/RC_gearbox256.v", 17, "", "Input combinational region did not converge.");
        }
        __VicoIterCount = ((IData)(1U) + __VicoIterCount);
        __VicoContinue = 0U;
        if (VRC_gearbox256___024root___eval_phase__ico(vlSelf)) {
            __VicoContinue = 1U;
        }
        vlSelf->__VicoFirstIteration = 0U;
    }
    __VnbaIterCount = 0U;
    __VnbaContinue = 1U;
    while (__VnbaContinue) {
        if (VL_UNLIKELY((0x64U < __VnbaIterCount))) {
#ifdef VL_DEBUG
            VRC_gearbox256___024root___dump_triggers__nba(vlSelf);
#endif
            VL_FATAL_MT("verilog_src/PCIe_related/dependencies/RC_gearbox256.v", 17, "", "NBA region did not converge.");
        }
        __VnbaIterCount = ((IData)(1U) + __VnbaIterCount);
        __VnbaContinue = 0U;
        vlSelf->__VactIterCount = 0U;
        vlSelf->__VactContinue = 1U;
        while (vlSelf->__VactContinue) {
            if (VL_UNLIKELY((0x64U < vlSelf->__VactIterCount))) {
#ifdef VL_DEBUG
                VRC_gearbox256___024root___dump_triggers__act(vlSelf);
#endif
                VL_FATAL_MT("verilog_src/PCIe_related/dependencies/RC_gearbox256.v", 17, "", "Active region did not converge.");
            }
            vlSelf->__VactIterCount = ((IData)(1U) 
                                       + vlSelf->__VactIterCount);
            vlSelf->__VactContinue = 0U;
            if (VRC_gearbox256___024root___eval_phase__act(vlSelf)) {
                vlSelf->__VactContinue = 1U;
            }
        }
        if (VRC_gearbox256___024root___eval_phase__nba(vlSelf)) {
            __VnbaContinue = 1U;
        }
    }
}

#ifdef VL_DEBUG
void VRC_gearbox256___024root___eval_debug_assertions(VRC_gearbox256___024root* vlSelf) {
    if (false && vlSelf) {}  // Prevent unused
    VRC_gearbox256__Syms* const __restrict vlSymsp VL_ATTR_UNUSED = vlSelf->vlSymsp;
    VL_DEBUG_IF(VL_DBG_MSGF("+    VRC_gearbox256___024root___eval_debug_assertions\n"); );
    // Body
    if (VL_UNLIKELY((vlSelf->clk & 0xfeU))) {
        Verilated::overWidthError("clk");}
    if (VL_UNLIKELY((vlSelf->rst_n & 0xfeU))) {
        Verilated::overWidthError("rst_n");}
    if (VL_UNLIKELY((vlSelf->m_axis_rc_tvalid & 0xfeU))) {
        Verilated::overWidthError("m_axis_rc_tvalid");}
    if (VL_UNLIKELY((vlSelf->m_axis_rc_tuser[2U] & 0xfffff800U))) {
        Verilated::overWidthError("m_axis_rc_tuser");}
    if (VL_UNLIKELY((vlSelf->m_axis_rc_tlast & 0xfeU))) {
        Verilated::overWidthError("m_axis_rc_tlast");}
}
#endif  // VL_DEBUG
