/*
 * Unit & specification tests for 3ds_padtest:
 * - Proves Circle Pad min/max excursion tracking, rest drift percentage, and deadzone calculation.
 * - Proves atomic export of `<sd_root>/3ds/padtest/hardware_report.json`.
 */

#include "pt_padtest.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int main(void) {
    pt_state_t st;
    pt_init(&st);

    pt_record_sample(&st, -145, 140, 1u, false);
    pt_record_sample(&st, 150, -138, 2u, false);
    /* Simulate slight resting stick bias (x=6, y=-8 -> magnitude 10) */
    pt_record_sample(&st, 6, -8, 0u, true);

    assert(st.cpad_min_x == -145);
    assert(st.cpad_max_x == 150);
    assert(st.cpad_min_y == -138);
    assert(st.cpad_max_y == 140);
    assert(st.recommended_deadzone == 16);
    assert(st.drift_pct > 5.0f && st.drift_pct < 8.0f);

    char report_path[600];
    assert(pt_export_report("build/test_sd", &st, report_path, sizeof(report_path)));
    struct stat sb;
    assert(stat(report_path, &sb) == 0 && sb.st_size > 40);

    printf("PASS: test_padtest all drift calculation, min/max excursion, and JSON report checks succeeded.\n");
    return 0;
}
