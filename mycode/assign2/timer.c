/* Lucas Boyle
 * CS 107E
 * 1/22/26
 *
 * File: timer.c
 * -------------
 * This file implements the timer module. Functions implemented here include getting the ticks of 
 * the internal timer and creating a delay in microseconds, milliseconds, or seconds. 
 */

#include "timer.h"

// Implemented in file timer_asm.s
extern unsigned long timer_get_ticks(void);

void timer_init(void) {
    // no initialization required for this peripheral
}

// Creates a delay for an inputted number of seconds
void timer_delay(int secs) {
    timer_delay_us(secs*1000000);
}

// Creates a delay for an inputted number of milliseconds.
void timer_delay_ms(int ms) {
    timer_delay_us(ms*1000);
}

// Creates a delay for an inputted number of microseconds.
void timer_delay_us(int us) {
    unsigned long busy_wait_until = timer_get_ticks() + us * TICKS_PER_USEC;
    while (timer_get_ticks() < busy_wait_until) {} // spin
}