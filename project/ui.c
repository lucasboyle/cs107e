/*
 * Lucas Boyle
 * CS 107e
 * 3/19/26
 *
 * File: ui.c
 * -----
 * This file implements ui.c, incorporating functionality to display the user interface
 * for a rubiks cube solver. Functionality includes clicking on specific tiles to designate
 * color, clicking buttons to mechanically rotate the cube, and calling upon the solving 
 * algorithm when appropriate. 
 */


#include "ui.h"
#include "rubiks_solver.h"
#include "gl.h"
#include "malloc.h"
#include "mouse.h"
#include "motor.h"
#include "timer.h"
#include "printf.h"
#include "strings.h"
#include "interrupts.h"
#include <stdbool.h>


// DEFINE CURSOR DIMENSIONS
#define CURSOR_WIDTH 6
#define CURSOR_HEIGHT 6


// DEFINE GRAPHICS DIMENSIONS
#define TILE_SIZE 44
#define SIDE_SIZE 160
#define BUTTON_SIZE 26
#define BUTTON_X_OFFSET 455
#define BUTTON_Y_OFFSET 385


// DEFINE CUSTOM COLORS
#define GREY 0xFF696969
#define BLUE 0xFF0055FF
#define ORANGE 0xFFFFA600


// STORE ALL COLORS IN AN ARRAY
const unsigned int colors[] = {GL_YELLOW, GL_WHITE, BLUE, GL_GREEN, ORANGE, GL_RED, GREY};
const int num_colors = 6;


// STORE IF SOLVE BUTTON PRESSED
static bool solve_pressed = false;


// CREATE A TRACKING CUBE THAT FOLLOWS THE BUTTON PRESSES
static tracking_cube_t *tracking_cube;


// FUNCTION PROTOTYPE FOR HELPER FUNCTION
static void solve_and_execute(cube_t *cube, motor_t *motor_u, 
                              motor_t *motor_d, motor_t *motor_f, 
                              motor_t *motor_b, motor_t *motor_l, motor_t *motor_r);


/*
 * This function initializes the UI. Initialization includes setting up the graphics, 
 * creating an unsolved cube, and setting up the tracking cube. A pointer to the cube that 
 * represents the current state of the real cube sill be returned. 
 */
cube_t *ui_init(void) {
    
    // INITIALIZE GRAPHICS
    gl_init(SCREEN_WIDTH, SCREEN_HEIGHT, GL_DOUBLEBUFFER);

    // CREATE A cube_t POINTER
    cube_t *cube = malloc(sizeof(*cube));

    // INITIALIZE EACH SIDE
    cube->up = malloc(9);
    cube->down = malloc(9);
    cube->left = malloc(9);
    cube->front = malloc(9);
    cube->right = malloc(9);
    cube->back = malloc(9);

    for (int i=0; i<9; i++) {
        if (i == 4) {
            cube->up[4] = CUBE_UP;
            cube->down[4] = CUBE_DOWN;
            cube->left[4] = CUBE_LEFT;
            cube->front[4] = CUBE_FRONT;
            cube->right[4] = CUBE_RIGHT;
            cube->back[4] = CUBE_BACK;
        } else {
            cube->up[i] = CUBE_EMPTY;
            cube->down[i] = CUBE_EMPTY;
            cube->left[i] = CUBE_EMPTY;
            cube->front[i] = CUBE_EMPTY;
            cube->right[i] = CUBE_EMPTY;
            cube->back[i] = CUBE_EMPTY;
        }
    }

    cube->num_blue = 1;
    cube->num_red = 1;
    cube->num_green = 1;
    cube->num_orange = 1;
    cube->num_yellow = 1;
    cube->num_white = 1;

    // CREATE A tracking_cube POINTER
    tracking_cube = malloc(sizeof(*tracking_cube));

    // INITIALIZE EACH SIDE TO SOLVED
    tracking_cube->up = malloc(9);
    tracking_cube->down = malloc(9);
    tracking_cube->front = malloc(9);
    tracking_cube->back = malloc(9);
    tracking_cube->left = malloc(9);
    tracking_cube->right = malloc(9);

    for (int i=0; i<9; i++) {
        tracking_cube->up[i] = CUBE_UP;
    }

    for (int i=0; i<9; i++) {
        tracking_cube->down[i] = CUBE_DOWN;
    }

    for (int i=0; i<9; i++) {
        tracking_cube->front[i] = CUBE_FRONT;
    }

    for (int i=0; i<9; i++) {
        tracking_cube->back[i] = CUBE_BACK;
    }

    for (int i=0; i<9; i++) {
        tracking_cube->left[i] = CUBE_LEFT;
    }

    for (int i=0; i<9; i++) {
        tracking_cube->right[i] = CUBE_RIGHT;
    }

    return cube;
}


void free_all(cube_t *cube) {
    free(cube->up);
    free(cube->down);
    free(cube->front);
    free(cube->back);
    free(cube->left);
    free(cube->right);

    free(tracking_cube->up);
    free(tracking_cube->down);
    free(tracking_cube->front);
    free(tracking_cube->back);
    free(tracking_cube->left);
    free(tracking_cube->right);

    free(cube);
    free(tracking_cube);
}


/*
 * This function will display the side of the cube specified by a side_num that
 * cooresponds to a cube_tile_t enum. 
 */
void display_side(cube_t *cube, uint8_t side_num) {
    // UP
    if (side_num == CUBE_UP) { 
        gl_draw_rect(245, 70, SIDE_SIZE, SIDE_SIZE, GL_BLACK);

        for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
                gl_draw_rect(255 + 48*j, 80 + 48*i, TILE_SIZE, TILE_SIZE, colors[cube->up[3*i + j]]);
            }
        }

        gl_draw_char(255+48 + (TILE_SIZE - gl_get_char_width())/2, 80+48 + (TILE_SIZE - gl_get_char_height())/2, 'U', GL_BLACK);

    // DOWN
    } else if (side_num == CUBE_DOWN) { 
        gl_draw_rect(245, 370, SIDE_SIZE, SIDE_SIZE, GL_BLACK);
        
        for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
                gl_draw_rect(255 + 48*j, 380 + 48*i, TILE_SIZE, TILE_SIZE, colors[cube->down[3*i + j]]);
            }
        }

        gl_draw_char(255+48 + (TILE_SIZE - gl_get_char_width())/2, 380+48 + (TILE_SIZE - gl_get_char_height())/2, 'D', GL_BLACK);

    // LEFT
    } else if (side_num == CUBE_LEFT){
        gl_draw_rect(95, 220, SIDE_SIZE, SIDE_SIZE, GL_BLACK);

        for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
                gl_draw_rect(105 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[cube->left[3*i + j]]);
            }
        }

        gl_draw_char(105+48 + (TILE_SIZE - gl_get_char_width())/2, 230+48 + (TILE_SIZE - gl_get_char_height())/2, 'L', GL_BLACK);

    // FRONT
    } else if (side_num == CUBE_FRONT){
        gl_draw_rect(245, 220, SIDE_SIZE, SIDE_SIZE, GL_BLACK);

        for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
                gl_draw_rect(255 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[cube->front[3*i + j]]);
            }
        }

        gl_draw_char(255+48 + (TILE_SIZE - gl_get_char_width())/2, 230+48 + (TILE_SIZE - gl_get_char_height())/2, 'F', GL_BLACK);

    // RIGHT
    } else if (side_num == CUBE_RIGHT){ 
        gl_draw_rect(395, 220, SIDE_SIZE, SIDE_SIZE, GL_BLACK);

        for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
                gl_draw_rect(405 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[cube->right[3*i + j]]);
            }
        }

        gl_draw_char(405+48 + (TILE_SIZE - gl_get_char_width())/2, 230+48 + (TILE_SIZE - gl_get_char_height())/2, 'R', GL_BLACK);

    // BACK
    } else if (side_num == CUBE_BACK){
        gl_draw_rect(545, 220, SIDE_SIZE, SIDE_SIZE, GL_BLACK);

        for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
                gl_draw_rect(555 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[cube->back[3*i + j]]);
            }
        }

        gl_draw_char(555+48 + (TILE_SIZE - gl_get_char_width())/2, 230+48 + (TILE_SIZE - gl_get_char_height())/2, 'B', GL_BLACK);
    }
}


/* 
 * This function displays the entire cube onto the UI.
 */
void display_cube(cube_t *cube) {
    for (uint8_t i=0; i<6; i++) {
        display_side(cube, i);
    }
}


/*
 * This function display all of the buttons onto the UI.
 */
