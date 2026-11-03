#include "sandlab.h"

#include <string.h>

static uint32_t xorshift32(uint32_t *s) {
    uint32_t x = *s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *s = x;
    return x;
}

const char *sand_elem_name(sand_elem_t el) {
    switch (el) {
    case SAND_EL_EMPTY: return "ERASER";
    case SAND_EL_WALL:  return "STONE WALL";
    case SAND_EL_SAND:  return "SAND";
    case SAND_EL_WATER: return "WATER";
    case SAND_EL_FIRE:  return "FIRE";
    case SAND_EL_PLANT: return "PLANT";
    case SAND_EL_OIL:   return "OIL";
    case SAND_EL_ACID:  return "ACID";
    default:            return "SAND";
    }
}

void sand_clear(sand_world_t *w) {
    if (w == NULL) return;
    memset(w->cells, 0, sizeof(w->cells));
    memset(w->life, 0, sizeof(w->life));
    w->particle_count = 0;
}

void sand_init(sand_world_t *w) {
    if (w == NULL) return;
    memset(w, 0, sizeof(*w));
    w->active_brush = SAND_EL_SAND;
    w->brush_radius = 2;
    w->rng_state = 0xCAFEBABEu;

    /* Seed a stone basin with water, plants, and falling sand */
    for (int x = 18; x <= 62; x++) {
        w->cells[38][x] = SAND_EL_WALL;
    }
    for (int y = 26; y <= 38; y++) {
        w->cells[y][18] = SAND_EL_WALL;
        w->cells[y][62] = SAND_EL_WALL;
    }
    sand_paint(w, 30, 32, 4, SAND_EL_WATER);
    sand_paint(w, 50, 32, 3, SAND_EL_PLANT);
    sand_paint(w, 40, 14, 5, SAND_EL_SAND);
}

bool sand_paint(sand_world_t *w, int cx, int cy, int radius, sand_elem_t el) {
    if (w == NULL || cx < 0 || cx >= SAND_W || cy < 0 || cy >= SAND_H) return false;
    if ((unsigned)el >= (unsigned)SAND_EL_COUNT || radius < 0 || radius > 12) return false;

    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy <= radius * radius) {
                int x = cx + dx;
                int y = cy + dy;
                if (x >= 0 && x < SAND_W && y >= 0 && y < SAND_H) {
                    w->cells[y][x] = (uint8_t)el;
                    w->life[y][x] = (el == SAND_EL_FIRE) ? 18u : 0u;
                }
            }
        }
    }
    return true;
}

void sand_step(sand_world_t *w) {
    if (w == NULL) return;
    w->step_count++;
    uint32_t count = 0;

    for (int y = SAND_H - 1; y >= 0; y--) {
        for (int x = 0; x < SAND_W; x++) {
            uint8_t el = w->cells[y][x];
            if (el == SAND_EL_EMPTY) continue;
            count++;

            if (el == SAND_EL_SAND) {
                if (y + 1 < SAND_H) {
                    uint8_t below = w->cells[y + 1][x];
                    if (below == SAND_EL_EMPTY || below == SAND_EL_WATER || below == SAND_EL_OIL) {
                        w->cells[y + 1][x] = SAND_EL_SAND;
                        w->cells[y][x] = below;
                        continue;
                    }
                    int dir = (xorshift32(&w->rng_state) & 1u) ? 1 : -1;
                    int nx = x + dir;
                    if (nx >= 0 && nx < SAND_W &&
                        (w->cells[y + 1][nx] == SAND_EL_EMPTY || w->cells[y + 1][nx] == SAND_EL_WATER)) {
                        uint8_t tmp = w->cells[y + 1][nx];
                        w->cells[y + 1][nx] = SAND_EL_SAND;
                        w->cells[y][x] = tmp;
                    }
                }
            } else if (el == SAND_EL_WATER || el == SAND_EL_OIL) {
                if (y + 1 < SAND_H && w->cells[y + 1][x] == SAND_EL_EMPTY) {
                    w->cells[y + 1][x] = el;
                    w->cells[y][x] = SAND_EL_EMPTY;
                } else {
                    int dir = (xorshift32(&w->rng_state) & 1u) ? 1 : -1;
                    int nx = x + dir;
                    if (nx >= 0 && nx < SAND_W && w->cells[y][nx] == SAND_EL_EMPTY) {
                        w->cells[y][nx] = el;
                        w->cells[y][x] = SAND_EL_EMPTY;
                    }
                }
                /* Water waters adjacent Plant -> grows Plant */
                if (el == SAND_EL_WATER && y + 1 < SAND_H && w->cells[y + 1][x] == SAND_EL_PLANT) {
                    w->cells[y][x] = SAND_EL_PLANT;
                }
            } else if (el == SAND_EL_FIRE) {
                /* Ignite adjacent Plant or Oil */
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx >= 0 && nx < SAND_W && ny >= 0 && ny < SAND_H) {
                            uint8_t nb = w->cells[ny][nx];
                            if (nb == SAND_EL_PLANT || nb == SAND_EL_OIL) {
                                w->cells[ny][nx] = SAND_EL_FIRE;
                                w->life[ny][nx] = 20u;
                            } else if (nb == SAND_EL_WATER) {
                                w->cells[y][x] = SAND_EL_EMPTY;
                            }
                        }
                    }
                }
                if (w->life[y][x] <= 1u) {
                    w->cells[y][x] = SAND_EL_EMPTY;
                } else {
                    w->life[y][x]--;
                    if (y > 0 && w->cells[y - 1][x] == SAND_EL_EMPTY && (xorshift32(&w->rng_state) & 1u)) {
                        w->cells[y - 1][x] = SAND_EL_FIRE;
                        w->life[y - 1][x] = w->life[y][x];
                        w->cells[y][x] = SAND_EL_EMPTY;
                    }
                }
            } else if (el == SAND_EL_ACID) {
                if (y + 1 < SAND_H) {
                    uint8_t below = w->cells[y + 1][x];
                    if (below == SAND_EL_EMPTY) {
                        w->cells[y + 1][x] = SAND_EL_ACID;
                        w->cells[y][x] = SAND_EL_EMPTY;
                    } else if (below != SAND_EL_ACID) {
                        /* Acid dissolves both target and itself */
                        w->cells[y + 1][x] = SAND_EL_EMPTY;
                        w->cells[y][x] = SAND_EL_EMPTY;
                    }
                }
            }
        }
    }
    w->particle_count = count;
}
