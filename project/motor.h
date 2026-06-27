#ifndef MOTOR_H
#define MOTOR_H

/* motor.h
 *
 * Header file for motor: Continously updating to add stepping functions interfacing with gpio_pins
 *
 */

#include <stdint.h>
#include "gpio.h"

//all my motor pin set up, following sketch on my ipad, also on the mango-pi
#define MOTOR_F_ENABLE_PIN GPIO_PB2 
#define MOTOR_F_STEP_PIN GPIO_PB3
#define MOTOR_F_DIR_PIN GPIO_PB4 

#define MOTOR_L_ENABLE_PIN GPIO_PB12
#define MOTOR_L_STEP_PIN GPIO_PB6
#define MOTOR_L_DIR_PIN GPIO_PD17 

#define MOTOR_U_ENABLE_PIN GPIO_PC0
#define MOTOR_U_STEP_PIN GPIO_PB10
#define MOTOR_U_DIR_PIN GPIO_PB11

#define MOTOR_D_ENABLE_PIN GPIO_PC1
#define MOTOR_D_STEP_PIN GPIO_PD10
#define MOTOR_D_DIR_PIN GPIO_PD15 

#define MOTOR_R_ENABLE_PIN GPIO_PD12
#define MOTOR_R_STEP_PIN GPIO_PD13
#define MOTOR_R_DIR_PIN GPIO_PD11

#define MOTOR_B_ENABLE_PIN GPIO_PB0
#define MOTOR_B_STEP_PIN GPIO_PB1
#define MOTOR_B_DIR_PIN GPIO_PD14 


typedef struct {
	gpio_id_t step_pin;
	gpio_id_t dir_pin;
	gpio_id_t enable_pin;
} motor_t;

typedef enum {
    MOTOR_CW = 0,
	MOTOR_CCW = 1
} motor_dir_t;

typedef enum {
	MOTOR_OFF = 0,
	MOTOR_ON = 1
} motor_state_t;


/*
 * 'motor_init' 
 *
 * Initialization for the motor, sets each pin to output
 *
 * @param *motor    specified motor struct with defined pins
 * @param step_pin
 * @param dir_pin
 * @param enable_pin
 *
 */
void motor_init(motor_t *motor, gpio_id_t step_pin, gpio_id_t dir_pin, gpio_id_t enable_pin);


/*
 * 'motor_step'
 *
 * Steps motor specified amount of times in desired direction
 *
 * @param *motor   motor to move
 * @param dir      direction for steps to move in
 * @param steps    amount of steps for motor to take
 *
 */
void motor_steps(motor_t *motor, motor_dir_t dir, int steps);

/*
 * 'motor_enablefn'
 *
 *  Helper for turning all the enables on or off
 *
 *  @param state    on/off for whether to enable all or disable
 *
 */
void motor_enablefn(motor_t *motor, motor_state_t state);

/*
 * 'motor_scramble_steps'
 *
 * Helper for scramble step, enabling and disabling each
 *
 * @param *motor   motor to move
 * @param dir      direction for steps to move in
 * @param steps    amount of steps for motor to take
*/
void motor_scramble_steps(motor_t *motor, motor_dir_t dir, int steps);
 



#endif 
