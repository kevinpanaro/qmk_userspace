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
#include "action.h"
#include "action_layer.h"
#include "action_util.h"
#include "keycodes.h"
#include "oled_driver.h"
#include "quantum_keycodes.h"
#include QMK_KEYBOARD_H

enum layer_names {
    /* _M_XYZ = Mac Os, _W_XYZ = Win/Linux */
    MACOS,
#if defined(TRI_LAYER_ENABLE)
    LOWER,
    UPPER,
    ADJUST,
#endif
    COMMAND,
    TOTAL_LAYERS,
};

#if defined(COMBO_ENABLE)
enum combos {
    CMD_LAYER,
};
#endif // defined(COMBO_ENABLE)

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
    KC_GRV,   KC_1,   KC_2,    KC_3,    KC_4,    KC_5,                       KC_6,     KC_7,     KC_8,    KC_9,    KC_0,    KC_MINS,
    KC_ESC,   KC_Q,   KC_W,    KC_E,    KC_R,    KC_T,                       KC_Y,     KC_U,     KC_I,    KC_O,    KC_P,    KC_BSPC,
    KC_TAB,   KC_A,   KC_S,    KC_D,    KC_F,    KC_G,                       KC_H,     KC_J,     KC_K,    KC_L,    KC_SCLN, KC_QUOT,
    KC_LSFT,  KC_Z,   KC_X,    KC_C,    KC_V,    KC_B,    KC_MUTE,   KC_F20,KC_N,     KC_M,     KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
                      TL_UPPR, KC_LCTL, KC_LOPT, KC_LCMD, KC_SPC,            KC_ENT, TL_LOWR, KC_RCMD,  KC_ROPT,  PB_1
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
    KC_GRV , KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                            KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, XXXXXXX,    KC_F12,
    _______, KC_EQL,  KC_MINS, KC_LPRN, KC_RPRN, KC_PLUS,                         KC_COLN, KC_LBRC, KC_RBRC, KC_UNDS, KC_PERC, KC_PIPE,
    _______, KC_EXLM, KC_AT ,  KC_HASH, KC_LCBR, KC_RCBR, _______,       _______, KC_CIRC, KC_AMPR, KC_ASTR, KC_DLR,  KC_BSLS, _______,
                      _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, _______,  XXXXXXX,  XXXXXXX, _______
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
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     KC_LEFT, KC_UP  , KC_DOWN, KC_RIGHT,XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
                      _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, _______,  XXXXXXX,  XXXXXXX, _______
),
/*
 * TRI_LAYER_ADJUST_LAYER
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │ BOOT │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │   6  │   7  │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ ESC  │   Q  │   W  │   E  │   R  │   T  │ │   │       │   │  │   Y  │   U  │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ Tab  │   A  │   S  │RGB_RM│RGB_T │RGB_M ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤   ◯   │    │   ◯   ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │LShift│   Z  │   X  │   C  │   V  │   B  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │UPPER │ LCTL │ LOPT │ LCMD │ Space │    │ Enter │ RCMD │ ROPT │ RALT │LOWER │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/

[ADJUST] = LAYOUT(
    QK_BOOT, XXXXXXX, XXXXXXX, XXXXXXX , XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX , XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, RGB_RMOD, RGB_TOG, RGB_MOD,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX , XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
                      _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, _______,  XXXXXXX,  XXXXXXX, _______
),
#endif // defined(TRI_LAYER_ENABLE)

/*
 * TRI_LAYER_ADJUST_LAYER
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │      │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │   6  │   7  │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ ESC  │   Q  │   W  │   E  │   R  │   T  │ │   │       │   │  │   Y  │   U  │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ Tab  │   A  │   S  │RGB_RM│RGB_T │RGB_M ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤   ◯   │    │   ◯   ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │LShift│   Z  │   X  │   C  │   V  │   B  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │UPPER │ LCTL │ LOPT │ LCMD │ Space │    │ Enter │ RCMD │ ROPT │ RALT │LOWER │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/
#if defined(KEY_OVERRIDE_ENABLE) || defined(COMBO_ENABLE)
[COMMAND] = LAYOUT(
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX , XXXXXXX, XXXXXXX,                     KC_LEFT, KC_DOWN, KC_UP  , KC_RGHT, XXXXXXX, XXXXXXX,
    XXXXXXX, KC_EQL,  KC_MINS, KC_LPRN , KC_RPRN, KC_PLUS,                     KC_COLN, KC_LBRC, KC_RBRC, KC_UNDS, KC_PERC, KC_PIPE,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX , XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
                              _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, _______,  XXXXXXX,  XXXXXXX, _______
),
#endif  // defined(KEY_OVERRIDE_ENABLE)
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    }
    return true;
};

#if defined(KEY_OVERRIDE_ENABLE)
// This doesn't work because left and right command are the same key
bool momentary_layer(bool key_down, void *layer) {
    if (key_down) {
        layer_on((uint8_t)(uintptr_t)layer);
    } else {
        layer_off((uint8_t)(uintptr_t)layer);
    }

    return false;
}
const key_override_t cmd_override = {.trigger_mods          = MOD_BIT(KC_LCMD) | MOD_BIT(KC_RCMD),                       //
                                   .layers                 = ~0,                                          //
                                   .suppressed_mods        = MOD_BIT(KC_LCMD) | MOD_BIT(KC_RCMD),                       //
                                   .options                = ko_option_activation_trigger_down,                 //
                                   .negative_mod_mask      = (uint8_t) ~(MOD_BIT(KC_LCMD) | MOD_BIT(KC_RCMD)),          //
                                   .custom_action          = momentary_layer,                                           //
                                   .context                = (void *)COMMAND,                                          //
                                   .trigger                = KC_NO,                                                     //
                                   .replacement            = KC_NO,                                                     //
                                   .enabled                = NULL};
const key_override_t *key_overrides[] = {
    &cmd_override,
};
#endif  // defined(KEY_OVERRIDE_ENABLE)

#if defined(COMBO_ENABLE)
const uint16_t PROGMEM cmd_combo[] = {KC_LCMD, KC_RCMD, COMBO_END};

combo_t key_combos[] = {
    [CMD_LAYER] = COMBO(cmd_combo, MO(COMMAND))
};
#endif // defined(COMBO_ENABLE)

#if defined(ENCODER_ENABLE)
uint8_t mod_state;
bool encoder_update_user(uint8_t index, bool clockwise) {
    mod_state = get_mods();
    clear_mods();
    if (index == 0) { /* First encoder */
        if (mod_state & MOD_MASK_GUI) { /* Command */
            // Change Brightness of Main Monitor
            if (clockwise) {
                tap_code(KC_BRMU);
            } else {
                tap_code(KC_BRMD);
            }
        } else if (mod_state & MOD_MASK_CTRL) { /* Control */
            // Change Brightness of Extended Monitor
            register_code(KC_LCTL);
            if (clockwise) {
                tap_code(KC_BRMU);
            } else {
                tap_code(KC_BRMD);
            }
            unregister_code(KC_LCTL);
        } else {
            // Change Volume
            if (clockwise) {
                tap_code(KC_VOLU);
            } else {
                tap_code(KC_VOLD);
            }
        }
    } else if (index == 1) { /* Second encoder */
        if (mod_state & MOD_MASK_GUI) { /* Command */
            if (clockwise) {
                tap_code(KC_F16);  /* VSCode Debug : Continue */
            } else {
                tap_code(KC_F11);  /* VSCode Debug : Step Out */
            }
        } else if (mod_state & MOD_MASK_ALT) { /* Option */
            register_code(KC_LOPT);
            if (clockwise) {
                tap_code(KC_RIGHT);  /* Next Word*/
            } else {
                tap_code(KC_LEFT);  /* Previous Word */
            }
            unregister_code(KC_LOPT);
        } else {
            if (clockwise) {
                tap_code(KC_F17);  /* VSCode Debug : Step Over */
            } else {
                tap_code(KC_F11);  /* VSCode Debug : Step Into */
            }
        }
    }
    set_mods(mod_state);
    return false;
}
#endif // defined(ENCODER_ENABLE)

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
#endif // defined(OS_DETECTION_ENABLE)

