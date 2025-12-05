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
// Standard C libraries
#include <math.h>
#include <stdint.h>
#include <string.h>

// QMK libraries
#include QMK_KEYBOARD_H
#include "action.h"
#include "action_layer.h"
#include "action_util.h"
#include "keycodes.h"
#include "oled_driver.h"
#include "quantum_keycodes.h"
#include "raw_hid.h"
#include "transactions.h"

// User defined libraries
#include "layer_names.h"
#include "socd_handler.h"


// Other definitions
#if defined(RAW_ENABLE)
#define RAW_EPSIZE 32
bool is_hid_connected = false;
#endif  // defined(RAW_ENABLE)


// Encoder action tracking structures
typedef struct {
    uint16_t keycode;
    uint32_t timestamp;
    char display_text[6];  // Reduced from 8 to 6 (5 chars + null terminator)
    bool is_active;
} encoder_action_t;

// Brightness tracking
typedef struct {
    uint8_t level;
    uint32_t timestamp;
    bool is_active;
    bool is_oled_brightness;  // true for OLED, false for system
} brightness_info_t;

static encoder_action_t last_encoder_action[2] = {
    {KC_NO, 0, "----", false},
    {KC_NO, 0, "----", false}
};

static brightness_info_t brightness_display = {0, 0, false, false};

#define ENCODER_DISPLAY_TIMEOUT 3000  // Show for 3 seconds

// Data structure for syncing encoder and brightness data
// Note: All encoders are processed on master side in split keyboards,
// so we need to sync encoder actions to slave for display purposes
typedef struct {
    encoder_action_t encoder_actions[2];
    brightness_info_t brightness_info;
} encoder_sync_data_t;

// enum layer_names {
//     MACOS,
//     PC,
//     GAME,
//     VLRNT,
// #if defined(TRI_LAYER_ENABLE)
//     LOWER = 29,
//     UPPER = 30,
//     ADJUST = 31,
// #endif
// };

// static enum socd_cleaner_resolution current_resolution = SOCD_CLEANER_OFF;
// enum custom_keycodes {
//     SOCD_CYCL = SAFE_RANGE,
// };
// Screensaver
// static uint32_t last_input = 0;  // timestamp of last key press
// #define SCREENSAVER_TIMEOUT 1000  // 60 seconds

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 * MacOS
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │  `   │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │   6  │   7  │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ ESC  │   Q  │   W  │   E  │   R  │   T  │ │   │       │   │  │   Y  │   U  │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ Tab  │   A  │   S  │   D  │   F  │   G  ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ MUTE  │    │  F20  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │LShift│   Z  │   X  │   C  │   V  │   B  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │UPPER │ LCTL │ LOPT │ LCMD │ Space │    │ Enter │LOWER │ RCMD │ ROPT │ PB_1 │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/

[MACOS] = LAYOUT(
    KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,                             KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,
    KC_ESC,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,                             KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_BSPC,
    KC_TAB,   KC_A,     KC_S,     KC_D,     KC_F,     KC_G,                             KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,
    KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_MUTE,      KC_F20,   KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,
                        TL_UPPR,  KC_LCTL,  KC_LOPT,  KC_LCMD,  KC_SPC,       KC_ENT,   TL_LOWR,  KC_RCMD,  KC_ROPT,  PB_1
),

/*
 * PC
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │  `   │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │   6  │   7  │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ ESC  │   Q  │   W  │   E  │   R  │   T  │ │   │       │   │  │   Y  │   U  │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ Tab  │   A  │   S  │   D  │   F  │   G  ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ MUTE  │    │  MPLY ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │LShift│   Z  │   X  │   C  │   V  │   B  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │UPPER │ LALT │ LGUI │ LCTL │ Space │    │ Enter │LOWER │ RGUI │ RALT │ RCTL │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/
[PC] = LAYOUT(
    KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,                             KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,
    KC_ESC,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,                             KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_BSPC,
    KC_TAB,   KC_A,     KC_S,     KC_D,     KC_F,     KC_G,                             KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,
    KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_MUTE,      KC_MPLY,  KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,
                        TL_UPPR,  KC_LALT,  KC_LGUI,  KC_LCTL,  KC_SPC,       KC_ENT,   TL_LOWR,  KC_RGUI,  KC_RALT,  KC_RCTL
),

