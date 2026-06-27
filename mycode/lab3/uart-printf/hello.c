#include "printf.h"
#include "timer.h"
#include "uart.h"

void main (void) {
    volatile unsigned int *pb_config0 = (unsigned int *)0x02000030;

    uart_init();    // must set up uart peripheral before using, init once

    for (int i = 0; i < 5; i++) {
        printf("Iteration %d: ", i);
        uart_putstring("hello, laptop\n");
        timer_delay(1);
    }
    printf("We %s printf!\n", "<3");

    printf("Value of pb_config0: 0x%08x\n", *pb_config0);

    gpio_set_output(GPIO_PB3);
    gpio_set_output(GPIO_PB7);
    printf("Value of pb_config0: 0x%08x\n", *pb_config0);

    gpio_set_input(GPIO_PB3);
    gpio_set_input(GPIO_PB7);
    printf("Value of pb_config0: 0x%08x\n", *pb_config0);

}