void display_buttons(void) {
    
    // CLOCKWISE TURN BUTTONS
    gl_draw_string(450, 20, "90 Degree CW Turn:", GL_BLACK);
    
    gl_draw_rect(20 + BUTTON_X_OFFSET, 105, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK); // LEFT
    gl_draw_rect(22 + BUTTON_X_OFFSET, 107, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_LEFT]);
    gl_draw_char(22 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 107 + (BUTTON_SIZE - gl_get_char_height())/2, 'L', GL_BLACK);

    gl_draw_rect(70 + BUTTON_X_OFFSET, 105, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK); // FRONT
    gl_draw_rect(72 + BUTTON_X_OFFSET, 107, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_FRONT]);
    gl_draw_char(72 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 107 + (BUTTON_SIZE - gl_get_char_height())/2, 'F', GL_BLACK);

    gl_draw_rect(120 + BUTTON_X_OFFSET, 105, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK); // RIGHT
    gl_draw_rect(122 + BUTTON_X_OFFSET, 107, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_RIGHT]);
    gl_draw_char(122 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 107 + (BUTTON_SIZE - gl_get_char_height())/2, 'R', GL_BLACK);

    gl_draw_rect(170 + BUTTON_X_OFFSET, 105, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK); // BACK
    gl_draw_rect(172 + BUTTON_X_OFFSET, 107, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_BACK]);
    gl_draw_char(172 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 107 + (BUTTON_SIZE - gl_get_char_height())/2, 'B', GL_BLACK);

    gl_draw_rect(70 + BUTTON_X_OFFSET, 55, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK); // UP
    gl_draw_rect(72 + BUTTON_X_OFFSET, 57, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_UP]);
    gl_draw_char(72 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 57 + (BUTTON_SIZE - gl_get_char_height())/2, 'U', GL_BLACK);

    gl_draw_rect(70 + BUTTON_X_OFFSET, 155, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK); // DOWN
    gl_draw_rect(72 + BUTTON_X_OFFSET, 157, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_DOWN]);
    gl_draw_char(72 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 157 + (BUTTON_SIZE - gl_get_char_height())/2, 'D', GL_BLACK);


    // COUNTER CLOCKWISE TURN BUTTONS
    gl_draw_string(450, 20 + BUTTON_Y_OFFSET, "90 Degree CCW Turn:", GL_BLACK);
    
    gl_draw_rect(20 + BUTTON_X_OFFSET, 105 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(22 + BUTTON_X_OFFSET, 107 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_LEFT]);
    gl_draw_char(22 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 107 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'L', GL_BLACK);

    gl_draw_rect(70 + BUTTON_X_OFFSET, 105 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(72 + BUTTON_X_OFFSET, 107 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_FRONT]);
    gl_draw_char(72 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 107 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'F', GL_BLACK);

    gl_draw_rect(120 + BUTTON_X_OFFSET, 105 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(122 + BUTTON_X_OFFSET, 107 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_RIGHT]);
    gl_draw_char(122 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 107 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'R', GL_BLACK);

    gl_draw_rect(170 + BUTTON_X_OFFSET, 105 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(172 + BUTTON_X_OFFSET, 107 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_BACK]);
    gl_draw_char(172 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 107 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'B', GL_BLACK);

    gl_draw_rect(70 + BUTTON_X_OFFSET, 55 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(72 + BUTTON_X_OFFSET, 57 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_UP]);
    gl_draw_char(72 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 57 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'U', GL_BLACK);

    gl_draw_rect(70 + BUTTON_X_OFFSET, 155 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(72 + BUTTON_X_OFFSET, 157 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_DOWN]);
    gl_draw_char(72 + BUTTON_X_OFFSET + (BUTTON_SIZE - gl_get_char_width())/2, 157 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'D', GL_BLACK);


    // CLOCKWISE STEP BUTTONS
    gl_draw_string(50, 20, "CW Align:", GL_BLACK);
    
    gl_draw_rect(20, 105, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(22, 107, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_LEFT]);
    gl_draw_char(22 + (BUTTON_SIZE - gl_get_char_width())/2, 107 + (BUTTON_SIZE - gl_get_char_height())/2, 'L', GL_BLACK);

    gl_draw_rect(70, 105, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(72, 107, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_FRONT]);
    gl_draw_char(72 + (BUTTON_SIZE - gl_get_char_width())/2, 107 + (BUTTON_SIZE - gl_get_char_height())/2, 'F', GL_BLACK);

    gl_draw_rect(120, 105, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(122, 107, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_RIGHT]);
    gl_draw_char(122 + (BUTTON_SIZE - gl_get_char_width())/2, 107 + (BUTTON_SIZE - gl_get_char_height())/2, 'R', GL_BLACK);

    gl_draw_rect(170, 105, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(172, 107, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_BACK]);
    gl_draw_char(172 + (BUTTON_SIZE - gl_get_char_width())/2, 107 + (BUTTON_SIZE - gl_get_char_height())/2, 'B', GL_BLACK);

    gl_draw_rect(70, 55, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(72, 57, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_UP]);
    gl_draw_char(72 + (BUTTON_SIZE - gl_get_char_width())/2, 57 + (BUTTON_SIZE - gl_get_char_height())/2, 'U', GL_BLACK);

    gl_draw_rect(70, 155, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(72, 157, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_DOWN]);
    gl_draw_char(72 + (BUTTON_SIZE - gl_get_char_width())/2, 157 + (BUTTON_SIZE - gl_get_char_height())/2, 'D', GL_BLACK);


    // COUNTER CLOCKWISE STEP BUTTONS
    gl_draw_string(50, 20 + BUTTON_Y_OFFSET, "CCW Align:", GL_BLACK);
    
    gl_draw_rect(20, 105 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(22, 107 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_LEFT]);
    gl_draw_char(22 + (BUTTON_SIZE - gl_get_char_width())/2, 107 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'L', GL_BLACK);

    gl_draw_rect(70, 105 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(72, 107 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_FRONT]);
    gl_draw_char(72 + (BUTTON_SIZE - gl_get_char_width())/2, 107 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'F', GL_BLACK);

    gl_draw_rect(120, 105 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(122, 107 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_RIGHT]);
    gl_draw_char(122 + (BUTTON_SIZE - gl_get_char_width())/2, 107 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'R', GL_BLACK);

    gl_draw_rect(170, 105 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(172, 107 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_BACK]);
    gl_draw_char(172 + (BUTTON_SIZE - gl_get_char_width())/2, 107 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'B', GL_BLACK);

    gl_draw_rect(70, 55 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(72, 57 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_UP]);
    gl_draw_char(72 + (BUTTON_SIZE - gl_get_char_width())/2, 57 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'U', GL_BLACK);

    gl_draw_rect(70, 155 + BUTTON_Y_OFFSET, BUTTON_SIZE + 4, BUTTON_SIZE + 4, GL_BLACK);
    gl_draw_rect(72, 157 + BUTTON_Y_OFFSET, BUTTON_SIZE, BUTTON_SIZE, colors[CUBE_DOWN]);
    gl_draw_char(72 + (BUTTON_SIZE - gl_get_char_width())/2, 157 + BUTTON_Y_OFFSET + (BUTTON_SIZE - gl_get_char_height())/2, 'D', GL_BLACK);


    // SOLVE BUTTON
    int text_width = gl_get_char_width() * 9;
    gl_draw_rect(245, 10, SIDE_SIZE, 50, GL_BLACK);
    gl_draw_rect(248, 13, SIDE_SIZE - 6, 44, GL_MAGENTA);
    gl_draw_string(245 + (SIDE_SIZE - text_width)/2, 10 + (50 - gl_get_char_height())/2, "> Solve <", GL_BLACK);


    // AUTOFILL BUTTON
    text_width = gl_get_char_width() * 8;
    gl_draw_rect(245, 540, SIDE_SIZE, 50, GL_BLACK);
    gl_draw_rect(248, 543, SIDE_SIZE - 6, 44, GL_CYAN);
    gl_draw_string(245 + (SIDE_SIZE - text_width)/2, 540 + (50 - gl_get_char_height())/2, "Autofill", GL_BLACK);
}


/*
 * This function determines if the cube is full (every tile has a color in it) and returns
 * a bool. 
 */
static bool is_full(cube_t *cube) {
    if (cube->num_blue + cube->num_green + cube->num_orange + cube->num_red + cube->num_white + cube->num_yellow == 54) {
        return true;
    } else {
        return false;
    }
}


/* 
 * This function handles all user input, displaying the cube, buttons, and reacting to all 
 * user input from the mouse. 
 */
