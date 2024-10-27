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

#include QMK_KEYBOARD_H
#include <stdio.h>
#ifdef RAW_ENABLE
    #include "raw_hid.h"
    #include <string.h>
    #define RAW_EPSIZE 32
    
    bool is_hid_connected = false; // is pc connected yet?
    uint8_t send_data[RAW_EPSIZE] = {0};  // buffer for raw_hid_send
    uint8_t request;  // SET, GET
    uint8_t action;  // OLED, RGB, LAYER
    uint8_t sub_action;  // sub section or layer
    uint8_t value;  // brightness value or line number
#endif

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
    KC_SUPRG
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
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            | LGUI | LAlt | LCTR |LOWER | /Space  /       \Enter \  |RAISE | RCTR | RAlt | RGUI |
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
 * MAC
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  `   |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  `   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | ESC  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            | LGUI | LAlt | LCTR |LOWER | /Space  /       \Enter \  |RAISE | RCTR | RAlt | RGUI |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */

[_MAC] = LAYOUT(
  KC_GRV,   KC_1,   KC_2,    KC_3,    KC_4,    KC_5,                     KC_6,    KC_7,    KC_8,    KC_9,    KC_0,  KC_MINS,
  KC_ESC,   KC_Q,   KC_W,    KC_E,    KC_R,    KC_T,                     KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,  KC_BSPC,
  KC_TAB,   KC_A,   KC_S,    KC_D,    KC_F,    KC_G,                     KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN,  KC_QUOT,
  KC_LSFT,  KC_Z,   KC_X,    KC_C,    KC_V,    KC_B, KC_MUTE,    KC_MPLY,KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH,  KC_RSFT,
               _______,KC_LCTL, KC_LOPT, KC_LCMD , KC_SPC,      KC_ENT,  _______, KC_RGUI, KC_ROPT, KC_RCTL
),
/*
 * VALORANT
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | ESC  |   T  |   1  |   2  |   3  |   4  |                    |   6  |   7  |   8  |   9  |   0  |  `   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | TAB  | TAB  |   Q  |   W  |   E  |   R  |                    |   Y  |   U  |   I  |   O  |   P  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | CAPS | CAPS |   A  |   S  |   D  |   F  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * | LCTR |LShift|   Z  |   X  |   C  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            | LCTR | LAlt |   G  | CTRL | /Space  /       \Enter \  |RAISE | RCTR | RAlt | RGUI |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */

[_VALORANT] = LAYOUT(
  KC_ESC  ,     KC_T,    KC_1,    KC_2,    KC_3,    KC_4,                     KC_6,    KC_7,    KC_8,    KC_9,    KC_0,  KC_MINS,
  KC_TAB  ,   KC_TAB,    KC_Q,    KC_W,    KC_E,    KC_R,                     KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,  KC_BSPC,
  KC_CAPS ,  KC_CAPS,    KC_A,    KC_S,    KC_D,    KC_F,                     KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN,  KC_QUOT,
  KC_LCTL ,  KC_LSFT,    KC_Z,    KC_X,    KC_C,    KC_B, KC_MUTE,    KC_MPLY,KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH,  KC_RSFT,
                _______,KC_LALT, KC_G,   KC_LCTL,    KC_SPC,           KC_ENT,  _______, KC_RGUI, KC_RALT, KC_RCTL
),
/*
 * APEX LEGENDS
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | ESC  |   1  |   2  |      |   3  |   4  |                    |   6  |   7  |   8  |   9  |   0  |  `   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |   M  | TAB  |   Q  |   W  |   E  |   R  |                    |   Y  |   U  |   I  |   O  |   P  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | CAPS |   G  |   A  |   S  |   D  |   F  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * | LCTR |LShift|   Z  |   X  |   C  |   V  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            |RAISE | LAlt |   B  |      | /Space  /       \   B  \  |LOWER | RCTR | RAlt | RGUI |
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
 * | ESC  |   1  |   2  |      |   3  |   4  |                    |   6  |   7  |   8  |   9  |   0  |  `   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |   M  | TAB  |   Q  |   W  |   E  |   R  |                    |   Y  |   U  |   I  |   O  |   P  | Bspc |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | CAPS |   G  |   A  |   S  |   D  |   F  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * | LCTR |LShift|   Z  |   X  |   C  |   V  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            |RAISE | LAlt |      |      | /Space  /       \   B  \  |LOWER | RCTR | RAlt | RGUI |
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
 * | Tab  |   !  |   @  |   #  |   $  |   %  |-------.    ,-------|   ^  |   &  |   *  |   (  |   )  |   |  |
 * |------+------+------+------+------+------|  MUTE |    |       |------+------+------+------+------+------|
 * | Shift|  =   |  -   |  +   |   {  |   }  |-------|    |-------|   [  |   ]  |   ;  |   :  |   \  | Shift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *            | LGUI | LAlt | LCTR |LOWER | /Enter  /       \Space \  |RAISE | RCTR | RAlt | RGUI |
 *            |      |      |      |      |/       /         \      \ |      |      |      |      |
 *            `----------------------------------'           '------''---------------------------'
 */
