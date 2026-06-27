/* Lucas Boyle 
 * CS 107e
 * 2/19/26
 * 
 * File: test_keyboard_shell.c
 * ---------------------------
 * This file provides a testing framework for keyboard.c and shell.c.
 */


#include "assert.h"
#include "keyboard.h"
#include "printf.h"
#include "shell.h"
#include "strings.h"
#include "uart.h"
#include "timer.h"


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
 * This function provides tests that ensure the shell correctly evaluates
 * the user's inputs, including filtering any whitespace. 
 */
static void test_shell_evaluate(void) {
    shell_init(keyboard_read_next, printf);

    // TEST ECHO
    printf("\nTest shell_evaluate on fixed commands.\n");
    int ret = shell_evaluate("echo hello, world!");
    printf("Command result is zero if successful, is it? %d\n", ret);

    printf("\nPress any key to continue.\n");
    keyboard_read_next();

    // TEST HELP
    printf("\nManually inspect that help is shown:\n");
    ret = shell_evaluate("help");

    printf("\nPress any key to continue.\n");
    keyboard_read_next();

    // TEST POKE WITH SPACES
    printf("\nCheck that blue LED on mango PI is now off.\n");
    ret = shell_evaluate("poke 0x20000a0 0");

    printf("\nPress any key to continue.\n");
    keyboard_read_next();

    printf("\nCheck that blue LED on mango PI is now on.\n");
    ret = shell_evaluate("poke    0x20000a0          0x40000");

    printf("\nPress any key to continue.\n");
    keyboard_read_next();

    // TEST PEEK WITH SPACED
    printf("\nVerify that 40000 is outputted.\n");
    ret = shell_evaluate("       peek 0x20000a0");

    printf("\nPress any key to continue.\n");
    keyboard_read_next();

    // TEST ECHO WITH SPACED
    printf("\nCheck that 'Hello, World!' is printed out correctly.\n");
    ret = shell_evaluate("echo       Hello, World!    ");

    printf("\nPress any key to clear and exit.\n");
    keyboard_read_next();
    ret = shell_evaluate("clear");
}


/*
 * This function is a mock input used for testing, returing the next character
 * from the input string each time the function is called. 
 */
static char read_fixed(void) {
    const char *input = "\b\b\becho hello, world\nhelp\npeek 0x20000000\b\b40 and you won't see this...\n";
    static int index = 0; // NOTE: DECLARED STATIC

    char next = input[index];
    index = (index + 1) % strlen(input);
    return next;
}


/*
 * This function provides tests for reading a fixed input from the shell. 
 */
static void test_shell_readline_fixed_input(void) {
    char buf[20];
    size_t bufsize = sizeof(buf);

    shell_init(read_fixed, printf); // input is fixed sequence of characters

    printf("\nTest shell_readline, feed chars from fixed string as mock input.\n");

    // BACKSPACE AT THE BEGINNING SHOULD TRIGGER shell_bell()
    printf("readline> ");
    shell_readline(buf, bufsize);

    printf("readline> ");
    shell_readline(buf, bufsize);
    
    // BUFFER SIZE SHOULD BE REACHED, ONLY A FEW CHARACTER WILL BE PRINTED
    printf("readline> ");
    shell_readline(buf, bufsize);
}


/*
 * This function provides tests for the user to enter a single line in shell
 * to ensure that the formatting and rules for display are correct. 
 */
static void test_shell_readline_keyboard(void) {
    char buf[80];
    size_t bufsize = sizeof(buf);

    shell_init(keyboard_read_next, printf);

    printf("\nTest shell_readline, type a line of input on ps2 keyboard.\n");
    printf("? ");
    shell_readline(buf, bufsize);
}


/*
 * This function allows you to selectively uncomment which test you want to run. 
 */
void main(void) {
    uart_init();
    keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);

    printf("Testing keyboard and shell.\n");

    // test_keyboard_scancodes();
    // timer_delay_ms(500);

    // test_keyboard_sequences();
    // timer_delay_ms(500);

    test_keyboard_events();
    timer_delay_ms(500);

    // test_keyboard_chars();

    // test_keyboard_assert();

    // test_shell_evaluate();

    // test_shell_readline_fixed_input();

    // test_shell_readline_keyboard();

    printf("Finished executing main() in test_keyboard_shell.c\n");
}
