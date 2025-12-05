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
#include "keycodes.h"
#include "oled_driver.h"
#include QMK_KEYBOARD_H
#include "socd_cleaner.h"

enum sofle_layers {
    /* _M_XYZ = Mac Os, _W_XYZ = Win/Linux */
    QWERTY,
    MACOS,
    GAME,
#if defined(TRI_LAYER_ENABLE)
    LOWER = 29,
    UPPER = 30,  // 4
    ADJUST = 31, // 5
#endif
};

static enum socd_cleaner_resolution current_resolution = SOCD_CLEANER_NEUTRAL;
enum custom_keycodes {
    SOCD_CYCL = SAFE_RANGE
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 * QWERTY
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  `   |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  `   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | ESC  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|  Mute |    | Pause |------+------+------+------+------+------|
 * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            | LCTL | LGUI | LCMD | LALT | /Enter  /       \Space \  | RALT | RCMD | RGUI | RCTL |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */
[QWERTY] = LAYOUT(
  KC_GRV,   KC_1,   KC_2,    KC_3,    KC_4,    KC_5,                     KC_6,    KC_7,    KC_8,    KC_9,    KC_0,  KC_MINS,
  KC_ESC,   KC_Q,   KC_W,    KC_E,    KC_R,    KC_T,                     KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,  KC_BSPC,
  KC_TAB,   KC_A,   KC_S,    KC_D,    KC_F,    KC_G,                     KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN,  KC_QUOT,
  KC_LSFT,  KC_Z,   KC_X,    KC_C,    KC_V,    KC_B, KC_MUTE,    KC_MPLY,KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH,  KC_RSFT,
               TL_UPPR , KC_LALT, KC_LGUI, KC_LCTL , KC_SPC,    KC_ENT,  TL_LOWR, KC_RGUI, KC_RALT, KC_RCTL
),

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
 * GAME
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │ ESC  │   1  │   2  │      │   3  │   4  │ │   │       │   │  │   6  │   7  │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │   M  │ TAB  │   Q  │   W  │   E  │   R  │ │   │       │   │  │   Y  │   U  │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ CAPS │   G  │   A  │   S  │   D  │   F  ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤   ◯   │    │   ◯   ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ LCTR │LShift│   Z  │   X  │   C  │   V  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │RAISE │ LALT │      │      │ Space │    │   B   │LOWER │ RGUI │ RALT │ RCTL │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/
[GAME] = LAYOUT(
  KC_ESC,   KC_1,   KC_2,    KC_3,    KC_4,    KC_5,                                  KC_6,    SOCD_CYCL,  KC_8,    KC_9,    KC_0,  KC_MINS,
  KC_M,     KC_T,   KC_Q,    KC_W,    KC_E,    KC_R,                                  KC_Y,    KC_U,     KC_I,    KC_O,    KC_P,  KC_BSPC,
  KC_TAB,   KC_G,   KC_A,    KC_S,    KC_D,    KC_F,                                  KC_H,    KC_J,     KC_K,    KC_L, KC_SCLN,  KC_QUOT,
  KC_LSFT,  KC_B,   KC_Z,    KC_X,    KC_C,    KC_V,    KC_MUTE,    KC_MPLY,  KC_N,    KC_M,     KC_COMM, KC_DOT,   KC_SLSH,  KC_RSFT,
                            XXXXXXX, KC_LALT, KC_PPLS,  KC_EQL,  KC_SPC,     KC_ENT,   _______, TL_UPPR,  KC_RALT, _______
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
    XXXXXXX, KC_F1  , KC_F2  , KC_F3  , KC_F4  , KC_F5  ,                     KC_F6 , KC_F7  , KC_F8  , KC_F9  , KC_F10 , XXXXXXX,
    KC_GRV , KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                            KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, XXXXXXX,    KC_F12,
    _______, KC_EQL,  KC_MINS, KC_LPRN, KC_RPRN, KC_PLUS,                         KC_COLN, KC_LBRC, KC_RBRC, KC_UNDS, KC_PERC, KC_PIPE,
    _______, KC_EXLM, KC_AT ,  KC_HASH, KC_LCBR, KC_RCBR, _______,       _______, KC_CIRC, KC_AMPR, KC_ASTR, KC_DLR,  KC_BSLS, _______,
                      _______, _______, _______, _______, _______,   _______, _______,  _______,  _______, _______
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
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_CAPS,                     KC_LEFT, KC_UP  , KC_DOWN, KC_RIGHT,XXXXXXX, XXXXXXX,
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
    QK_BOOT, DF(0), DF(MACOS), DF(GAME) , XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_BOOT,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX , XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, RM_PREV, RM_TOGG, RM_NEXT,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX , XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
                      _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,   XXXXXXX, _______,  XXXXXXX,  DF(0), QK_RBT
),
#endif // defined(TRI_LAYER_ENABLE)

};