/*
 * GAME
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │ ESC  │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │   6  │SOCDT │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │   M  │   T  │   Q  │   W  │   E  │   R  │ │   │       │   │  │   Y  │SOCDC │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ TAB  │   G  │   A  │   S  │   D  │   F  ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ MUTE  │    │  MPLY ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │LShift│   B  │   Z  │   X  │   C  │   V  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │      │ LALT │  +   │  =   │ Space │    │ Enter │ TRNS │UPPER │ RALT │ TRNS │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/
[GAME] = LAYOUT(
    KC_ESC,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,                             KC_6,     SOCDTOG,  KC_8,     KC_9,     KC_0,     KC_MINS,
    KC_M,     KC_T,     KC_Q,     KC_W,     KC_E,     KC_R,                             KC_Y,     SOCD_CYCL,KC_I,     KC_O,     KC_P,     KC_BSPC,
    KC_TAB,   KC_G,     KC_A,     KC_S,     KC_D,     KC_F,                             KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,
    KC_LSFT,  KC_B,     KC_Z,     KC_X,     KC_C,     KC_V,     KC_MUTE,      KC_MPLY,  KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,
                        XXXXXXX,  KC_LALT,  KC_PPLS,  KC_EQL,   KC_SPC,       KC_ENT,   _______,  TL_UPPR,  KC_RALT,  _______
),
/*
 * VLRNT
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │ ESC  │   T  │   1  │   2  │   3  │   4  │ │   │       │   │  │   6  │SOCDT │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ TAB  │ TAB  │   Q  │   W  │   E  │   R  │ │   │       │   │  │   Y  │SOCDC │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ CAPS │ CAPS │   A  │   S  │   D  │   F  ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ MUTE  │    │  MPLY ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ LCTL │LShift│   Z  │   X  │   C  │   B  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │      │ LALT │  G   │ LCTL │ Space │    │ Enter │ TRNS │UPPER │ RALT │ TRNS │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/
[VLRNT] = LAYOUT(
    KC_ESC,   KC_T,     KC_1,     KC_2,     KC_3,     KC_4,                             KC_6,     SOCDTOG,  KC_8,     KC_9,     KC_0,     KC_MINS,
    KC_TAB,   KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,                             KC_Y,     SOCD_CYCL,KC_I,     KC_O,     KC_P,     KC_BSPC,
    KC_CAPS,  KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,                             KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,
    KC_LCTL,  KC_LSFT,  KC_Z,     KC_X,     KC_C,     KC_B,     KC_MUTE,      KC_MPLY,  KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,
                        _______,  KC_LALT,  KC_G,     KC_LCTL,  KC_SPC,       KC_ENT,   _______,  TL_UPPR,  KC_RALT,  _______
),
/*
 * TRI_LAYER_LOWER_LAYER
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │      │  F1  │  F2  │  F3  │  F4  │  F5  │ │   │       │   │  │  F6  │  F7  │  F8  │  F9  │ F10  │      │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │  `   │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │ LEFT │ DOWN │  UP  │RIGHT │      │ F12  │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ TRNS │  =   │  -   │  (   │  )   │  +   ├───────╮    ╭───────┤  :   │  [   │  ]   │  _   │  %   │  |   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ TRNS  │    │  TRNS ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ TRNS │  !   │  @   │  #   │  {   │  }   ├───────┤    ├───────┤  ^   │  &   │  *   │  $   │  \   │ TRNS │
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │ TRNS │ TRNS │ TRNS │ TRNS │ TRNS  │    │ TRNS  │ TRNS │ TRNS │ TRNS │ TRNS │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/
#if defined(TRI_LAYER_ENABLE)
[LOWER] = LAYOUT(
    XXXXXXX,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,                            KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   XXXXXXX,
    KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,                             KC_LEFT,  KC_DOWN,  KC_UP,    KC_RGHT,  XXXXXXX,  KC_F12,
    _______,  KC_EQL,   KC_MINS,  KC_LPRN,  KC_RPRN,  KC_PLUS,                          KC_COLN,  KC_LBRC,  KC_RBRC,  KC_UNDS,  KC_PERC,  KC_PIPE,
    _______,  KC_EXLM,  KC_AT,    KC_HASH,  KC_LCBR,  KC_RCBR,  _______,      _______,  KC_CIRC,  KC_AMPR,  KC_ASTR,  KC_DLR,   KC_BSLS,  _______,
                        _______,  _______,  _______,  _______,  _______,      _______,  _______,  _______,  _______,  _______
),
/*
 * TRI_LAYER_UPPER_LAYER
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │      │      │      │      │      │      │ │   │       │   │  │      │      │      │      │      │      │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │      │      │      │      │      │      │ │   │       │   │  │      │      │      │      │      │      │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │      │      │      │      │      │ CAPS ├───────╮    ╭───────┤ LEFT │  UP  │ DOWN │RIGHT │      │      │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │      │      │      │      │      │      ├───────┤    ├───────┤      │      │      │      │      │      │
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │ TRNS │      │      │      │       │    │       │ TRNS │      │      │ TRNS │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/
[UPPER] = LAYOUT(
    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,                          XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,                          XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  KC_CAPS,                          KC_LEFT,  KC_UP,    KC_DOWN,  KC_RIGHT, XXXXXXX,  XXXXXXX,
    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,      XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
              _______,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,      _______,  XXXXXXX,  XXXXXXX,  _______
),
/*
 * TRI_LAYER_ADJUST_LAYER
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │ BOOT │MACOS │  PC  │ GAME │VLRNT │      │ │   │       │   │  │      │      │      │      │      │ BOOT │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │      │      │      │      │      │      │ │   │       │   │  │      │      │      │      │      │      │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │      │      │      │RM_PRV│RM_TGL│RM_NXT├───────╮    ╭───────┤      │      │      │      │      │      │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │      │      │      │      │      │      ├───────┤    ├───────┤      │      │      │      │      │      │
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │ TRNS │      │      │      │       │    │       │ TRNS │      │DF(0) │QK_RBT│
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/
[ADJUST] = LAYOUT(
    QK_BOOT,  DF(MACOS),DF(PC),   DF(GAME), DF(VLRNT),XXXXXXX,                          XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  QK_BOOT,
    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,                          XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
    XXXXXXX,  XXXXXXX,  XXXXXXX,  RM_PREV,  RM_TOGG,  RM_NEXT,                          XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX , XXXXXXX,  XXXXXXX,  XXXXXXX,      XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
                        _______,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,      XXXXXXX,  _______,  XXXXXXX,  DF(0),    QK_RBT
),
#endif // defined(TRI_LAYER_ENABLE)
};

layer_state_t layer_state_set_user(layer_state_t state) {
    return socd_layer_state_set(state);
}

// SOCD Cleaner : https://getreuer.info/posts/keyboards/socd-cleaner/
// socd_cleaner_t socd_opposing_pairs[] = {
//     {{KC_A, KC_D}, SOCD_CLEANER_LAST},
// };

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!socd_process_record(keycode, record)) {
        return false;
    }
    // Any other process_record_user logic here
    return true;
};


#if defined(COMBO_ENABLE)
const uint16_t PROGMEM combo_corner_quantum_boot[] = {
    KC_LCMD, KC_LSFT, KC_GRV, KC_5, COMBO_END
};

combo_t key_combos[] = {
    COMBO(combo_corner_quantum_boot, QK_BOOT)
};
#endif // defined(COMBO_ENABLE)


#if defined(OLED_ENABLE)

// Screensaver
// #define NUM_DOTS 64   // number of dots on the screen

// typedef struct {
//     uint8_t x;
//     uint8_t y;
//     int8_t vx;
//     int8_t vy;
// } dot_t;

// static dot_t dots[NUM_DOTS];
// static bool dots_initialized = false;

// void init_dots(void) {
//     const uint8_t w = OLED_DISPLAY_HEIGHT;
//     const uint8_t h = OLED_DISPLAY_WIDTH;

//     for (uint8_t i = 0; i < NUM_DOTS; i++) {
//         dots[i].x = rand() % w;
//         dots[i].y = rand() % h;

//         // Random velocity -1 or 1
//         dots[i].vx = (rand() % 2) ? 1 : -1;
//         dots[i].vy = (rand() % 2) ? 1 : -1;
//     }

//     dots_initialized = true;
// }

// void crazy_bouncing_dots(void) {
//     const uint8_t w = OLED_DISPLAY_HEIGHT;
//     const uint8_t h = OLED_DISPLAY_WIDTH;

//     if (!dots_initialized) init_dots();

//     oled_clear();

//     for (uint8_t i = 0; i < NUM_DOTS; i++) {
//         dot_t *d = &dots[i];

//         // Draw dot
//         if (d->x < w && d->y < h)
//             oled_write_pixel(d->x, d->y, true);

//         // Move dot
//         d->x += d->vx;
//         d->y += d->vy;

//         // Bounce on edges
//         if (d->x == 0 || d->x >= w - 1) d->vx = -d->vx;
//         if (d->y == 0 || d->y >= h - 1) d->vy = -d->vy;
//     }
// }

// #define MAX_OFFSET 6       // max pixels moved
// #define TARGET_DOTS 108    // total dots
// #define WAVE_WIDTH 5        // number of rows affected per wavefront
// #define OVERSHOOT 1.0f      // fraction of MAX_OFFSET to overshoot past original

// static float ripple_phase = -WAVE_WIDTH;
// static float ripple_speed = 0.6f;
// static bool ripple_initialized = false;

// void init_ripple(void) {
//     ripple_phase = -WAVE_WIDTH;  // negative so wave enters gradually
//     ripple_initialized = true;
// }

// void screensaver_ripple(void) {
//     const uint8_t w = OLED_DISPLAY_HEIGHT; // vertical OLED width
//     const uint8_t h = OLED_DISPLAY_WIDTH;  // vertical OLED height

//     // Compute near-square grid
//     uint8_t grid_cols = sqrt(TARGET_DOTS * w / (float)h);
//     if (grid_cols == 0) grid_cols = 1;
//     uint8_t grid_rows = (TARGET_DOTS + grid_cols - 1) / grid_cols;

//     float spacing_x = w / (float)grid_cols;
//     float spacing_y = h / (float)grid_rows;
//     float spacing = (spacing_x < spacing_y ? spacing_x : spacing_y);

//     if (!ripple_initialized) init_ripple();

//     oled_clear();

//     for (uint8_t row = 0; row < grid_rows; row++) {
//         for (uint8_t col = 0; col < grid_cols; col++) {
//             int x = col * spacing + spacing / 2;
//             int y = row * spacing + spacing / 2;

//             float offset = 0.0f;
//             float phase_diff = row - ripple_phase;

//             if (phase_diff >= 0 && phase_diff <= WAVE_WIDTH) {
//                 float t = (phase_diff / WAVE_WIDTH) * 3.14159265f;
//                 offset = MAX_OFFSET * sinf(t) * (1.0f + OVERSHOOT * sinf(t));
//                 offset -= MAX_OFFSET * OVERSHOOT * sinf(t) * sinf(t); // subtle rebound
//             }

//             int draw_y = y + (int)offset;
//             if (draw_y < h && x < w) oled_write_pixel(x, draw_y, true);
//         }
//     }


//     ripple_phase += ripple_speed;

//     // Reset phase off-screen for smooth entry
//     if (ripple_phase > grid_rows + WAVE_WIDTH) ripple_phase = -WAVE_WIDTH;

//     // // slow the ripple down
//     // float t = ripple_phase / grid_rows; // normalized 0..1 across visible grid

//     // float speed_factor = 1.0f;  // default normal speed

//     // if (ripple_phase >= 0 && ripple_phase <= grid_rows) {
//     //     speed_factor = cosf(t * (3.14159 / 2)); // slows down only while on-screen
//     // }

//     // ripple_phase += ripple_speed * speed_factor;

//     // // Reset to start just above top so it enters smoothly
//     // if (ripple_phase > grid_rows + WAVE_WIDTH) ripple_phase = -WAVE_WIDTH;



// }


// // -------------------- Screensaver action --------------------
// void (*current_screensaver)(void) = screensaver_ripple;
// void screensaver_update(void) {
//     current_screensaver();
// }
// end of screensaver

enum brightness {
    DOWN,
    UP,
};

void set_brightness(uint8_t delta) {
    uint8_t max_brightness = 0x80;
    uint8_t current_brightness = oled_get_brightness();
    uint8_t level = 0;
    uint8_t level_shift = 0x10;
    switch ( delta ) {
        case UP:
            level = current_brightness + level_shift;
            if ( current_brightness > level || level > max_brightness) {
                level = max_brightness;
            }
            break;
        case DOWN:
            if (current_brightness == 0xff && level_shift != 0x1) {
                level = current_brightness - level_shift + 1;
            } else {
                level = current_brightness - level_shift;
            }
            if ( current_brightness < level ) {
                level = 0;
            }
            break;
        default:
            break;
    }
    oled_set_brightness(level);

    // Update brightness display info
    brightness_display.level = level;
    brightness_display.timestamp = timer_read32();
    brightness_display.is_active = true;
    brightness_display.is_oled_brightness = true;

    // Immediately sync brightness to slave side if we're master
    if (is_keyboard_master()) {
        encoder_sync_data_t sync_data = {
            .encoder_actions = {last_encoder_action[0], last_encoder_action[1]},
            .brightness_info = brightness_display
        };
        transaction_rpc_send(ENCODER_SYNC, sizeof(sync_data), &sync_data);
    }
}

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
    // Current Layer - check both layer_state and default_layer_state
    uint8_t layer = get_highest_layer(layer_state | default_layer_state);
    switch (layer) {
        case MACOS:
            oled_write_P(PSTR("macos"), false);
            break;
        case PC:
            oled_write_P(PSTR("pc   "), false);
            break;
        case GAME:
            oled_write_P(PSTR("game "), false);
            break;
        case VLRNT:
            oled_write_P(PSTR("val  "), false);
            break;
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
            oled_write_ln_P(PSTR("undef"), false);
    }
}

void oled_render_encoder_status(void) {
    uint32_t current_time = timer_read32();

    // Each side shows only its own encoder
    uint8_t encoder_index = is_keyboard_master() ? 0 : 1;

    // Position cursor at second to last line
    uint8_t max_lines = oled_max_lines();
    uint8_t target_line = max_lines >= 2 ? max_lines - 2 : 0;
    oled_set_cursor(0, target_line);

    // Display this side's encoder action
    if (last_encoder_action[encoder_index].is_active &&
        (current_time - last_encoder_action[encoder_index].timestamp < ENCODER_DISPLAY_TIMEOUT)) {
        oled_write(last_encoder_action[encoder_index].display_text, false);
    } else {
        oled_write_P(PSTR("     "), false);  // Clear with 5 spaces
        last_encoder_action[encoder_index].is_active = false;
    }

    // Show brightness level temporarily if active on the last line
    if (brightness_display.is_active &&
        (current_time - brightness_display.timestamp < ENCODER_DISPLAY_TIMEOUT)) {
        // Position cursor at last line
        oled_set_cursor(0, max_lines - 1);

        if (brightness_display.is_oled_brightness) {
            // Show exact OLED brightness level, pad to 5 chars
            char brightness_str[6];
            snprintf(brightness_str, sizeof(brightness_str), "%-5d", brightness_display.level);
            oled_write(brightness_str, false);
        } else {
            // System brightness - show indicator, pad to 5 chars
            oled_write_P(PSTR("SYS  "), false);
        }
    } else {
        // Clear brightness line when not active
        if (brightness_display.is_active == false) {
            oled_set_cursor(0, max_lines - 1);
            oled_write_P(PSTR("     "), false);
        }
        brightness_display.is_active = false;
    }
}


void oled_master(void) {
    oled_current_layer();
    oled_write_P(PSTR("\n"), false);

    // Only show SOCD state when on GAME layer
    uint8_t current_layer = get_highest_layer(layer_state | default_layer_state);
    if ((current_layer == GAME) || (current_layer == VLRNT)) {
        oled_current_socd_state();
    } else {
        oled_write_P(PSTR("     "), false);  // Clear with 5 spaces
    }

    oled_write_P(PSTR("\n"), false);

    oled_render_encoder_status();
}

void oled_slave(void) {
    // Show encoder status on slave side too
    oled_current_layer();
    oled_write_P(PSTR("\n"), false);
    oled_render_encoder_status();
    // screensaver_update();
}

bool oled_task_user(void) {
    // uint32_t idle = timer_elapsed32(last_input);

    // if (idle > SCREENSAVER_TIMEOUT) {
    //     screensaver_update();   // Run screensaver when idle
    //     return false;           // Skip normal OLED drawing
    // }

    if (is_keyboard_master()) {
        oled_master();  // Renders master
    } else {
        oled_slave();  // Renders slave
    }
    return false;
}

void oled_render_boot(bool bootloader) {
    oled_clear();
    oled_set_cursor(0, 0);
    if (bootloader) {
        oled_write_P(PSTR("fw update"), true);
    } else {
        oled_write_P(PSTR("rebooting"), true);
    }

    oled_render_dirty(true);
}

#endif // defined(OLED_ENABLE)

#if defined(ENCODER_ENABLE)
uint8_t mod_state;

void update_encoder_display(uint8_t index, uint16_t keycode, bool clockwise) {
    last_encoder_action[index].keycode = keycode;
    last_encoder_action[index].timestamp = timer_read32();
    last_encoder_action[index].is_active = true;

    // Handle brightness actions specially
    if (keycode == KC_BRMU || keycode == KC_BRMD) {
        // For system brightness, we can't track the actual level
        brightness_display.level = 0;  // Unknown level
        brightness_display.timestamp = timer_read32();
        brightness_display.is_active = true;
        brightness_display.is_oled_brightness = false;
    }

    // Map keycodes to display strings (5 chars max)
    switch (keycode) {
        case KC_VOLU: strcpy(last_encoder_action[index].display_text, "VOL+ "); break;
        case KC_VOLD: strcpy(last_encoder_action[index].display_text, "VOL- "); break;
        case KC_BRMU: strcpy(last_encoder_action[index].display_text, "BRT+ "); break;
        case KC_BRMD: strcpy(last_encoder_action[index].display_text, "BRT- "); break;
        case KC_F16:  strcpy(last_encoder_action[index].display_text, "CONT "); break;
        case KC_F11:  strcpy(last_encoder_action[index].display_text, "STEP "); break;
        case KC_F17:  strcpy(last_encoder_action[index].display_text, "OVER "); break;
        case KC_RIGHT: strcpy(last_encoder_action[index].display_text, "WRD> "); break;
        case KC_LEFT:  strcpy(last_encoder_action[index].display_text, "<WRD "); break;
        case KC_NO:   // Special case for OLED brightness
            strcpy(last_encoder_action[index].display_text, clockwise ? "OLD+ " : "OLD- ");
            break;
        default:
            snprintf(last_encoder_action[index].display_text, sizeof(last_encoder_action[index].display_text), "%s%d  ",
                    clockwise ? "CW" : "CC", index);
            break;
    }

    // Sync encoder actions to slave for display
    // Note: In split keyboards, all encoder processing happens on master,
    // so we must sync to slave for the slave OLED to display encoder 1 actions
    if (is_keyboard_master()) {
        encoder_sync_data_t sync_data = {
            .encoder_actions = {last_encoder_action[0], last_encoder_action[1]},
            .brightness_info = brightness_display
        };
        transaction_rpc_send(ENCODER_SYNC, sizeof(sync_data), &sync_data);
    }
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    mod_state = get_mods();
    clear_mods();

    if (index == 0) { /* First encoder - Master side */
        switch (get_highest_layer(layer_state)) {
            case MACOS:
                if (mod_state & MOD_MASK_GUI) { /* Command */
                    // Change Brightness of Main Monitor
                    if (clockwise) {
                        tap_code(KC_BRMU);
                        update_encoder_display(index, KC_BRMU, clockwise);
                    } else {
                        tap_code(KC_BRMD);
                        update_encoder_display(index, KC_BRMD, clockwise);
                    }
                } else if (mod_state & MOD_MASK_CTRL) { /* Control */
                    // // Change Brightness of Extended Monitor
                    // register_code(KC_LCTL);
                    // if (clockwise) {
                    //     tap_code(KC_BRMU);
                    // } else {
                    //     tap_code(KC_BRMD);
                    // }
                    // unregister_code(KC_LCTL);
                } else {
                    // Change Volume
                    if (clockwise) {
                        tap_code(KC_VOLU);
                        update_encoder_display(index, KC_VOLU, clockwise);
                    } else {
                        tap_code(KC_VOLD);
                        update_encoder_display(index, KC_VOLD, clockwise);
                    }
                }
                break;
            case LOWER:
                #if defined(OLED_ENABLE)
                    // Change OLED Brightness
                    if (clockwise) {
                        set_brightness(UP);
                        update_encoder_display(index, KC_NO, clockwise);
                    } else {
                        set_brightness(DOWN);
                        update_encoder_display(index, KC_NO, clockwise);
                    }
                    break;
                #endif
            case UPPER:
            case ADJUST:
            default:
                break;
        }
    } else if (index == 1) { /* Second encoder - Slave side */
        switch (get_highest_layer(layer_state)) {
            case MACOS:
                if (mod_state & MOD_MASK_GUI) { /* Command */
                    if (clockwise) {
                        tap_code(KC_F16);  /* VSCode Debug : Continue */
                        update_encoder_display(index, KC_F16, clockwise);
                    } else {
                        tap_code(KC_F11);  /* VSCode Debug : Step Out */
                        update_encoder_display(index, KC_F11, clockwise);
                    }
                } else if (mod_state & MOD_MASK_ALT) { /* Option */
                    register_code(KC_LOPT);
                    if (clockwise) {
                        tap_code(KC_RIGHT);  /* Next Word*/
                        update_encoder_display(index, KC_RIGHT, clockwise);
                    } else {
                        tap_code(KC_LEFT);  /* Previous Word */
                        update_encoder_display(index, KC_LEFT, clockwise);
                    }
                    unregister_code(KC_LOPT);
                } else {
                    if (clockwise) {
                        tap_code(KC_F17);  /* VSCode Debug : Step Over */
                        update_encoder_display(index, KC_F17, clockwise);
                    } else {
                        tap_code(KC_F11);  /* VSCode Debug : Step Into */
                        update_encoder_display(index, KC_F11, clockwise);
                    }
                }
                break;
            case LOWER:
                break;
            case UPPER:
                #if defined(OLED_ENABLE)
                    // Change OLED Brightness
                    if (clockwise) {
                        set_brightness(UP);
                        update_encoder_display(index, KC_NO, clockwise);
                    } else {
                        set_brightness(DOWN);
                        update_encoder_display(index, KC_NO, clockwise);
                    }
                    break;
                #endif
            case ADJUST:
            default:
                break;
        }
    }
    set_mods(mod_state);
    return false;
}