[_LOWER] = LAYOUT(
  _______,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                       KC_F6,   KC_F7,   KC_F8,   KC_F9,  KC_F10,  KC_F11,
  KC_GRV ,    KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                       KC_LEFT,  KC_DOWN, KC_UP,   KC_RGHT, KC_0,    KC_F12,
  _______, KC_EXLM,   KC_AT, KC_HASH,  KC_DLR, KC_PERC,                       KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_PIPE,
  _______,  KC_EQL, KC_MINS, KC_PLUS, KC_LCBR, KC_RCBR, _______,       _______, KC_LBRC, KC_RBRC, KC_SCLN, KC_COLN, KC_BSLS, _______,
                       _______, _______, _______, _______, _______,       _______, _______, _______, _______, _______
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
  XXXXXXX , XXXXXXX,DF(_VALORANT) ,DF(_GAME) , DF(_APEX), XXXXXXX,            XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  QK_RBT  , XXXXXXX, DF(_QWERTY) , DF(_MAC) , XXXXXXX, XXXXXXX,                     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  XXXXXXX , XXXXXXX,  XXXXXXX   , XXXXXXX , XXXXXXX, XXXXXXX,                     XXXXXXX, KC_VOLD, KC_MUTE, KC_VOLU, XXXXXXX, XXXXXXX,
  XXXXXXX , XXXXXXX,  XXXXXXX   , XXXXXXX , XXXXXXX, XXXXXXX, XXXXXXX,     XXXXXXX, XXXXXXX, KC_MPRV, KC_MEDIA_PLAY_PAUSE, KC_MNXT, XXXXXXX, XXXXXXX,
                   _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______
  ),
};

#if defined(RAW_ENABLE) || defined(OS_DETECTION_ENABLE)
void activate_layer(uint8_t layer) {
    set_single_persistent_default_layer(layer);
}
#endif

#ifdef OLED_ENABLE
static void render_logo(void) {
    static const char PROGMEM raw_logo[] = {
        0,  0,  0,252,252,252,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,252,252,252,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,128,128,128,192,192,192,192,224,224,224,224,224,224, 96, 96, 96, 96,240,240,112,112,112,112, 96, 96,224,224,192,128,128,  0,  0,  0,  0,  0,
        0,  0,  0,255,255,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,255,255,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,192,224, 96, 48, 48, 56, 24, 28, 28, 60, 46,102,198,135,  3,  3,  3,  1,  1,  3,  3,  3,  7,  5, 12, 24, 48,224,128,  0,  0,  0,  0,  0,  0,  0,  1,  1,  3,  6, 28,112,  0,  0,  0,  1,  3,  7,255,254,248,  0,  0, 
        0,  0,  0,255,255,255,255,  7,  7,  7,  7,  7,255,255,255,  0,255,255,255,  7,  7,  7,  7,  7,  7,  7,  0,255,255,255,199,199,199,199,199,199,255,255,255,  0,199,199,199,199,199,199,199,199,199,255,255,255,  0,255,255,255,  7,  7,  7,  7,  7,255,255,255,255,  0,  0,  0,  0,  0,248,252,254, 15,  3,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  3,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,128,192,240,255, 63, 15,  0,  0, 
        0,  0,  0, 63, 63, 63, 63, 56, 56, 56, 56, 56, 63, 63, 63,  0, 63, 63, 63,  0,  0,  0,  0,  0,  0,  0,  0, 63, 63, 63, 57, 57, 57, 57, 57, 57, 57, 57, 57,  0, 63, 63, 63, 57, 57, 57, 57, 57, 63, 63, 63, 63,  0, 63, 63, 63, 56, 56, 56, 56, 56, 63, 63, 63, 63,  0,  0,  0,  0,  0,  3, 15, 15, 30, 28, 28, 60, 60, 56, 56,120,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,120, 56, 56, 56, 56, 60, 60, 60, 28, 30, 30, 14, 15, 15,  7,  7,  3,  1,  0,  0,  0,  0,  0,
    };
    oled_write_raw_P(raw_logo, sizeof(raw_logo));
}

#ifdef RGB_MATRIX_ENABLE        
void render_rgb_status(void) {
    oled_write_ln("RGB:", false);
    static char temp[20] = {0};
    snprintf(temp, sizeof(temp) + 1, "M:%3dH:%3dS:%3dV:%3d", rgb_matrix_config.mode, rgb_matrix_config.hsv.h, rgb_matrix_config.hsv.s, rgb_matrix_config.hsv.v);
    oled_write(temp, false);
}
#endif


