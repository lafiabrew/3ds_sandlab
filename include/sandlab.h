#ifndef SANDLAB_H
#define SANDLAB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SAND_W 80
#define SAND_H 48

typedef enum {
    SAND_EL_EMPTY = 0,
    SAND_EL_WALL  = 1,
    SAND_EL_SAND  = 2,
    SAND_EL_WATER = 3,
    SAND_EL_FIRE  = 4,
    SAND_EL_PLANT = 5,
    SAND_EL_OIL   = 6,
    SAND_EL_ACID  = 7,
    SAND_EL_COUNT = 8
} sand_elem_t;

typedef struct {
    uint8_t cells[SAND_H][SAND_W];
    uint8_t life[SAND_H][SAND_W];
    sand_elem_t active_brush;
    int brush_radius;
    uint32_t step_count;
    uint32_t particle_count;
    uint32_t rng_state;
} sand_world_t;

void sand_init(sand_world_t *w);
void sand_clear(sand_world_t *w);
bool sand_paint(sand_world_t *w, int cx, int cy, int radius, sand_elem_t el);
void sand_step(sand_world_t *w);
const char *sand_elem_name(sand_elem_t el);

#endif /* SANDLAB_H */
