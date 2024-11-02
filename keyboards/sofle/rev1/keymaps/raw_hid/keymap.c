/* Copyright 2021 Carlos Martins
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

#include "quantum.h"
#include QMK_KEYBOARD_H
#include "raw_hid.h"
#include <string.h>
#define RAW_EPSIZE 32
bool is_hid_connected = false; // is pc connected yet?


enum sofle_layers {
    /* _M_XYZ = Mac Os, _W_XYZ = Win/Linux */
    _QWERTY,
    _MAC,
    _VALORANT,
    _APEX,
    _GAME,
    _LOWER,
    _RAISE,
    _ADJUST,
};

enum custom_keycodes {
    KC_LOWER = SAFE_RANGE,
    KC_RAISE,
    KC_ADJUST,
    KC_PRVWD,
    KC_NXTWD,
    KC_LSTRT,
    KC_LEND,
    KC_DLINE,
    KC_SUPRG,
    KC_M20RR,
    KC_NO_TIME,
#ifdef OS_DETECTION_DEBUG_ENABLE
    STORE_SETUPS,
    PRINT_SETUPS,
#endif
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

/*
 * QWERTY
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  `   |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  -   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | ESC  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            |RAISE | LAlt | LGUI | LCTL | /Space  /       \Enter \  |LOWER | RGUI | RAlt | RCTL |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */

[_QWERTY] = LAYOUT(
  KC_GRV,   KC_1,   KC_2,    KC_3,    KC_4,    KC_5,                     KC_6,    KC_7,    KC_8,    KC_9,    KC_0,  KC_MINS,
  KC_ESC,   KC_Q,   KC_W,    KC_E,    KC_R,    KC_T,                     KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,  KC_BSPC,
  KC_TAB,   KC_A,   KC_S,    KC_D,    KC_F,    KC_G,                     KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN,  KC_QUOT,
  KC_LSFT,  KC_Z,   KC_X,    KC_C,    KC_V,    KC_B, KC_MUTE,    KC_MPLY,KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH,  KC_RSFT,
               KC_RAISE , KC_LALT, KC_LGUI, KC_LCTL , KC_SPC,    KC_ENT,  KC_LOWER, KC_RGUI, KC_RALT, KC_RCTL
),
/*
 * MAC          ALT -> CTL   GUI -> OPT  CTL -> CMD  AG_LSWP -> CG_LSWP
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  `   |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  -   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | ESC  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            |RAISE | LCTL | LOPT | LCMD | /Space  /       \Enter \  |LOWER | RCMD  | ROPT | RCTL |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */

[_MAC] = LAYOUT(
  KC_GRV,   KC_1,   KC_2,    KC_3,    KC_4,    KC_5,                     KC_6,    KC_7,    KC_8,    KC_9,    KC_0,  KC_MINS,
  KC_ESC,   KC_Q,   KC_W,    KC_E,    KC_R,    KC_T,                     KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,  KC_BSPC,
  KC_TAB,   KC_A,   KC_S,    KC_D,    KC_F,    KC_G,                     KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN,  KC_QUOT,
  KC_LSFT,  KC_Z,   KC_X,    KC_C,    KC_V,    KC_B, KC_MUTE,    KC_MPLY,KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH,  KC_RSFT,
               _______,KC_LCTL, KC_LOPT, KC_LCMD , KC_SPC,      KC_ENT,  _______, KC_RCMD, KC_ROPT, KC_RCTL
),
/*
 * VALORANT
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | ESC  |   T  |   1  |   2  |   3  |   4  |                    |NOTIME|   7  |   8  |   9  |   0  |  -   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | TAB  | TAB  |   Q  |   W  |   E  |   R  |                    |M20RR |   U  |   I  |   O  |   P  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | CAPS | CAPS |   A  |   S  |   D  |   F  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * | LCTR |LShift|   Z  |   X  |   C  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            | RAISE| LAlt |   G  | CTRL | /Space  /       \Enter \  |LOWER | RGUI | RAlt | RCTL |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */
[_VALORANT] = LAYOUT(
  KC_ESC  ,     KC_T,    KC_1,    KC_2,    KC_3,    KC_4,                     KC_NO_TIME, KC_7,    KC_8,    KC_9,    KC_0,  KC_MINS,
  KC_TAB  ,   KC_TAB,    KC_Q,    KC_W,    KC_E,    KC_R,                     KC_M20RR,   KC_U,    KC_I,    KC_O,    KC_P,  KC_BSPC,
  KC_CAPS ,  KC_CAPS,    KC_A,    KC_S,    KC_D,    KC_F,                     KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN,  KC_QUOT,
  KC_LCTL ,  KC_LSFT,    KC_Z,    KC_X,    KC_C,    KC_B, KC_MUTE,    KC_MPLY,KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH,  KC_RSFT,
                _______,KC_LALT, KC_G,   KC_LCTL,    KC_SPC,           KC_ENT,  _______, KC_RGUI, KC_RALT, KC_RCTL
),
/*
 * APEX LEGENDS
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | ESC  |   1  |   2  |      |   3  |   4  |                    |   6  |   7  |   8  |   9  |   0  |  -   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |   M  | TAB  |   Q  |   W  |   E  |   R  |                    |   Y  |   U  |   I  |   O  |   P  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | CAPS |   G  |   A  |   S  |   D  |   F  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * | LCTR |LShift|   Z  |   X  |   C  |   V  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            |RAISE | LAlt |   B  |SUPRG | /Space  /       \   B  \  |LOWER | RGUI | RAlt | RCTL |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */

[_APEX] = LAYOUT(
  KC_ESC  ,     KC_1,   KC_2, XXXXXXX,    KC_3,    KC_4,                     KC_6,    KC_7,    KC_8,    KC_9,    KC_0,  KC_MINS,
  KC_M    ,   KC_TAB,   KC_Q,    KC_W,    KC_E,    KC_R,                     KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,  KC_BSPC,
  KC_CAPS ,     KC_G,   KC_A,    KC_S,    KC_D,    KC_F,                     KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN,  KC_QUOT,
  KC_LCTL ,  KC_LSFT,   KC_Z,    KC_X,    KC_C,    KC_V, KC_MUTE,    KC_MPLY,KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH,  KC_RSFT,
                _______, KC_LALT,    KC_B    , KC_SUPRG, KC_SPC,      KC_B,  _______, KC_RGUI, KC_RALT, KC_RCTL
),
/*
 * GAME
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | ESC  |   1  |   2  |      |   3  |   4  |                    |   6  |   7  |   8  |   9  |   0  |  -   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |   M  | TAB  |   Q  |   W  |   E  |   R  |                    |   Y  |   U  |   I  |   O  |   P  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | CAPS |   G  |   A  |   S  |   D  |   F  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * | LCTR |LShift|   Z  |   X  |   C  |   V  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            |RAISE | LAlt |      |      | /Space  /       \   B  \  |LOWER | RGUI | RAlt | RCTL |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */

[_GAME] = LAYOUT(
  KC_ESC  ,     KC_1,   KC_2, XXXXXXX,    KC_3,    KC_4,                     KC_6,    KC_7,    KC_8,    KC_9,    KC_0,  KC_MINS,
  KC_M    ,   KC_TAB,   KC_Q,    KC_W,    KC_E,    KC_R,                     KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,  KC_BSPC,
  KC_CAPS ,     KC_G,   KC_A,    KC_S,    KC_D,    KC_F,                     KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN,  KC_QUOT,
  KC_LCTL ,  KC_LSFT,   KC_Z,    KC_X,    KC_C,    KC_V, KC_MUTE,    KC_MPLY,KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH,  KC_RSFT,
                _______, KC_LALT, XXXXXXX, XXXXXXX, KC_SPC,      KC_B,  _______, KC_RGUI, KC_RALT, KC_RCTL
),
/* LOWER
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |      |  F1  |  F2  |  F3  |  F4  |  F5  |                    |  F6  |  F7  |  F8  |  F9  | F10  | F11  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |  `   |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  | F12  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   -  |   =  |   (  |   )  |   +  |-------.    ,-------|   :  |   [  |   ]  |   _  |   %  |   |  |
 * |------+------+------+------+------+------|  MUTE |    |       |------+------+------+------+------+------|
 * | Shift|   !  |  @   |   #  |   {  |   }  |-------|    |-------|   ^  |   &  |   *  |      |   \  | Shift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            | LGUI | LAlt | LCTR |LOWER | /Space  /       \Enter \  |RAISE | RCTR | RAlt | RGUI |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */
[_LOWER] = LAYOUT(
  _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                           KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
  KC_GRV , KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                            KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_0,    KC_F12,
  _______, KC_EQL,  KC_MINS, KC_LPRN, KC_RPRN, KC_PLUS,                         KC_COLN, KC_LBRC, KC_RBRC, KC_UNDS, KC_PERC, KC_PIPE,
  _______, KC_EXLM, KC_AT ,  KC_HASH, KC_LCBR, KC_RCBR, _______,       _______, KC_CIRC, KC_AMPR, KC_ASTR, KC_DLR,  KC_BSLS, _______,
                  _______, _______, _______, _______, _______,           _______, _______, _______, _______, _______
),
/* RAISE
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |      |      |      |      |      |      |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Esc  | Ins  | Pscr | Menu |      |RGBTog|                    |      | PWrd |  Up  | NWrd | DLine| Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  | LAt  | LCtl |LShift|      | Caps |-------.    ,-------|      | Left | Down | Rigth|  Del | Bspc |
 * |------+------+------+------+------+------|  MUTE |    |       |------+------+------+------+------+------|
 * |Shift | Undo |  Cut | Copy | Paste|      |-------|    |-------|      | LStr |      | LEnd |      | Shift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            | LGUI | LAlt | LCTR |LOWER | /Enter  /       \Space \  |RAISE | RCTR | RAlt | RGUI |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */
[_RAISE] = LAYOUT(
  _______, _______ , _______ , _______ , RGB_RMOD , RGB_MOD,                           _______,  _______  , _______,  _______ ,  _______ ,_______,
  _______,  KC_INS,  KC_PSCR,   KC_APP,  XXXXXXX, RGB_TOG,                        KC_PGUP, KC_PRVWD,   KC_UP, KC_NXTWD,KC_DLINE, KC_BSPC,
  _______, KC_LALT,  KC_LCTL,  KC_LSFT,  XXXXXXX, KC_CAPS,                       KC_PGDN,  KC_LEFT, KC_DOWN, KC_RGHT,  KC_DEL, KC_BSPC,
  _______,XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,  _______,       _______,  XXXXXXX, KC_LSTRT, XXXXXXX, KC_LEND,   XXXXXXX, _______,
                         _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______
),
/* ADJUST
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |      |      |      |      |      |      |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | RESET|      |QWERTY|COLEMAK|      |      |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      |MACWIN|      |      |      |-------.    ,-------|      | VOLDO| MUTE | VOLUP|      |      |
 * |------+------+------+------+------+------|  MUTE |    |       |------+------+------+------+------+------|
 * |      |      |      |      |      |      |-------|    |-------|      | PREV | PLAY | NEXT |      |      |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            | LGUI | LAlt | LCTR |LOWER | /Enter  /       \Space \  |RAISE | RCTR | RAlt | RGUI |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */
  [_ADJUST] = LAYOUT(
  EE_CLR , XXXXXXX,DF(_VALORANT) ,DF(_GAME) , DF(_APEX), XXXXXXX,            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  QK_RBT  , XXXXXXX, DF(_QWERTY) , DF(_MAC) , XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  XXXXXXX , XXXXXXX,  XXXXXXX   , XXXXXXX , XXXXXXX, XXXXXXX,                     XXXXXXX, KC_VOLD, KC_MUTE, KC_VOLU, XXXXXXX, XXXXXXX,
  XXXXXXX , XXXXXXX,  XXXXXXX   , XXXXXXX , XXXXXXX, XXXXXXX, XXXXXXX,     XXXXXXX, XXXXXXX, KC_MPRV, KC_MEDIA_PLAY_PAUSE, KC_MNXT, XXXXXXX, XXXXXXX,
                   _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______
  ),
};