#if defined(OLED_ENABLE)

// Rotate the slave OLED
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;
    }
    else {
        return OLED_ROTATION_0;
    }
    return rotation;
}

void oled_current_layer(void) {
    // Current Layer
    switch (get_highest_layer(layer_state)) {
        case MACOS:
            oled_write_P(PSTR("MacOS"), false);
            break;
        case LOWER:
            oled_write_P(PSTR("Lower"), false);
            break;
        case UPPER:
            oled_write_P(PSTR("Raise"), false);
            break;
        case ADJUST:
            oled_write_P(PSTR("Adjust"), false);
            break;
        case COMMAND:
            oled_write_P(PSTR("cmd"), false);
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

void oled_render_boot(bool bootloader) {
    oled_clear();
    for (int i = 0; i < 16; i++) {
        oled_set_cursor(0, i);
        if (bootloader) {
            oled_write_P(PSTR("Awaiting New Firmware "), false);
        } else {
            oled_write_P(PSTR("Rebooting "), false);
        }
    }

    oled_render_dirty(true);
}

#endif // defined(OLED_ENABLE)

// Shutdown User : https://docs.qmk.fm/custom_quantum_functions#keyboard-shutdown-reboot-code
bool shutdown_user(bool jump_to_bootloader) {
    oled_render_boot(jump_to_bootloader);
    // false to not process kb level
    return false;
}


#if defined(RAW_ENABLE)
void raw_hid_receive(uint8_t *data, uint8_t length) {
    uint8_t response[length];
    memset(response, 0, length);
    response[0] = 'B';

    if(data[0] == 'A') {
        raw_hid_send(response, length);
    }
}
#endif // defined(RAW_ENABLE)
