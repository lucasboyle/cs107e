/* Lucas Boyle
 * CS 107e
 * 3/4/26
 * 
 * File: test_interrupts.c
 * -----------------------
 * This file provides a testing framework for ps2 with interrupts enabled. 
 */


#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupt.h"
#include "keyboard.h"
#include "printf.h"
#include "timer.h"
#include "uart.h"
#include "ps2.h"
#include "assert.h"
#include "mouse.h"


/*
 * This function tests the behavior of the assign5 ps2
 * implementation versus the new-improved assign7 version. If using the
 * ps2 module as written for assign5, a scancode that arrives while the main program
 * is waiting in delay is simply dropped. Once you upgrade your
 * ps2 module for assign7 to be interrupt-driven, those scancodes should
 * be queued up and can be read after delay finishes.
 */
static void check_read_delay(void) {
    keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);
    printf("\n%s() will read each typed key and pause for a few seconds\n", __func__);
    while (1) {
        printf("\nType a key on PS/2 keyboard (q to quit): ");
        char ch = keyboard_read_next();
        printf("\nRead: %c\n", ch);
        if (ch == 'q') break;
        printf("Pausing for 2 seconds (type ahead now on PS/2 keyboard to queue events)... ");
        timer_delay(2);
        printf("done.\n");
    }
}


/*
 * This function is a interrupt handler for testing, printing a digit for each 
 * bit in the scancode that is produced by a single keypress. 
 */
static void clock_edge(void *clientData) {
    static int count = 0;
    gpio_interrupt_clear(KEYBOARD_CLOCK);
    uart_putchar('0' + count); // PRINT DIGIT
    if (++count == 11) { // AFTER ONE SCANCODE, RESET COUNTER
        count = 0;
        uart_putchar('\n');
    }
}


/*
 * This function is a simple test to confirm interrupts are being generated
 * and that the interrupt handler is being called. It configures interrupts
 * on the clock gpio using the handler function above a counter.
 */
static void check_interrupts_received(void) {
    gpio_interrupt_init();
    gpio_interrupt_config(KEYBOARD_CLOCK, GPIO_INTERRUPT_NEGATIVE_EDGE, false);
    gpio_interrupt_register_handler(KEYBOARD_CLOCK, clock_edge, NULL);
    gpio_interrupt_enable(KEYBOARD_CLOCK);
    interrupts_global_enable();

    printf("\n%s() waiting for interrupts on keyboard clock gpio\n", __func__);
    printf("Type on your PS/2 keyboard. I'll wait for 5 seconds...\n");
    timer_delay(5);
    printf("Time's up!\n");
}


// DEFINE SCANCODE FOR ESCAPE KEY
#define ESC_SCANCODE 0x76


/*
 * This function provides tests to print single scancodes entered by a 
 * PS2 keyboard.
 */
static void test_keyboard_scancodes(void) {
    printf("\nNow reading single scancodes. Type ESC to finish this test.\n");
    while (1) {
        uint8_t scancode = keyboard_read_scancode();
        printf("[%02x]\n", scancode);
        if (scancode == ESC_SCANCODE) break;
    }
    printf("\nDone with scancode test.\n");
}


/*
 * This function provides tests to print a sequence produced by a PS2
 * keyboard.
 */
static void test_keyboard_sequences(void) {
    printf("\nNow reading scancode sequences (key actions). Type ESC to finish this test.\n");
    while (1) {
        key_action_t action = keyboard_read_sequence();
        printf("%s [%02x]\n", action.what == KEY_PRESS ? "  Press" :"Release", action.keycode);
        if (action.keycode == ESC_SCANCODE) break;
    }
    printf("Done with scancode sequences test.\n");
}


/*
 * This function provides tests to print an event produced by a PS2
 * keyboard.
 */
static void test_keyboard_events(void) {
    printf("\nNow reading key events. Type ESC to finish this test.\n");
    while (1) {
        key_event_t evt = keyboard_read_event();
        printf("%s PS2_key: {%c,%c} Modifiers: 0x%x\n", evt.action.what == KEY_PRESS? "  Press" : "Release", evt.key.ch, evt.key.other_ch, evt.modifiers);
        if (evt.action.keycode == ESC_SCANCODE) break;
    }
    printf("Done with key events test.\n");
}


/*
 * This function provides tests to print the character with modifiers that 
 * is produced by a PS2 keyboard. 
 */
static void test_keyboard_chars(void) {
    printf("\nNow reading chars. Type ESC to finish this test.\n");
    while (1) {
        char c = keyboard_read_next();
        if (c >= '\t' && c <= 0x80)
            printf("%c", c);
        else
            printf("[%02x]", c);
        if (c == ps2_keys[ESC_SCANCODE].ch) break;
    }
    printf("\nDone with key chars test.\n");
}


/*
 * This function provides tests that wait for the user to enter various
 * requests on a PS2 keyboard and includes assertions to ensure the program
 * accuratly converts the input. 
 */
static void test_keyboard_assert(void) {
    char ch;

    // TEST SHIFT
    printf("\nHold down Shift and type 'a'\n");
    ch = keyboard_read_next();
    assert(ch == 'A'); 

    // TEST CAPS LOCK
    printf("\nPress Caps Lock and type 'b'\n");
    ch = keyboard_read_next();
    assert(ch == 'B'); 

    // TEST THAT CAPS LOCK DOES NOT AFFECT NUMBERS
    printf("\nPress Caps Lock and type '1'\n");
    ch = keyboard_read_next();
    assert(ch == '1'); 

    // TEST SHIFT PRECIDENCE
    printf("\nPress Caps Lock, hold down Shift, and type '2'\n");
    ch = keyboard_read_next();
    assert(ch == '@'); 

    // TEST EXTENDED KEYS
    printf("\nPress 'F1'\n");
    ch = keyboard_read_next();
    assert(ch == PS2_KEY_F1); 

    // TEST ENTER
    printf("\nPress 'Enter'\n");
    ch = keyboard_read_next();
    assert(ch == '\n'); 
}


/* 
 * This function provides tests for the mouse, printing out all data assiciated 
 * with an event. 
 */
static void test_mouse(void) {
    while (1) {
        mouse_event_t event = mouse_read_event();
        printf("Left: %d\t\tMiddle: %d\tRight: %d\n", event.left, event.middle, event.right);
        printf("dx: %d\t\tdy: %d\t\tMode: %d\n\n", event.dx, event.dy, event.action);
        timer_delay_ms(1);
    }
}


/*
 * This function initializes each component needed for the tests and allows the user
 * to easily comment/uncomment the features they want to test. 
 */
void main(void) {
    gpio_init();
    timer_init();
    uart_init();
    printf("\nStarting main() in %s\n", __FILE__);
    interrupts_init();
    // ps2_new(KEYBOARD_CLOCK, KEYBOARD_DATA);
    mouse_init(MOUSE_CLOCK, MOUSE_DATA);
    interrupts_global_enable();

    // check_interrupts_received();
    // check_read_delay();

    // keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);

    // test_keyboard_scancodes();

    // test_keyboard_sequences();

    // test_keyboard_events();

    // test_keyboard_chars();

    // test_keyboard_assert();
    
    test_mouse();

    printf("\nCompleted execution of main() in %s\n", __FILE__);
}
