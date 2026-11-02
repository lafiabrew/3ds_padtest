#ifndef PT_PADTEST_H
#define PT_PADTEST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PT_PATH_MAX 512

typedef struct {
    int cpad_x;       /* -156..156 */
    int cpad_y;       /* -156..156 */
    int cpad_min_x;
    int cpad_max_x;
    int cpad_min_y;
    int cpad_max_y;
    int rest_x;
    int rest_y;
    float drift_pct;  /* 0.0..100.0% */
    int recommended_deadzone;
    int pixel_test_mode; /* 0=Off, 1=Red, 2=Green, 3=Blue, 4=White, 5=Black */
    uint32_t buttons_tested_mask;
    uint32_t sample_count;
} pt_state_t;

void pt_init(pt_state_t *st);
void pt_record_sample(pt_state_t *st, int cpad_x, int cpad_y, uint32_t held_buttons, bool at_rest);
bool pt_export_report(const char *sd_root, const pt_state_t *st, char *out_path, size_t cap);

#endif /* PT_PADTEST_H */