#if OS_DETECTION_ENABLE
bool process_detected_host_os_kb(os_variant_t detected_os) {
    if (!process_detected_host_os_user(detected_os)) {
        return false;
    }
    switch (detected_os) {
        case OS_MACOS:
            oled_write_P(PSTR("macos"), false);
            break;
        case OS_IOS:
            oled_write_P(PSTR("ios  "), false);
            break;
        case OS_WINDOWS:
            oled_write_P(PSTR("win  "), false);
            break;
        case OS_LINUX:
            oled_write_P(PSTR("linux"), false);
            break;
        case OS_UNSURE:
            oled_write_P(PSTR("unsre"), false);
            break;
    }
    
    return true;
}
#endif  // OS_DETECTION_ENABLE

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
    #if OS_DETECTION_ENABLE
    process_detected_host_os_kb(detected_host_os() );
    #endif  // OS_DETECTION_ENABLE
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
    }
    return true;
}

#ifdef ENCODER_ENABLE

bool encoder_update_user(uint8_t index, bool clockwise) {
    uint8_t temp_mod = get_mods();
    uint8_t temp_osm = get_oneshot_mods();
    bool    is_ctrl  = (temp_mod | temp_osm) & MOD_MASK_CTRL;
    bool    is_shift = (temp_mod | temp_osm) & MOD_MASK_SHIFT;

    if (is_shift) {

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

    } else if (is_ctrl) {

        // #ifdef RGB_MATRIX_ENABLE

        if (index == 0) { /* First encoder */
            if (clockwise) {
                oled_set_brightness(oled_get_brightness() + 0x10);
            } else {
                oled_set_brightness(oled_get_brightness() - 0x10);
            }}
        // } else if (index == 1) { /* Second encoder */
        //     if (clockwise) {
        //         rgb_matrix_increase_speed();
        //     } else {
        //         rgb_matrix_decrease_speed();
        //     }
        // }

        // #endif

    } else {
        if (index == 1) { /* First encoder */
            if (clockwise) {
                tap_code(KC_PGUP);
                // tap_code(KC_MS_WH_UP);
            } else {
                tap_code(KC_PGDN);
                // tap_code(KC_MS_WH_DOWN);
            }
        } else if (index == 0) { /* Second encoder */
            uint16_t mapped_code        = 0;
            if (clockwise) {
                mapped_code = KC_VOLU;
            } else {
                mapped_code = KC_VOLD;
            }
            tap_code_delay(mapped_code, MEDIA_KEY_DELAY);
        }
    }
    return true;
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

enum hid_requests {
    SET=1,
    GET=2,
};

enum status_requests {
    CONNECT=0,
    DISCONNECT=1
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

void clear_send_buffer(void) {
    memset(send_data, 0, sizeof(send_data));
}

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

void oled_get_action(uint8_t *data) {
    sub_action = data[2];
    switch ( sub_action ) {
        case BRIGHTNESS:
            send_data[0] = oled_get_brightness();
            break;
        case STATE:
            if ( is_oled_on() ) {
                send_data[0] = 1;
            } else {
                send_data[0] = 0;
            }
            break;
        case LINES:
            send_data[0] = oled_max_lines();
            break;
        case CHARS:
            send_data[0] = oled_max_chars();
            break;
        default:
            memset(send_data, 255, sizeof(send_data));
            break;
    }
    raw_hid_send(send_data, sizeof(send_data));
}

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

void get_action(uint8_t *data) {
    clear_send_buffer();
    action = data[1];

    switch ( action ) {
        case OLED_ACTION:
            oled_get_action(data);
            break;
        case RGB_ACTION:
            break;
        case LAYER_ACTION:
            send_data[0] = get_highest_layer(default_layer_state) ;
            raw_hid_send(send_data, sizeof(send_data));
            break;
        case EXIT_ACTION:
            is_hid_connected = false;
            break;
        default:
            break;
    }
}

void oled_set_action(uint8_t *data) {
    sub_action = data[2];
    value = data[3];
    uint8_t string[5];  // buffer to write to oled
    switch ( sub_action ) {
        case BRIGHTNESS:
            oled_set_brightness(value);
            break;
        case STATE:
            if ( value ) {
                oled_on();
            } else {
                oled_off();
            }
            break;
        case WRITE:
            memcpy(string, &data[4], sizeof(string));
            oled_set_cursor(0, value);
            oled_write((char*)string, false);
            break;
        default:
            break;
    }
}

void set_action(uint8_t *data) {
    action = data[1];
    sub_action = data[2];
    switch ( action ) {
        case OLED_ACTION:
            oled_set_action(data);
            break;
        case RGB_ACTION:
            break;
        case LAYER_ACTION:
            activate_layer(sub_action);
            break;
        case EXIT_ACTION:
            is_hid_connected = false;
            break;
    }
}

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

void set(uint8_t *data, uint8_t *response) {
    switch (data[1]) {
        case CONNECT:
            is_hid_connected = true;
            response[0] = 0x3;
            break;
        case DISCONNECT:
            is_hid_connected = false;
            response[0] = 0x4;
            break;
    }
}

void raw_hid_receive(uint8_t *data, uint8_t length) {
    uint8_t response[length];
    memset(response, 0, length);

    switch (data[0]) {
        case SET:
            set(data, response);
            break;
        case GET:
            // get(data, response);
            break;
    }
    raw_hid_send(response, length);
}
#endif  // RAW_ENABLE
 