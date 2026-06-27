/* 
 * rubiks.solver.c
 *
 * Implemenets Kociemba's two-phase algorithm into C coming from online references
 * and adapted from already existing formulas and coding methods.
 *
 * All references are listed in the README.md
 *
 *
 */


#include "rubiks_solver.h"
#include <stdint.h>
#include <stdbool.h>
#include "printf.h"

#define MAX_DEPTH 30
#define N_PHASE1_MOVES 18
#define N_PHASE2_MOVES 10

#define TWIST_COUNT 2187
#define FLIP_COUNT 2048
#define SLICE_COUNT 495
#define CORNER_PERM_COUNT 40320
#define EDGE8_PERM_COUNT 40320
#define SLICE_PERM_COUNT 24

/* ---------------- public-facing stored solution buffer ---------------- */
static int solution[MAX_SOLUTION_MOVES];
static int solution_length = 0;

void solver_reset_solution(void) {
    solution_length = 0;
}

void add_move(int move_num) {
    if (solution_length < MAX_SOLUTION_MOVES) {
        solution[solution_length++] = move_num;
    }
}

static int inverse_move(int move_num) {
    switch (move_num) {
        case U_CW:   return U_CCW;
        case U_CCW:  return U_CW;
        case U_180:  return U_180;
        case D_CW:   return D_CCW;
        case D_CCW:  return D_CW;
        case D_180:  return D_180;
        case F_CW:   return F_CCW;
        case F_CCW:  return F_CW;
        case F_180:  return F_180;
        case B_CW:   return B_CCW;
        case B_CCW:  return B_CW;
        case B_180:  return B_180;
        case L_CW:   return L_CCW;
        case L_CCW:  return L_CW;
        case L_180:  return L_180;
        case R_CW:   return R_CCW;
        case R_CCW:  return R_CW;
        case R_180:  return R_180;
        default:     return move_num;
    }
}

void add_counter_move(int move_num) {
    (void)move_num; /* not used in real solver path */
}

/* ---------------- internal move encoding ----------------
   internal face order for move groups:
   0:U 1:D 2:F 3:B 4:L 5:R
   power 0:CW 1:CCW 2:180
*/

typedef struct {
    uint8_t cp[8];
    uint8_t co[8];
    uint8_t ep[12];
    uint8_t eo[12];
} solver_cube_t;

static void cube_init(solver_cube_t *c) {
    for (int i = 0; i < 8; i++) {
        c->cp[i] = (uint8_t)i;
        c->co[i] = 0;
    }
    for (int i = 0; i < 12; i++) {
        c->ep[i] = (uint8_t)i;
        c->eo[i] = 0;
    }
}

static void cube_copy(const solver_cube_t *src, solver_cube_t *dst) {
    *dst = *src;
}

/* corners: URF,UFL,ULB,UBR,DFR,DLF,DBL,DRB
   edges:   UR,UF,UL,UB,DR,DF,DL,DB,FR,FL,BL,BR */

static void cycle4_u8(uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d) {
    uint8_t t = *a;
    *a = *b;
    *b = *c;
    *c = *d;
    *d = t;
}

static void apply_move_tables(
    solver_cube_t *c,
    const uint8_t cp_src[8], const uint8_t co_add[8],
    const uint8_t ep_src[12], const uint8_t eo_xor[12]
) {
    solver_cube_t old = *c;

    for (int pos = 0; pos < 8; pos++) {
        uint8_t from = cp_src[pos];
        c->cp[pos] = old.cp[from];
        c->co[pos] = (uint8_t)((old.co[from] + co_add[pos]) % 3);
    }

    for (int pos = 0; pos < 12; pos++) {
        uint8_t from = ep_src[pos];
        c->ep[pos] = old.ep[from];
        c->eo[pos] = (uint8_t)((old.eo[from] ^ eo_xor[pos]) & 1);
    }
}

