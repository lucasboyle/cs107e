#include "uart.h"
#include "motor.h"
#include "mouse.h"
#include "interrupts.h"
#include "timer.h"
#include "printf.h"
#include "rubiks_solver.h"
#include "ui.h"
#include <stdint.h>

void main(void) {
    interrupts_init();
    gpio_init();
    timer_init();
    uart_init();
    mouse_init(MOUSE_CLOCK, MOUSE_DATA);
    interrupts_global_enable();

    motor_t motors[6];

    motor_t motorF;
    motor_init(&motorF, MOTOR_F_STEP_PIN, MOTOR_F_DIR_PIN, MOTOR_F_ENABLE_PIN);
    motors[0] = motorF;

    motor_t motorL;
    motor_init(&motorL, MOTOR_L_STEP_PIN, MOTOR_L_DIR_PIN, MOTOR_L_ENABLE_PIN);
    motors[1] = motorL;

    motor_t motorU;
    motor_init(&motorU, MOTOR_U_STEP_PIN, MOTOR_U_DIR_PIN, MOTOR_U_ENABLE_PIN);
    motors[2] = motorU;

    motor_t motorD;
    motor_init(&motorD, MOTOR_D_STEP_PIN, MOTOR_D_DIR_PIN, MOTOR_D_ENABLE_PIN);
    motors[3] = motorD;

    motor_t motorR;
    motor_init(&motorR, MOTOR_R_STEP_PIN, MOTOR_R_DIR_PIN, MOTOR_R_ENABLE_PIN);
    motors[4] = motorR;

    motor_t motorB;
    motor_init(&motorB, MOTOR_B_STEP_PIN, MOTOR_B_DIR_PIN, MOTOR_B_ENABLE_PIN);
    motors[5] = motorB;

    while (1) {
        cube_t *cube = ui_init();
        user_input(cube, &motorU, &motorD, &motorF, &motorB, &motorL, &motorR);
        free_all(cube);
        while (!mouse_read_event().left);
    }

    for (int i = 0; i < 6; i++) {
        motor_enablefn(&motors[i], MOTOR_OFF);
    }

	// while (1);
}
