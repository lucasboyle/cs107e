#include "motor.h"
#include "timer.h"
#include "gpio.h"
#include <stdint.h>


/* motor.c
 *
 * Implementation of motor.h for basic motor commands
 *
 */


void motor_init(motor_t *motor, gpio_id_t step_pin, gpio_id_t dir_pin, gpio_id_t enable_pin){
	motor->step_pin = step_pin;
	motor->dir_pin = dir_pin;
	motor->enable_pin = enable_pin;

	gpio_set_output(motor->step_pin);
	gpio_set_output(motor->dir_pin);
	gpio_set_output(motor->enable_pin);

	gpio_write(motor->step_pin,0);
	gpio_write(motor->dir_pin,0);
	gpio_write(motor->enable_pin,1);

}

//10us pulse always works
//500us delay is too fast
//750us and 10us fast:
//single motor 1250 sweet spot
static void step(motor_t *motor, int speed){
	gpio_write(motor->step_pin,1);
	timer_delay_us(10); //send pulse
	gpio_write(motor->step_pin,0);
	timer_delay_us(speed); //delay
}

void motor_steps(motor_t *motor, motor_dir_t dir, int steps){
	
	gpio_write(motor->dir_pin,dir);

	for(int i = 0; i < steps; i++){        
		int speed = 1800; 

        	if (steps == 50) {
            		if (i < 15) {
                		speed = 1800 - i * 50;  
            		} else if (i >= 35) {
                		speed = 1050 + (i - 32) * 50; 
            		} else {
                		speed = 1800;
            		}
        	}

		step(motor, speed);
	}
	
	timer_delay_us(1000);

}

void motor_enablefn(motor_t *motor, motor_state_t state){

	if(state == MOTOR_ON){
		gpio_write(motor->enable_pin,0);
	} if(state == MOTOR_OFF){
		gpio_write(motor->enable_pin,1);
	}
	timer_delay_us(1000);

}
void motor_scramble_steps(motor_t *motor, motor_dir_t dir, int steps){

	motor_enablefn(motor, MOTOR_ON);
	timer_delay_us(1000);
	motor_steps(motor,dir,steps);
	timer_delay_us(1000);
	motor_enablefn(motor, MOTOR_OFF);

}
