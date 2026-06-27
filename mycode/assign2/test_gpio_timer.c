/* Lucas Boyle
 * CS 107E
 * 1/22/26
 * 
 * File: test_gpio_timer.c
 * -----------------------
 * This file implements a testing framework for gpio.c and timer.c. It also implemenets a testing
 * framework for the breadboard connects. To test certain aspects, comment/uncomment the tests 
 * in main().
 */

#include "gpio.h"
#include "timer.h"

// You call assert on an expression that you expect to be true. If expr
// instead evaluates to false, then assert calls abort, which stops
// your program and flashes onboard led.
#define assert(expr) if(!(expr)) abort()

// infinite loop that flashes onboard blue LED (GPIO PD18)
void abort(void) {
    volatile unsigned int *GPIO_CFG2 = (unsigned int *)0x02000098;
    volatile unsigned int *GPIO_DATA = (unsigned int *)0x020000a0;

    // Configure GPIO PD18 function to be output.
    *GPIO_CFG2 = (*GPIO_CFG2 & ~(0xf00)) | 0x100;
    while (1) { // infinite loop
        *GPIO_DATA ^= (1 << 18); // invert value
        for (volatile int delay = 0x100000; delay > 0; delay--) ; // wait
    }
}


// This function includes a variety of tests for gpio_set_function() and gpio_get_function().
void test_gpio_set_get_function(void) {
    // Test get pin function (pin defaults to disabled)
    assert( gpio_get_function(GPIO_PC0) == GPIO_FN_DISABLED);

    // Set pin to input in PB register 0, confirm results
    gpio_set_input(GPIO_PB0);
    assert( gpio_get_function(GPIO_PB0) == GPIO_FN_INPUT );

    // Set pin to input in PB register 1, confirm results
    gpio_set_input(GPIO_PB9);
    assert( gpio_get_function(GPIO_PB9) == GPIO_FN_INPUT);

    // Set pin tp input in PC register 0, confirm results
    gpio_set_input(GPIO_PC1);
    assert( gpio_get_function(GPIO_PC1) == GPIO_FN_INPUT);

    // Set pin tp input in PD register 0, confirm results
    gpio_set_input(GPIO_PD6);
    assert( gpio_get_function(GPIO_PD6) == GPIO_FN_INPUT);

    // Set pin tp input in PD register 1, confirm results
    gpio_set_input(GPIO_PD12);
    assert( gpio_get_function(GPIO_PD12) == GPIO_FN_INPUT);

    // Set pin to input in PD register 2, confirm results
    gpio_set_input(GPIO_PD22);
    assert( gpio_get_function(GPIO_PD22) == GPIO_FN_INPUT);

    // Set pin to input in PE register 0, confirm results
    gpio_set_input(GPIO_PE2);
    assert( gpio_get_function(GPIO_PE2) == GPIO_FN_INPUT);

    // Set pin to input in PE register 1, confirm results
    gpio_set_input(GPIO_PE17);
    assert( gpio_get_function(GPIO_PE17) == GPIO_FN_INPUT);

    // Set pin to input in PF register 0, confirm results
    gpio_set_input(GPIO_PF6);
    assert( gpio_get_function(GPIO_PF6) == GPIO_FN_INPUT);

    // Set pin to input in PG register 0, confirm results
    gpio_set_input(GPIO_PG3);
    assert( gpio_get_function(GPIO_PG3) == GPIO_FN_INPUT);

    // Set pin to input in PG register 1, confirm results
    gpio_set_input(GPIO_PG8);
    assert( gpio_get_function(GPIO_PG8) == GPIO_FN_INPUT);

    // Test that all previous configurations have stayed constant
    assert( gpio_get_function(GPIO_PB0) == GPIO_FN_INPUT);
    assert( gpio_get_function(GPIO_PB9) == GPIO_FN_INPUT);
    assert( gpio_get_function(GPIO_PC1) == GPIO_FN_INPUT);
    assert( gpio_get_function(GPIO_PD6) == GPIO_FN_INPUT);
    assert( gpio_get_function(GPIO_PD12) == GPIO_FN_INPUT);
    assert( gpio_get_function(GPIO_PD22) == GPIO_FN_INPUT);
    assert( gpio_get_function(GPIO_PE2) == GPIO_FN_INPUT);
    assert( gpio_get_function(GPIO_PE17) == GPIO_FN_INPUT);
    assert( gpio_get_function(GPIO_PF6) == GPIO_FN_INPUT);
    assert( gpio_get_function(GPIO_PG3) == GPIO_FN_INPUT);
    assert( gpio_get_function(GPIO_PG8) == GPIO_FN_INPUT);

    // Test alt function
    gpio_set_function(GPIO_PG8, GPIO_FN_ALT3);
    assert( gpio_get_function(GPIO_PG8) == GPIO_FN_ALT3);

    // Test interrupt function
    gpio_set_function(GPIO_PD0, GPIO_FN_INTERRUPT);
    assert( gpio_get_function(GPIO_PD0) == GPIO_FN_INTERRUPT);

    // Test disabled function, also reconfiguring PD0
    gpio_set_function(GPIO_PD0, GPIO_FN_DISABLED);
    assert( gpio_get_function(GPIO_PD0) == GPIO_FN_DISABLED);

    // Test reconfiguring to output
    gpio_set_output(GPIO_PD0);
    assert( gpio_get_function(GPIO_PD0) == GPIO_FN_OUTPUT);

    // Test invalid pin
    assert( gpio_get_function(0x600) == GPIO_INVALID_REQUEST);

    // Test invalid function; ensure configuration doesn't change
    assert( gpio_get_function(GPIO_PD0) == GPIO_FN_OUTPUT);
    gpio_set_function(GPIO_PD0, 20);
    assert( gpio_get_function(GPIO_PD0) == GPIO_FN_OUTPUT);

    // Test invalid function; ensure configuration doesn't change
    assert( gpio_get_function(GPIO_PD0) == GPIO_FN_OUTPUT);
    gpio_set_function(GPIO_PD0, -5);
    assert( gpio_get_function(GPIO_PD0) == GPIO_FN_OUTPUT);
}