layer_state_t layer_state_set_user(layer_state_t state) {
    socd_cleaner_enabled = IS_LAYER_ON_STATE(state, GAME);
    return state;
}

// for more dynamic encoders, see encoder_update_user
#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT) },
    { ENCODER_CCW_CW(KC_VOLD, KC_VOLU), ENCODER_CCW_CW(KC_MPRV, KC_MNXT) },
};
#endif

socd_cleaner_t socd_opposing_pairs[] = {
    // {{KC_W, KC_S}, SOCD_CLEANER_LAST},
    {{KC_A, KC_D}, SOCD_CLEANER_NEUTRAL},
};


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case SOCD_CYCL:
            if (record->event.pressed) {
                if (current_resolution == SOCD_CLEANER_NUM_RESOLUTIONS) {
                    current_resolution = SOCD_CLEANER_OFF;
                }
                socd_opposing_pairs[0].resolution = current_resolution;

                current_resolution++;

                // socd_opposing_pairs[1].resolution = SOCD_CLEANER_LAST;
            }
      break;
    }
    return true;
};

#if defined(OLED_ENABLE)

// Rotate both OLEDs the same way
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;
    }
    else {
        return OLED_ROTATION_270;  // Match master rotation
    }
    return rotation;
}

void oled_current_layer(void) {
    // Current Layer
    switch (get_highest_layer(default_layer_state)) {
        case QWERTY:
            oled_write_P(PSTR("pc   "), false);
            break;
        case GAME:
            oled_write_P(PSTR("game "), false);
            break;
        case MACOS:
            oled_write_P(PSTR("macos"), false);
            break;
        default:
            oled_write_P(PSTR("undef"), false);
    }
}

void oled_current_tri_layer(void) {
    switch (get_highest_layer(layer_state)) {
        case LOWER:
            oled_write_P(PSTR("lower"), false);
            break;
        case UPPER:
            oled_write_P(PSTR("raise"), false);
            break;
        case ADJUST:
            oled_write_P(PSTR("adjst"), false);
            break;
        default:
            oled_write_P(PSTR("base "), false);
    }
}

void oled_current_socd_state(void) {
    switch (current_resolution) {
        // Disable SOCD filtering for this key pair.
        case SOCD_CLEANER_OFF:
            oled_write_P(PSTR("off  "), false);
            break;

        // Last input priority with reactivation.
        case SOCD_CLEANER_LAST:
            oled_write_P(PSTR("last  "), false);
            break;

        // Neutral resolution. When both keys are pressed, they cancel.
        case SOCD_CLEANER_NEUTRAL:
            oled_write_P(PSTR("nutrl"), false);
            break;

        // Key 0 always wins.
        case SOCD_CLEANER_0_WINS:
            oled_write_P(PSTR("0wins"), false);
            break;

        // Key 1 always wins.
        case SOCD_CLEANER_1_WINS:
            oled_write_P(PSTR("1wins"), false);
            break;

        // Sentinel to count the number of resolution strategies.
        // case SOCD_CLEANER_NUM_RESOLUTIONS:
        //     oled_write_P(PSTR("undef"), false);
        //     break;
        default:
            oled_write_P(PSTR("     "), false);
    }
}


void oled_master(void) {
    if (host_keyboard_led_state().caps_lock) {
        oled_write_P(PSTR("BREAD"), true);
    } else {
        oled_write_P(PSTR("bread"), false);
    }
    oled_write_P(PSTR("\n"), false);
    oled_current_layer();
    oled_write_P(PSTR("\n"), false);
    oled_current_tri_layer();
    oled_write_P(PSTR("\n"), false);

    oled_current_socd_state();
}

void oled_slave(void) {
    // Show encoder status on slave side too
    oled_current_layer();
    oled_write_P(PSTR("\n"), false);

    oled_current_tri_layer();
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
