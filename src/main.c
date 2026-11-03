#include "pk_platform.h"
#include "sandlab.h"

#include <stdio.h>
#include <string.h>

static uint32_t elem_color(sand_elem_t el) {
    switch (el) {
    case SAND_EL_WALL:  return PK_RGBA8(140, 148, 165, 255);
    case SAND_EL_SAND:  return PK_RGBA8(245, 210, 85, 255);
    case SAND_EL_WATER: return PK_RGBA8(65, 165, 255, 255);
    case SAND_EL_FIRE:  return PK_RGBA8(255, 95, 45, 255);
    case SAND_EL_PLANT: return PK_RGBA8(75, 225, 110, 255);
    case SAND_EL_OIL:   return PK_RGBA8(165, 110, 65, 255);
    case SAND_EL_ACID:  return PK_RGBA8(180, 255, 60, 255);
    default:            return PK_RGBA8(14, 18, 28, 255);
    }
}

typedef struct {
    char sd_root[512];
    sand_world_t world;
    bool paused;
    char status_msg[128];
} sand_app_t;

static void app_init(sand_app_t *app, const char *sd_root) {
    memset(app, 0, sizeof(*app));
    snprintf(app->sd_root, sizeof(app->sd_root), "%s",
             sd_root != NULL ? sd_root : pk_platform_default_sd_root());
    sand_init(&app->world);
    for (int i = 0; i < 6; i++) sand_step(&app->world);
    snprintf(app->status_msg, sizeof(app->status_msg),
             "BRUSH: %s | STYLUS: DRAW | L/R: ELEMENT | Y: CLEAR",
             sand_elem_name(app->world.active_brush));
    pk_log_event(app->sd_root, "sandlab", "INFO", app->status_msg);
}

static void app_step(sand_app_t *app, const pk_input_t *in) {
    if (in->down & PK_BTN_L) {
        app->world.active_brush = (sand_elem_t)((app->world.active_brush + SAND_EL_COUNT - 1) % SAND_EL_COUNT);
    }
    if (in->down & PK_BTN_R) {
        app->world.active_brush = (sand_elem_t)((app->world.active_brush + 1) % SAND_EL_COUNT);
    }
    if (in->down & PK_BTN_A) {
        sand_paint(&app->world, SAND_W / 2, 10, app->world.brush_radius + 1, app->world.active_brush);
    }
    if (in->down & PK_BTN_X) {
        app->paused = !app->paused;
    }
    if (in->down & PK_BTN_Y) {
        sand_clear(&app->world);
        pk_log_event(app->sd_root, "sandlab", "INFO", "CLEARED SANDBOX");
    }
    if (in->touching) {
        if (in->touch_y < 32) {
            int idx = in->touch_x / 40;
            if (idx >= 0 && idx < SAND_EL_COUNT) {
                app->world.active_brush = (sand_elem_t)idx;
            }
        } else {
            int gx = in->touch_x / 4;
            int gy = (in->touch_y - 32) / 4;
            sand_paint(&app->world, gx, gy, app->world.brush_radius, app->world.active_brush);
        }
    }
    if (!app->paused) {
        sand_step(&app->world);
    }
    snprintf(app->status_msg, sizeof(app->status_msg),
             "BRUSH:%s | PARTICLES:%u | TICKS:%u | %s",
             sand_elem_name(app->world.active_brush),
             (unsigned)app->world.particle_count,
             (unsigned)app->world.step_count,
             app->paused ? "PAUSED" : "RUNNING");
}

static void app_render(const sand_app_t *app) {
    pk_render_begin_frame();
    pk_render_begin_target(PK_TARGET_TOP_LEFT, PK_RGBA8(12, 16, 26, 255));
    pk_draw_rect(0, 0, 400, 26, PK_RGBA8(28, 44, 74, 255));
    pk_draw_text(8, 7, 0.48f, PK_RGBA8(255, 215, 85, 255),
                 "SANDLAB 3DS - CELLULAR AUTOMATA PHYSICS SANDBOX");

    /* Render 80x48 world scaled 4x (320x192) centered on top screen (x=40, y=26) */
    for (int y = 0; y < SAND_H; y++) {
        for (int x = 0; x < SAND_W; x++) {
            sand_elem_t el = (sand_elem_t)app->world.cells[y][x];
            if (el != SAND_EL_EMPTY) {
                pk_draw_rect((float)(40 + x * 4), (float)(26 + y * 4), 4.0f, 4.0f, elem_color(el));
            }
        }
    }
    pk_draw_rect_outline(40, 26, 320, 192, 1.0f, PK_RGBA8(80, 120, 185, 255));

    pk_draw_rect(0, 218, 400, 22, PK_RGBA8(20, 30, 50, 255));
    pk_draw_text(8, 223, 0.44f, PK_RGBA8(120, 250, 175, 255), app->status_msg);

    /* Bottom Touch Screen (320x240): Element Palette Bar + Direct Stylus Canvas */
    pk_render_begin_target(PK_TARGET_BOTTOM, PK_RGBA8(14, 18, 28, 255));
    for (int i = 0; i < SAND_EL_COUNT; i++) {
        float bx = (float)(i * 40 + 2);
        pk_draw_rect(bx, 2.0f, 36.0f, 26.0f, elem_color((sand_elem_t)i));
        if ((int)app->world.active_brush == i) {
            pk_draw_rect_outline(bx, 2.0f, 36.0f, 26.0f, 2.0f, PK_RGBA8(255, 255, 255, 255));
        }
    }
    for (int y = 0; y < SAND_H; y++) {
        for (int x = 0; x < SAND_W; x++) {
            sand_elem_t el = (sand_elem_t)app->world.cells[y][x];
            if (el != SAND_EL_EMPTY) {
                pk_draw_rect((float)(x * 4), (float)(32 + y * 4), 4.0f, 4.0f, elem_color(el));
            }
        }
    }
    pk_draw_text(8, 226, 0.42f, PK_RGBA8(180, 210, 245, 255),
                 "DRAW WITH STYLUS | L/R:ELEMENT | A:DROP | Y:CLEAR");
    pk_render_end_frame();
}

int main(int argc, char **argv) {
    const char *sd_root = NULL;
    bool one_shot = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--sd-root") == 0 && i + 1 < argc) sd_root = argv[++i];
        else if (strcmp(argv[i], "--once") == 0) one_shot = true;
    }
    if (!pk_platform_init()) return 1;
    sand_app_t app;
    app_init(&app, sd_root);
    while (pk_platform_main_loop()) {
        pk_input_t in;
        pk_platform_poll_input(&in);
        if (in.down & PK_BTN_START) break;
        app_step(&app, &in);
        app_render(&app);
        if (one_shot) break;
    }
    pk_platform_shutdown();
    return 0;
}
