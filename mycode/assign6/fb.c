/* 
 * Lucas Boyle
 * CS 107e
 * 3/2/26
 * 
 * File: fb.c
 * ----------
 * The file implememnts fb.h, which gives functionality for a single or double
 * framebuffer used for graphics. 
 */


#include "fb.h"
#include "de.h"
#include "hdmi.h"
#include "malloc.h"
#include "strings.h"

// MODULE LEVEL VARIALBES
static struct {
    int width;                // COUNT OF HORIZONTAL PIXELS
    int height;               // COUNT OF VERTICALS PIXELS
    int depth;                // NUM BYTES PER PIXEL
    void *framebuffer[2];     // FRAMBUFFER MEMORY
} module;


/*
 * This function initializes the frambuffer, using malloc to allocate the 
 * correct amount of memory depending on the display size. If this function
 * is called more than one, the subsequent calls will free the memory 
 * associarted with the buffers and will reinitialize them.
 */
void fb_init(int width, int height, fb_mode_t mode) {

    // FREE FRAMEBUFFER MEMORY IF NEEDED
    if (module.framebuffer[0]) {
        free(module.framebuffer[0]);
        module.framebuffer[0] = NULL;
    }
    if (module.framebuffer[1]) {
        free(module.framebuffer[1]);
        module.framebuffer[1] = NULL;
    }

    // MALLOC THE CORRECT AMOUNT OF MEMORY, INITIALIZE TO 0
    module.width = width;
    module.height = height;
    module.depth = 4;
    int nbytes = module.width * module.height * module.depth;
    module.framebuffer[0] = malloc(nbytes);
    memset(module.framebuffer[0], 0x0, nbytes);
    if (mode == FB_DOUBLEBUFFER) {
        module.framebuffer[1] = malloc(nbytes);
        memset(module.framebuffer[1], 0x0, nbytes);
    } 

    // INIT DISPLAY ENGINE
    hdmi_resolution_id_t id = hdmi_best_match(width, height);
    hdmi_init(id);
    de_init(width, height, hdmi_get_screen_width(), hdmi_get_screen_height());

    // SET framebuffer[0] AS THE ACTIVE FRAMEBUFFER
    de_set_active_framebuffer(module.framebuffer[0]);
}


/*
 * This function returns the width of the framebuffer.  
 */
int fb_get_width(void) {
    return module.width;
}


/*
 * This function returns the height of the framebuffer.  
 */
int fb_get_height(void) {
    return module.height;
}


/*
 * This function returns the depth of the framebuffer.  
 */
int fb_get_depth(void) {
    return module.depth;
}


/*
 * This function returns a pointer to the draw framebuffer.  
 */
void* fb_get_draw_buffer(void){
    if (module.framebuffer[1]) {
        return module.framebuffer[1];
    }
    return module.framebuffer[0];
}


/*
 * This function swaps the draw and active framebuffers if a double
 * framebuffer is used. 
 */
void fb_swap_buffer(void) {
    if (!module.framebuffer[1]) {
        return;
    }
    void *temp = module.framebuffer[0];
    module.framebuffer[0] = module.framebuffer[1];
    module.framebuffer[1] = temp;
    
    // SET THE NEWLY SWAPPED FRAMBUFFER AS THE ACTIVE FRAMEBUFFER
    de_set_active_framebuffer(module.framebuffer[0]);
}
