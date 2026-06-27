/* Lucas Boyle
 * CS 107E
 * 1/24/26
 *
 * File: clock.c
 * -------------
 * This file implements functionality for a timer, representing the time on a display
 * using the first two digits as the minutes and the second two as the seconds. When the
 * timer goes off, the display flashes and a buzzer sounds. To adjust the sound, a rotary
 * encoder is used to increase or decrease the time before starting the countdown. The GPIO
 * pins used for the display, buttons, buzzer, and rotary encoder are all specified in 
 * `pinout.py clock`. 
 */

#include "gpio.h"
#include "gpio_extra.h"
#include "timer.h"

/* __Note__: Do not edit/remove the three lines of code below.
 * These preprocessor directives cooperate with Makefile to allow
 * setting DURATION as part of the build process. You can test your
 * clock application on the duration of your choice by specifying
 * value when invoking make like so
 *       make run DURATION=30
 * The value for DURATION is expressed in seconds and defaults to
 * 67 seconds if not explicitly set. (i.e. 1 min 7 seconds)
 */
#ifndef DURATION
#define DURATION 67
#endif

// SET CONSTANTS FOR THE NUMBER OF SEGMENTS AND DIGITS
static const int N_SEG = 7, N_DIG = 4;

// SET CONSTANTS TO HOLD GPIO PINS FOR SEGEMENTS, DIGITS, BUTTONS, BUZZER, AND ROTARY ENCODER
static const gpio_id_t segment[] = {GPIO_PD17, GPIO_PB6, GPIO_PB12, GPIO_PB11, GPIO_PB10, GPIO_PD11, GPIO_PD13};
static const gpio_id_t digit[] = {GPIO_PB4, GPIO_PB3, GPIO_PB2, GPIO_PC0};
static const gpio_id_t button = GPIO_PD12;
static const gpio_id_t buzzer = GPIO_PB0;
static const gpio_id_t rotary[] = {GPIO_PB9, GPIO_PB8};

// REPRESENT NUMBERS 0-9
static const unsigned char numbers[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
                                        0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};


/* This function initialized all of the gpio pins, setting the segments and digits to output,
 * the button to input, and the buzzer to output. 
 */
void set_gpio() {
    
    for (int i=0; i<N_SEG; i++) {
        gpio_set_output(segment[i]);
    }

    for (int i=0; i<N_DIG; i++) {
        gpio_set_output(digit[i]);
    }

    gpio_set_input(button);
    gpio_set_output(buzzer);

    gpio_set_input(rotary[0]);
    gpio_set_input(rotary[1]); 
    gpio_set_pullup(rotary[0]);
    gpio_set_pullup(rotary[1]);

}


/* This function takes in a gpio_id_t that represents a certain digit. The function writes 
 * all of the segments to 0 and turns off the inputting digit. 
 */
void clear_digit(gpio_id_t digit) {
    
    gpio_write(digit, 0);

    for (int i=0; i<N_SEG; i++) {
        gpio_write(segment[i], 0);
    }
}


/* This function takes in a gpio_id_t that represents a certain digit and a char represntation
 * of a number to display. The function then writes the correct segments pattern and displays
 * it on the correct digit. 
 */
void display_digit(gpio_id_t digit, char num) {
    
    // CLEAR ANY DATA CURRENTLY ON THE DIGIT, TURN IT OFF
    clear_digit(digit);
    gpio_write(digit,0);

    // WRITE 1 TO THE CORRECT SEGMENTS
    for (int i=0; i<N_SEG; i++) {
        if ((num & 1) == 1) {
            gpio_write(segment[i], 1);
        }

        num = num >> 1;
    }

    // TURN THE DIGIT ON
    gpio_write(digit, 1);
}


/* The function takes in a number to display and a length to display it in milliseconds. The 
 * function will rapidly loop through each digit, turning on the correct segments and off after
 * 1 ms to create the illusion that each digit is showing a different number. Numbers up to 
 * 9999 can be displayed. 
 */
