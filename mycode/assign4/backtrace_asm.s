# Lucas Boyle 
# CS 107e
# 2/8/26
#
# File: backtrace_asm.s
# ---------------------
# This file provides a function to get the current frame pointer in 
# the runtime of the program. 

.globl backtrace_get_fp
backtrace_get_fp:
    mv a0,s0        # COPY THE fp TO a0 TO BE RETURNED
    ret