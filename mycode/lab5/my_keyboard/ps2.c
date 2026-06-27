#include "gpio.h"
#include "gpio_extra.h"
#include "malloc.h"
#include "ps2.h"
#include "uart.h"

// A ps2_device is a structure that stores all of the state and information
// needed for a PS2 device. The clock field stores the gpio id for the
// clock pin, and the data field stores the gpio id for the data pin.
// Read ps2_new for example code that sets and uses these fields.
//
// You may extend the ps2_device structure with additional fields as needed.
// A pointer to the current ps2_device is passed into all ps2_ calls.
// Storing state in this structure is preferable to using global variables:
// it allows your driver to support multiple PS2 devices accessed concurrently
// (e.g., a keyboard and a mouse).
// 
// This definition fills out the structure declared in ps2.h.
struct ps2_device {
    gpio_id_t clock;
    gpio_id_t data;
};

// Creates a new PS2 device connected to given clock and data pins,
// The gpios are configured as input and set to use internal pull-up
// (PS/2 protocol requires clock/data to be high default)
ps2_device_t *ps2_new(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    // consider why must malloc be used to allocate device
    ps2_device_t *dev = malloc(sizeof(*dev));

    dev->clock = clock_gpio;
    gpio_set_input(dev->clock);
    gpio_set_pullup(dev->clock);

    dev->data = data_gpio;
    gpio_set_input(dev->data);
    gpio_set_pullup(dev->data);
    return dev;
}

static uint8_t read_bit(ps2_device_t *dev) {
    while (!gpio_read(dev->clock));
    while (gpio_read(dev->clock));
    return gpio_read(dev->data);
}

// Read a single PS2 scan code.
uint8_t ps2_read(ps2_device_t *dev) {
    while (0x107e) {

        // CHECK START BIT
        while (read_bit(dev));

        // READ CODE, UPDATE PARITY
        uint8_t parity = 0;
        uint8_t code = 0;
        for (int i=0; i<8; i++) {
            uint8_t bit = read_bit(dev);
            parity ^= bit;
            code |= (bit << i);
        }

        // CHECK PARITY BIT
        if (read_bit(dev) == parity) {
            continue;
        }
        
        // CHECK END BIT
        if (!read_bit(dev)) {
            continue;
        }

        return code;
    }
}
