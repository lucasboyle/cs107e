/* Lucas Boyle
 * CS 107e
 * 3/4/26
 * 
 * File: ps2.c
 * -------------------
 * This file implements ps2.h, implementing functionality for creating a new 
 * ps2 device and reading a PS2 scancode with interrupts. It also supportes 
 * writing a scancode using the correct PS2 protocol. 
 */


#include "gpio.h"
#include "gpio_extra.h"
#include "malloc.h"
#include "ps2.h"
#include "timer.h"
#include "gpio_interrupt.h"
#include "interrupts.h"
#include "ringbuffer.h"


// THIS STRUCT HOLDS ALL INFORMATION ABOUT THE PS2 DEVICE, INCLUDING DATA, CLOCK, AND 
// SCANCODE INFORMATION
struct ps2_device {
    gpio_id_t clock;
    gpio_id_t data;
    uint8_t code;

    uint8_t start;
    uint8_t parity;
    uint8_t length;
    unsigned long ticks;

    rb_t *rb;
};


/*
 * This is the handler function for the ps2 interrupts. It will clear the interrupt when 
 * it comes in and correctly build onto the scancode. 
 */
static void handler(void *aux_data) {
    ps2_device_t *dev = (ps2_device_t *)aux_data;
    gpio_interrupt_clear(dev->clock);

    // CHECK START BIT
    if (dev->start == 1) {
        dev->start = gpio_read(dev->data);
        dev->code = dev->length = dev->parity = 0;
        
    // CHECK TIMING
    } else if ((timer_get_ticks() - dev->ticks) > 500 * TICKS_PER_USEC) {
        dev->start = gpio_read(dev->data);
        dev->code = dev->length = dev->parity = 0;

    // GET THE DATA BITS
    } else if (dev->length <= 8) {
        dev->parity ^= gpio_read(dev->data);
        dev->code |= (gpio_read(dev->data) << (dev->length-1));

    // CHECK THE PARITY BIT
    } else if (dev->length == 9) {
        if (gpio_read(dev->data) == dev->parity) {
            dev->start = gpio_read(dev->data);
            dev->code = dev->length = dev->parity = 0;
        }

    // CHECK THE END BIT, ENQUEUE IF ALL CORRECT
    } else if (dev->length == 10) {
        if (gpio_read(dev->data) == 0) {
            dev->start = gpio_read(dev->data);
            dev->code = dev->length = dev->parity = 0;

        } else {
            rb_enqueue(dev->rb, dev->code);
            dev->start = 1;
            dev->code = dev->length = dev->parity = 0;
        }
    }

    dev->length++;
    dev->ticks = timer_get_ticks();
}


/* 
 * This function created a new PS2 device connected to give clock and 
 * data pins. The gpios are configured as input and set to use internal 
 * pull-up. A ringbuffer is also created to store the order of the scancodes
 * that come in during interrupts. At the end of the function, interrupts
 * are enables for when the clock gpio goes low. 
 */
ps2_device_t *ps2_new(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    
    // INITIALIZE BASIC VARIABLES IN THE PS2 DEVICE
    ps2_device_t *dev = malloc(sizeof(*dev));
    dev->code = dev->length = dev->parity = 0;
    dev->ticks = timer_get_ticks();
    dev->start = 1;

    // SET CLOCK AND DATA TO PULLUP AND INPUT
    dev->clock = clock_gpio;
    gpio_set_input(dev->clock);
    gpio_set_pullup(dev->clock);

    dev->data = data_gpio;
    gpio_set_input(dev->data);
    gpio_set_pullup(dev->data);

    // CREATE THE RINGBUFFER
    rb_t *rb = rb_new();
    dev->rb = rb;

    // ENABLE GPIO INTERRUPTS FOR THE CLOCK
    gpio_interrupt_init();
    gpio_interrupt_config(dev->clock, GPIO_INTERRUPT_NEGATIVE_EDGE, false);
    gpio_interrupt_register_handler(dev->clock, handler, dev);
    gpio_interrupt_enable(dev->clock);

    return dev;
}


/*
 * This function takes in a PS2 device and waits for there to be something in the 
 * ringbuffer, returning the first element in the queue. 
 */
uint8_t ps2_read(ps2_device_t *dev) {
    while (rb_empty(dev->rb));
    return rb_dequeue(dev->rb);
}


/*
 * This function takes in a PS2 device and a scancode and will write the scancode to 
 * the device following the correct protocol. 
 */
bool ps2_write(ps2_device_t *dev, uint8_t command) {

    // DISABLE INTERRUPTS
    gpio_interrupt_disable(dev->clock);

    // WRITE CLOCK LOW FOR 150us
    gpio_set_output(dev->clock);
    gpio_write(dev->clock, 0);
    timer_delay_us(150);

    // PULL DATA LOW
    gpio_set_output(dev->data);
    gpio_write(dev->data, 0);

    // RELEASE CLOCK
    gpio_write(dev->clock, 1);
    gpio_set_input(dev->clock);
    gpio_set_pullup(dev->clock);

    // WAIT FOR CLOCK TO GO LOW
    while (gpio_read(dev->clock));

    // TRACK PARITY BIT
    int parity = 1;

    // WRITE THE DATA BITS
    for (int i = 0; i < 8; i++) {
        int bit = (command >> i) & 1;
        gpio_write(dev->data, bit);

        parity ^= bit;

        // WAIT FOR CLOCK TO GO HIGH AND THEN LOW
        while (!gpio_read(dev->clock));
        while (gpio_read(dev->clock));
    }

    // WRITE THE PARITY BIT
    gpio_write(dev->data, parity);

    // WAIT FOR CLOCK TO GO HIGH AND THEN LOW
    while (!gpio_read(dev->clock));
    while (gpio_read(dev->clock));

    // RELEASE DATA LINE
    gpio_write(dev->data, 1);
    gpio_set_input(dev->data);
    gpio_set_pullup(dev->data);

    // WAIT FOR DATA TO GO LOW
    while (gpio_read(dev->data));

    // WAIT FOR CLOCK TO GO LOW
    while (gpio_read(dev->clock));

    // WAIT FOR RELEASE OF DATA AND CLOCK
    while (!gpio_read(dev->clock) || !gpio_read(dev->data));

    // ENABLE INTERRUPTS
    gpio_interrupt_config(dev->clock, GPIO_INTERRUPT_NEGATIVE_EDGE, false);
    gpio_interrupt_register_handler(dev->clock, handler, dev);
    gpio_interrupt_enable(dev->clock);

    return true;
}