void user_input(cube_t *cube, motor_t *motor_u, motor_t *motor_d, motor_t *motor_f, motor_t *motor_b, motor_t *motor_l, motor_t *motor_r) {

    // CREATE ARRAYS TO HOLD THE CURRENT AND PREVIOUS PIXELS AT THE CURSOR
    unsigned int (*cursor_pixels_curr)[CURSOR_WIDTH] = malloc(CURSOR_WIDTH * CURSOR_HEIGHT * SCREEN_DEPTH);
    unsigned int (*cursor_pixels_prev)[CURSOR_WIDTH] = malloc(CURSOR_WIDTH * CURSOR_HEIGHT * SCREEN_DEPTH);
    
    // CREATE VARIABLES TO HOLD OLD AND NEW MOUSE LOCATIONS
    int mouse_x_curr = SCREEN_WIDTH/2 - 1, mouse_y_curr = SCREEN_HEIGHT/2;
    int mouse_x_prev = SCREEN_WIDTH/2 - 1, mouse_y_prev = SCREEN_HEIGHT/2;

    uint8_t color_index = 0;

    // SET INITIAL FRAMEBUFFER AND SHOW TO USER
    gl_clear(GL_WHITE);
    gl_draw_rect(mouse_x_curr, mouse_y_curr, CURSOR_WIDTH, CURSOR_HEIGHT, GL_SILVER);
    display_cube(cube);
    display_buttons();
    gl_draw_rect(SCREEN_WIDTH - 4 - 27, 4, 27, 27, GREY);
    gl_draw_rect(SCREEN_WIDTH - 5 - 25, 5, 25, 25, colors[color_index]);
    gl_swap_buffer();
    gl_clear(GL_WHITE);
    display_cube(cube);
    display_buttons();

    // SET PREVIOUS AND CURRENT CURSOR PIXELS TO BALCK
    for (int i=0; i<CURSOR_WIDTH; i++) {
        for (int j=0; j<CURSOR_HEIGHT; j++) {
            cursor_pixels_curr[i][j] = GL_BLACK;
            cursor_pixels_prev[i][j] = GL_BLACK;
        }
    }

    // VARIABLE TO HOLD STATE OF WHETHER THE BUTTON CLICKS HAVE BEEN REGISTERED
    bool registered_left = false;
    bool registered_right = false;
    bool registered_middle = false;

    // LOOP
    while (!solve_pressed) {
        
        // DRAW A RECTANGLE WITH THE CURRENT COLOR IN THE TOP LEFT CORNER
        gl_draw_rect(SCREEN_WIDTH - 4 - 27, 4, 27, 27, GREY);
        gl_draw_rect(SCREEN_WIDTH - 5 - 25, 5, 25, 25, colors[color_index]);

        // GET THE NEXT EVENT
        mouse_event_t event = mouse_read_event();

        // ENSURE MOUSE IS IN BOUNDS
        if (mouse_x_curr + event.dx < 0 || mouse_x_curr + event.dx >= SCREEN_WIDTH ||
            mouse_y_curr + event.dy < 0 || mouse_y_curr + event.dy >= SCREEN_HEIGHT) {
                continue;
        }

        // DRAW THE PIXELS WHERE THE MOUSE USED TO BE
        for (int i=0; i<CURSOR_WIDTH; i++) {
            for (int j=0; j<CURSOR_HEIGHT; j++) {
                gl_draw_pixel(mouse_x_prev + i, mouse_y_prev + j, cursor_pixels_prev[i][j]);
            }
        }

        // UPDATE MOUSE COORDS
        mouse_x_prev = mouse_x_curr;
        mouse_y_prev = mouse_y_curr;
        mouse_x_curr += event.dx;
        mouse_y_curr += event.dy;
        
        // DRAW COLOR IF LEFT BUTTON CLICKED
        if (event.left && !registered_left) {
            bool valid = register_press(cube, motor_u, motor_d, motor_f, motor_b, motor_l, motor_r, mouse_x_curr, mouse_y_curr, color_index);
            
            if (valid) {
                for (int i=0; i<CURSOR_WIDTH; i++) {
                    for (int j=0; j<CURSOR_HEIGHT; j++) {
                        cursor_pixels_prev[i][j] = gl_read_pixel(mouse_x_curr + i, mouse_y_curr + j);
                    }
                }

            } else {
                for (int i=0; i<CURSOR_WIDTH; i++) {
                    for (int j=0; j<CURSOR_HEIGHT; j++) {
                        cursor_pixels_prev[i][j] = gl_read_pixel(mouse_x_prev + i, mouse_y_prev + j);
                    }
                }
            }

            registered_left = true;

        // UPDATE CURSOR COLORS
        } else {
            for (int i=0; i<CURSOR_WIDTH; i++) {
                for (int j=0; j<CURSOR_HEIGHT; j++) {
                    cursor_pixels_prev[i][j] = gl_read_pixel(mouse_x_prev + i, mouse_y_prev + j);
                }
            }
            
            if (!event.left) {
                registered_left = false;
            }
        }

        // IF MIDDLE BUTTON IS PRESSED, RESET THE LOCATION
        if (event.middle && !registered_middle) {
            bool valid = register_press(cube, motor_u, motor_d, motor_f, motor_b, motor_l, motor_r, mouse_x_curr, mouse_y_curr, 6);
            if (valid) {
                for (int i=0; i<CURSOR_WIDTH; i++) {
                    for (int j=0; j<CURSOR_HEIGHT; j++) {
                        cursor_pixels_prev[i][j] = gl_read_pixel(mouse_x_curr + i, mouse_y_curr + j);
                    }
                }

            } else {
                for (int i=0; i<CURSOR_WIDTH; i++) {
                    for (int j=0; j<CURSOR_HEIGHT; j++) {
                        cursor_pixels_prev[i][j] = gl_read_pixel(mouse_x_prev + i, mouse_y_prev + j);
                    }
                }
            }

            registered_middle = true;

        } else if (!event.middle) {
            registered_middle = false;
        }
        
        // IF THE RIGHT BUTTON HAS JUST BEEN CLICKED, INCREMENT THE COLOR
        if (event.right && !registered_right) {
            color_index = (color_index + 1) % (num_colors);
            registered_right = true;
        } else if (!event.right) {
            registered_right = false;
        }
        
        // GET WHAT IS UNDER THE CURSOR
        for (int i=0; i<CURSOR_WIDTH; i++) {
            for (int j=0; j<CURSOR_HEIGHT; j++) {
                cursor_pixels_curr[i][j] = gl_read_pixel(mouse_x_curr + i, mouse_y_curr + j);
            }
        }

        // DRAW THE CURSOR
        gl_draw_rect(mouse_x_curr, mouse_y_curr, CURSOR_WIDTH, CURSOR_HEIGHT, GL_WHITE);
        gl_draw_rect(mouse_x_curr + 1, mouse_y_curr + 1, CURSOR_WIDTH - 2, CURSOR_HEIGHT - 2, GL_BLACK);

        // SWAP THE BUFFER
        gl_swap_buffer();
    }

    // DISABLE INTERRUPTS AND SOLVE
    interrupts_global_disable();
    solve_pressed = false;
    solve_and_execute(cube, motor_u, motor_d, motor_f, motor_b, motor_l, motor_r);
}


/* 
 * This function changes the count of a certain color stored on the cube, returning
 * true if the count was able to change and false if not. 
 */
static bool change_color_count(cube_t *cube, uint8_t color, int change) {
    if (color == 0) {
        if ((cube->num_blue == 9 && change > 0) || (cube->num_blue == 0 && change < 0)) {
            return false;
        }
        cube->num_blue += change;

    } else if (color == 1) {
        if ((cube->num_red == 9 && change > 0) || (cube->num_red == 0 && change < 0)) {
            return false;
        }
        cube->num_red += change;

    } else if (color == 2) {
        if ((cube->num_green == 9 && change > 0) || (cube->num_green == 0 && change < 0)) {
            return false;
        }
        cube->num_green += change;

    } else if (color == 3) {
        if ((cube->num_orange == 9 && change > 0) || (cube->num_orange == 0 && change < 0)) {
            return false;
        }
        cube->num_orange += change;

    } else if (color == 4) {
        if ((cube->num_yellow == 9 && change > 0) || (cube->num_yellow == 0 && change < 0)) {
            return false;
        }
        cube->num_yellow += change;

    } else if (color == 5) {
        if ((cube->num_white == 9 && change > 0) || (cube->num_white == 0 && change < 0)) {
            return false;
        }
        cube->num_white += change;
    }

    return true;
}


/*
 * This function registers a mouse press and will determine which action to perform, 
 * whether it is changing the color of a tile or clicking a button. 
 */