// This function includes a variety of tests for gpio_write() and gpio_read()
void test_gpio_read_write(void) {
    // set pins to output before gpio_write
    gpio_set_output(GPIO_PB4);
    gpio_set_output(GPIO_PC2);
    gpio_set_output(GPIO_PD3);
    gpio_set_output(GPIO_PE17);
    gpio_set_output(GPIO_PF6);
    gpio_set_output(GPIO_PG2);

    // gpio_write low, confirm gpio_read reads what was written
    gpio_write(GPIO_PB4, 0);
    assert( gpio_read(GPIO_PB4) ==  0 );

    // gpio_write high for PB, confirm gpio_read reads what was written
    gpio_write(GPIO_PB4, 1);
    assert( gpio_read(GPIO_PB4) ==  1 );

    // gpio_write high for PC, confirm gpio_read reads what was written
    gpio_write(GPIO_PC2, 1);
    assert( gpio_read(GPIO_PC2) ==  1 );

    // gpio_write high for PD, confirm gpio_read reads what was written
    gpio_write(GPIO_PD3, 1);
    assert( gpio_read(GPIO_PD3) ==  1 );

    // gpio_write high for PE, confirm gpio_read reads what was written
    gpio_write(GPIO_PE17, 1);
    assert( gpio_read(GPIO_PE17) ==  1 );

    // gpio_write high for PF, confirm gpio_read reads what was written
    gpio_write(GPIO_PF6, 1);
    assert( gpio_read(GPIO_PF6) ==  1 );

    // gpio_write high for PG, confirm gpio_read reads what was written
    gpio_write(GPIO_PG2, 1);
    assert( gpio_read(GPIO_PG2) ==  1 );

    // gpio_write low, confirm gpio_read reads what was written
    gpio_write(GPIO_PC2, 0);
    assert( gpio_read(GPIO_PC2) ==  0 );

    // gpio_write low, confirm gpio_read reads what was written
    gpio_write(GPIO_PD3, 0);
    assert( gpio_read(GPIO_PD3) ==  0 );

    // gpio_write low, confirm gpio_read reads what was written
    gpio_write(GPIO_PF6, 0);
    assert( gpio_read(GPIO_PF6) ==  0 );

    // Test that each state has remained constant
    assert( gpio_read(GPIO_PB4) ==  1 );
    assert( gpio_read(GPIO_PC2) ==  0 );
    assert( gpio_read(GPIO_PD3) ==  0 );
    assert( gpio_read(GPIO_PE17) ==  1 );
    assert( gpio_read(GPIO_PF6) ==  0 );
    assert( gpio_read(GPIO_PG2) ==  1 );

    // Test invalid pin
    assert( gpio_read(0x600) ==  GPIO_INVALID_REQUEST );

    // Test invalid value; nothing should change
    gpio_write(GPIO_PB4, -1 );
    assert( gpio_read(GPIO_PB4) ==  1 );
    gpio_write(GPIO_PB4, 2 );
    assert( gpio_read(GPIO_PB4) ==  1 );
}