#ifdef OLED_ENABLE
static void render_logo(void) {
    static const char PROGMEM qmk_logo[] = {
        // 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE, 0xDE,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0x00
    };
    oled_advance_page(true);
    oled_write_P(qmk_logo, false);
    oled_set_brightness(oled_get_brightness());
}

#ifdef RGB_MATRIX_ENABLE        
void render_rgb_status(void) {
    oled_write_ln("RGB:", false);
    static char temp[20] = {0};
    snprintf(temp, sizeof(temp) + 1, "M:%3dH:%3dS:%3dV:%3d", rgb_matrix_config.mode, rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v);
    oled_write(temp, false);
}
#endif


static void print_status_narrow(void) {
    // Print current mode

    // LINE 1
    if (host_keyboard_led_state().caps_lock) {
        oled_write_P(PSTR("BREAD"), false);
    } else {
        oled_write_P(PSTR("bread"), false);
    }
    
    // LINE 3
    oled_advance_page(true);
    switch (get_highest_layer(default_layer_state)) {
        case _QWERTY:
            oled_write_P(PSTR("win "), false);
            break;
        case _MAC:
            oled_write_P(PSTR("mac "), false);
            break;
        case _VALORANT:
            oled_write_P(PSTR("val "), false);
            break;
        case _APEX:
            oled_write_P(PSTR("apex"), false);
            break;
        case _GAME:
            oled_write_P(PSTR("game"), false);
            break;
        default:
            oled_write_P(PSTR("null"), false);
    }

    #ifdef RAW_ENABLE
    if ( is_hid_connected ) {
        oled_write_char(0x04, false);
        // oled_write_P(PSTR("\n\n\n"), false);
    } else {
        oled_advance_page(true);
    }
    #endif  // RAW_ENABLE

    // LINE 5
    oled_advance_page(true);
    // Print current layer
    switch (get_highest_layer(layer_state)) {
        case _QWERTY:
            oled_write_P(PSTR("base "), false);
            break;
        case _RAISE:
            oled_write_P(PSTR("raise"), false);
            break;
        case _LOWER:
            oled_write_P(PSTR("lower"), false);
            break;
        case _ADJUST:
            oled_write_P(PSTR("adjst"), false);
            break;
        default:
            oled_write_P(PSTR("null "), false);
    }

    // LINE 6
    oled_advance_page(true);
    // #if OS_DETECTION_ENABLE
    //     process_detected_host_os_user(detected_host_os() );
    // #endif
    oled_write_P(PSTR("B-"), false);
    oled_write(get_u8_str(oled_get_brightness(), ' '), false);
    oled_advance_page(true);
    // render_rgb_status();
    //oled_write_P(PSTR("\n\n"), false);
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;
    }
    else {
        return OLED_ROTATION_0;
    }
    return rotation;
}

