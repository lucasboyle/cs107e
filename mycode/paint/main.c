/* Lucas Boyle
 * CS 107e
 * 2/4/26
 * 
 * File: main.c
 * ------------
 * This program runs a paint application on the user's monitor, using a
 * PS2 mouse as input. 
 */


#include "console.h"
#include "interrupts.h"
#include "keyboard.h"
#include "shell.h"
#include "timer.h"
#include "uart.h"
#include "mouse.h"
#include "gl.h"
#include "malloc.h"

// DEFINE SCREEN DIMENSIONS
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_DEPTH 4

// DEFINE CURSOR DIMENSIONS
#define CURSOR_WIDTH 2
#define CURSOR_HEIGHT 2


/*
 * This function runs the paint application. 
 */
void main(void) {
    
    // INITIALIZE EVERYTHING, ENABLE INTERRUPTS
    interrupts_init();
    gpio_init();
    timer_init();
    uart_init();
    gl_init(SCREEN_WIDTH, SCREEN_HEIGHT, GL_DOUBLEBUFFER);
    mouse_init(MOUSE_CLOCK, MOUSE_DATA);
    interrupts_global_enable();
    
    // CREATE ARRAYS TO HOLD THE CURRENT AND PREVIOUS PIXELS AT THE CURSOR
    unsigned int (*cursor_pixels_curr)[CURSOR_WIDTH] = malloc(CURSOR_WIDTH * CURSOR_HEIGHT * SCREEN_DEPTH);
    unsigned int (*cursor_pixels_prev)[CURSOR_WIDTH] = malloc(CURSOR_WIDTH * CURSOR_HEIGHT * SCREEN_DEPTH);
    
    // CREATE VARIABLES TO HOLD OLD AND NEW MOUSE LOCATIONS
    int mouse_x_curr = SCREEN_WIDTH/2, mouse_y_curr = SCREEN_HEIGHT/2;
    int mouse_x_prev = SCREEN_WIDTH/2, mouse_y_prev = SCREEN_HEIGHT/2;

    // STORE ALL COLORS
    const unsigned int colors[] = {GL_BLACK, GL_RED, GL_GREEN, GL_BLUE, GL_CYAN, 
                                   GL_MAGENTA, GL_YELLOW, GL_AMBER, GL_ORANGE, GL_PURPLE, 
                                   GL_INDIGO, GL_CAYENNE, GL_MOSS, GL_SILVER};
    int num_colors = 14;
    int color_index = 0;

    // SET INITIAL FRAMEBUFFER AND SHOW TO USER
    gl_clear(GL_WHITE);
    gl_draw_rect(mouse_x_curr, mouse_y_curr, CURSOR_WIDTH, CURSOR_HEIGHT, GL_BLACK);
    gl_draw_rect(5, 5, 15, 15, colors[color_index]);
    gl_swap_buffer();
    gl_clear(GL_WHITE);

    // SET PREVIOUS AND CURRENT CURSOR PIXELS TO WHITE SINCE THERE IS NOTHING ON SCREEN
    cursor_pixels_prev[0][0] = cursor_pixels_prev[0][1] = cursor_pixels_prev[1][0] = 
    cursor_pixels_prev[1][1] = GL_WHITE;

    cursor_pixels_curr[0][0] = cursor_pixels_curr[0][1] = cursor_pixels_curr[1][0] = 
    cursor_pixels_curr[1][1] = GL_WHITE;

    // VARIABLE TO HOLD STATE OF WHETHER THE RIGHT BUTTON CLICK HAS BEEN REGISTERED
    int registered_right = 0;

    // LOOP
    while (1) {
        
        // DRAW A RECTANGLE WITH THE CURRENT COLOR IN THE TOP LEFT CORNER
        gl_draw_rect(5, 5, 15, 15, colors[color_index]);

        // GET THE NEXT EVENT
        mouse_event_t event = mouse_read_event();

        // ENSURE MOUSE IS IN BOUNDS
        if (mouse_x_curr + event.dx < 0 || mouse_x_curr + event.dx >= SCREEN_WIDTH ||
            mouse_y_curr + event.dy < 0 || mouse_y_curr + event.dy >= SCREEN_HEIGHT) {
                continue;
        }

        // DRAW THE PIXELS WHERE THE MOUSE USED TO BE
        gl_draw_pixel(mouse_x_prev, mouse_y_prev, cursor_pixels_prev[0][0]);
        gl_draw_pixel(mouse_x_prev + 1, mouse_y_prev, cursor_pixels_prev[1][0]);
        gl_draw_pixel(mouse_x_prev, mouse_y_prev + 1, cursor_pixels_prev[0][1]);
        gl_draw_pixel(mouse_x_prev + 1, mouse_y_prev + 1, cursor_pixels_prev[1][1]);

        // UPDATE MOUSE COORDS
        mouse_x_prev = mouse_x_curr;
        mouse_y_prev = mouse_y_curr;
        mouse_x_curr += event.dx;
        mouse_y_curr += event.dy;
        
        // DRAW COLOR IF LEFT BUTTON CLICKED
        if (event.left) {
            gl_draw_rect(mouse_x_curr - CURSOR_WIDTH + 1, mouse_y_curr - CURSOR_HEIGHT + 1, CURSOR_WIDTH*2, CURSOR_HEIGHT*2, colors[color_index]);
            cursor_pixels_prev[0][0] = colors[color_index];
            cursor_pixels_prev[1][0] = colors[color_index];
            cursor_pixels_prev[0][1] = colors[color_index];
            cursor_pixels_prev[1][1] = colors[color_index];

        // ERASE (DRAW WHITE) IF MIDDLE BUTTON CLICKED
        } else if (event.middle) {
            gl_draw_rect(mouse_x_curr - CURSOR_WIDTH * 2 + 1, mouse_y_curr - CURSOR_HEIGHT * 2 + 1, CURSOR_WIDTH*4, CURSOR_HEIGHT*4, GL_WHITE);
            cursor_pixels_prev[0][0] = GL_WHITE;
            cursor_pixels_prev[1][0] = GL_WHITE;
            cursor_pixels_prev[0][1] = GL_WHITE;
            cursor_pixels_prev[1][1] = GL_WHITE;

        // UPDATE CURSOR COLORS
        } else {
            cursor_pixels_prev[0][0] = cursor_pixels_curr[0][0];
            cursor_pixels_prev[1][0] = cursor_pixels_curr[1][0];
            cursor_pixels_prev[0][1] = cursor_pixels_curr[0][1];
            cursor_pixels_prev[1][1] = cursor_pixels_curr[1][1];
        }
        
        // IF THE RIGHT BUTTON HAS JUST BEEN CLICKED, INCREMENT THE COLOR
        if (event.right && !registered_right) {
            color_index = (color_index + 1) % (num_colors);
            registered_right = 1;
        } else if (!event.right) {
            registered_right = 0;
        }
        
        // GET WHAT IS UNDER THE CURSOR
        cursor_pixels_curr[0][0] = gl_read_pixel(mouse_x_curr, mouse_y_curr);
        cursor_pixels_curr[1][0] = gl_read_pixel(mouse_x_curr + 1, mouse_y_curr);
        cursor_pixels_curr[0][1] = gl_read_pixel(mouse_x_curr, mouse_y_curr + 1);
        cursor_pixels_curr[1][1] = gl_read_pixel(mouse_x_curr + 1, mouse_y_curr + 1);

        // DRAW THE CURSOR
        gl_draw_rect(mouse_x_curr, mouse_y_curr, CURSOR_WIDTH, CURSOR_HEIGHT, GL_BLACK);

        // SWAP THE BUFFER
        gl_swap_buffer();

        // DEPLOY CHANGES TO OLD BUFFER
        if (event.left) {
            gl_draw_rect(mouse_x_curr - CURSOR_WIDTH + 1, mouse_y_curr - CURSOR_HEIGHT + 1, CURSOR_WIDTH*2, CURSOR_HEIGHT*2, colors[color_index]);
        } else if (event.middle) {
            gl_draw_rect(mouse_x_curr - CURSOR_WIDTH * 2 + 1, mouse_y_curr - CURSOR_HEIGHT * 2 + 1, CURSOR_WIDTH*4, CURSOR_HEIGHT*4, GL_WHITE);
        }
    }
}
