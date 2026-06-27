/* 
 * Lucas Boyle
 * CS 107e
 * 2/22/26
 * 
 * File: test_gl_console.c
 * -----------------------
 * This file provides a testing framework for fb.c, gl.c, and console.c.
 */


#include "assert.h"
#include "console.h"
#include "fb.h"
#include "gl.h"
#include "printf.h"
#include "strings.h"
#include "timer.h"
#include "uart.h"
#include "malloc.h"


/*
 * The function is used to pause the state of a function and wait for the user to 
 * type something into tio to allow them to view the current state of the graphics. 
 */
static void pause(const char *message) {
    if (message) printf("\n%s\n", message);
    printf("[PAUSED] type any key in tio to continue: ");
    int ch = uart_getchar();
    uart_putchar(ch);
    uart_putchar('\n');
}


/*
 * This function provides tests for a single and double framebuffer, ensuring that
 * they can be filled with values that convert to correct colors. 
 */
static void test_fb(void) {
    
    // INITIALIZE SINGLE FRAMEBUFFER
    const int SIZE = 500;
    fb_init(SIZE, SIZE, FB_SINGLEBUFFER); 

    // CHECK DIMENSIONS
    assert(fb_get_width() == SIZE);
    assert(fb_get_height() == SIZE);
    assert(fb_get_depth() == 4);

    // FILL ENTIRE FRAMEBUFFER WITH LIGHT GREY PIXELS
    void *cptr = fb_get_draw_buffer();
    assert(cptr != NULL);
    int nbytes = fb_get_width() * fb_get_height() * fb_get_depth();
    memset(cptr, 0x99, nbytes);
    pause("Now displaying 500 x 500 screen of light gray pixels");

    malloc_report();

    // INITIALIZE A DOUBLE FRAMEBUFFER, FILL DRAW BUFFER WITH WHITE PIXELS AND SWAP
    fb_init(1280, 720, FB_DOUBLEBUFFER);
    cptr = fb_get_draw_buffer();
    nbytes =  fb_get_width() * fb_get_height() * fb_get_depth();
    memset(cptr, 0xff, nbytes);
    fb_swap_buffer();
    pause("Now displaying 1280 x 720 white pixels");

    // FILL OTHER BUFFER WITH DARK GREY PIXELS AND SWAP
    cptr = fb_get_draw_buffer();
    memset(cptr, 0x33, nbytes);
    fb_swap_buffer();
    pause("Now displaying 1280 x 720 dark gray pixels");

    // CONTINUOUSLY SWAP BUFFERS BETWEEN WHITE AND GREY
    for (int i = 0; i < 5; i++) {
        fb_swap_buffer();
        timer_delay_ms(250);
    }

    fb_init(1280, 720, FB_SINGLEBUFFER);
    malloc_report();
}


/*
 * This function provides tests for gl.c, testing drawing pixels, chars, and strings.
 * Testing of cutoff drawings is also included and tested for correct functionality. 
 */
