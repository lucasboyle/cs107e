# Lucas Boyle
# CS 107E
# 1/22/26
# 
# File: timer_asm.s
# ------------------
# The file implements the timer_get_ticks method, which uses a CSR to access the tick
# count continuously incremented at a rate of 24 Mhz. 

.attribute arch, "rv64imac_zicsr"

.globl timer_get_ticks

timer_get_ticks:    
    csrr a0,time       # access CSR time, store in a0
    ret                # return value in a0