bool oled_task_user(void) {
    if (is_keyboard_master()) {
        print_status_narrow();
    } else {
        render_logo();
    }
    return false;
}

#endif // OLED_ENABLE

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_LOWER:
            if (record->event.pressed) {
                layer_on(_LOWER);
                update_tri_layer(_LOWER, _RAISE, _ADJUST);
            } else {
                layer_off(_LOWER);
                update_tri_layer(_LOWER, _RAISE, _ADJUST);
            }
            return false;
        case KC_RAISE:
            if (record->event.pressed) {
                layer_on(_RAISE);
                update_tri_layer(_LOWER, _RAISE, _ADJUST);
            } else {
                layer_off(_RAISE);
                update_tri_layer(_LOWER, _RAISE, _ADJUST);
            }
            return false;
        case KC_ADJUST:
            if (record->event.pressed) {
                layer_on(_ADJUST);
            } else {
                layer_off(_ADJUST);
            }
            return false;
        case KC_PRVWD:
            if (record->event.pressed) {
                if (keymap_config.swap_lctl_lgui) {
                    register_mods(mod_config(MOD_LALT));
                    register_code(KC_LEFT);
                } else {
                    register_mods(mod_config(MOD_LCTL));
                    register_code(KC_LEFT);
                }
            } else {
                if (keymap_config.swap_lctl_lgui) {
                    unregister_mods(mod_config(MOD_LALT));
                    unregister_code(KC_LEFT);
                } else {
                    unregister_mods(mod_config(MOD_LCTL));
                    unregister_code(KC_LEFT);
                }
            }
            break;
        case KC_NXTWD:
             if (record->event.pressed) {
                if (keymap_config.swap_lctl_lgui) {
                    register_mods(mod_config(MOD_LALT));
                    register_code(KC_RIGHT);
                } else {
                    register_mods(mod_config(MOD_LCTL));
                    register_code(KC_RIGHT);
                }
            } else {
                if (keymap_config.swap_lctl_lgui) {
                    unregister_mods(mod_config(MOD_LALT));
                    unregister_code(KC_RIGHT);
                } else {
                    unregister_mods(mod_config(MOD_LCTL));
                    unregister_code(KC_RIGHT);
                }
            }
            break;
        case KC_LSTRT:
            if (record->event.pressed) {
                if (keymap_config.swap_lctl_lgui) {
                     //CMD-arrow on Mac, but we have CTL and GUI swapped
                    register_mods(mod_config(MOD_LCTL));
                    register_code(KC_LEFT);
                } else {
                    register_code(KC_HOME);
                }
            } else {
                if (keymap_config.swap_lctl_lgui) {
                    unregister_mods(mod_config(MOD_LCTL));
                    unregister_code(KC_LEFT);
                } else {
                    unregister_code(KC_HOME);
                }
            }
            break;
        case KC_LEND:
            if (record->event.pressed) {
                if (keymap_config.swap_lctl_lgui) {
                    //CMD-arrow on Mac, but we have CTL and GUI swapped
                    register_mods(mod_config(MOD_LCTL));
                    register_code(KC_RIGHT);
                } else {
                    register_code(KC_END);
                }
            } else {
                if (keymap_config.swap_lctl_lgui) {
                    unregister_mods(mod_config(MOD_LCTL));
                    unregister_code(KC_RIGHT);
                } else {
                    unregister_code(KC_END);
                }
            }
            break;
        case KC_DLINE:
            if (record->event.pressed) {
                register_mods(mod_config(MOD_LCTL));
                register_code(KC_BSPC);
            } else {
                unregister_mods(mod_config(MOD_LCTL));
                unregister_code(KC_BSPC);
            }
            break;
        case KC_SUPRG:
            if (record->event.pressed) {
                tap_code_delay(KC_SPC, 7);
                tap_code(KC_LCTL);
            }
            break;
        case KC_M20RR:
        // Originally I wanted this to be built in, but instead for space and speed
        //  I just added it to WinCompose Sequences.
            if (record->event.pressed) {
                tap_code(KC_RALT);
                tap_code(KC_MINUS);
                tap_code(KC_2);
                tap_code(KC_0);
            }    
            break;  
        case KC_NO_TIME:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_ENT);
                unregister_code(KC_LSFT);
                SEND_STRING("NO TIME");
                tap_code(KC_ENT);
            }
            break;
    }
    return true;
}

