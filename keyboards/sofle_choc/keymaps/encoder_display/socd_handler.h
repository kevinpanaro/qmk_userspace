#pragma once

#include "quantum.h"
#include "socd_cleaner.h"

#ifdef SOCD_CLEANER_ENABLE

// Expose the current SOCD resolution
extern enum socd_cleaner_resolution current_resolution;

// SOCD key pair array
#define NUM_SOCD_PAIRS 1
extern socd_cleaner_t socd_opposing_pairs[NUM_SOCD_PAIRS];

// Last input timestamp (for use by other modules if needed)
extern uint32_t last_input;

// Custom keycodes for SOCD
enum custom_keycodes {
    SOCD_CYCL = SAFE_RANGE,
};

// Keycode aliases for use in keymaps
#define SOCDTOG SOCDTOG  // Will be defined in socd_cleaner.h

// Functions
layer_state_t socd_layer_state_set(layer_state_t state);
bool socd_process_record(uint16_t keycode, keyrecord_t *record);
void oled_current_socd_state(void);

#else
// When SOCD is disabled, provide minimal definitions

// SOCD key pair array (dummy when disabled)
#define NUM_SOCD_PAIRS 1
extern socd_cleaner_t socd_opposing_pairs[NUM_SOCD_PAIRS];

// Last input timestamp (for use by other modules if needed)
extern uint32_t last_input;

// Custom keycodes placeholder (empty when SOCD disabled)
enum custom_keycodes {
    SOCD_PLACEHOLDER = SAFE_RANGE,
};

// Map SOCD keycodes to regular keys when disabled
#define SOCDTOG KC_7
#define SOCD_CYCL KC_U

// Stub functions that do nothing when SOCD is disabled
layer_state_t socd_layer_state_set(layer_state_t state);
bool socd_process_record(uint16_t keycode, keyrecord_t *record);
#ifdef OLED_ENABLE
static inline void oled_current_socd_state(void) { oled_write_P(PSTR("     "), false); }
#endif

#endif // SOCD_CLEANER_ENABLE
