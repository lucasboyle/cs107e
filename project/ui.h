#ifndef _UI_H
#define _UI_H

/*
 * Lucas Boyle
 * CS 107e
 * 3/17/26
 *
 * File: ui.h
 * -----
 * This module configures a user interface for a rubiks cube solver. 
 */


#include <stdint.h>
#include <stdbool.h>
#include "motor.h"


// SCREEN DIMENSIONS
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_DEPTH 4


// ENUM TO TRACK TILES ORIENTATIONS
typedef enum {
    CUBE_UP = 0,
    CUBE_DOWN = 1,
    CUBE_FRONT = 2,
    CUBE_BACK = 3,
    CUBE_LEFT = 4, 
    CUBE_RIGHT = 5,
    CUBE_EMPTY = 6
} cube_tile_t;


// STRUCT TO HOLD PERTINENT INFORMATION FOR THE RUBIKS CUBE
typedef struct {
    uint8_t *up;
    uint8_t *down;
    uint8_t *left;
    uint8_t *front;
    uint8_t *right;
    uint8_t *back;

    int num_blue;
    int num_red;
    int num_green;
    int num_orange;
    int num_yellow;
    int num_white;
} cube_t;


// STRUCT TO HOLD INFORMATION ABOUT THE CUBE AS TRACKED BY BUTTON PRESSES
typedef struct {
    uint8_t *up;
    uint8_t *down;
    uint8_t *left;
    uint8_t *front;
    uint8_t *right;
    uint8_t *back;
} tracking_cube_t;


/*
 * This function should initialize the screen size associated with the constants above and
 * should allocate the memory assicated with each side in a cube_t struct.
 */
cube_t *ui_init(void);


void free_all(cube_t *cube);


/*
 * This function displays the current state of a single side of the cube
 */
void display_side(cube_t *cube, uint8_t side_num);


/*
 * This function should display the current state of the cube in a flattened format to the
 * HDMI display.
 */
void display_cube(cube_t *cube);


/*
 * This function should display all of the buttons to the HDMI display. 
 */
void display_buttons(void);


/*
 * This function should loop through allowing the user to use a mouse to select the starting
 * configuration of the cube. 
 */
void user_input(cube_t *cube, motor_t *motor_u, motor_t *motor_d, motor_t *motor_f, motor_t *motor_b, motor_t *motor_l, motor_t *motor_r);


/* 
 * This function should wait for a mouse click, determine which button on the screen is pressed
 * (if any) and register the correct corresponding action. If a location on the cube is selected,
 * it should update the cube. If a movement button is selected, it should call upon motor.c for
 * the appropriate action. If the solve button is pressed, a verification is conducted to ensure
 * every location is filled and will then call upon the algorithm to solve the cube. 
 */
bool register_press(cube_t *cube, motor_t *motor_u, motor_t *motor_d, motor_t *motor_f, motor_t *motor_b, motor_t *motor_l, motor_t *motor_r, int x, int y, uint8_t color);


#endif