#ifdef ENCODER_ENABLE
uint8_t mod_state;
uint8_t brightness_step = 0x10;
bool encoder_update_user(uint8_t index, bool clockwise) {
    mod_state = get_mods();
    
    if (mod_state & MOD_MASK_SHIFT) {

        #ifdef RGB_MATRIX_ENABLE

        if (index == 0) { /* First encoder */
            if (clockwise) {
                rgb_matrix_increase_hue();
            } else {
                rgb_matrix_decrease_hue();
            }
        } else if (index == 1) { /* Second encoder */
            if (clockwise) {
                rgb_matrix_decrease_sat();
            } else {
                rgb_matrix_increase_sat();
            }
        }

        #endif

    } else if (mod_state & MOD_MASK_CTRL) {

        // #ifdef RGB_MATRIX_ENABLE

        // if (index == 0) { /* First encoder */
            if (clockwise) {
                if ((oled_get_brightness() + brightness_step) <= OLED_BRIGHTNESS) {
                    oled_set_brightness(oled_get_brightness() + brightness_step);
                } else {
                    oled_set_brightness(OLED_BRIGHTNESS);
                }
            } else {
                if (oled_get_brightness() > brightness_step) {
                    oled_set_brightness(oled_get_brightness() - brightness_step);
                } else {
                    oled_set_brightness(0x00);
                }
            }
        // }
    } else {
        if (index == 0) { /* First encoder */
            if (clockwise) {
                tap_code(KC_PGUP);
                // tap_code(KC_MS_WH_UP);
            } else {
                tap_code(KC_PGDN);
                // tap_code(KC_MS_WH_DOWN);
            }
        } else if (index == 1) { /* Second encoder */
            register_code(KC_LCTL);
            if (clockwise) {
                tap_code(KC_EQL);
            } else {
                tap_code(KC_MINS);
            }
            unregister_code(KC_LCTL);
        }
    }
    return false;  // don't allow keyboard/core level encoder code to run.
}