#endif // defined(ENCODER_ENABLE)

// Custom data sync handler for encoder actions and brightness
void encoder_sync_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    const encoder_sync_data_t *sync_data = (const encoder_sync_data_t*)in_data;

    // Update slave side encoder actions for display
    memcpy(last_encoder_action, sync_data->encoder_actions, sizeof(last_encoder_action));

    // Check if brightness changed and apply it to slave OLED
    if (sync_data->brightness_info.is_oled_brightness &&
        sync_data->brightness_info.level != brightness_display.level) {
        oled_set_brightness(sync_data->brightness_info.level);
    }

    // Update local brightness display state
    brightness_display = sync_data->brightness_info;
}


void keyboard_post_init_user(void) {
    transaction_register_rpc(ENCODER_SYNC, encoder_sync_slave_handler);
}

void housekeeping_task_user(void) {
    if (is_keyboard_master()) {
        // Track last synced state to avoid redundant updates
        static encoder_action_t last_synced_encoders[2] = {{KC_NO, 0, "----", false}, {KC_NO, 0, "----", false}};
        static brightness_info_t last_synced_brightness = {0, 0, false, false};
        static uint32_t last_sync = 0;

        // Check if encoder actions or brightness changed
        bool encoder_changed = (memcmp(last_encoder_action, last_synced_encoders, sizeof(last_encoder_action)) != 0);
        bool brightness_changed = (brightness_display.level != last_synced_brightness.level ||
                                   brightness_display.is_active != last_synced_brightness.is_active ||
                                   brightness_display.is_oled_brightness != last_synced_brightness.is_oled_brightness);

        // Sync if anything changed, or fallback sync every 1 second
        if (encoder_changed || brightness_changed || timer_elapsed32(last_sync) > 1000) {
            encoder_sync_data_t sync_data = {
                .encoder_actions = {last_encoder_action[0], last_encoder_action[1]},
                .brightness_info = brightness_display
            };

            if (transaction_rpc_send(ENCODER_SYNC, sizeof(sync_data), &sync_data)) {
                memcpy(last_synced_encoders, last_encoder_action, sizeof(last_encoder_action));
                last_synced_brightness = brightness_display;
                last_sync = timer_read32();
            }
        }
    }
}

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