static void test_gl(gl_mode_t mode) {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    assert(gl_color(0xFF, 0, 0) == GL_RED);
    assert(gl_color(0xFF, 0, 0xFF) == GL_MAGENTA);
    assert(gl_color(0, 0x40, 0) == GL_MOSS);


    // INITIALIZE BUFFER
    gl_init(WIDTH, HEIGHT, mode);
    assert(gl_get_height() == HEIGHT);
    assert(gl_get_width() == WIDTH);

    // PURPLE BACKGROUND
    gl_clear(gl_color(0x55, 0, 0x55));

    // DRAW GREEN PIXEL IN LOWER RIGHT
    gl_draw_pixel(WIDTH-200, HEIGHT-100, GL_GREEN);

    assert(gl_read_pixel(WIDTH-200, HEIGHT-100) == GL_GREEN);

    // BLUE RECTANGLE IN CENTER OF SCREEN
    gl_draw_rect(WIDTH/2 - 100, HEIGHT/2 - 50, 200, 100, GL_BLUE);

    // CHECK CLIPPING IN TOP LEFT CORNER
    gl_draw_rect(-50, -20, 100, 70, GL_ORANGE);
    assert(gl_read_pixel(30, 50) == gl_color(0x55, 0, 0x55));
    assert(gl_read_pixel(50, 30) == gl_color(0x55, 0, 0x55));
    assert(gl_read_pixel(30, 30) == GL_ORANGE);

    // CHECK CLIPPING ON TOP
    gl_draw_rect(400, -20, 80, 120, GL_ORANGE);
    assert(gl_read_pixel(440, 100) == gl_color(0x55, 0, 0x55));
    assert(gl_read_pixel(440, 99) == GL_ORANGE);

    // CHECK CLIPPING IN TOP RIGHT CORNER
    gl_draw_rect(WIDTH - 50, -20, 50, 70, GL_ORANGE);
    assert(gl_read_pixel(WIDTH - 50 - 1, 40) == gl_color(0x55, 0, 0x55));
    assert(gl_read_pixel(WIDTH - 20, 50) == gl_color(0x55, 0, 0x55));
    assert(gl_read_pixel(WIDTH - 50, 40) == GL_ORANGE);

    // CHECK CLIPPING ON RIGHT
    gl_draw_rect(WIDTH - 50, 300, 100, 120, GL_ORANGE);
    assert(gl_read_pixel(WIDTH - 50 - 1, 400) == gl_color(0x55, 0, 0x55));
    assert(gl_read_pixel(WIDTH - 50, 400) == GL_ORANGE);

    // CHECK CLIPPING IN BOTTOM RIGHT CORNER
    gl_draw_rect(WIDTH - 50, HEIGHT - 20, 110, 120, GL_ORANGE);
    assert(gl_read_pixel(WIDTH - 50 - 1, HEIGHT - 20) == gl_color(0x55, 0, 0x55));
    assert(gl_read_pixel(WIDTH - 50, HEIGHT - 20) == GL_ORANGE);

    // CHECK CLIPPING ON BOTTOM
    gl_draw_rect(350, HEIGHT - 100, 100, 120, GL_ORANGE);
    assert(gl_read_pixel(400, HEIGHT - 100 - 1) == gl_color(0x55, 0, 0x55));
    assert(gl_read_pixel(400, HEIGHT - 100) == GL_ORANGE);

    // CHECK CLIPPING IN BOTTOM LEFT CORNER
    gl_draw_rect(-10, HEIGHT - 50, 110, 100, GL_ORANGE);
    assert(gl_read_pixel(50, HEIGHT - 50 - 1) == gl_color(0x55, 0, 0x55));
    assert(gl_read_pixel(50, HEIGHT - 50) == GL_ORANGE);

    // CHECK CLIPPING ON LEFT
    gl_draw_rect(-10, 300, 110, 120, GL_ORANGE);
    assert(gl_read_pixel(100, 400) == gl_color(0x55, 0, 0x55));
    assert(gl_read_pixel(99, 400) == GL_ORANGE);

    // Single amber character
    gl_draw_char(60, 10, 'A', GL_AMBER);

    // SINGLE AMBER O CUT IN HALF ON LEFT
    gl_draw_char(gl_get_char_width()/-2, 200, 'O', GL_AMBER);

    // SINGLE AMBER O CUT IN HALF ON RIGHT
    gl_draw_char(gl_get_width() - gl_get_char_width()/2, 200, 'O', GL_AMBER);

    // SINGLE AMBER O CUT IN HALF ON TOP
    gl_draw_char(100, gl_get_char_height()/-2, 'O', GL_AMBER);

    // SINGLE AMBER O CUT IN HALF ON BOTTOM
    gl_draw_char(100, gl_get_height() - gl_get_char_height()/2, 'O', GL_AMBER);

    // CUTOFF STRING
    gl_draw_string(-10, 100, "CS107e", GL_AMBER);

    // CONGRATS STRING
    gl_draw_string(300, 200, "Congrats on displaying text!!", GL_GREEN);

    // SHOW BUFFER
    gl_swap_buffer();
    pause("Displaying gl test.");
}


/*
 * This function provides tests for console.c, displaying different lines of text that
 * employ special characters and text wrapping. It also tests to ensure the return values 
 * are correct. 
 */
static void test_console(void) {

    // INITIALIZE CONSOLE
    console_init(25, 50, GL_CYAN, GL_INDIGO);
    pause("Now displaying console: 25 rows x 50 columns, bg indigo, fg cyan");

    // LINE 1: Hello, world!
    int num = console_printf("Hello, world!\n");
    assert(num == 13);
    pause("Hello, world!");

    // LINE 2: Happiness == CODING
    console_printf("Happiness");
    console_printf(" == ");
    console_printf("CODING\n");

    // ADD 2 BLANK LINES AND LINE 5: I am Pi, hear me roar!
    num = console_printf("\n\nI am Pi, hear me v\b \broar!\n"); // typo, backspace, correction
    assert(num == 22);
    pause("Console printfs");

    // GO PAST THE END OF THE LINE AND TEST HORIZONTAL WRAPPING
    num = console_printf("\nHow long do you think it takes to reach the end of the line?\n"); 
    assert(num == strlen("How long do you think it takes to reach the end of the line?"));
    pause("Horizontal wrapping");

    // CLEAR ALL LINES
    num = console_printf("\f");
    assert(num == 0);

    // PRINT 27 LINES, CHECK FOR VERTICAL WRAPPING
    for (int i=0; i<27; i++) {
        console_printf("Line #%d\n", i);
    }
    console_printf("The top should say 'Line #3'.");
    pause("Vertical wrapping");
    console_printf("Actually, it should now say 'Line #4'");
    pause("Horizontal and vertical wrapping");

    // CLEAR ALL LINES
    console_printf("\f");

    // LINE 1: "Goodbye"
    console_printf("Goodbye!\n");
    pause("Console clear");
}


/*
 * This function calls upon the others tests in this file. 
 */
void main(void) {
    timer_init();
    uart_init();
    printf("Executing main() in test_gl_console.c\n");

    test_fb();
    // test_gl(FB_SINGLEBUFFER);
    // test_gl(FB_DOUBLEBUFFER);
    // test_console();

    printf("Completed main() in test_gl_console.c\n");
}
