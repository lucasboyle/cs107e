/* Lucas Boyle
 * CS 107E
 * 1/22/26
 * 
 * File: gpio.c
 * ------------
 * This file implements the gpio module. Functions implemented here include setting
 * the function of GPIO ports, accessing the functions, writing to the data registers,
 * and accessing the values in the data registers. 
 */

#include "gpio.h"
#include <stddef.h>

enum { GROUP_B = 0, GROUP_C, GROUP_D, GROUP_E, GROUP_F, GROUP_G };

#define BASE_ADDRESS (volatile unsigned int *)(0x02000000)

typedef struct  {
    unsigned int group;
    unsigned int pin_index;
} gpio_pin_t;


typedef struct {
    volatile unsigned int cfg0;
    volatile unsigned int cfg1;
    volatile unsigned int cfg2;
    volatile unsigned int data_pad;         // DATA REGISTER IS OFFSET BY 4 BITS
    volatile unsigned int data;
    volatile unsigned int pull_pad[5];      // PULL REGISTER IS OFFSET BY 20 BITS
    volatile unsigned int pull;
} gpio_reg;


// The gpio_id_t enumeration assigns a symbolic constant for each
// in such a way to use a single hex constant. The more significant
// hex digit identifies the group and lower 2 hex digits are pin index:
//       constant 0xNnn  N = which group,  nn = pin index within group
//
// This helper function extracts the group and pin index from a gpio_id_t
// e.g. GPIO_PB4 belongs to GROUP_B and has pin_index 4
static gpio_pin_t get_group_and_index(gpio_id_t gpio) {
    gpio_pin_t gp;
    gp.group = gpio >> 8;
    gp.pin_index = gpio & 0xff; // lower 2 hex digits
    return gp;
} 


// The gpio groups are differently sized, e.g. B has 13 pins, C only 8.
// This helper function confirms that a gpio_id_t is valid (group
// and pin index are valid)
bool gpio_id_is_valid(gpio_id_t pin) {
    gpio_pin_t gp = get_group_and_index(pin);
    switch (gp.group) {
        case GROUP_B: return (gp.pin_index <= GPIO_PB_LAST_INDEX);
        case GROUP_C: return (gp.pin_index <= GPIO_PC_LAST_INDEX);
        case GROUP_D: return (gp.pin_index <= GPIO_PD_LAST_INDEX);
        case GROUP_E: return (gp.pin_index <= GPIO_PE_LAST_INDEX);
        case GROUP_F: return (gp.pin_index <= GPIO_PF_LAST_INDEX);
        case GROUP_G: return (gp.pin_index <= GPIO_PG_LAST_INDEX);
        default:      return false;
    }
}


// This helper function returns whether a given config register function is valid (between 0 and 15)
bool function_is_valid(unsigned int function) {
    return function >= 0 && function <= 15;
}


// This helper function is suggested to return the address of
// the config0 register for a gpio group, i.e. get_cfg0_reg(GROUP_B)
static volatile unsigned int *get_cfg0_reg(unsigned int group) {
    // NOTE: if multiple config registers, each has an addition offset of 0x0004
    switch(group) {
        case GROUP_B: return (BASE_ADDRESS + 0x0030/4); // 2 registers
        case GROUP_C: return (BASE_ADDRESS + 0x0060/4); // 1 register
        case GROUP_D: return (BASE_ADDRESS + 0x0090/4); // 3 registers
        case GROUP_E: return (BASE_ADDRESS + 0x00C0/4); // 2 registers
        case GROUP_F: return (BASE_ADDRESS + 0x00F0/4); // 1 register
        case GROUP_G: return (BASE_ADDRESS + 0x0120/4); // 2 registers
        default:      return BASE_ADDRESS;
    }
}