// Shutdown User : https://docs.qmk.fm/custom_quantum_functions#keyboard-shutdown-reboot-code
bool shutdown_user(bool jump_to_bootloader) {
    oled_render_boot(jump_to_bootloader);
    // false to not process kb level
    return false;
}


#if defined(RAW_ENABLE)

enum sub_auto_layers {
    id_layer_state_set = 1,
    id_layer_clear = 2,
    id_layer_move = 3,
    id_layer_on = 4,
    id_layer_off = 5,
    id_get_highest_layer = 6,
    id_ping = 7,
};

enum report_ids {
    auto_layers = 1,
};

void raw_hid_receive(uint8_t *data, uint8_t length) {
    uint8_t *report_id = &(data[0]);
    uint8_t *sub_id = &(data[1]);
    uint8_t *command_data = &(data[2]);

    switch (*report_id) {
        // auto_layers
        case auto_layers: {
            switch (*sub_id) {
                // layer_state_set
                case id_layer_state_set: {
                    // uint8_t layer_mask
                    layer_state_set(command_data[0]);
                    break;
                }
                // layer_clear
                case id_layer_clear: {
                    layer_clear();
                    break;
                }
                // layer_move
                case id_layer_move: {
                    // uint8_t layer
                    layer_move(command_data[0]);
                    break;
                }
                // layer_on
                case id_layer_on: {
                    // uint8_t layer
                    layer_on(command_data[0]);
                    break;
                }
                // layer_off
                case id_layer_off: {
                    // uint8_t layer
                    layer_off(command_data[0]);
                    break;
                }
                // get_highest_layer
                case id_get_highest_layer: {
                    command_data[0] = get_highest_layer(layer_state);
                    break;
                }
                // ping
                case id_ping: {
                    // uint8_t ping
                    break;
                }
            }
            break;
        }
        default: {
            command_data[0] = SPLIT_KEYBOARD;
            command_data[1] = OLED_ENABLE;
            break;
        }
    }
    raw_hid_send(data, length);
}
#endif // defined(RAW_ENABLE)
