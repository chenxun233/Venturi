# Verilated -*- Makefile -*-
# DESCRIPTION: Verilator output: Make include file with class lists
#
# This file lists generated Verilated files, for including in higher level makefiles.
# See Vtb_order_book_parser_builder_arbiter.mk for the caller.

### Switches...
# C11 constructs required?  0/1 (always on now)
VM_C11 = 1
# Timing enabled?  0/1
VM_TIMING = 1
# Coverage output mode?  0/1 (from --coverage)
VM_COVERAGE = 0
# Parallel builds?  0/1 (from --output-split)
VM_PARALLEL_BUILDS = 1
# Tracing output mode?  0/1 (from --trace/--trace-fst)
VM_TRACE = 0
# Tracing output mode in VCD format?  0/1 (from --trace)
VM_TRACE_VCD = 0
# Tracing output mode in FST format?  0/1 (from --trace-fst)
VM_TRACE_FST = 0

### Object file lists...
# Generated module classes, fast-path, compile with highest optimization
VM_CLASSES_FAST += \
	Vtb_order_book_parser_builder_arbiter \
	Vtb_order_book_parser_builder_arbiter___024root__DepSet_hb2f63005__0 \
	Vtb_order_book_parser_builder_arbiter___024root__DepSet_h5c65e5e5__0 \
	Vtb_order_book_parser_builder_arbiter___024root__DepSet_h5c65e5e5__1 \
	Vtb_order_book_parser_builder_arbiter___024root__DepSet_h5c65e5e5__2 \
	Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2__DepSet_h31436579__0 \
	Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2__DepSet_hd9111951__0 \
	Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8__DepSet_hc48aec07__0 \
	Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8__DepSet_h2ecaa1e7__0 \
	Vtb_order_book_parser_builder_arbiter__main \

# Generated module classes, non-fast-path, compile with low/medium optimization
VM_CLASSES_SLOW += \
	Vtb_order_book_parser_builder_arbiter__ConstPool_0 \
	Vtb_order_book_parser_builder_arbiter___024root__Slow \
	Vtb_order_book_parser_builder_arbiter___024root__DepSet_hb2f63005__0__Slow \
	Vtb_order_book_parser_builder_arbiter___024root__DepSet_h5c65e5e5__0__Slow \
	Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2__Slow \
	Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2__DepSet_h31436579__0__Slow \
	Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8_Bz2__DepSet_hd9111951__0__Slow \
	Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8__Slow \
	Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8__DepSet_hc48aec07__0__Slow \
	Vtb_order_book_parser_builder_arbiter_qty_book_wrapper__Q83_QB8__DepSet_h2ecaa1e7__0__Slow \

# Generated support classes, fast-path, compile with highest optimization
VM_SUPPORT_FAST += \

# Generated support classes, non-fast-path, compile with low/medium optimization
VM_SUPPORT_SLOW += \
	Vtb_order_book_parser_builder_arbiter__Syms \

# Global classes, need linked once per executable, fast-path, compile with highest optimization
VM_GLOBAL_FAST += \
	verilated \
	verilated_timing \
	verilated_threads \

# Global classes, need linked once per executable, non-fast-path, compile with low/medium optimization
VM_GLOBAL_SLOW += \


# Verilated -*- Makefile -*-