bool register_press(cube_t *cube, motor_t *motor_u, motor_t *motor_d, motor_t *motor_f, motor_t *motor_b, motor_t *motor_l, motor_t *motor_r, int x, int y, uint8_t color) {

    // UP TURN CW
    if (x >= 72 + BUTTON_X_OFFSET && x < 72 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 57 && y < 57 + BUTTON_SIZE) {
        printf("u_turn_cw\n");
        motor_scramble_steps(motor_u, MOTOR_CW, 50);
        add_counter_move(U_CW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->up[0];
        uint8_t temp_1 = tracking_cube->up[1];
        uint8_t temp_2 = tracking_cube->up[2];
        uint8_t temp_3 = tracking_cube->up[3];
        uint8_t temp_5 = tracking_cube->up[5];
        uint8_t temp_6 = tracking_cube->up[6];
        uint8_t temp_7 = tracking_cube->up[7];
        uint8_t temp_8 = tracking_cube->up[8];

        tracking_cube->up[0] = temp_6;
        tracking_cube->up[1] = temp_3;
        tracking_cube->up[2] = temp_0;
        tracking_cube->up[3] = temp_7;
        tracking_cube->up[5] = temp_1;
        tracking_cube->up[6] = temp_8;
        tracking_cube->up[7] = temp_5;
        tracking_cube->up[8] = temp_2;

        // ROTATE ADJACENT EDGES
        uint8_t temp_left_0 = tracking_cube->left[0];
        uint8_t temp_left_1 = tracking_cube->left[1];
        uint8_t temp_left_2 = tracking_cube->left[2];

        tracking_cube->left[0] = tracking_cube->front[0];
        tracking_cube->left[1] = tracking_cube->front[1];
        tracking_cube->left[2] = tracking_cube->front[2];

        tracking_cube->front[0] = tracking_cube->right[0];
        tracking_cube->front[1] = tracking_cube->right[1];
        tracking_cube->front[2] = tracking_cube->right[2];

        tracking_cube->right[0] = tracking_cube->back[0];
        tracking_cube->right[1] = tracking_cube->back[1];
        tracking_cube->right[2] = tracking_cube->back[2];

        tracking_cube->back[0] = temp_left_0;
        tracking_cube->back[1] = temp_left_1;
        tracking_cube->back[2] = temp_left_2;

        return true;
    }

    // DOWN TURN CW
    if (x >= 72 + BUTTON_X_OFFSET && x < 72 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 157 && y < 157 + BUTTON_SIZE) {
        printf("d_turn_cw\n");
        motor_scramble_steps(motor_d, MOTOR_CW, 50);
        add_counter_move(D_CW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->down[0];
        uint8_t temp_1 = tracking_cube->down[1];
        uint8_t temp_2 = tracking_cube->down[2];
        uint8_t temp_3 = tracking_cube->down[3];
        uint8_t temp_5 = tracking_cube->down[5];
        uint8_t temp_6 = tracking_cube->down[6];
        uint8_t temp_7 = tracking_cube->down[7];
        uint8_t temp_8 = tracking_cube->down[8];

        tracking_cube->down[0] = temp_6;
        tracking_cube->down[1] = temp_3;
        tracking_cube->down[2] = temp_0;
        tracking_cube->down[3] = temp_7;
        tracking_cube->down[5] = temp_1;
        tracking_cube->down[6] = temp_8;
        tracking_cube->down[7] = temp_5;
        tracking_cube->down[8] = temp_2;

        // ROTATE ADJACENT EDGES
        uint8_t temp_front_6 = tracking_cube->front[6];
        uint8_t temp_front_7 = tracking_cube->front[7];
        uint8_t temp_front_8 = tracking_cube->front[8];

        tracking_cube->front[6] = tracking_cube->left[6];
        tracking_cube->front[7] = tracking_cube->left[7];
        tracking_cube->front[8] = tracking_cube->left[8];

        tracking_cube->left[6] = tracking_cube->back[6];
        tracking_cube->left[7] = tracking_cube->back[7];
        tracking_cube->left[8] = tracking_cube->back[8];

        tracking_cube->back[6] = tracking_cube->right[6];
        tracking_cube->back[7] = tracking_cube->right[7];
        tracking_cube->back[8] = tracking_cube->right[8];

        tracking_cube->right[6] = temp_front_6;
        tracking_cube->right[7] = temp_front_7;
        tracking_cube->right[8] = temp_front_8;

        return true;
    }

    // LEFT TURN CW
    if (x >= 22 + BUTTON_X_OFFSET && x < 22 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 107 && y < 107 + BUTTON_SIZE) {
        printf("l_turn_cw\n");
        motor_scramble_steps(motor_l, MOTOR_CW, 50);
        add_counter_move(L_CW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->left[0];
        uint8_t temp_1 = tracking_cube->left[1];
        uint8_t temp_2 = tracking_cube->left[2];
        uint8_t temp_3 = tracking_cube->left[3];
        uint8_t temp_5 = tracking_cube->left[5];
        uint8_t temp_6 = tracking_cube->left[6];
        uint8_t temp_7 = tracking_cube->left[7];
        uint8_t temp_8 = tracking_cube->left[8];

        tracking_cube->left[0] = temp_6;
        tracking_cube->left[1] = temp_3;
        tracking_cube->left[2] = temp_0;
        tracking_cube->left[3] = temp_7;
        tracking_cube->left[5] = temp_1;
        tracking_cube->left[6] = temp_8;
        tracking_cube->left[7] = temp_5;
        tracking_cube->left[8] = temp_2;

        // ROTATE ADJACENT EDGES
        uint8_t temp_up_0 = tracking_cube->up[0];
        uint8_t temp_up_3 = tracking_cube->up[3];
        uint8_t temp_up_6 = tracking_cube->up[6];

        tracking_cube->up[0] = tracking_cube->back[8];
        tracking_cube->up[3] = tracking_cube->back[5];
        tracking_cube->up[6] = tracking_cube->back[2];

        tracking_cube->back[2] = tracking_cube->down[6];
        tracking_cube->back[5] = tracking_cube->down[3];
        tracking_cube->back[8] = tracking_cube->down[0];

        tracking_cube->down[0] = tracking_cube->front[0];
        tracking_cube->down[3] = tracking_cube->front[3];
        tracking_cube->down[6] = tracking_cube->front[6];

        tracking_cube->front[0] = temp_up_0;
        tracking_cube->front[3] = temp_up_3;
        tracking_cube->front[6] = temp_up_6;

        return true;
    }

    // FRONT TURN CW
    if (x >= 72 + BUTTON_X_OFFSET && x < 72 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 107 && y < 107 + BUTTON_SIZE) {
        printf("f_turn_cw\n");
        motor_scramble_steps(motor_f, MOTOR_CW, 50);
        add_counter_move(F_CW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->front[0];
        uint8_t temp_1 = tracking_cube->front[1];
        uint8_t temp_2 = tracking_cube->front[2];
        uint8_t temp_3 = tracking_cube->front[3];
        uint8_t temp_5 = tracking_cube->front[5];
        uint8_t temp_6 = tracking_cube->front[6];
        uint8_t temp_7 = tracking_cube->front[7];
        uint8_t temp_8 = tracking_cube->front[8];

        tracking_cube->front[0] = temp_6;
        tracking_cube->front[1] = temp_3;
        tracking_cube->front[2] = temp_0;
        tracking_cube->front[3] = temp_7;
        tracking_cube->front[5] = temp_1;
        tracking_cube->front[6] = temp_8;
        tracking_cube->front[7] = temp_5;
        tracking_cube->front[8] = temp_2;

        // ROTATE ADJACENT EDGES
        uint8_t temp_up_6 = tracking_cube->up[6];
        uint8_t temp_up_7 = tracking_cube->up[7];
        uint8_t temp_up_8 = tracking_cube->up[8];

        tracking_cube->up[6] = tracking_cube->left[8];
        tracking_cube->up[7] = tracking_cube->left[5];
        tracking_cube->up[8] = tracking_cube->left[2];

        tracking_cube->left[2] = tracking_cube->down[0];
        tracking_cube->left[5] = tracking_cube->down[1];
        tracking_cube->left[8] = tracking_cube->down[2];

        tracking_cube->down[0] = tracking_cube->right[6];
        tracking_cube->down[1] = tracking_cube->right[3];
        tracking_cube->down[2] = tracking_cube->right[0];

        tracking_cube->right[0] = temp_up_6;
        tracking_cube->right[3] = temp_up_7;
        tracking_cube->right[6] = temp_up_8;

        return true;
    }

    // RIGHT TURN CW
    if (x >= 122 + BUTTON_X_OFFSET && x < 122 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 107 && y < 107 + BUTTON_SIZE) {
        printf("r_turn_cw\n");
        motor_scramble_steps(motor_r, MOTOR_CW, 50);
        add_counter_move(R_CW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->right[0];
        uint8_t temp_1 = tracking_cube->right[1];
        uint8_t temp_2 = tracking_cube->right[2];
        uint8_t temp_3 = tracking_cube->right[3];
        uint8_t temp_5 = tracking_cube->right[5];
        uint8_t temp_6 = tracking_cube->right[6];
        uint8_t temp_7 = tracking_cube->right[7];
        uint8_t temp_8 = tracking_cube->right[8];

        tracking_cube->right[0] = temp_6;
        tracking_cube->right[1] = temp_3;
        tracking_cube->right[2] = temp_0;
        tracking_cube->right[3] = temp_7;
        tracking_cube->right[5] = temp_1;
        tracking_cube->right[6] = temp_8;
        tracking_cube->right[7] = temp_5;
        tracking_cube->right[8] = temp_2;

        // ROTATE ADJACENT EDGES
        uint8_t temp_up_2 = tracking_cube->up[2];
        uint8_t temp_up_5 = tracking_cube->up[5];
        uint8_t temp_up_8 = tracking_cube->up[8];

        tracking_cube->up[2] = tracking_cube->front[2];
        tracking_cube->up[5] = tracking_cube->front[5];
        tracking_cube->up[8] = tracking_cube->front[8];

        tracking_cube->front[2] = tracking_cube->down[2];
        tracking_cube->front[5] = tracking_cube->down[5];
        tracking_cube->front[8] = tracking_cube->down[8];

        tracking_cube->down[2] = tracking_cube->back[6];
        tracking_cube->down[5] = tracking_cube->back[3];
        tracking_cube->down[8] = tracking_cube->back[0];

        tracking_cube->back[0] = temp_up_8;
        tracking_cube->back[3] = temp_up_5;
        tracking_cube->back[6] = temp_up_2;

        return true;
    }

    // BACK TURN CW
    if (x >= 172 + BUTTON_X_OFFSET && x < 172 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 107 && y < 107 + BUTTON_SIZE) {
        printf("b_turn_cw\n");
        motor_scramble_steps(motor_b, MOTOR_CW, 50);
        add_counter_move(B_CW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->back[0];
        uint8_t temp_1 = tracking_cube->back[1];
        uint8_t temp_2 = tracking_cube->back[2];
        uint8_t temp_3 = tracking_cube->back[3];
        uint8_t temp_5 = tracking_cube->back[5];
        uint8_t temp_6 = tracking_cube->back[6];
        uint8_t temp_7 = tracking_cube->back[7];
        uint8_t temp_8 = tracking_cube->back[8];

        tracking_cube->back[0] = temp_6;
        tracking_cube->back[1] = temp_3;
        tracking_cube->back[2] = temp_0;
        tracking_cube->back[3] = temp_7;
        tracking_cube->back[5] = temp_1;
        tracking_cube->back[6] = temp_8;
        tracking_cube->back[7] = temp_5;
        tracking_cube->back[8] = temp_2;

        // ROTATE ADJACENT EDGES
        uint8_t temp_up_0 = tracking_cube->up[0];
        uint8_t temp_up_1 = tracking_cube->up[1];
        uint8_t temp_up_2 = tracking_cube->up[2];

        tracking_cube->up[0] = tracking_cube->right[2];
        tracking_cube->up[1] = tracking_cube->right[5];
        tracking_cube->up[2] = tracking_cube->right[8];

        tracking_cube->right[2] = tracking_cube->down[8];
        tracking_cube->right[5] = tracking_cube->down[7];
        tracking_cube->right[8] = tracking_cube->down[6];

        tracking_cube->down[6] = tracking_cube->left[0];
        tracking_cube->down[7] = tracking_cube->left[3];
        tracking_cube->down[8] = tracking_cube->left[6];

        tracking_cube->left[0] = temp_up_2;
        tracking_cube->left[3] = temp_up_1;
        tracking_cube->left[6] = temp_up_0;

        return true;
    }

    // UP TURN CCW
    if (x >= 72 + BUTTON_X_OFFSET && x < 72 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 57 + BUTTON_Y_OFFSET && y < 57 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("u_turn_ccw\n");
        motor_scramble_steps(motor_u, MOTOR_CCW, 50);
        add_counter_move(U_CCW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->up[0];
        uint8_t temp_1 = tracking_cube->up[1];
        uint8_t temp_2 = tracking_cube->up[2];
        uint8_t temp_3 = tracking_cube->up[3];
        uint8_t temp_5 = tracking_cube->up[5];
        uint8_t temp_6 = tracking_cube->up[6];
        uint8_t temp_7 = tracking_cube->up[7];
        uint8_t temp_8 = tracking_cube->up[8];

        tracking_cube->up[0] = temp_2;
        tracking_cube->up[1] = temp_5;
        tracking_cube->up[2] = temp_8;
        tracking_cube->up[3] = temp_1;
        tracking_cube->up[5] = temp_7;
        tracking_cube->up[6] = temp_0;
        tracking_cube->up[7] = temp_3;
        tracking_cube->up[8] = temp_6;

        // ROTATE ADJACENT EDGES
        uint8_t temp_left_0 = tracking_cube->left[0];
        uint8_t temp_left_1 = tracking_cube->left[1];
        uint8_t temp_left_2 = tracking_cube->left[2];

        tracking_cube->left[0] = tracking_cube->back[0];
        tracking_cube->left[1] = tracking_cube->back[1];
        tracking_cube->left[2] = tracking_cube->back[2];

        tracking_cube->back[0] = tracking_cube->right[0];
        tracking_cube->back[1] = tracking_cube->right[1];
        tracking_cube->back[2] = tracking_cube->right[2];

        tracking_cube->right[0] = tracking_cube->front[0];
        tracking_cube->right[1] = tracking_cube->front[1];
        tracking_cube->right[2] = tracking_cube->front[2];

        tracking_cube->front[0] = temp_left_0;
        tracking_cube->front[1] = temp_left_1;
        tracking_cube->front[2] = temp_left_2;

        return true;
    }

    // DOWN TURN CCW
    if (x >= 72 + BUTTON_X_OFFSET && x < 72 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 157 + BUTTON_Y_OFFSET && y < 157 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("d_turn_ccw\n");
        motor_scramble_steps(motor_d, MOTOR_CCW, 50);
        add_counter_move(D_CCW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->down[0];
        uint8_t temp_1 = tracking_cube->down[1];
        uint8_t temp_2 = tracking_cube->down[2];
        uint8_t temp_3 = tracking_cube->down[3];
        uint8_t temp_5 = tracking_cube->down[5];
        uint8_t temp_6 = tracking_cube->down[6];
        uint8_t temp_7 = tracking_cube->down[7];
        uint8_t temp_8 = tracking_cube->down[8];

        tracking_cube->down[0] = temp_2;
        tracking_cube->down[1] = temp_5;
        tracking_cube->down[2] = temp_8;
        tracking_cube->down[3] = temp_1;
        tracking_cube->down[5] = temp_7;
        tracking_cube->down[6] = temp_0;
        tracking_cube->down[7] = temp_3;
        tracking_cube->down[8] = temp_6;

        // ROTATE ADJACENT EDGES
        uint8_t temp_front_6 = tracking_cube->front[6];
        uint8_t temp_front_7 = tracking_cube->front[7];
        uint8_t temp_front_8 = tracking_cube->front[8];

        tracking_cube->front[6] = tracking_cube->right[6];
        tracking_cube->front[7] = tracking_cube->right[7];
        tracking_cube->front[8] = tracking_cube->right[8];

        tracking_cube->right[6] = tracking_cube->back[6];
        tracking_cube->right[7] = tracking_cube->back[7];
        tracking_cube->right[8] = tracking_cube->back[8];

        tracking_cube->back[6] = tracking_cube->left[6];
        tracking_cube->back[7] = tracking_cube->left[7];
        tracking_cube->back[8] = tracking_cube->left[8];

        tracking_cube->left[6] = temp_front_6;
        tracking_cube->left[7] = temp_front_7;
        tracking_cube->left[8] = temp_front_8;

        return true;
    }

    // LEFT TURN CCW
    if (x >= 22 + BUTTON_X_OFFSET && x < 22 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 107 + BUTTON_Y_OFFSET && y < 107 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("l_turn_ccw\n");
        motor_scramble_steps(motor_l, MOTOR_CCW, 50);
        add_counter_move(L_CCW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->left[0];
        uint8_t temp_1 = tracking_cube->left[1];
        uint8_t temp_2 = tracking_cube->left[2];
        uint8_t temp_3 = tracking_cube->left[3];
        uint8_t temp_5 = tracking_cube->left[5];
        uint8_t temp_6 = tracking_cube->left[6];
        uint8_t temp_7 = tracking_cube->left[7];
        uint8_t temp_8 = tracking_cube->left[8];

        tracking_cube->left[0] = temp_2;
        tracking_cube->left[1] = temp_5;
        tracking_cube->left[2] = temp_8;
        tracking_cube->left[3] = temp_1;
        tracking_cube->left[5] = temp_7;
        tracking_cube->left[6] = temp_0;
        tracking_cube->left[7] = temp_3;
        tracking_cube->left[8] = temp_6;

        // ROTATE ADJACENT EDGES
        uint8_t temp_up_0 = tracking_cube->up[0];
        uint8_t temp_up_3 = tracking_cube->up[3];
        uint8_t temp_up_6 = tracking_cube->up[6];

        tracking_cube->up[0] = tracking_cube->front[0];
        tracking_cube->up[3] = tracking_cube->front[3];
        tracking_cube->up[6] = tracking_cube->front[6];

        tracking_cube->front[0] = tracking_cube->down[0];
        tracking_cube->front[3] = tracking_cube->down[3];
        tracking_cube->front[6] = tracking_cube->down[6];

        tracking_cube->down[0] = tracking_cube->back[8];
        tracking_cube->down[3] = tracking_cube->back[5];
        tracking_cube->down[6] = tracking_cube->back[2];

        tracking_cube->back[2] = temp_up_6;
        tracking_cube->back[5] = temp_up_3;
        tracking_cube->back[8] = temp_up_0;

        return true;
    }

    // FRONT TURN CCW
    if (x >= 72 + BUTTON_X_OFFSET && x < 72 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 107 + BUTTON_Y_OFFSET && y < 107 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("f_turn_ccw\n");
        motor_scramble_steps(motor_f, MOTOR_CCW, 50);
        add_counter_move(F_CCW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->front[0];
        uint8_t temp_1 = tracking_cube->front[1];
        uint8_t temp_2 = tracking_cube->front[2];
        uint8_t temp_3 = tracking_cube->front[3];
        uint8_t temp_5 = tracking_cube->front[5];
        uint8_t temp_6 = tracking_cube->front[6];
        uint8_t temp_7 = tracking_cube->front[7];
        uint8_t temp_8 = tracking_cube->front[8];

        tracking_cube->front[0] = temp_2;
        tracking_cube->front[1] = temp_5;
        tracking_cube->front[2] = temp_8;
        tracking_cube->front[3] = temp_1;
        tracking_cube->front[5] = temp_7;
        tracking_cube->front[6] = temp_0;
        tracking_cube->front[7] = temp_3;
        tracking_cube->front[8] = temp_6;

        // ROTATE ADJACENT EDGES
        uint8_t temp_up_6 = tracking_cube->up[6];
        uint8_t temp_up_7 = tracking_cube->up[7];
        uint8_t temp_up_8 = tracking_cube->up[8];

        tracking_cube->up[6] = tracking_cube->right[0];
        tracking_cube->up[7] = tracking_cube->right[3];
        tracking_cube->up[8] = tracking_cube->right[6];

        tracking_cube->right[0] = tracking_cube->down[2];
        tracking_cube->right[3] = tracking_cube->down[1];
        tracking_cube->right[6] = tracking_cube->down[0];

        tracking_cube->down[0] = tracking_cube->left[2];
        tracking_cube->down[1] = tracking_cube->left[5];
        tracking_cube->down[2] = tracking_cube->left[8];

        tracking_cube->left[2] = temp_up_8;
        tracking_cube->left[5] = temp_up_7;
        tracking_cube->left[8] = temp_up_6;

        return true;
    }

    // RIGHT TURN CCW
    if (x >= 122 + BUTTON_X_OFFSET && x < 122 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 107 + BUTTON_Y_OFFSET && y < 107 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("r_turn_ccw\n");
        motor_scramble_steps(motor_r, MOTOR_CCW, 50);
        add_counter_move(R_CCW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->right[0];
        uint8_t temp_1 = tracking_cube->right[1];
        uint8_t temp_2 = tracking_cube->right[2];
        uint8_t temp_3 = tracking_cube->right[3];
        uint8_t temp_5 = tracking_cube->right[5];
        uint8_t temp_6 = tracking_cube->right[6];
        uint8_t temp_7 = tracking_cube->right[7];
        uint8_t temp_8 = tracking_cube->right[8];

        tracking_cube->right[0] = temp_2;
        tracking_cube->right[1] = temp_5;
        tracking_cube->right[2] = temp_8;
        tracking_cube->right[3] = temp_1;
        tracking_cube->right[5] = temp_7;
        tracking_cube->right[6] = temp_0;
        tracking_cube->right[7] = temp_3;
        tracking_cube->right[8] = temp_6;

        // ROTATE ADJACENT EDGES
        uint8_t temp_up_2 = tracking_cube->up[2];
        uint8_t temp_up_5 = tracking_cube->up[5];
        uint8_t temp_up_8 = tracking_cube->up[8];

        tracking_cube->up[2] = tracking_cube->back[6];
        tracking_cube->up[5] = tracking_cube->back[3];
        tracking_cube->up[8] = tracking_cube->back[0];

        tracking_cube->back[0] = tracking_cube->down[8];
        tracking_cube->back[3] = tracking_cube->down[5];
        tracking_cube->back[6] = tracking_cube->down[2];

        tracking_cube->down[2] = tracking_cube->front[2];
        tracking_cube->down[5] = tracking_cube->front[5];
        tracking_cube->down[8] = tracking_cube->front[8];

        tracking_cube->front[2] = temp_up_2;
        tracking_cube->front[5] = temp_up_5;
        tracking_cube->front[8] = temp_up_8;

        return true;
    }

    // BACK TURN CCW
    if (x >= 172 + BUTTON_X_OFFSET && x < 172 + BUTTON_X_OFFSET + BUTTON_SIZE && y >= 107 + BUTTON_Y_OFFSET && y < 107 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("b_turn_ccw\n");
        motor_scramble_steps(motor_b, MOTOR_CCW, 50);
        add_counter_move(B_CCW);

        // ROTATE FACE
        uint8_t temp_0 = tracking_cube->back[0];
        uint8_t temp_1 = tracking_cube->back[1];
        uint8_t temp_2 = tracking_cube->back[2];
        uint8_t temp_3 = tracking_cube->back[3];
        uint8_t temp_5 = tracking_cube->back[5];
        uint8_t temp_6 = tracking_cube->back[6];
        uint8_t temp_7 = tracking_cube->back[7];
        uint8_t temp_8 = tracking_cube->back[8];

        tracking_cube->back[0] = temp_2;
        tracking_cube->back[1] = temp_5;
        tracking_cube->back[2] = temp_8;
        tracking_cube->back[3] = temp_1;
        tracking_cube->back[5] = temp_7;
        tracking_cube->back[6] = temp_0;
        tracking_cube->back[7] = temp_3;
        tracking_cube->back[8] = temp_6;

        // ROTATE ADJACENT EDGES
        uint8_t temp_up_0 = tracking_cube->up[0];
        uint8_t temp_up_1 = tracking_cube->up[1];
        uint8_t temp_up_2 = tracking_cube->up[2];

        tracking_cube->up[0] = tracking_cube->left[6];
        tracking_cube->up[1] = tracking_cube->left[3];
        tracking_cube->up[2] = tracking_cube->left[0];

        tracking_cube->left[0] = tracking_cube->down[6];
        tracking_cube->left[3] = tracking_cube->down[7];
        tracking_cube->left[6] = tracking_cube->down[8];

        tracking_cube->down[6] = tracking_cube->right[8];
        tracking_cube->down[7] = tracking_cube->right[5];
        tracking_cube->down[8] = tracking_cube->right[2];

        tracking_cube->right[2] = temp_up_0;
        tracking_cube->right[5] = temp_up_1;
        tracking_cube->right[8] = temp_up_2;

        return true;
    }

    // -------------

    // UP STEP CW
    if (x >= 72 && x < 72 + BUTTON_SIZE && y >= 57 && y < 57 + BUTTON_SIZE) {
        printf("u_step_cw\n");
        motor_scramble_steps(motor_u, MOTOR_CW, 1);
        return true;
    }

    // DOWN STEP CW
    if (x >= 72 && x < 72 + BUTTON_SIZE && y >= 157 && y < 157 + BUTTON_SIZE) {
        printf("d_step_cw\n");
        motor_scramble_steps(motor_d, MOTOR_CW, 1);
        return true;
    }

    // LEFT STEP CW
    if (x >= 22 && x < 22 + BUTTON_SIZE && y >= 107 && y < 107 + BUTTON_SIZE) {
        printf("l_step_cw\n");
        motor_scramble_steps(motor_l, MOTOR_CW, 1);
        return true;
    }

    // FRONT STEP CW
    if (x >= 72 && x < 72 + BUTTON_SIZE && y >= 107 && y < 107 + BUTTON_SIZE) {
        printf("f_step_cw\n");
        motor_scramble_steps(motor_f, MOTOR_CW, 1);
        return true;
    }

    // RIGHT STEP CW
    if (x >= 122 && x < 122 + BUTTON_SIZE && y >= 107 && y < 107 + BUTTON_SIZE) {
        printf("r_step_cw\n");
        motor_scramble_steps(motor_r, MOTOR_CW, 1);
        return true;
    }

    // BACK STEP CW
    if (x >= 172 && x < 172 + BUTTON_SIZE && y >= 107 && y < 107 + BUTTON_SIZE) {
        printf("b_step_cw\n");
        motor_scramble_steps(motor_b, MOTOR_CW, 1);
        return true;
    }

    // UP STEP CCW
    if (x >= 72 && x < 72 + BUTTON_SIZE && y >= 57 + BUTTON_Y_OFFSET && y < 57 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("u_step_ccw\n");
        motor_scramble_steps(motor_u, MOTOR_CCW, 1);
        return true;
    }

    // DOWN STEP CCW
    if (x >= 72 && x < 72 + BUTTON_SIZE && y >= 157 + BUTTON_Y_OFFSET && y < 157 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("d_step_ccw\n");
        motor_scramble_steps(motor_d, MOTOR_CCW, 1);
        return true;
    }

    // LEFT STEP CCW
    if (x >= 22 && x < 22 + BUTTON_SIZE && y >= 107 + BUTTON_Y_OFFSET && y < 107 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("l_step_ccw\n");
        motor_scramble_steps(motor_l, MOTOR_CCW, 1);
        return true;
    }

    // FRONT STEP CCW
    if (x >= 72 && x < 72 + BUTTON_SIZE && y >= 107 + BUTTON_Y_OFFSET && y < 107 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("f_step_ccw\n");
        motor_scramble_steps(motor_f, MOTOR_CCW, 1);
        return true;
    }

    // RIGHT STEP CCW
    if (x >= 122 && x < 122 + BUTTON_SIZE && y >= 107 + BUTTON_Y_OFFSET && y < 107 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("r_step_ccw\n");
        motor_scramble_steps(motor_r, MOTOR_CCW, 1);
        return true;
    }

    // BACK STEP CCW
    if (x >= 172 && x < 172 + BUTTON_SIZE && y >= 107 + BUTTON_Y_OFFSET && y < 107 + BUTTON_Y_OFFSET + BUTTON_SIZE) {
        printf("b_step_ccw\n");
        motor_scramble_steps(motor_b, MOTOR_CCW, 1);
        return true;
    }

    // SOLVE
    if (x >= 248 && x < 248 + SIDE_SIZE - 6 && y >= 13 && y < 57) {
        if (is_full(cube)) {
            solve_pressed = true;
        }

        return true;
    }

    // AUTOFILL
    if (x >= 248 && x < 248 + SIDE_SIZE - 6 && y >= 543 && y < 587) {
        memcpy(cube->up, tracking_cube->up, 9);
        memcpy(cube->down, tracking_cube->down, 9);
        memcpy(cube->front, tracking_cube->front, 9);
        memcpy(cube->back, tracking_cube->back, 9);
        memcpy(cube->left, tracking_cube->left, 9);
        memcpy(cube->right, tracking_cube->right, 9);

        cube->num_blue = cube->num_red = cube->num_green = cube->num_white = 
        cube->num_yellow = cube->num_orange = 9;
        
        display_cube(cube);
        display_buttons();
        gl_swap_buffer();
        display_cube(cube);
        display_buttons();

        return true;
    }

    // USER IS COLORING A TILE
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            if (3*i + j == 4) {
                continue;
            }

            // TILE PRESSED IN UP REGION
            if (x >= 255 + 48*j && x < 299 + 48*j && y >= 80 + 48*i && y < 124 + 48*i) {
                if (!change_color_count(cube, color, 1)) {
                    return false;
                }
                
                change_color_count(cube, cube->up[3*i + j], -1);
                cube->up[3*i + j] = color;
                gl_draw_rect(255 + 48*j, 80 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                gl_draw_rect(255 + 48*j, 80 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                return true;
            }

            // TILE PRESSED IN LEFT REGION
            if (x >= 105 + 48*j && x < 149 + 48*j && y >= 230 + 48*i && y < 274 + 48*i) {
                if (!change_color_count(cube, color, 1)) {
                    return false;
                }
                
                change_color_count(cube, cube->left[3*i + j], -1);
                cube->left[3*i + j] = color;
                gl_draw_rect(105 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                gl_draw_rect(105 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                return true;
            }

            // TILE PRESSED IN FRONT REGION
            if (x >= 255 + 48*j && x < 299 + 48*j && y >= 230 + 48*i && y < 274 + 48*i) {
                if (!change_color_count(cube, color, 1)) {
                    return false;
                }
                
                change_color_count(cube, cube->front[3*i + j], -1);
                cube->front[3*i + j] = color;
                gl_draw_rect(255 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                gl_draw_rect(255 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                return true;
            }

            // TILE PRESSED IN RIGHT REGION
            if (x >= 405 + 48*j && x < 449 + 48*j && y >= 230 + 48*i && y < 274 + 48*i) {
                if (!change_color_count(cube, color, 1)) {
                    return false;
                }
                
                change_color_count(cube, cube->right[3*i + j], -1);
                cube->right[3*i + j] = color;
                gl_draw_rect(405 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                gl_draw_rect(405 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                return true;
            }

            // TILE PRESSED IN BACK REGION
            if (x >= 555 + 48*j && x < 599 + 48*j && y >= 230 + 48*i && y < 274 + 48*i) {
                if (!change_color_count(cube, color, 1)) {
                    return false;
                }
                
                change_color_count(cube, cube->back[3*i + j], -1);
                cube->back[3*i + j] = color;
                gl_draw_rect(555 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                gl_draw_rect(555 + 48*j, 230 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                return true;
            }

            // TILE PRESSED IN DOWN REGION
            if (x >= 255 + 48*j && x < 299 + 48*j && y >= 380 + 48*i && y < 424 + 48*i) {
                if (!change_color_count(cube, color, 1)) {
                    return false;
                }
                
                change_color_count(cube, cube->down[3*i + j], -1);
                cube->down[3*i + j] = color;
                gl_draw_rect(255 + 48*j, 380 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                gl_draw_rect(255 + 48*j, 380 + 48*i, TILE_SIZE, TILE_SIZE, colors[color]);
                gl_swap_buffer();
                return true;
            }
        }
    }

    return false;
}


/*
 * This function returns a char representation of a tile. 
 */
static char get_tile_char(uint8_t num) {
    if (num == CUBE_UP) {
        return 'U';
    } else if (num == CUBE_DOWN) {
        return 'D';
    } else if (num == CUBE_FRONT) {
        return 'F';
    } else if (num == CUBE_BACK) {
        return 'B';
    } else if (num == CUBE_LEFT) {
        return 'L';
    } else if (num == CUBE_RIGHT){
        return 'R';
    } else{
	    return '?';
    }
}


// THE INPUT WILL BE 9 TILES * 6 SIDES + NULL TERMINATOR = 55
#define INPUT_SIZE 55


/* 
 * This fuction will build a string representation of the cube, setting input 
 * to the representation. 
 */
static void build_solver_input(cube_t *cube, char input[INPUT_SIZE]) {
    
    // SET INITIAL CHARACTER TO NULL TERMINATOR
    input[0] = '\0';

    // CREATE TEMPORARY STRING TO GET EACH CHARACTER AND CONCATONATE
    char temp[2];
    temp[1] = '\0';

    // READ UP FACE
    for (int i = 0; i < 9; i++) {
        temp[0] = get_tile_char(cube->up[i]);
        strlcat(input, temp, INPUT_SIZE);
    }

    // READ DOWN FACE
    for (int i = 0; i < 9; i++) {  
        temp[0] = get_tile_char(cube->down[i]);
        strlcat(input, temp, INPUT_SIZE);
    }

    // READ FRONT FACE
    for (int i = 0; i < 9; i++) { 
        temp[0] = get_tile_char(cube->front[i]);
        strlcat(input, temp, INPUT_SIZE);
    }

    // READ BACK FACE
    for (int i = 0; i < 9; i++) { 
        temp[0] = get_tile_char(cube->back[i]);
        strlcat(input, temp, INPUT_SIZE);
    }

    // READ LEFT FACE
    for (int i = 0; i < 9; i++) { 
        temp[0] = get_tile_char(cube->left[i]);
        strlcat(input, temp, INPUT_SIZE);
    }

    // READ RIGHT FACE
    for (int i = 0; i < 9; i++) { 
        temp[0] = get_tile_char(cube->right[i]);
        strlcat(input, temp, INPUT_SIZE);
    }
}


/* 
 * This function executes each move of the solver. 
 */
static void execute_solver_move(int move,
                                motor_t *motor_u, motor_t *motor_d,
                                motor_t *motor_f, motor_t *motor_b,
                                motor_t *motor_l, motor_t *motor_r) {
    switch (move) {
        case U_CW:   motor_steps(motor_u, MOTOR_CW, 50); break;
        case U_CCW:  motor_steps(motor_u, MOTOR_CCW, 50); break;
        case U_180:  motor_steps(motor_u, MOTOR_CW, 100); break;

        case D_CW:   motor_steps(motor_d, MOTOR_CW, 50); break;
        case D_CCW:  motor_steps(motor_d, MOTOR_CCW, 50); break;
        case D_180:  motor_steps(motor_d, MOTOR_CW, 100); break;

        case F_CW:   motor_steps(motor_f, MOTOR_CW, 50); break;
        case F_CCW:  motor_steps(motor_f, MOTOR_CCW, 50); break;
        case F_180:  motor_steps(motor_f, MOTOR_CW, 100); break;

        case B_CW:   motor_steps(motor_b, MOTOR_CW, 50); break;
        case B_CCW:  motor_steps(motor_b, MOTOR_CCW, 50); break;
        case B_180:  motor_steps(motor_b, MOTOR_CW, 100); break;

        case L_CW:   motor_steps(motor_l, MOTOR_CW, 50); break;
        case L_CCW:  motor_steps(motor_l, MOTOR_CCW, 50); break;
        case L_180:  motor_steps(motor_l, MOTOR_CW, 100); break;

        case R_CW:   motor_steps(motor_r, MOTOR_CW, 50); break;
        case R_CCW:  motor_steps(motor_r, MOTOR_CCW, 50); break;
        case R_180:  motor_steps(motor_r, MOTOR_CW, 100); break;
    }
}


/*
 * This function...
 */
static void solve_and_execute(cube_t *cube,
                              motor_t *motor_u, motor_t *motor_d,
                              motor_t *motor_f, motor_t *motor_b,
                              motor_t *motor_l, motor_t *motor_r) {
    
    // CREATE INPUT AND MOVES STRINGS
    char input[INPUT_SIZE];
    int moves[MAX_SOLUTION_MOVES];
    int move_count = 0;

    // STORE ALL MOTOR POINTERS
    motor_t *motors[6] = {motor_u, motor_d, motor_f, motor_b, motor_l, motor_r};
    
    // PRINT SOLUTION INFO
    printf("center values numeric: U=%d D=%d F=%d B=%d L=%d R=%d\n",
        cube->up[4], cube->down[4], cube->front[4],
        cube->back[4], cube->left[4], cube->right[4]);

    printf("center values chars:   U=%c D=%c F=%c B=%c L=%c R=%c\n",
        get_tile_char(cube->up[4]),
        get_tile_char(cube->down[4]),
        get_tile_char(cube->front[4]),  
        get_tile_char(cube->back[4]),
        get_tile_char(cube->left[4]),
        get_tile_char(cube->right[4]));
    
    // BUILD THE SOLVER INPUT
    build_solver_input(cube, input);
    
    // PRINT OUT SOLUTION INFO
    printf("solver input: %s\n", input);
    printf("input centers chars:   U=%c D=%c F=%c B=%c L=%c R=%c\n",
        input[4], input[13], input[22], input[31], input[40], input[49]);

    // DISPLAY LOADING SCREEN
    int len = 10;
    gl_clear(GL_WHITE);
    gl_draw_string(SCREEN_WIDTH/2 - (len*gl_get_char_width())/2, 
                   SCREEN_HEIGHT/2 - gl_get_char_height()/2, "Loading...", GL_BLACK);
    gl_swap_buffer();

    if (!solver_get_solution(input, moves, &move_count)) {
        printf("solver failed\n");
        return;
    }

    // DISPLAY COUNTDOWN
    len = 10;
    gl_clear(GL_WHITE);
    gl_draw_string(SCREEN_WIDTH/2 - (len*gl_get_char_width())/2, 
                   SCREEN_HEIGHT/2 - gl_get_char_height()/2 - gl_get_char_height()*2, 
                   "Solving in", GL_BLACK);
    gl_swap_buffer();

    timer_delay_ms(500);

    gl_clear(GL_WHITE);
    gl_draw_string(SCREEN_WIDTH/2 - (len*gl_get_char_width())/2, 
                   SCREEN_HEIGHT/2 - gl_get_char_height()/2 - gl_get_char_height()*2, 
                   "Solving in", GL_BLACK);
    gl_draw_string(SCREEN_WIDTH/2 - (gl_get_char_width())/2, 
                   SCREEN_HEIGHT/2 - gl_get_char_height()/2 + gl_get_char_height()*2, 
                   "3", GL_BLACK);
    gl_swap_buffer();

    timer_delay(1);

    gl_clear(GL_WHITE);
    gl_draw_string(SCREEN_WIDTH/2 - (len*gl_get_char_width())/2, 
                   SCREEN_HEIGHT/2 - gl_get_char_height()/2 - gl_get_char_height()*2, 
                   "Solving in", GL_BLACK);
    gl_draw_string(SCREEN_WIDTH/2 - (gl_get_char_width())/2, 
                   SCREEN_HEIGHT/2 - gl_get_char_height()/2 + gl_get_char_height()*2, 
                   "2", GL_BLACK);
    gl_swap_buffer();

    timer_delay(1);

    gl_clear(GL_WHITE);
    gl_draw_string(SCREEN_WIDTH/2 - (len*gl_get_char_width())/2, 
                   SCREEN_HEIGHT/2 - gl_get_char_height()/2 - gl_get_char_height()*2, 
                   "Solving in", GL_BLACK);
    gl_draw_string(SCREEN_WIDTH/2 - (gl_get_char_width())/2, 
                   SCREEN_HEIGHT/2 - gl_get_char_height()/2 + gl_get_char_height()*2, 
                   "1", GL_BLACK);
    gl_swap_buffer();

    timer_delay(1);

    len = 10;
    gl_clear(GL_WHITE);
    gl_draw_string(SCREEN_WIDTH/2 - (len*gl_get_char_width())/2, 
                   SCREEN_HEIGHT/2 - gl_get_char_height()/2, "Solving...", GL_BLACK);
    gl_swap_buffer();

    // PRINT SOLUTION LENGTH
    printf("solution length = %d\n", move_count);

    // ENABLE ALL MOTORS FOR SOLVING
    for (int i = 0; i < 6; i++) {
        motor_enablefn(motors[i], MOTOR_ON);
    }

    // EXECUTE EACH MOVE
    for (int i = 0; i < move_count; i++) {
        printf("executing move %d = %d\n", i, moves[i]);
        execute_solver_move(moves[i], motor_u, motor_d, motor_f, motor_b, motor_l, motor_r);
    }

    // DISABLE ALL MOTORS
    for (int i = 0; i < 6; i++) {
        motor_enablefn(motors[i], MOTOR_OFF);
    }


    // DISPLAY PLAY AGAIN
    len = 10;
    gl_clear(GL_WHITE);
    gl_draw_string(SCREEN_WIDTH/2 - (len*gl_get_char_width())/2, 
                   SCREEN_HEIGHT/2 - gl_get_char_height()/2 - gl_get_char_height()*2, 
                   "Solved!!", GL_BLACK);
    
    len = 29;
    gl_draw_string(SCREEN_WIDTH/2 - (len*gl_get_char_width())/2, 
                   SCREEN_HEIGHT/2 - gl_get_char_height()/2 + gl_get_char_height()*2, 
                   "Click anywhere to play again!", GL_MAGENTA);
    gl_swap_buffer();

    interrupts_global_enable();
}