// This helper function is suggested to return the address of
// the data register for a gpio group. 
static volatile unsigned int *get_data_reg(unsigned int group) {
    switch(group) {
        case GROUP_B: return (BASE_ADDRESS + 0x0040/4);
        case GROUP_C: return (BASE_ADDRESS + 0x0070/4);
        case GROUP_D: return (BASE_ADDRESS + 0x00A0/4);
        case GROUP_E: return (BASE_ADDRESS + 0x00D0/4);
        case GROUP_F: return (BASE_ADDRESS + 0x0100/4);
        case GROUP_G: return (BASE_ADDRESS + 0x0130/4);
        default:      return BASE_ADDRESS;
    }
}


// This helper function returns the address of the pull register for a gpio group.  
static volatile unsigned int *get_pull_reg(unsigned int group) {
    switch(group) {
        case GROUP_B: return (BASE_ADDRESS + 0x0054/4);
        case GROUP_C: return (BASE_ADDRESS + 0x0084/4);
        case GROUP_D: return (BASE_ADDRESS + 0x00B4/4); // 2 registers
        case GROUP_E: return (BASE_ADDRESS + 0x00E4/4);
        case GROUP_F: return (BASE_ADDRESS + 0x0114/4);
        case GROUP_G: return (BASE_ADDRESS + 0x0144/4);
        default:      return BASE_ADDRESS;
    }
}


void gpio_init(void) {
    // no initialization required for this peripheral
}


// This function utilized gpio_set_function to set the function for a specified pin to input
void gpio_set_input(gpio_id_t pin) {
    gpio_set_function(pin, GPIO_FN_INPUT);
}


// This function utilized gpio_set_function to set the function for a specified pin to output
void gpio_set_output(gpio_id_t pin) {
    gpio_set_function(pin, GPIO_FN_OUTPUT);
}


/* This function takes in a pin and a function and sets the cooresponding configuration register
 * to the value of the function, maintaining the values in the other registers. 
 */
void gpio_set_function(gpio_id_t pin, unsigned int function) {

    // DETERMINE THAT THE PARAMETERS ARE VALID
    if (gpio_id_is_valid(pin) && function_is_valid(function)) {
        
        // DETERMINE DATA AT CONFIGURATION REGISTER
        gpio_pin_t gpio = get_group_and_index(pin);
        volatile unsigned int *config_reg = get_cfg0_reg(gpio.group);

        // SHIFT INTO CORRECT REGISTER IF NEEDED
        if (gpio.pin_index >= 16) {
            config_reg += 2;
            gpio.pin_index -= 16;
        } else if (gpio.pin_index >= 8) {
            config_reg += 1;
            gpio.pin_index -= 8;
        }

        // GET THE DATA FROM THE REGISTER
        unsigned int data = *config_reg;

        // CREATE AND APPLY A MASK TO REMOVE CURRENT CONTENTS AT INDEX
        unsigned int mask = 0xF;
        function = function << 4*gpio.pin_index;
        mask = mask << 4*gpio.pin_index;

        // PLACE THE FUNCTION IN THE CORRECT LOCATION
        data = (data & ~mask) | function;

        // STORE THE NEW DATA IN THE CONFIGURATION REGISTER
        *config_reg = data;
    }
}


// This function takes in a pin and returns the function of its cooresponding coniguration register
unsigned int gpio_get_function(gpio_id_t pin) {
    
    // DETERINE THAT THE PIN IS VALID
    if (gpio_id_is_valid(pin)) {

        // DETERMINE DATA AT CONFIGURATION REGISTER
        gpio_pin_t gpio = get_group_and_index(pin);

        // CREATE REG STRUCT FOR GROUP
        gpio_reg *reg = (gpio_reg *)(BASE_ADDRESS + (0x0030/4)*(gpio.group+1));
        
        // SHIFT INTO CORRECT REGISTER IF NEEDED
        unsigned int cfg_num;
        unsigned int pin_index = gpio.pin_index;
        if (gpio.pin_index >= 16) {
            cfg_num = 2;
            pin_index -= 16;
        } else if (gpio.pin_index >= 8) {
            cfg_num = 1;
            gpio.pin_index -= 8;
        } else {
            cfg_num = 0;
        }

        // GET CONFIG REGISTER
        volatile unsigned int *config_reg = &reg->cfg0 + cfg_num;

        // GET THE FUNCTION FROM THE CORRECT LOCATION IN THE DATA
        return (*config_reg >> 4*gpio.pin_index) & 0xF;
    }

    // IF THE PIN IS INVALID, RETURN THE GPIO_INVALID_REQUEST
    return GPIO_INVALID_REQUEST;
}


