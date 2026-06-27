/*
 * rubiks.solver.h
 *
 * Defines the move types as used by the rubik's cube solver alogrithm
 * Defines the function as used by the rubik's cube solver
 *
 */

#ifndef RUBIKS_SOLVER_H
#define RUBIKS_SOLVER_H

#include <stdint.h>

#define MAX_SOLUTION_MOVES 128

typedef enum {
    U_CW = 0,
    U_CCW,
    U_180,
    D_CW,
    D_CCW,
    D_180,
    F_CW,
    F_CCW,
    F_180,
    B_CW,
    B_CCW,
    B_180,
    L_CW,
    L_CCW,
    L_180,
    R_CW,
    R_CCW,
    R_180
} rubiks_move_t;

void solver_reset_solution(void);
void add_move(int move_num);
void add_counter_move(int move_num);
int solver_get_solution(const char *input54, int *out_moves, int *out_len);

#endif

