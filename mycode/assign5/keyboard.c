/* Lucas Boyle
 * CS 107e
 * 2/19/25
 * 
 * File: keyboard.c
 * -----------------
 * This function implements keyboard.h, including functionality to read a 
 * PS2 keyboard scancode, sequence, event, and ultimately the next character
 * typed by the user. 
 */


#include "keyboard.h"
#include "ps2.h"


// CREATE STATICS TO HOLD DEVICE AND CURRENT MODIFIERS
static ps2_device_t *dev;
static keyboard_modifiers_t modifiers = 0;
static uint8_t caps_lock_registered = 0;


/*
 * This function initializes a PS2 keyboard.
 */
void keyboard_init(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    dev = ps2_new(clock_gpio, data_gpio);
}


/*
 * This function calls upon ps2_read() to reach the next scancode
 */
uint8_t keyboard_read_scancode(void) {
    return ps2_read(dev);
}


/*
 * This function returns the next sequence from a keyboard, returning a 
 * key_action_t that stores the scancode and press/release action.
 */
key_action_t keyboard_read_sequence(void) {

    // CREATE EMPTY key_action_t
    key_action_t action = { 0 };

    // GET FIRST SCAN CODE
    uint8_t first_code = keyboard_read_scancode();

    // CHECK TO SEE IF IT IS AN EXTENDED SCANCODE, STORE ACCORDINGLY
    if (first_code == 0xe0) {
        uint8_t second_code = keyboard_read_scancode();

        if(second_code == 0xf0) {
            action.what = KEY_RELEASE;
            action.keycode = keyboard_read_scancode();
        } else {
            action.what = KEY_PRESS;
            action.keycode = second_code;
        }
        
    // CHECK TO SEE IF IT IS A SINGLE RELEASED SCANCODE
    } else if (first_code == 0xf0) {
        action.what = KEY_RELEASE;
        action.keycode = keyboard_read_scancode();

    // CHECK TO SEE IF IT IS A SINGLE PRESSED SCANCODE
    } else {
        action.what = KEY_PRESS;
        action.keycode = first_code;
    }

    return action;
}


/*
 * This fucntion takes in a scancode and determines whether the code is
 * a modifier key (shift, ctrl, alt, caps lock), returning a bool. 
 */
static bool is_modifier(uint8_t code) {
    uint8_t modifiers[] = {0x12, 0x59, 0x14, 0x11, 0x58};

    for (int i=0; i<sizeof(modifiers); i++) {
        if (code == modifiers[i]) {
            return true;
        }
    }

    return false;
}


/*
 * This function takes in a scancode (precondition: code is a modifier)
 * and adjusts the modifiers to add whichever modifier was pressed. 
 */
static void press_modifier(uint8_t code) {

    if (code == 0x12 || code == 0x59) {
        modifiers |= KEYBOARD_MOD_SHIFT;
    } else if (code == 0x14) {
        modifiers |= KEYBOARD_MOD_CTRL;
    } else if (code == 0x11) {
        modifiers |= KEYBOARD_MOD_ALT;
    } else if (!caps_lock_registered) {
        modifiers ^= KEYBOARD_MOD_CAPS_LOCK;    // ALRERNATE CAPS LOCK
        caps_lock_registered = 1;
    }
}


/*
 * This function takes in a scancode (precondition: code is a modifier)
 * and adjusts the modifiers to add whichever modifier was released. 
 */
static void release_modifier(uint8_t code) {

    if (code == 0x12 || code == 0x59) {
        modifiers &= ~KEYBOARD_MOD_SHIFT;
    } else if (code == 0x14) {
        modifiers &= ~KEYBOARD_MOD_CTRL;
    } else if (code == 0x11) {
        modifiers &= ~KEYBOARD_MOD_ALT;
    } else {
        caps_lock_registered = 0;
    }
}


/*
 * This function returns the next event from a keyboard, storing the action,
 * key, and modifiers present for a single key press or release.
 */
key_event_t keyboard_read_event(void) {

    // CREATE EMPTY EVENT AND GET FIRST SEQUENCE
    key_event_t event = { 0 };
    key_action_t action = keyboard_read_sequence();

    // LOOP WHILE ONLY MODIFIERS ARE PRESSED, UPDATE MODIFIERS
    while (is_modifier(action.keycode)) {
        if (action.what == KEY_PRESS) {
            press_modifier(action.keycode);
        } else {
            release_modifier(action.keycode);
        }
        action = keyboard_read_sequence();
    }

    // UPDATE EVENT AND RETURN
    event.action = action;
    event.key = ps2_keys[action.keycode];
    event.modifiers = modifiers;
    return event;
}


/*
 * This function returns the char associated with a single key press and/or
 * release, adding any modifications from the previously pressed modifiers. 
 */
char keyboard_read_next(void) {
    
    // GET THE EVENT
    key_event_t event = keyboard_read_event();

    // LOOP UNTIL A KEY IS PRESSED
    while (event.action.what == KEY_RELEASE) {
        event = keyboard_read_event();
    }

    // RETURN SHIFTED OR CTRL VERSION OF KEY IF NECESSARY
    if (event.key.ch <= 0x7f) {
        if (event.modifiers & KEYBOARD_MOD_CTRL && (event.key.ch == 'a' || event.key.ch == 'd' || event.key.ch == 'e')) {
            return event.key.ch - 'a' + 1;
        } else if (event.modifiers & KEYBOARD_MOD_SHIFT) {
            return event.key.other_ch;
        } else if ((event.modifiers & KEYBOARD_MOD_CAPS_LOCK) && (event.key.ch >= 'a') && (event.key.ch <= 'z')) {
            return event.key.other_ch;
        } 
    } 

    // RETURN REGULAR VERSION OF KEY
    return event.key.ch;
}
