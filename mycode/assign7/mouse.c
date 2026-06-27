/* Lucas Boyle
 * CS 107e
 * 3/4/26
 * 
 * File: mouse.c
 * -------------------
 * This file implements mouse.h, including functionality to initialize a PS2 mouse
 * and read an event that occurs. 
 */


#include "mouse.h"
#include "ps2.h"
#include "timer.h"
#include "interrupts.h"

// CREATE MODULE LEVEL VARIABLES TO TRACK THE MOUSE AND THE STATES
static ps2_device_t *mouse;
static bool prev_left;
static bool prev_right;
static bool prev_middle;


/*
 * This function will initialize a PS2 mouse, following the correct procedure to
 * reset and enable reporting. 
 */
void mouse_init(gpio_id_t clock, gpio_id_t data) {
    
    // CREATE A NEW PS2 DEVICE
    mouse = ps2_new(clock, data);

    // RESET (0xFF)
    ps2_write(mouse, 0xFF);

    interrupts_global_enable();
    ps2_read(mouse);    // READ 0xFA
    ps2_read(mouse);    // READ 0xAA
    ps2_read(mouse);    // READ 0x00
    interrupts_global_disable();

    // ENABLE REPORTING (0xF4)
    ps2_write(mouse, 0xF4);

    interrupts_global_enable();
    ps2_read(mouse);    // READ 0xFA
    interrupts_global_disable();
}


/*
 * This function will read a single event from the PS2 mouse, allowing the user
 * to get information such as the button presses/releases and mouse movement. 
 */
mouse_event_t mouse_read_event(void) {
    mouse_event_t event;
    
    // READ FIRST CODE, STORE BUTTON PRESSES, NEGATIVES, AND OVERFLOW
    uint8_t code = ps2_read(mouse);
    event.left = code & 1;
    event.right = (code >> 1) & 1;
    event.middle = (code >> 2) & 1;
    bool x_neg = (code >> 4) & 1;
    bool y_neg = (code >> 5) & 1;
    event.x_overflow = (code >> 6) & 1;
    event.y_overflow = (code >> 7) & 1;

    // READ SECOND CODE, STORE dx WITH APPROPRIATE NEGATION
    code = ps2_read(mouse);
    event.dx = code;
    if (event.x_overflow) {
        event.dx = 0;
    } else if (x_neg) {
        event.dx = -(256 - event.dx);
    }

    // READ THIRD CODE, STORE dy WITH APPROPRIATE NEGATION
    code = ps2_read(mouse);
    event.dy = code;
    if (event.y_overflow) {
        event.dy = 0;
    } else if (y_neg) {
        event.dy = -(256 - event.dy);
    }
    event.dy *= -1;     // NEGATIVE SINCE POSITIVE Y IS DOWNWARDS

    // DETERMINE THE CORRECT ACTION THAT OCCURRED
    mouse_action_t action = 0;
    bool button_change = (prev_left != event.left) || (prev_right != event.right) || (prev_middle != event.middle);
    bool moved = event.dx != 0 || event.dy != 0;

    if (button_change) {
        if (event.left || event.right || event.middle) {
            action = MOUSE_BUTTON_PRESS;
        } else {
            action = MOUSE_BUTTON_RELEASE;
        }
    } else if (moved) {
        if (event.left || event.right || event.middle) {
            action = MOUSE_DRAGGED;
        } else {
            action = MOUSE_MOVED;
        }
    }
    event.action = action;

    // SET MODULE LEVEL VARIABLES
    prev_left = event.left;
    prev_right = event.right;
    prev_middle = event.middle;    
    
    return event;
}