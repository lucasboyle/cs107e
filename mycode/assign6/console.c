/* 
 * Lucas Boyle
 * CS 107e
 * 2/23/26
 * 
 * File: console.c
 * ---------------
 * This file implements console.h, which gives functionality to print text to a graphics
 * console. 
 */


#include "console.h"
#include "gl.h"
#include "printf.h"
#include "malloc.h"
#include "strings.h"


// ASSUME THE OUTPUT IS NEVER LONGER THAN 1024
#define MAX_OUTPUT_LEN 1024


// MODULE LEVEL VARIABLES
static struct {
    color_t bg_color, fg_color;
    int line_height;
    int cursor;
    int line_index;
    int num_rows;
    int num_cols;
    char **text;
} module;


/*
 * This function initializes the console, setting the module level variables and
 * initializing the graphics. 
 */
void console_init(int nrows, int ncols, color_t foreground, color_t background) {
    
    // VERTICAL SPACE BETWEEN ROWS
    const static int LINE_SPACING = 5;

    // SET MODULE LEVEL VARIABLES
    module.line_height = gl_get_char_height() + LINE_SPACING;
    module.fg_color = foreground;
    module.bg_color = background;
    module.cursor = 0;
    module.line_index = 0;
    module.num_rows = nrows;
    module.num_cols = ncols;
    module.text = malloc(module.num_rows * sizeof(char *));
    for (int i=0; i<module.num_rows; i++) {
        module.text[i] = malloc(module.num_cols + 1);
        memset(module.text[i], '\0', module.num_cols + 1);
    }

    // INITIALIZE THE GRAPHICS AND SET AN EMPTY SCREEN
    gl_init(ncols * gl_get_char_width(), nrows * module.line_height, GL_DOUBLEBUFFER);
    gl_clear(module.bg_color);
    gl_swap_buffer();
}


/*
 * This function clears the console. 
 */
void console_clear(void) {
    
    // CLEAR ALL GRAPHICS
    gl_clear(module.bg_color);
    gl_swap_buffer();

    // RESET MODULE LEVEL VARIABLES
    module.cursor = 0;
    module.line_index = 0;
    for (int i=0; i<module.num_rows; i++) {
        memset(module.text[i], '\0', module.num_cols + 1);
    }
}


/*
 * This function handles vertical wrapping if text continues to be written off of the 
 * bottom of the console screen. 
 */
static void vertical_wrap() {

    // FREE THE MEMORY OF THE FIRST LINE
    free(module.text[0]);

    // SHIFT EACH LINE UPWARDS
    for (int i = 0; i < module.num_rows - 1; i++) {
        module.text[i] = module.text[i+1];
    }

    // ALLOCATE MEMORY FOR THE NEW LINE
    module.text[module.num_rows-1] = malloc(module.num_cols + 1);
    memset(module.text[module.num_rows - 1], '\0', module.num_cols + 1);
    module.line_index--;
}


/*
 * This function updates the text stored in the module level text variable, accounting
 * for any of the special characters that could be present in the string. 
 */
static int update_text(const char *format, va_list args) {
    // CREATE A BUFFER AND USE vsnprintf TO GET THE FORMATTED STRING
    char buf[MAX_OUTPUT_LEN];
    int length = vsnprintf(buf, MAX_OUTPUT_LEN, format, args);

    // LOOP THROUGH EACH CHARACTER IN THE STRING
    for (int i=0; i<strlen(buf); i++) {

        // VERTICAL WRAP IF NEEDED
        if (module.line_index == module.num_rows) {
            vertical_wrap();

        // HANDLE BACKSPACE
        } else if (buf[i] == '\b') {
            if (module.cursor > 0) {
                module.cursor--;
            } else if (module.line_index > 0) {
                module.line_index--;
                module.cursor = module.num_cols - 1;
            }
            length -= 2;
            continue;

        // HANDLE NEW LINE
        } else if (buf[i] == '\n') {
            module.cursor = 0;
            module.line_index++;
            if (module.line_index == module.num_rows) {
                vertical_wrap();
            }
            length--;
            continue;

        // HANDLE FORM FEED
        } else if (buf[i] == '\f') {
            console_clear();
            length = 0;
            continue;

        // HORIZONTAL WRAP IF NEEDED
        } else if (module.cursor == module.num_cols) {
            module.cursor = 0;
            module.line_index++;
            if (module.line_index == module.num_rows) {
                vertical_wrap();
            } 
        }
        
        // ADD THE NEXT CHARACTER TO TEXT
        module.text[module.line_index][module.cursor] = buf[i];
        module.cursor++;
    }

    return length;
}


/*
 * This function displays the text currently stored in the module level text variable. 
 */
static void display_text() {
    gl_clear(module.bg_color);
    for (int i=0; i<module.num_rows; i++) {
        gl_draw_string(0, i*module.line_height, module.text[i], module.fg_color);
    }
    gl_swap_buffer();
}


/*
 * This function is the user-facing function that allows for printing with arguments to 
 * the console. 
 */
int console_printf(const char *format, ...) {
    
    // UPDATE THE MODULE LEVEL TEXT VARIABLE
    va_list(args);
    va_start(args, format);
    int num_written = update_text(format, args);
    va_end(args);

    // DISPLAY THE FORMATTED TEXT
    display_text();

    return num_written;
}


/*
 * This function produces a startup screen that will be automatically shown when 
 * using the console. 
 */
void console_startup_screen(void) {
   /***** TODO: Your code goes here IFF doing the extension *****/
}