#endif

/*
enum raw_hid_commands {
    WRITE=1,
    PIXEL=2,
    SCROLL=3,
    BRIGHTNESS=4,
    QUERY=5,
    LAYER=6,
    EXIT=7,
    CLEAR=8,
};
*/
#if RAW_ENABLE

enum action {
    SET=1,
    GET=2,
};

enum status_report {
    CONNECT=0,
    DISCONNECT=1
};

enum feature {
    OLED=0x1,
};
enum oled_driver {
    ALL=0x1,
};
enum action_requests {
    OLED_ACTION=1,
    RGB_ACTION=2,
    LAYER_ACTION=3,
    EXIT_ACTION=4,
};

enum oled_args {
    BRIGHTNESS=1,
    STATE=2,
    LINES=3,
    CHARS=4,
    WRITE=5,
    READ=6,
};

// void clear_send_buffer(void) {
//     memset(send_data, 0, sizeof(send_data));
// }

// void oled_set_action(uint8_t arg, uint8_t value) {
//     switch ( arg ) {
//         case BRIGHTNESS:
//             oled_set_brightness(value);
//             break;
//         case STATE:
//             if ( value == 0 ) {
//                 oled_off();
//             } else {
//                 oled_on();
//             }
//             break;
//         case WRITE:
//             break;
//         default:
//             break;
//     }
// }

