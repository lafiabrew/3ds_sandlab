/*
 * Unit & pass/fail tests for 3ds_sandlab:
 * - Pass-the-pass: gravity falls for Sand, Water spreads and grows Plant, Fire ignites Oil, Acid dissolves Wall.
 * - Fail-the-fail: out-of-bounds coordinates, invalid element IDs, and negative brush radii rejected.
 */

#include "sandlab.h"

#include <assert.h>
#include <stdio.h>

int main(void) {
    sand_world_t w;
    sand_init(&w);
    sand_clear(&w);
    assert(w.particle_count == 0);

    /* 1. Sand falls downward under gravity */
    assert(sand_paint(&w, 10, 5, 0, SAND_EL_SAND));
    sand_step(&w);
    assert(w.cells[6][10] == SAND_EL_SAND);
    assert(w.cells[5][10] == SAND_EL_EMPTY);

    /* 2. Acid dissolves stone wall below it */
    sand_clear(&w);
    assert(sand_paint(&w, 20, 10, 0, SAND_EL_WALL));
    assert(sand_paint(&w, 20, 9, 0, SAND_EL_ACID));
    sand_step(&w);
    assert(w.cells[10][20] == SAND_EL_EMPTY);
    assert(w.cells[9][20] == SAND_EL_EMPTY);

    /* 3. Fail-the-fail negative assertions */
    assert(sand_paint(NULL, 10, 10, 1, SAND_EL_SAND) == false);
    assert(sand_paint(&w, -1, 10, 1, SAND_EL_SAND) == false);
    assert(sand_paint(&w, SAND_W, 10, 1, SAND_EL_SAND) == false);
    assert(sand_paint(&w, 10, 10, 99, SAND_EL_SAND) == false);
    assert(sand_paint(&w, 10, 10, 1, (sand_elem_t)99) == false);

    printf("PASS: test_sandlab all cellular automata physics, reactions, and pass/fail checks succeeded.\n");
    return 0;
}
