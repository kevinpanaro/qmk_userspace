#include "socd_handler.h"

#ifdef SOCD_CLEANER_ENABLE
#include "layer_names.h"

enum socd_cleaner_resolution current_resolution = SOCD_CLEANER_OFF;

socd_cleaner_t socd_opposing_pairs[NUM_SOCD_PAIRS] = {
    {{KC_A, KC_D}, SOCD_CLEANER_OFF},
};

#else
// When SOCD is disabled, provide dummy array to satisfy introspection
socd_cleaner_t socd_opposing_pairs[NUM_SOCD_PAIRS] = {
    {{KC_NO, KC_NO}, SOCD_CLEANER_OFF},
};
#endif

uint32_t last_input = 0;

layer_state_t socd_layer_state_set(layer_state_t state) {
#ifdef SOCD_CLEANER_ENABLE
    socd_cleaner_enabled =
        IS_LAYER_ON_STATE(state, GAME) ||
        IS_LAYER_ON_STATE(state, VLRNT);
#endif
    return state;
}

bool socd_process_record(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        last_input = timer_read32();
    }

#ifdef SOCD_CLEANER_ENABLE
    switch (keycode) {
        case SOCD_CYCL:
            if (record->event.pressed) {
                current_resolution++;
                if (current_resolution == SOCD_CLEANER_NUM_RESOLUTIONS) {
                    current_resolution = SOCD_CLEANER_OFF;
                }
                socd_opposing_pairs[0].resolution = current_resolution;
            }
            return false;
    }
#endif

    return true;
}

#ifdef SOCD_CLEANER_ENABLE
void oled_current_socd_state(void) {
    switch (current_resolution) {
        case SOCD_CLEANER_OFF: oled_write_P(PSTR("off  "), false); break;
        case SOCD_CLEANER_LAST: oled_write_P(PSTR("last  "), false); break;
        case SOCD_CLEANER_NEUTRAL: oled_write_P(PSTR("nutrl"), false); break;
        case SOCD_CLEANER_0_WINS: oled_write_P(PSTR("0wins"), false); break;
        case SOCD_CLEANER_1_WINS: oled_write_P(PSTR("1wins"), false); break;
        default: oled_write_P(PSTR("     "), false); break;
    }
}
#endif
