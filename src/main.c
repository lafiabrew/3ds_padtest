#include "pk_platform.h"
#include "pt_padtest.h"

#include <stdio.h>
#include <string.h>
#ifdef __3DS__
#include <3ds.h>
#endif

typedef struct {
    char sd_root[PT_PATH_MAX];
    pt_state_t st;
    char status_msg[128];
} pt_app_t;

static void pt_app_init(pt_app_t *app, const char *sd_root) {
    memset(app, 0, sizeof(*app));
    snprintf(app->sd_root, sizeof(app->sd_root), "%s",
             sd_root != NULL ? sd_root : pk_platform_default_sd_root());
    pt_init(&app->st);
    snprintf(app->status_msg, sizeof(app->status_msg),
             "PADTEST READY: MOVE CIRCLE PAD / TOUCH / PRESS Y FOR PIXEL TEST");
}

static void pt_app_step(pt_app_t *app, const pk_input_t *in) {
    int cx = 0;
    int cy = 0;
#ifdef __3DS__
    circlePosition pos;
    hidCircleRead(&pos);
    cx = pos.dx;
    cy = pos.dy;
#else
    if (in->held & PK_BTN_DLEFT) cx = -120;
    if (in->held & PK_BTN_DRIGHT) cx = 120;
    if (in->held & PK_BTN_DUP) cy = 120;
    if (in->held & PK_BTN_DDOWN) cy = -120;
#endif
    pt_record_sample(&app->st, cx, cy, in->held, (in->held == 0));

    if (in->down & PK_BTN_Y) {
        app->st.pixel_test_mode = (app->st.pixel_test_mode + 1) % 6;
    }
    if (in->down & PK_BTN_X) {
        char out[PT_PATH_MAX + 64];
        if (pt_export_report(app->sd_root, &app->st, out, sizeof(out))) {
            snprintf(app->status_msg, sizeof(app->status_msg),
                     "SAVED REPORT: 3DS/PADTEST/HARDWARE_REPORT.JSON");
        }
    }
}

static void pt_app_render(const pt_app_t *app) {
    pk_render_begin_frame();

    if (app->st.pixel_test_mode > 0) {
        static const uint32_t k_colors[6] = {
            0,
            PK_RGBA8(255, 0, 0, 255),
            PK_RGBA8(0, 255, 0, 255),
            PK_RGBA8(0, 0, 255, 255),
            PK_RGBA8(255, 255, 255, 255),
            PK_RGBA8(0, 0, 0, 255)
        };
        uint32_t c = k_colors[app->st.pixel_test_mode % 6];
        pk_render_begin_target(PK_TARGET_TOP_LEFT, c);
        pk_render_begin_target(PK_TARGET_BOTTOM, c);
        pk_render_end_frame();
        return;
    }

    pk_render_begin_target(PK_TARGET_TOP_LEFT, PK_RGBA8(16, 18, 28, 255));
    pk_draw_rect(0, 0, 400, 26, PK_RGBA8(42, 48, 72, 255));
    pk_draw_text(8, 7, 0.55f, PK_RGBA8(255, 175, 85, 255),
                 "3DS PADTEST - DRIFT, SENSOR & PIXEL LAB");

    char line[160];
    snprintf(line, sizeof(line), "CIRCLE PAD: X=%4d Y=%4d  |  MIN/MAX X:[%d..%d] Y:[%d..%d]",
             app->st.cpad_x, app->st.cpad_y,
             app->st.cpad_min_x, app->st.cpad_max_x,
             app->st.cpad_min_y, app->st.cpad_max_y);
    pk_draw_text(8, 36, 0.48f, PK_RGBA8(215, 230, 250, 255), line);

    snprintf(line, sizeof(line), "DRIFT SCORE: %.1f%%   |   RECOMMENDED DEADZONE: %d",
             app->st.drift_pct, app->st.recommended_deadzone);
    pk_draw_text(8, 54, 0.5f,
                 app->st.drift_pct < 8.0f ? PK_RGBA8(110, 250, 165, 255) : PK_RGBA8(255, 130, 120, 255),
                 line);

    /* Crosshair visualizer box */
    pk_draw_rect(140, 76, 120, 120, PK_RGBA8(24, 30, 46, 255));
    pk_draw_rect_outline(140, 76, 120, 120, 1.0f, PK_RGBA8(95, 135, 195, 255));
    float dot_x = 200.0f + ((float)app->st.cpad_x / 156.0f) * 50.0f;
    float dot_y = 136.0f - ((float)app->st.cpad_y / 156.0f) * 50.0f;
    pk_draw_rect(dot_x - 4.0f, dot_y - 4.0f, 8.0f, 8.0f, PK_RGBA8(255, 190, 75, 255));

    pk_draw_rect(0, 216, 400, 24, PK_RGBA8(26, 32, 48, 255));
    pk_draw_text(8, 222, 0.48f, PK_RGBA8(125, 245, 175, 255), app->status_msg);

    pk_render_begin_target(PK_TARGET_BOTTOM, PK_RGBA8(14, 16, 24, 255));
    pk_draw_rect(12, 18, 296, 204, PK_RGBA8(24, 28, 42, 255));
    pk_draw_text(22, 36, 0.52f, PK_RGBA8(255, 215, 95, 255),
                 "Y: CYCLE DEAD-PIXEL SCREEN TEST");
    pk_draw_text(22, 66, 0.52f, PK_RGBA8(130, 235, 255, 255),
                 "X: EXPORT HARDWARE_REPORT.JSON");
    snprintf(line, sizeof(line), "3D SLIDER: %.0f%%  |  SAMPLES: %u",
             0.0f, (unsigned)app->st.sample_count);
    pk_draw_text(22, 106, 0.48f, PK_RGBA8(200, 215, 235, 255), line);
    pk_draw_text(22, 178, 0.48f, PK_RGBA8(165, 185, 210, 255),
                 "PRESS START TO EXIT");
    pk_render_end_frame();
}

int main(int argc, char **argv) {
    const char *sd_root = NULL;
    bool one_shot = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--sd-root") == 0 && i + 1 < argc) {
            sd_root = argv[++i];
        } else if (strcmp(argv[i], "--once") == 0) {
            one_shot = true;
        }
    }

    if (!pk_platform_init()) return 1;
    pt_app_t app;
    pt_app_init(&app, sd_root);

    while (pk_platform_main_loop()) {
        pk_input_t in;
        pk_platform_poll_input(&in);
        if (in.down & PK_BTN_START) break;
        pt_app_step(&app, &in);
        pt_app_render(&app);
        if (one_shot) break;
    }

    pk_platform_shutdown();
    return 0;
}
