#include "pt_padtest.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void ensure_dir_p(const char *dir_path) {
    char tmp[PT_PATH_MAX + 64];
    snprintf(tmp, sizeof(tmp), "%s", dir_path);
    for (char *p = tmp + 1; *p != '\0'; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0777);
            *p = '/';
        }
    }
    mkdir(tmp, 0777);
}

void pt_init(pt_state_t *st) {
    if (st == NULL) return;
    memset(st, 0, sizeof(*st));
    st->recommended_deadzone = 12;
}

void pt_record_sample(pt_state_t *st, int cpad_x, int cpad_y, uint32_t held_buttons, bool at_rest) {
    if (st == NULL) return;
    st->cpad_x = cpad_x;
    st->cpad_y = cpad_y;
    if (cpad_x < st->cpad_min_x) st->cpad_min_x = cpad_x;
    if (cpad_x > st->cpad_max_x) st->cpad_max_x = cpad_x;
    if (cpad_y < st->cpad_min_y) st->cpad_min_y = cpad_y;
    if (cpad_y > st->cpad_max_y) st->cpad_max_y = cpad_y;

    st->buttons_tested_mask |= held_buttons;
    st->sample_count++;

    if (at_rest) {
        st->rest_x = cpad_x;
        st->rest_y = cpad_y;
        float mag = sqrtf((float)(cpad_x * cpad_x + cpad_y * cpad_y));
        st->drift_pct = (mag / 156.0f) * 100.0f;
        if (st->drift_pct > 100.0f) st->drift_pct = 100.0f;
        int dz = (int)ceilf(mag) + 6;
        st->recommended_deadzone = dz < 12 ? 12 : dz;
    }
}

bool pt_export_report(const char *sd_root, const pt_state_t *st, char *out_path, size_t cap) {
    if (sd_root == NULL || st == NULL) return false;
    char dir[PT_PATH_MAX + 32];
    snprintf(dir, sizeof(dir), "%s/3ds/padtest", sd_root);
    ensure_dir_p(dir);

    char final_path[PT_PATH_MAX + 64];
    char tmp_path[PT_PATH_MAX + 80];
    snprintf(final_path, sizeof(final_path), "%s/hardware_report.json", dir);
    snprintf(tmp_path, sizeof(tmp_path), "%s/hardware_report.json.tmp", dir);

    FILE *fp = fopen(tmp_path, "wb");
    if (fp == NULL) return false;
    fprintf(fp, "{\n");
    fprintf(fp, "  \"samples\": %u,\n", (unsigned)st->sample_count);
    fprintf(fp, "  \"cpad_min_x\": %d,\n", st->cpad_min_x);
    fprintf(fp, "  \"cpad_max_x\": %d,\n", st->cpad_max_x);
    fprintf(fp, "  \"cpad_min_y\": %d,\n", st->cpad_min_y);
    fprintf(fp, "  \"cpad_max_y\": %d,\n", st->cpad_max_y);
    fprintf(fp, "  \"rest_x\": %d,\n", st->rest_x);
    fprintf(fp, "  \"rest_y\": %d,\n", st->rest_y);
    fprintf(fp, "  \"drift_pct\": %.2f,\n", st->drift_pct);
    fprintf(fp, "  \"recommended_deadzone\": %d,\n", st->recommended_deadzone);
    fprintf(fp, "  \"buttons_tested_mask\": %u\n", (unsigned)st->buttons_tested_mask);
    fprintf(fp, "}\n");
    fflush(fp);
    fclose(fp);

    if (rename(tmp_path, final_path) != 0) {
        remove(tmp_path);
        return false;
    }
    if (out_path != NULL && cap > 0) {
        snprintf(out_path, cap, "%s", final_path);
    }
    return true;
}