// void oled_get_action(uint8_t *data) {
//     sub_action = data[2];
//     switch ( sub_action ) {
//         case BRIGHTNESS:
//             send_data[0] = oled_get_brightness();
//             break;
//         case STATE:
//             if ( is_oled_on() ) {
//                 send_data[0] = 1;
//             } else {
//                 send_data[0] = 0;
//             }
//             break;
//         case LINES:
//             send_data[0] = oled_max_lines();
//             break;
//         case CHARS:
//             send_data[0] = oled_max_chars();
//             break;
//         default:
//             memset(send_data, 255, sizeof(send_data));
//             break;
//     }
//     raw_hid_send(send_data, sizeof(send_data));
// }

// void set_action(uint8_t action, uint8_t arg, uint8_t value) {
//     switch ( action ) {
//         case OLED_ACTION:
//             oled_set_action(arg, value);
//             break;
//         case RGB_ACTION:
//             break;
//         case LAYER_ACTION:
//             activate_layer(arg);
//             break;
//         case EXIT_ACTION:
//             is_hid_connected = false;
//             break;
//     }
// }

// void get_action(uint8_t *data) {
//     clear_send_buffer();
//     action = data[1];

//     switch ( action ) {
//         case OLED_ACTION:
//             oled_get_action(data);
//             break;
//         case RGB_ACTION:
//             break;
//         case LAYER_ACTION:
//             send_data[0] = get_highest_layer(default_layer_state) ;
//             raw_hid_send(send_data, sizeof(send_data));
//             break;
//         case EXIT_ACTION:
//             is_hid_connected = false;
//             break;
//         default:
//             break;
//     }
// }

// void oled_set_action(uint8_t *data) {
//     sub_action = data[2];
//     value = data[3];
//     uint8_t string[5];  // buffer to write to oled
//     switch ( sub_action ) {
//         case BRIGHTNESS:
//             oled_set_brightness(value);
//             break;
//         case STATE:
//             if ( value ) {
//                 oled_on();
//             } else {
//                 oled_off();
//             }
//             break;
//         case WRITE:
//             memcpy(string, &data[4], sizeof(string));
//             oled_set_cursor(0, value);
//             oled_write((char*)string, false);
//             break;
//         default:
//             break;
//     }
// }

// void set_action(uint8_t *data) {
//     action = data[1];
//     sub_action = data[2];
//     switch ( action ) {
//         case OLED_ACTION:
//             oled_set_action(data);
//             break;
//         case RGB_ACTION:
//             break;
//         case LAYER_ACTION:
//             activate_layer(sub_action);
//             break;
//         case EXIT_ACTION:
//             is_hid_connected = false;
//             break;
//     }
// }

// void raw_hid_receive(uint8_t *data, uint8_t length) {
//     is_hid_connected = true;
//     request = data[0]; // set or get

//     switch( request ) {
//         case SET_REQ:
//             set_action(data);
//             break;
//         case GET_REQ:
//             get_action(data);
//             break;
//         default:
//             break;
//     }
// }
void oled_set_report(uint8_t *data, uint8_t *response) {
    switch (data[2]) {
    }
}

void oled_get_report(uint8_t *data, uint8_t *response) {
    switch (data[2]) {
        case ALL:
            response[0] = OLED_DISPLAY_WIDTH;
            response[1] = OLED_DISPLAY_HEIGHT;
            // response[2] = OLED_MATRIX_SIZE;
            // response[3] = OLED_BLOCK_TYPE;
            response[4] = OLED_BLOCK_COUNT;
            // response[5] = OLED_BLOCK_SIZE;
            break;
    }
}

void oled_report(uint8_t *data, uint8_t *response) {
    switch (data[1]) {
        case SET:
            oled_set_report(data, response);
            break;
        case GET:
            oled_get_report(data, response);
            break;
    }
}


void raw_hid_receive(uint8_t *data, uint8_t length) {
    uint8_t response[length];
    memset(response, 0, length);

    switch (data[0]) {
        case OLED:
            oled_report(data, response);
            break;
    }
    raw_hid_send(response, length);
}
#endif  // RAW_ENABLE
 