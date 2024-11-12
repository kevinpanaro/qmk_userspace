/* Copyright 2023 Brian Low
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "action_layer.h"
#include "oled_driver.h"
#include QMK_KEYBOARD_H

enum layer_names {
    /* _M_XYZ = Mac Os, _W_XYZ = Win/Linux */
    MACOS,
#if defined(TRI_LAYER_ENABLE)
    LOWER,
    UPPER,
    ADJUST,
#endif
    TOTAL_LAYERS,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 * MacOS
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │  `   │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │   6  │   7  │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ ESC  │   Q  │   W  │   E  │   R  │   T  │ │   │       │   │  │   Y  │   U  │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ Tab  │   A  │   S  │   D  │   F  │   G  ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤   ◯   │    │   ◯   ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │LShift│   Z  │   X  │   C  │   V  │   B  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │UPPER │ LCTL │ LOPT │ LCMD │ Space │    │ Enter │ RCMD │ ROPT │ RALT │LOWER │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/

[MACOS] = LAYOUT(
    KC_GRV,   KC_1,   KC_2,    KC_3,    KC_4,    KC_5,                       KC_6,     KC_7,     KC_8,    KC_9,    KC_0,    KC_GRV,
    KC_ESC,   KC_Q,   KC_W,    KC_E,    KC_R,    KC_T,                       KC_Y,     KC_U,     KC_I,    KC_O,    KC_P,    KC_BSPC,
    KC_TAB,   KC_A,   KC_S,    KC_D,    KC_F,    KC_G,                       KC_H,     KC_J,     KC_K,    KC_L,    KC_SCLN, KC_QUOT,
    KC_LSFT,  KC_Z,   KC_X,    KC_C,    KC_V,    KC_B,    KC_MUTE,   KC_MPLY,KC_N,     KC_M,     KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
                      TL_UPPR, KC_LCTL, KC_LOPT, KC_LCMD, KC_SPC,    KC_ENT, KC_RCMD,  KC_RCMD,  KC_RGUI, TL_LOWR
),
/*
 * TRI_LAYER_LOWER_LAYER
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │  `   │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │   6  │   7  │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ ESC  │   Q  │   W  │   E  │   R  │   T  │ │   │       │   │  │   Y  │   U  │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ Tab  │   A  │   S  │   D  │   F  │   G  ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤   ◯   │    │   ◯   ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │LShift│   Z  │   X  │   C  │   V  │   B  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │UPPER │ LCTL │ LOPT │ LCMD │ Space │    │ Enter │ RCMD │ ROPT │ RALT │LOWER │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/

#if defined(TRI_LAYER_ENABLE)
[LOWER] = LAYOUT(
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
                      _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX,  XXXXXXX,  XXXXXXX, _______
),
/*
 * TRI_LAYER_UPPER_LAYER
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │  `   │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │   6  │   7  │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ ESC  │   Q  │   W  │   E  │   R  │   T  │ │   │       │   │  │   Y  │   U  │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ Tab  │   A  │   S  │   D  │   F  │   G  ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤   ◯   │    │   ◯   ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │LShift│   Z  │   X  │   C  │   V  │   B  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │UPPER │ LCTL │ LOPT │ LCMD │ Space │    │ Enter │ RCMD │ ROPT │ RALT │LOWER │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/

[UPPER] = LAYOUT(
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
                      _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX,  XXXXXXX,  XXXXXXX, _______
),
/*
 * TRI_LAYER_ADJUST_LAYER
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │  `   │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │   6  │   7  │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ ESC  │   Q  │   W  │   E  │   R  │   T  │ │   │       │   │  │   Y  │   U  │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ Tab  │   A  │   S  │   D  │   F  │   G  ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤   ◯   │    │   ◯   ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │LShift│   Z  │   X  │   C  │   V  │   B  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │UPPER │ LCTL │ LOPT │ LCMD │ Space │    │ Enter │ RCMD │ ROPT │ RALT │LOWER │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/

[ADJUST] = LAYOUT(
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
                      _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX,  XXXXXXX,  XXXXXXX, _______
),
#endif // defined(TRI_LAYER_ENDABLE)
};

#if defined(KEY_OVERRIDE_ENABLE)

#endif

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [MACOS] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT) },

#if defined(TRI_LAYER_ENABLE)
    [LOWER] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT) },
    [UPPER] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT) },
    [ADJUST] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT) },
#endif  // defined(TRI_LAYER_ENABLE)
};
#endif


#if defined(OS_DETECTION_ENABLE)

bool process_detected_host_os_user(os_variant_t detected_os) {
    switch (detected_os) {
        case OS_MACOS:
        case OS_IOS:
            break;
        case OS_WINDOWS:
            break;
        case OS_LINUX:
            break;
        case OS_UNSURE:
            break;
    }
    return true;
}
#endif

#if defined(OLED_ENABLE)

// Rotate the slave OLED
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (!is_keyboard_master()) {
        return OLED_ROTATION_180;  // flips the display 180 degrees if offhand
    }

    return rotation;
}

void oled_current_layer(void) {
    // Current Layer
    switch (get_highest_layer(layer_state)) {
        case MACOS:
            oled_write_P(PSTR("MacOS"), false);
            break;
        default:
            oled_write_ln_P(PSTR("Undef"), false);
    }
}

void oled_master(void) {
    oled_current_layer();
}

void oled_slave(void) {
}

bool oled_task_user(void) {
    if (is_keyboard_master()) {
        oled_master();  // Renders master
    } else {
        oled_slave();  // Renders slave
    }
    return false;
}
#endif