static void move_U_cw(solver_cube_t *c) {
    static const uint8_t CP_SRC[8]  = {3,0,1,2,4,5,6,7};
    static const uint8_t CO_ADD[8]  = {0,0,0,0,0,0,0,0};
    static const uint8_t EP_SRC[12] = {3,0,1,2,4,5,6,7,8,9,10,11};
    static const uint8_t EO_XOR[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    apply_move_tables(c, CP_SRC, CO_ADD, EP_SRC, EO_XOR);
}

static void move_D_cw(solver_cube_t *c) {
    static const uint8_t CP_SRC[8]  = {0,1,2,3,5,6,7,4};
    static const uint8_t CO_ADD[8]  = {0,0,0,0,0,0,0,0};
    static const uint8_t EP_SRC[12] = {0,1,2,3,5,6,7,4,8,9,10,11};
    static const uint8_t EO_XOR[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    apply_move_tables(c, CP_SRC, CO_ADD, EP_SRC, EO_XOR);
}

static void move_F_cw(solver_cube_t *c) {
    static const uint8_t CP_SRC[8]  = {1,5,2,3,0,4,6,7};
    static const uint8_t CO_ADD[8]  = {1,2,0,0,2,1,0,0};
    static const uint8_t EP_SRC[12] = {0,9,2,3,4,8,6,7,1,5,10,11};
    static const uint8_t EO_XOR[12] = {0,1,0,0,0,1,0,0,1,1,0,0};
    apply_move_tables(c, CP_SRC, CO_ADD, EP_SRC, EO_XOR);
}

static void move_B_cw(solver_cube_t *c) {
    static const uint8_t CP_SRC[8]  = {0,1,3,7,4,5,2,6};
    static const uint8_t CO_ADD[8]  = {0,0,1,2,0,0,2,1};
    static const uint8_t EP_SRC[12] = {0,1,2,11,4,5,6,10,8,9,3,7};
    static const uint8_t EO_XOR[12] = {0,0,0,1,0,0,0,1,0,0,1,1};
    apply_move_tables(c, CP_SRC, CO_ADD, EP_SRC, EO_XOR);
}

static void move_L_cw(solver_cube_t *c) {
    static const uint8_t CP_SRC[8]  = {0,2,6,3,4,1,5,7};
    static const uint8_t CO_ADD[8]  = {0,1,2,0,0,2,1,0};
    static const uint8_t EP_SRC[12] = {0,1,10,3,4,5,9,7,8,2,6,11};
    static const uint8_t EO_XOR[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    apply_move_tables(c, CP_SRC, CO_ADD, EP_SRC, EO_XOR);
}

static void move_R_cw(solver_cube_t *c) {
    static const uint8_t CP_SRC[8]  = {4,1,2,0,7,5,6,3};
    static const uint8_t CO_ADD[8]  = {2,0,0,1,1,0,0,2};
    static const uint8_t EP_SRC[12] = {8,1,2,3,11,5,6,7,4,9,10,0};
    static const uint8_t EO_XOR[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    apply_move_tables(c, CP_SRC, CO_ADD, EP_SRC, EO_XOR);
}

static void apply_face_cw(solver_cube_t *c, int face) {
    switch (face) {
        case 0: move_U_cw(c); break;
        case 1: move_D_cw(c); break;
        case 2: move_F_cw(c); break;
        case 3: move_B_cw(c); break;
        case 4: move_L_cw(c); break;
        case 5: move_R_cw(c); break;
    }
}

static void apply_internal_move(solver_cube_t *c, int move) {
    int face = move / 3;
    int type = move % 3; /* 0 cw, 1 ccw, 2 180 */
    if (type == 0) {
        apply_face_cw(c, face);
    } else if (type == 1) {
        apply_face_cw(c, face);
        apply_face_cw(c, face);
        apply_face_cw(c, face);
    } else {
        apply_face_cw(c, face);
        apply_face_cw(c, face);
    }
}

static int map_internal_to_public(int move) {
    switch (move) {
        case 0:  return U_CW;
        case 1:  return U_CCW;
        case 2:  return U_180;
        case 3:  return D_CW;
        case 4:  return D_CCW;
        case 5:  return D_180;
        case 6:  return F_CW;
        case 7:  return F_CCW;
        case 8:  return F_180;
        case 9:  return B_CW;
        case 10: return B_CCW;
        case 11: return B_180;
        case 12: return L_CW;
        case 13: return L_CCW;
        case 14: return L_180;
        case 15: return R_CW;
        case 16: return R_CCW;
        case 17: return R_180;
    }
    return U_CW;
}

static const int phase2_moves[N_PHASE2_MOVES] = {
    0, 1, 2,
    3, 4, 5,
    8, 11, 14, 17
};

/* ---------------- factorial / choose helpers ---------------- */
static const int fact[13] = {
    1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600
};

static int nCk(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    int res = 1;
    for (int i = 1; i <= k; i++) {
        res = res * (n - k + i) / i;
    }
    return res;
}

static int perm8_rank(const uint8_t *p) {
    int rank = 0;
    for (int i = 0; i < 8; i++) {
        int smaller = 0;
        for (int j = i + 1; j < 8; j++) {
            if (p[j] < p[i]) smaller++;
        }
        rank += smaller * fact[7 - i];
    }
    return rank;
}

static void perm8_unrank(int rank, uint8_t *p) {
    uint8_t elems[8];
    for (int i = 0; i < 8; i++) elems[i] = (uint8_t)i;
    int len = 8;
    for (int i = 0; i < 8; i++) {
        int f = fact[7 - i];
        int idx = rank / f;
        rank %= f;
        p[i] = elems[idx];
        for (int j = idx; j < len - 1; j++) elems[j] = elems[j + 1];
        len--;
    }
}

static int perm4_rank(const uint8_t *p) {
    int rank = 0;
    for (int i = 0; i < 4; i++) {
        int smaller = 0;
        for (int j = i + 1; j < 4; j++) {
            if (p[j] < p[i]) smaller++;
        }
        rank += smaller * fact[3 - i];
    }
    return rank;
}

static void perm4_unrank(int rank, uint8_t *p) {
    uint8_t elems[4] = {0,1,2,3};
    int len = 4;
    for (int i = 0; i < 4; i++) {
        int f = fact[3 - i];
        int idx = rank / f;
        rank %= f;
        p[i] = elems[idx];
        for (int j = idx; j < len - 1; j++) elems[j] = elems[j + 1];
        len--;
    }
}

/* ---------------- coordinates ---------------- */
static int get_twist(const solver_cube_t *c) {
    int idx = 0;
    for (int i = 0; i < 7; i++) idx = idx * 3 + c->co[i];
    return idx;
}

static void set_twist(solver_cube_t *c, int idx) {
    cube_init(c);
    int sum = 0;
    for (int i = 6; i >= 0; i--) {
        c->co[i] = (uint8_t)(idx % 3);
        sum += c->co[i];
        idx /= 3;
    }
    c->co[7] = (uint8_t)((3 - (sum % 3)) % 3);
}

static int get_flip(const solver_cube_t *c) {
    int idx = 0;
    for (int i = 0; i < 11; i++) idx = (idx << 1) | c->eo[i];
    return idx;
}

static void set_flip(solver_cube_t *c, int idx) {
    cube_init(c);
    int sum = 0;
    for (int i = 10; i >= 0; i--) {
        c->eo[i] = (uint8_t)(idx & 1);
        sum += c->eo[i];
        idx >>= 1;
    }
    c->eo[11] = (uint8_t)(sum & 1);
}

static bool is_slice_edge(uint8_t e) {
    return (e == 8 || e == 9 || e == 10 || e == 11);
}

static int get_slice(const solver_cube_t *c) {
    int idx = 0;
    int r = 4;
    for (int i = 11; i >= 0; i--) {
        if (is_slice_edge(c->ep[i])) {
            r--;
        } else {
            idx += nCk(i, r - 1);
        }
    }
    return idx;
}

static void set_slice(solver_cube_t *c, int idx) {
    cube_init(c);
    bool chosen[12] = {false};
    int r = 4;
    for (int i = 11; i >= 0; i--) {
        int comb = nCk(i, r - 1);
        if (idx >= comb) {
            idx -= comb;
        } else {
            chosen[i] = true;
            r--;
            if (r == 0) break;
        }
    }
    uint8_t ud[8] = {0,1,2,3,4,5,6,7};
    int udi = 0;
    uint8_t sl[4] = {8,9,10,11};
    int sli = 0;
    for (int i = 0; i < 12; i++) {
        c->ep[i] = chosen[i] ? sl[sli++] : ud[udi++];
    }
}

static int get_corner_perm(const solver_cube_t *c) {
    return perm8_rank(c->cp);
}

static void set_corner_perm(solver_cube_t *c, int idx) {
    cube_init(c);
    perm8_unrank(idx, c->cp);
}

static int get_edge8_perm(const solver_cube_t *c) {
    uint8_t p[8];
    for (int i = 0; i < 8; i++) p[i] = c->ep[i];
    return perm8_rank(p);
}

static void set_edge8_perm(solver_cube_t *c, int idx) {
    cube_init(c);
    perm8_unrank(idx, c->ep);
    c->ep[8] = 8; c->ep[9] = 9; c->ep[10] = 10; c->ep[11] = 11;
}

static int get_slice_perm(const solver_cube_t *c) {
    uint8_t p[4];
    for (int i = 0; i < 4; i++) p[i] = (uint8_t)(c->ep[8 + i] - 8);
    return perm4_rank(p);
}

static void set_slice_perm(solver_cube_t *c, int idx) {
    cube_init(c);
    uint8_t p[4];
    perm4_unrank(idx, p);
    for (int i = 0; i < 4; i++) c->ep[8 + i] = (uint8_t)(8 + p[i]);
}

/* ---------------- facelet parsing (U D F B L R order) ---------------- */
static int face_char_to_color(char ch) {
    switch (ch) {
        case 'U': return 0;
        case 'D': return 1;
        case 'F': return 2;
        case 'B': return 3;
        case 'L': return 4;
        case 'R': return 5;
        default: return -1;
    }
}

/* convert UI order U D F B L R into solver color IDs U,R,F,D,L,B */
static int remap_ui_color(char ch) {
    switch (ch) {
        case 'U': return 0;
        case 'R': return 1;
        case 'F': return 2;
        case 'D': return 3;
        case 'L': return 4;
        case 'B': return 5;
        default: return -1;
    }
}

/* facelet indices in internal URFDLB order */
enum {
    U1=0,U2,U3,U4,U5,U6,U7,U8,U9,
    R1,R2,R3,R4,R5,R6,R7,R8,R9,
    F1,F2,F3,F4,F5,F6,F7,F8,F9,
    D1,D2,D3,D4,D5,D6,D7,D8,D9,
    L1,L2,L3,L4,L5,L6,L7,L8,L9,
    B1,B2,B3,B4,B5,B6,B7,B8,B9
};

static const uint8_t corner_facelet[8][3] = {
    {U9, R1, F3}, {U7, F1, L3}, {U1, L1, B3}, {U3, B1, R3},
    {D3, F9, R7}, {D1, L9, F7}, {D7, B9, L7}, {D9, R9, B7}
};

static const uint8_t corner_color[8][3] = {
    {0,1,2}, {0,2,4}, {0,4,5}, {0,5,1},
    {3,2,1}, {3,4,2}, {3,5,4}, {3,1,5}
};

static const uint8_t edge_facelet[12][2] = {
    {U6,R2},{U8,F2},{U4,L2},{U2,B2},
    {D6,R8},{D2,F8},{D4,L8},{D8,B8},
    {F6,R4},{F4,L6},{B6,L4},{B4,R6}
};

static const uint8_t edge_color[12][2] = {
    {0,1},{0,2},{0,4},{0,5},
    {3,1},{3,2},{3,4},{3,5},
    {2,1},{2,4},{5,4},{5,1}
};

static int parse_input_to_colors(const char *input54, uint8_t f[54]) {
    int count[6] = {0};
    for (int i = 0; i < 54; i++) {
        if (input54[i] == '\0') return 0;
    }
    if (input54[54] != '\0') {
        /* okay if longer, ignore extra */
    }

    /* input order is U D F B L R blocks. Convert to internal URFDLB blocks. */
    int map_src_face[6] = {
        0, /* U */
        5, /* R from src block 5 */
        2, /* F */
        1, /* D */
        4, /* L */
        3  /* B */
    };

    for (int face = 0; face < 6; face++) {
        int src_face = map_src_face[face];
        for (int j = 0; j < 9; j++) {
            char ch = input54[src_face * 9 + j];
            int col = remap_ui_color(ch);
            if (col < 0) return 0;
            f[face * 9 + j] = (uint8_t)col;
            count[col]++;
        }
    }

    for (int i = 0; i < 6; i++) {
        if (count[i] != 9) return 0;
    }

    if (f[U5] != 0 || f[R5] != 1 || f[F5] != 2 || f[D5] != 3 || f[L5] != 4 || f[B5] != 5) {
        return 0;
    }

    return 1;
}

static int parity_perm(const uint8_t *p, int n) {
    int parity = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (p[i] > p[j]) parity ^= 1;
        }
    }
    return parity;
}

static int facelets_to_cube(const uint8_t f[54], solver_cube_t *c) {
    cube_init(c);

    for (int pos = 0; pos < 8; pos++) {
        int ori;
        for (ori = 0; ori < 3; ori++) {
            uint8_t col = f[corner_facelet[pos][ori]];
            if (col == 0 || col == 3) break;
        }
        if (ori == 3) return 0;

        uint8_t c1 = f[corner_facelet[pos][(ori + 1) % 3]];
        uint8_t c2 = f[corner_facelet[pos][(ori + 2) % 3]];

        int cubie = -1;
        for (int i = 0; i < 8; i++) {
            if (corner_color[i][1] == c1 && corner_color[i][2] == c2) {
                cubie = i;
                break;
            }
        }
        if (cubie < 0) return 0;
        c->cp[pos] = (uint8_t)cubie;
        c->co[pos] = (uint8_t)(ori % 3);
    }

    for (int pos = 0; pos < 12; pos++) {
        uint8_t a = f[edge_facelet[pos][0]];
        uint8_t b = f[edge_facelet[pos][1]];
        int cubie = -1;
        for (int i = 0; i < 12; i++) {
            if (edge_color[i][0] == a && edge_color[i][1] == b) {
                cubie = i;
                c->ep[pos] = (uint8_t)i;
                c->eo[pos] = 0;
                break;
            }
            if (edge_color[i][0] == b && edge_color[i][1] == a) {
                cubie = i;
                c->ep[pos] = (uint8_t)i;
                c->eo[pos] = 1;
                break;
            }
        }
        if (cubie < 0) return 0;
    }

    int twist_sum = 0;
    for (int i = 0; i < 8; i++) twist_sum += c->co[i];
    if ((twist_sum % 3) != 0) return 0;

    int flip_sum = 0;
    for (int i = 0; i < 12; i++) flip_sum += c->eo[i];
    if ((flip_sum % 2) != 0) return 0;

    if (parity_perm(c->cp, 8) != parity_perm(c->ep, 12)) return 0;

    return 1;
}

static void dump_cube(const char *tag, const solver_cube_t *c) {
    printf("\n[%s]\n", tag);

    printf("cp: ");
    for (int i = 0; i < 8; i++) printf("%d ", c->cp[i]);
    printf("\nco: ");
    for (int i = 0; i < 8; i++) printf("%d ", c->co[i]);

    printf("\nep: ");
    for (int i = 0; i < 12; i++) printf("%d ", c->ep[i]);
    printf("\neo: ");
    for (int i = 0; i < 12; i++) printf("%d ", c->eo[i]);

    printf("\ncoords: twist=%d flip=%d slice=%d\n",
           get_twist(c), get_flip(c), get_slice(c));
}


/* ---------------- tables ---------------- */
static bool tables_ready = false;
static uint16_t twist_move[TWIST_COUNT][N_PHASE1_MOVES];
static uint16_t flip_move[FLIP_COUNT][N_PHASE1_MOVES];
static uint16_t slice_move[SLICE_COUNT][N_PHASE1_MOVES];

static uint16_t cperm_move[CORNER_PERM_COUNT][N_PHASE2_MOVES];
static uint16_t eperm_move[EDGE8_PERM_COUNT][N_PHASE2_MOVES];
static uint8_t  sperm_move[SLICE_PERM_COUNT][N_PHASE2_MOVES];

static uint8_t prun_twist_slice[TWIST_COUNT * SLICE_COUNT];
static uint8_t prun_flip_slice[FLIP_COUNT * SLICE_COUNT];
static uint8_t prun_cperm_sperm[CORNER_PERM_COUNT * SLICE_PERM_COUNT];
static uint8_t prun_eperm_sperm[EDGE8_PERM_COUNT * SLICE_PERM_COUNT];

static void build_twist_move(void) {
    solver_cube_t c, d;
    for (int i = 0; i < TWIST_COUNT; i++) {
        set_twist(&c, i);
        for (int m = 0; m < N_PHASE1_MOVES; m++) {
            cube_copy(&c, &d);
            apply_internal_move(&d, m);
            twist_move[i][m] = (uint16_t)get_twist(&d);
        }
    }
}

static void build_flip_move(void) {
    solver_cube_t c, d;
    for (int i = 0; i < FLIP_COUNT; i++) {
        set_flip(&c, i);
        for (int m = 0; m < N_PHASE1_MOVES; m++) {
            cube_copy(&c, &d);
            apply_internal_move(&d, m);
            flip_move[i][m] = (uint16_t)get_flip(&d);
        }
    }
}

static void build_slice_move(void) {
    solver_cube_t c, d;
    for (int i = 0; i < SLICE_COUNT; i++) {
        set_slice(&c, i);
        for (int m = 0; m < N_PHASE1_MOVES; m++) {
            cube_copy(&c, &d);
            apply_internal_move(&d, m);
            slice_move[i][m] = (uint16_t)get_slice(&d);
        }
    }
}

static void build_phase2_moves(void) {
    solver_cube_t c, d;
    for (int i = 0; i < CORNER_PERM_COUNT; i++) {
        set_corner_perm(&c, i);
        for (int m = 0; m < N_PHASE2_MOVES; m++) {
            cube_copy(&c, &d);
            apply_internal_move(&d, phase2_moves[m]);
            cperm_move[i][m] = (uint16_t)get_corner_perm(&d);
        }
    }

    for (int i = 0; i < EDGE8_PERM_COUNT; i++) {
        set_edge8_perm(&c, i);
        for (int m = 0; m < N_PHASE2_MOVES; m++) {
            cube_copy(&c, &d);
            apply_internal_move(&d, phase2_moves[m]);
            eperm_move[i][m] = (uint16_t)get_edge8_perm(&d);
        }
    }

    for (int i = 0; i < SLICE_PERM_COUNT; i++) {
        set_slice_perm(&c, i);
        for (int m = 0; m < N_PHASE2_MOVES; m++) {
            cube_copy(&c, &d);
            apply_internal_move(&d, phase2_moves[m]);
            sperm_move[i][m] = (uint8_t)get_slice_perm(&d);
        }
    }
}

static void fill_u8(uint8_t *arr, int n, uint8_t value) {
    for (int i = 0; i < n; i++) arr[i] = value;
}

static void build_prun_twist_slice(void) {
    fill_u8(prun_twist_slice, TWIST_COUNT * SLICE_COUNT, 0xFF);
    static uint32_t queue[TWIST_COUNT * SLICE_COUNT];
    uint32_t head = 0, tail = 0;
    prun_twist_slice[0] = 0;
    queue[tail++] = 0;
    while (head < tail) {
        uint32_t idx = queue[head++];
        uint16_t t = (uint16_t)(idx % TWIST_COUNT);
        uint16_t s = (uint16_t)(idx / TWIST_COUNT);
        uint8_t d = prun_twist_slice[idx];
        for (int m = 0; m < N_PHASE1_MOVES; m++) {
            uint16_t nt = twist_move[t][m];
            uint16_t ns = slice_move[s][m];
            uint32_t nidx = (uint32_t)ns * TWIST_COUNT + nt;
            if (prun_twist_slice[nidx] == 0xFF) {
                prun_twist_slice[nidx] = (uint8_t)(d + 1);
                queue[tail++] = nidx;
            }
        }
    }
}

static void build_prun_flip_slice(void) {
    fill_u8(prun_flip_slice, FLIP_COUNT * SLICE_COUNT, 0xFF);
    static uint32_t queue[FLIP_COUNT * SLICE_COUNT];
    uint32_t head = 0, tail = 0;
    prun_flip_slice[0] = 0;
    queue[tail++] = 0;
    while (head < tail) {
        uint32_t idx = queue[head++];
        uint16_t f = (uint16_t)(idx % FLIP_COUNT);
        uint16_t s = (uint16_t)(idx / FLIP_COUNT);
        uint8_t d = prun_flip_slice[idx];
        for (int m = 0; m < N_PHASE1_MOVES; m++) {
            uint16_t nf = flip_move[f][m];
            uint16_t ns = slice_move[s][m];
            uint32_t nidx = (uint32_t)ns * FLIP_COUNT + nf;
            if (prun_flip_slice[nidx] == 0xFF) {
                prun_flip_slice[nidx] = (uint8_t)(d + 1);
                queue[tail++] = nidx;
            }
        }
    }
}

static void build_prun_cperm_sperm(void) {
    fill_u8(prun_cperm_sperm, CORNER_PERM_COUNT * SLICE_PERM_COUNT, 0xFF);
    static uint32_t queue[CORNER_PERM_COUNT * SLICE_PERM_COUNT];
    uint32_t head = 0, tail = 0;
    prun_cperm_sperm[0] = 0;
    queue[tail++] = 0;
    while (head < tail) {
        uint32_t idx = queue[head++];
        uint16_t cp = (uint16_t)(idx % CORNER_PERM_COUNT);
        uint8_t sp = (uint8_t)(idx / CORNER_PERM_COUNT);
        uint8_t d = prun_cperm_sperm[idx];
        for (int m = 0; m < N_PHASE2_MOVES; m++) {
            uint16_t ncp = cperm_move[cp][m];
            uint8_t nsp = sperm_move[sp][m];
            uint32_t nidx = (uint32_t)nsp * CORNER_PERM_COUNT + ncp;
            if (prun_cperm_sperm[nidx] == 0xFF) {
                prun_cperm_sperm[nidx] = (uint8_t)(d + 1);
                queue[tail++] = nidx;
            }
        }
    }
}

static void build_prun_eperm_sperm(void) {
    fill_u8(prun_eperm_sperm, EDGE8_PERM_COUNT * SLICE_PERM_COUNT, 0xFF);
    static uint32_t queue[EDGE8_PERM_COUNT * SLICE_PERM_COUNT];
    uint32_t head = 0, tail = 0;
    prun_eperm_sperm[0] = 0;
    queue[tail++] = 0;
    while (head < tail) {
        uint32_t idx = queue[head++];
        uint16_t ep = (uint16_t)(idx % EDGE8_PERM_COUNT);
        uint8_t sp = (uint8_t)(idx / EDGE8_PERM_COUNT);
        uint8_t d = prun_eperm_sperm[idx];
        for (int m = 0; m < N_PHASE2_MOVES; m++) {
            uint16_t nep = eperm_move[ep][m];
            uint8_t nsp = sperm_move[sp][m];
            uint32_t nidx = (uint32_t)nsp * EDGE8_PERM_COUNT + nep;
            if (prun_eperm_sperm[nidx] == 0xFF) {
                prun_eperm_sperm[nidx] = (uint8_t)(d + 1);
                queue[tail++] = nidx;
            }
        }
    }
}

static void ensure_tables(void) {
    if (tables_ready) return;
    build_twist_move();
    build_flip_move();
    build_slice_move();
    build_phase2_moves();
    build_prun_twist_slice();
    build_prun_flip_slice();
    build_prun_cperm_sperm();
    build_prun_eperm_sperm();
    tables_ready = true;
}

/* ---------------- search ---------------- */
static uint8_t phase1_path[32];
static uint8_t phase2_path[32];
static int phase1_len = 0;
static int phase2_len = 0;

static int opp_face(int f) {
    switch (f) {
        case 0: return 1;
        case 1: return 0;
        case 2: return 3;
        case 3: return 2;
        case 4: return 5;
        case 5: return 4;
    }
    return -1;
}

static int h1(int twist, int flip, int slice) {
    int a = prun_twist_slice[slice * TWIST_COUNT + twist];
    int b = prun_flip_slice[slice * FLIP_COUNT + flip];
    return (a > b) ? a : b;
}

static int h2(int cperm, int eperm, int sperm) {
    int a = prun_cperm_sperm[sperm * CORNER_PERM_COUNT + cperm];
    int b = prun_eperm_sperm[sperm * EDGE8_PERM_COUNT + eperm];
    return (a > b) ? a : b;
}

static bool phase2_search(int depth, int maxd, int last_face,
                          int cperm, int eperm, int sperm) {
    int heur = h2(cperm, eperm, sperm);
    if (depth + heur > maxd) return false;
    if (cperm == 0 && eperm == 0 && sperm == 0) {
        phase2_len = depth;
        return true;
    }

    for (int m = 0; m < N_PHASE2_MOVES; m++) {
        int face = phase2_moves[m] / 3;
        if (last_face >= 0) {
            if (face == last_face) continue;
            if (face == opp_face(last_face) && face < last_face) continue;
        }

        phase2_path[depth] = (uint8_t)phase2_moves[m];
        if (phase2_search(depth + 1, maxd, face,
                          cperm_move[cperm][m],
                          eperm_move[eperm][m],
                          sperm_move[sperm][m])) {
            return true;
        }
    }
    return false;
}

static bool phase1_search(const solver_cube_t *start_cube,
                          int twist, int flip, int slice,
                          int depth, int maxd, int last_face) {
    int heur = h1(twist, flip, slice);
    if (depth + heur > maxd) return false;

    if (twist == 0 && flip == 0 && slice == 0) {
        solver_cube_t c;
        cube_copy(start_cube, &c);
        for (int i = 0; i < depth; i++) {
            apply_internal_move(&c, phase1_path[i]);
        }

        int cperm = get_corner_perm(&c);
        int eperm = get_edge8_perm(&c);
        int sperm = get_slice_perm(&c);

        for (int d2 = 0; d2 <= 18; d2++) {
            if (phase2_search(0, d2, -1, cperm, eperm, sperm)) {
                phase1_len = depth;
                return true;
            }
        }
    }

    for (int m = 0; m < N_PHASE1_MOVES; m++) {
        int face = m / 3;
        if (last_face >= 0) {
            if (face == last_face) continue;
            if (face == opp_face(last_face) && face < last_face) continue;
        }
        phase1_path[depth] = (uint8_t)m;
        if (phase1_search(start_cube,
                          twist_move[twist][m],
                          flip_move[flip][m],
                          slice_move[slice][m],
                          depth + 1, maxd, face)) {
            return true;
        }
    }
    return false;
}

/* ---------------- solver entry ---------------- */
int solver_get_solution(const char *input54, int *out_moves, int *out_len) {
    uint8_t facelets[54];
    solver_cube_t cube;

    ensure_tables();

    if (!parse_input_to_colors(input54, facelets)) {
        *out_len = 0;
        return 0;
    }

    if (!facelets_to_cube(facelets, &cube)) {
        *out_len = 0;
        return 0;
    }
dump_cube("from facelets", &cube);

    phase1_len = 0;
    phase2_len = 0;

    int twist = get_twist(&cube);
    int flip = get_flip(&cube);
    int slice = get_slice(&cube);

    bool found = false;
    for (int d1 = 0; d1 <= 12 && !found; d1++) {
        found = phase1_search(&cube, twist, flip, slice, 0, d1, -1);
    }

    if (!found) {
        *out_len = 0;
        return 0;
    }

    int k = 0;
    for (int i = 0; i < phase1_len; i++) {
        out_moves[k++] = map_internal_to_public(phase1_path[i]);
    }
    for (int i = 0; i < phase2_len; i++) {
        out_moves[k++] = map_internal_to_public(phase2_path[i]);
    }

    *out_len = k;
    return 1;
}