// This function includes a variety of tests for the timer.c file. 
void test_timer(void) {
    // Test timer tick count incrementing
    unsigned long start = timer_get_ticks();
    for( int i=0; i<10; i++ ) { /* Spin */ }
    unsigned long finish = timer_get_ticks();
    assert( finish > start );

    // Test timer delay (us)
    int usecs = 100;
    start = timer_get_ticks();
    timer_delay_us(usecs);
    finish = timer_get_ticks();
    assert( finish >= start + usecs*TICKS_PER_USEC );

    // Test timer delay (ms)
    int msecs = 100;
    start = timer_get_ticks();
    timer_delay_ms(msecs);
    finish = timer_get_ticks();
    assert( finish >= start + msecs*1000*TICKS_PER_USEC );

    // Test timer delay (s)
    int secs = 1;
    start = timer_get_ticks();
    timer_delay(secs);
    finish = timer_get_ticks();
    assert( finish >= start + secs*1000000*TICKS_PER_USEC );
}


// This function provides a test for breadboard connecions. 
void test_breadboard_connections(void) {
    const int N_SEG = 7, N_DIG = 4;
    gpio_id_t segment[] = {GPIO_PD17, GPIO_PB6, GPIO_PB12, GPIO_PB11, GPIO_PB10, GPIO_PD11, GPIO_PD13};
    gpio_id_t digit[] = {GPIO_PB4, GPIO_PB3, GPIO_PB2, GPIO_PC0};
    gpio_id_t button = GPIO_PD12;

    for (int i = 0; i < N_SEG; i++) {   // configure segments & digits as output
        gpio_set_output(segment[i]);
    }
    for (int i = 0; i < N_DIG; i++) {
        gpio_set_output(digit[i]);
    }
    gpio_set_input(button);         // configure button

    while (1) { // loop forever
        for (int i = 0; i < N_DIG; i++) {   // iterate over digits
            gpio_write(digit[i], 1);        // turn on digit
            for (int j = 0; j < N_SEG; j++) {   // iterate over segments
                gpio_write(segment[j], 1);      // turn on segment
                timer_delay_ms(200);
                while (gpio_read(button) == 0)
                    ;                       // pause while button pressed
                gpio_write(segment[j], 0);  // turn off segment
            }
            gpio_write(digit[i], 0);    // turn off digit
        }
    }
}


void main(void) {
    gpio_init();
    timer_init();

    // Uncomment the call to each test function below when you have implemented
    // the functions and are ready to test them

    test_gpio_set_get_function();
    test_gpio_read_write();
    // test_timer();
    // test_breadboard_connections();
}
