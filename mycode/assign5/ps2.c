/* Lucas Boyle
 * CS 107e
 * 2/25/26
 * 
 * File: ps2_assign5.c
 * -------------------
 * This file implements ps2.h, implementing functionality for creating a new 
 * ps2 device and reading a PS2 scancode. 
 */


#include "gpio.h"
#include "gpio_extra.h"
#include "malloc.h"
#include "ps2.h"
#include "timer.h"


// THIS STRUCT HOLDS THE CLOCK AND DATA GPIO DATA
struct ps2_device {
    gpio_id_t clock;
    gpio_id_t data;
};


/* 
 * This function created a new PS2 device connected to give clock and 
 * data pins. The gpios are configured as input and set to use internal 
 * pull-up.
 */
ps2_device_t *ps2_new(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    ps2_device_t *dev = malloc(sizeof(*dev));

    dev->clock = clock_gpio;
    gpio_set_input(dev->clock);
    gpio_set_pullup(dev->clock);

    dev->data = data_gpio;
    gpio_set_input(dev->data);
    gpio_set_pullup(dev->data);
    return dev;
}


/*
 * This function takes in a PS2 device and returns the next data bit that is 
 * read when the clock goes low. 
 */
static uint8_t read_bit(ps2_device_t *dev) {
    while (!gpio_read(dev->clock));
    while (gpio_read(dev->clock));
    return gpio_read(dev->data);
}


/*
 * This function takes in a PS2 device and reads a single PS2 scancode. It
 * always returns a correctly received scancode. If an error occurs, the 
 * function will read another scancode. 
 */
uint8_t ps2_read(ps2_device_t *dev) {

    // READ INITIAL BIT
    uint8_t bit = read_bit(dev);

start:
    // CHECK START BIT
    if (bit) {
        bit = read_bit(dev);
        goto start;
    }

    // READ CODE, UPDATE PARITY
    uint8_t parity = 0;
    uint8_t code = 0;

    // STORE CURRENT TICK COUNT
    unsigned long ticks = timer_get_ticks();

    // LOOP THROUGH EACH DATA BIT
    for (int i=0; i<8; i++) {
        bit = read_bit(dev);

        // IF 500 us PASSED SINCE THE LAST READ, RESTART THE LOOP
        if ((timer_get_ticks() - ticks) > 500 * TICKS_PER_USEC) {
            goto start;
        }

        ticks = timer_get_ticks();

        // STORE PARITY AND CODE
        parity ^= bit;
        code |= (bit << i);
    }

    // CHECK PARITY BIT
    bit = read_bit(dev);
    if (bit == parity) {
        goto start;
    }

    // TIMING CHECK
    if ((timer_get_ticks() - ticks) > 500 * TICKS_PER_USEC) {
        goto start;
    }

    ticks = timer_get_ticks();
    
    bit = read_bit(dev);
    // CHECK END BIT
    if (!bit) {
        goto start;
    }

    // TIMING CHECK
    if ((timer_get_ticks() - ticks) > 500 * TICKS_PER_USEC) {
        goto start;
    }

    return code;
}