// This helper function determins whether a value to store in the data register is valid (0 or 1)
bool value_is_valid(int value) {
    return value == 0 || value == 1;
}


/* This function takes in a pin and an integer value and stores the value in the correct location in 
 * the cooresponding data register. 
 */
void gpio_write(gpio_id_t pin, int value) {

    // DETERMINE THAT THE PIN AND VALUE ARE VALID
    if (gpio_id_is_valid(pin) && value_is_valid(value)) {
        
        // DERTERMINE DATA AT THE DATA REGISTER
        gpio_pin_t gpio = get_group_and_index(pin);
        volatile unsigned int *data_reg = get_data_reg(gpio.group);

        // GET THE DATA FROM THE REGISTER
        unsigned int data = *data_reg;
        
        // CREATE AND APPLY A MASK TO REMOVE CURRENT CONTENTS AT INDEX
        unsigned int mask = 0x1;
        value = value << gpio.pin_index;
        mask = mask << gpio.pin_index;

        // PLACE THE VALUE AT THE CORRECT LOCATION
        data = (data & ~mask) | value;

        // STORE THE NEW DATA IN THE DATA REGISTER
        *data_reg = data;
    }
}


// This function take in a pin and returns the value at the cooresponding loation in the data register. 
int gpio_read(gpio_id_t pin) {
    
    // DETERMINE THAT THE PIN IS VALID
    if (gpio_id_is_valid(pin)) {
        
        // DETERMINE DATA AT THE DATA REGISTER
        gpio_pin_t gpio = get_group_and_index(pin);
        volatile unsigned int *data_reg = get_data_reg(gpio.group);

        // GET THE VALUE AT THE CORECT LOCATION IN THE DATA
        return (*data_reg >> gpio.pin_index) & 1;
    }

    // IF THE PIN IS INVALID, RETURN THE GPIO_INVALID_REQUEST
    return GPIO_INVALID_REQUEST;
}


/* This function takes in a pin and sets the cooresponding pull register to enable the pull-up
 * resistor for that pin, maintaining the values in the other registers. 
 */
void gpio_set_pullup(gpio_id_t pin) {
    
    // DETERMINE THAT THE PIN AND VALUE ARE VALID
    if (gpio_id_is_valid(pin)) {
        
        // DERTERMINE DATA AT THE PULL REGISTER
        gpio_pin_t gpio = get_group_and_index(pin);
        volatile unsigned int *pull_reg = get_pull_reg(gpio.group);


        // SHIFT INTO CORRECT REGISTER IF NEEDED
        if (gpio.pin_index >= 16) {
            pull_reg += 1;
            gpio.pin_index -= 16;
        }

        // GET THE DATA FROM THE REGISTER
        unsigned int data = *pull_reg;
        
        // CREATE AND APPLY A MASK TO REMOVE CURRENT CONTENTS AT INDEX
        unsigned int value = 0b01;
        unsigned int mask = 0b11;
        value = value << 2*gpio.pin_index;
        mask = mask << 2*gpio.pin_index;

        // PLACE THE VALUE AT THE CORRECT LOCATION
        data = (data & ~mask) | value;

        // STORE THE NEW DATA IN THE PULL REGISTER
        *pull_reg = data;
    }
}

