/* Lucas Boyle
 * CS 107e
 * 2/8/26
 *
 * File: backtrace.c
 * -----------------
 * This file implements backtrace.h, providing functionality to gather the frames for 
 * a backtrace, print them out using printf, and detect if the stack has been 
 * "smashed".
 */

 
#include "backtrace.h"
#include "mango.h"
#include "printf.h"
#include "symtab.h"


// HELPER FUNCTION IMPLEMENTED IN FILE backtrace_asm.s
extern unsigned long backtrace_get_fp(void);


/*
 * This function takes in an array of frame_t and a maximum number of frames
 * and will store the return addresses for each function in the backtrace, 
 * up to max_frames backwards from the current frame. 
 */
int backtrace_gather_frames(frame_t f[], int max_frames) {
    size_t n = 0;

    // GET THE FP FOR THE CURRENT FUNCTION CALL
    unsigned long *curr_fp = (unsigned long *)backtrace_get_fp();

    // WHILE THERE ARE MORE FRAMES TO TRAVERSE AND WE ARE UNDER max_frames
    while ((curr_fp != NULL) && (n < max_frames)) {
        
        // THE SAVED RA IS LOCATED AT 8 BYTES BELOW THE FP
        f[n].resume_addr = *(curr_fp - 1);
        n++;

        // THE SAVED FP IS LOCATED 16 BYTES BELOW THE CURRENT FP
        curr_fp = (unsigned long *)*(curr_fp - 2);
    }

    return n;
}


/*
 * This function takes in an array of frame_t and a number of frames, and it
 * will use printf to print each of the frames with their label names. 
 */
void backtrace_print_frames(frame_t f[], int n) {
    char labelbuf[128];
    for (int i = 0; i < n; i++) {
        symtab_label_for_addr(labelbuf, sizeof(labelbuf), f[i].resume_addr);
        printf("#%d 0x%08lx at %s\n", i, f[i].resume_addr, labelbuf);
    }
}


/*
 * This function is the public interface to print the entire backtrace.
 */
void backtrace_print(void) {
   
    // SET MAXIMUM NUMBER OF FRAMES
    int max = 50;
    frame_t arr[max];

    // CALL HELPER FUNCTION TO GATHER FRAMES AND PRINT ENTIRE BACKTRACE
    int n = backtrace_gather_frames(arr, max);
    backtrace_print_frames(arr+1, n-1);
}


// SET CANARY VALUE FOR STACK GAURD TO DETECT (RANDOM VALUE)
long __stack_chk_guard = 240256;


/*
 * This function will be called whwen the StackGaurd detects a stack overflow. It
 * will determine which function caused the stack overflow, print it out, and 
 * put the program in hang so that it cannot proceed. 
 */
void __stack_chk_fail(void)  {

    // GET THE ra
    unsigned long *fp = (unsigned long *)backtrace_get_fp();
    unsigned long saved_ra = *(fp - 1) - 8;     // OFFSET FOUND THROUGH GDB

    // GET THE NAME OF THE FUNCTION
    symbol_t function;
    symtab_symbol_for_addr(saved_ra, &function);

    // PRINT DETAILS AND HANG
    printf("\n *** Stack smashing detected at end of function %s() *** \n", function.name);
    mango_abort();
}
