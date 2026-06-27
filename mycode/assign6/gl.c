/* 
 * Lucas Boyle
 * CS 107e
 * 2/21/26
 * 
 * File: gl.c
 * ----------
 * This file implements gl.h, which gives functionality for a graphics library.
 * Funtions are included that allow a user to draw a pixel, rectangle, char, or 
 * string. 
 */


#include "gl.h"
#include "gl_mine.h"
#include "font.h"
#include "strings.h"
#include "assert.h"
#include "printf.h"


/* 
 * This function initializes the framebuffer so graphics can be displayed. 
 */
void gl_init(int width, int height, gl_mode_t mode) {
    fb_init(width, height, mode);
}


/*
 * This function returns the width of the framebuffer. 
 */
int gl_get_width(void) {
    return fb_get_width();
}


/*
 * This function returns the height of the framebuffer. 
 */
int gl_get_height(void) {
    return fb_get_height();
}


/* 
 * This function accepts the r, g, and b components of a color and returns
 * a color_t that can be used to fill the framebuffer. 
 */
color_t gl_color(uint8_t r, uint8_t g, uint8_t b) {
    return (color_t)((0xFF << 24) | (r << 16) | (g << 8) | (b));
}


/*
 * This function swaps the framebuffers if a double buffer is used. 
 */
void gl_swap_buffer(void) {
    fb_swap_buffer();
}


/*
 * This function clears the entire screen to a certain color c. 
 */
void gl_clear(color_t c) {
    int width = gl_get_width();
    int height = gl_get_height();

    // GET THE DRAW BUFFER
    unsigned int (*im)[width] = fb_get_draw_buffer();

    // SET EACH PIXEL TO THE COLOR
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            im[y][x] = c;
        }
    }
}


/* 
 * This function draws a pixel at the coordinates x, y, and sets it to the 
 * color c. If the coordinates are out of bounds of the framebuffer, the
 * function will do nothing. 
 */
void gl_draw_pixel(int x, int y, color_t c) {
    int width = gl_get_width();

    // CHECK BOUNDS
    if (x < 0 || x >= width || y < 0 || y >= gl_get_height()) {
        return;
    }

    // DRAW PIXEL
    unsigned int (*im)[width] = fb_get_draw_buffer();
    im[y][x] = c;
}


/*
 * This function will return the color of pixel at the coordinates x, y. 
 * If the coordinates are out of bounds, 0 will be returned. 
 */
color_t gl_read_pixel(int x, int y) {
    int width = gl_get_width();
    
    // CHECK BOUNDS
    if (x < 0 || x >= gl_get_width() || y < 0 || y >= gl_get_height()) {
        return 0;
    }

    // GET VALUE AND RETURN
    unsigned int (*im)[width] = fb_get_draw_buffer();
    return im[y][x];
}


/* 
 * This function will draw a rectangle starting at the coordinates x, y. The
 * rectangle has width w, height h, and color c. If the rectangle would go 
 * out of bounds in any direction, the function will clip it so that only
 * the section in bounds is drawn. 
 */
void gl_draw_rect(int x, int y, int w, int h, color_t c) {
    int width = gl_get_width();
    int height = gl_get_height();

    // IF RECTANGLE WILL NOT BE VISIBLE, RETURN
    if (x >= width || y >= height || w <= 0 || h <= 0) {
        return;
    }
    
    // CLIP COORDINATES IF X OR Y ARE NEGATIVE
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }

    // CLIP WIDTH/HEIGHT IF OUT OF BOUNDS ON RIGHT/BOTTOM
    if (x + w >= width) {
        w = width - x;
    }
    if (y + h >= height) {
        h = height - y;
    }

    // GET THE DRAW BUFFER AND DRAW THE RECTANGLE WITH CLIPPED COORDINATES
    unsigned int (*im)[width] = fb_get_draw_buffer();
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            im[y + j][x + i] = c;
        }
    }
}


/*
 * This function draws a single character at the coordinates x, y, in the 
 * color c. Only the pixels of the character in bounds of the framebuffer
 * will be drawn. 
 */
void gl_draw_char(int x, int y, char ch, color_t c) {
    int char_width = gl_get_char_width();
    int char_height = gl_get_char_height();
    int width = gl_get_width();
    int height = gl_get_height();

    // RETURN EARLY IF NO PART OF THE CHARACTER WILL BE VISIBLE
    if (x + char_width < 0 || y + char_height < 0 || x >= width || y >= height) {
        return;
    }
    
    // GET THE GLYPH FOR THE CHARACTER
    uint8_t buf[char_width * char_height];
    bool got_glyph = font_get_glyph(ch, buf, sizeof(buf));
    uint8_t (*img)[char_width] = (void *)buf;

    // printf("%c\n", ch);
    if (!got_glyph) {
        return;
    }

    assert(got_glyph);

    // GET THE DRAW BUFFER
    unsigned int (*im)[gl_get_width()] = fb_get_draw_buffer();

    for (int j = 0; j < char_height; j++) {

        // CONTINUE IF THE ENTIRE ROW IS OUT OF BOUNDS
        if (y+j < 0 || y+j >= height) {
            continue;
        }

        for (int i = 0; i < char_width; i++) {

            // CONTINUE IF THE ENTIRE COLUMN IS OUT OF BOUNDS
            if (x+i < 0 || x+i >= width) {
                continue;
            }

            // DRAW THE PIXEL IF THE GLYPH AT THAT LOCATION IS 0xFF
            if (img[j][i] == 0xff) {
                im[y + j][x + i] = c;
            }
        }
    }
}


/*
 * This function draws an entire string at coordinates x, y, in color c. 
 * Only the pixels of the string in bounds of the framebuffer will be
 * displayed. 
 */
void gl_draw_string(int x, int y, const char* str, color_t c) {
    int char_width = gl_get_char_width();
    int len = strlen(str);
    
    // FOR EACH CHAR IN THE STRING, DRAW IT
    for (int i = 0; i < len; i++) {
        gl_draw_char(x + i*char_width, y, str[i], c);
    }
}


/*
 * This function gets the height of a char to draw. 
 */
int gl_get_char_height(void) {
    return font_get_glyph_height();
}


/*
 * This function gets the width of a char to draw. 
 */
int gl_get_char_width(void) {
    return font_get_glyph_width();
}