void display_number(int num, int length_ms) {
    
    // DETERMINE THE TIME TO STOP DISPLAYING THE NUMBER
    unsigned long end_time = timer_get_ticks() + 1000 * length_ms * TICKS_PER_USEC;

    // STORE THE NUMBER TO DISPLAY IN A VARIABLE THAT IS ABLE TO BE CHANGED
    int temp = num;

    // WHILE THERE IS STILL TIME TO DISPLAY THE NUMBER...
    while (timer_get_ticks() < end_time) {

        // RESET TEMP TO NUM
        temp = num;

        // LOOP THROUGH EACH DIGIT, DISPLAY CORRECT SEGMENTS FOR 1ms
        for (int i=3; i>=0; i--) {
            display_digit(digit[i], numbers[temp%10]);
            timer_delay_ms(1);
            clear_digit(digit[i]);
            temp /= 10;
        }
    }
}


/* This function takes in a note character and a length in milliseconds. The function plays
 * the corresponding tone on the buzzer for the specified length of time. 
 */
void play_tone(char note, int length_ms) {

    // DETERMINE THE TIME TO STOP DISPLAYING THE NUMBER
    unsigned long end_time = timer_get_ticks() + 1000 * length_ms * TICKS_PER_USEC;

    // DETERMINE FREQUENCY OF NOTE
    int hz;
    switch (note) {
        case 'A':
            hz = 440;
            break;
        case 'B':
            hz = 493;
            break;
        case 'C':
            hz = 261;
            break;
        case 'D':
            hz = 294;
            break;
        case 'E':
            hz = 329;
            break;
        case 'F':
            hz = 349;
            break;
        case 'G':
            hz = 392;
            break;
        default:
            hz = 0;
    }
 
    // PLAY THE TONE UNTIL THE END TIME IS REACHED
    while(timer_get_ticks() < end_time) {
        gpio_write(buzzer, 1);
        timer_delay_us(1000000/(hz*2));
        gpio_write(buzzer, 0);
        timer_delay_us(1000000/(hz*2));
    }
}


// This function plays a simple song on the buzzer. 
void play_song() {
    char song[] = "CCGGAAGFFEEDDCGGFFEEDGGFFEEDCCGGAAGFFEEDDC";
    int song_length = sizeof(song)/sizeof(song[0]);

    for (int i = 0; i < song_length; i++) {
        play_tone(song[i], 500);
        timer_delay_ms(50);
    }
}


// This function calls play_song() and flashes the display for when the countdown ends
void end_sequence() {

    play_song();

    while (true) {
        display_number(0, 500);
        clear_digit(digit[0]);
        clear_digit(digit[1]);
        clear_digit(digit[2]);
        clear_digit(digit[3]);
        timer_delay_ms(500);
    }
}


// Main function
int main(void) {

    // STORE THE DURATION OF THE TIMER
    int countdown = DURATION;

    // INITIALIZE ALL GPIO PINS
    set_gpio();

    // STORE NUMBER OF MINUTES AND SECONDS, DETERMINE NUMBER TO DISPLAY
    int minutes = countdown / 60;
    int seconds = countdown % 60;
    int num_to_display = minutes*100 + seconds;

    unsigned int last_a_state = gpio_read(rotary[0]);

    // DISPLAY STARTING COUNT AND ALLOW FOR ROTARY ADJUSTMENT UNTIL BUTTON IS PRESSED
    while (gpio_read(button) != 0) {
        display_number(num_to_display, 1);

        unsigned int a_state = gpio_read(rotary[0]);

        // IF THE STATE OF A HAS CHANGED FROM HIGH TO LOW
        if (last_a_state == 1 && a_state == 0) {
            
            // CHECK TO SEE IF B IS HIGH OR LOW TO DETERMINE DIRECTION
            if (gpio_read(rotary[1]) != a_state) {
                timer_delay_us(500);
                seconds++;
            } else {
                seconds--;
            }
        }
        
        // STORE CURRENT STATE OF A
        last_a_state = a_state;

        // DETERMINE NEW TIME
        if (seconds >= 60) {
            if (minutes == 99) {
                seconds = 59;
            } else {
                seconds = 0;
                minutes++;
            }
        } else if (seconds < 0) {
            if (minutes == 0) {
                seconds = 0;
            } else {
                seconds = 59;
                minutes--;
            }
        }

        num_to_display = minutes*100 + seconds;
    }

    // COUNT DOWN UNTIL 0 IS REACHED
    while (num_to_display > 0) {
        display_number(num_to_display, 1000);
        
        seconds--;
        if (seconds < 0) {
            seconds = 59;
            minutes--;
        }
        num_to_display = minutes*100 + seconds;
    }

    // PLAY END SEQUENCE: SOUND AND FLASH DISPKAY
    end_sequence();

    return countdown;
}
