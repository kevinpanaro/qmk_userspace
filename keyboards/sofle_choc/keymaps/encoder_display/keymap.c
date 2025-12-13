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
#include <stdint.h>
#include "action.h"
#include "action_layer.h"
#include "action_util.h"
#include "keycodes.h"
#include "oled_driver.h"
#include "os_detection.h"
#include "quantum_keycodes.h"
#include "socd_cleaner.h"

#include "print.h"

#if defined(RAW_ENABLE)
#include "raw_hid.h"
// #include <string.h>
#define RAW_EPSIZE 32
bool is_hid_connected = false;
#endif  // defined(RAW_ENABLE)
#include QMK_KEYBOARD_H
#include "transactions.h"
#include <string.h>

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

enum layer_names {
    /* _M_XYZ = Mac Os, _W_XYZ = Win/Linux */
    MACOS,
    WIN,
    GAME,
    VLRNT,
#if defined(TRI_LAYER_ENABLE)
    LOWER = 29,
    UPPER = 30,  // 4
    ADJUST = 31, // 5
#endif
};

static enum socd_cleaner_resolution current_resolution = SOCD_CLEANER_OFF;
enum custom_keycodes {
    SOCD_CYCL = SAFE_RANGE,
    KC_OS,
};


#if defined(OS_DETECTION_ENABLE)
// Track detected OS
os_variant_t current_os = OS_UNSURE;
bool os_detection_complete = false;

// Helper to get the layer for a detected OS
uint8_t get_os_layer(os_variant_t os) {
    switch (os) {
        case OS_MACOS:
            return MACOS;
        case OS_IOS:
            return MACOS;
        case OS_WINDOWS:
            return WIN;
        case OS_LINUX:
        //     return LINUX;
        case OS_UNSURE:
        default:
            return WIN; // default to UNSURE
    }
}

static const char os_names[][8] = {
    [OS_UNSURE]  = "?",
    [OS_LINUX]   = "linux",
    [OS_WINDOWS] = "win",
    [OS_MACOS]   = "mac",
    [OS_IOS]     = "ios",
};


// Check if current base layer matches detected OS
bool is_on_correct_os_layer(void) {
    if (!os_detection_complete) return true; // No warning if OS not detected yet
    uint8_t base_layer = get_highest_layer(default_layer_state & 0x7); // Only check first 2 layers (base OS layers)
    return (base_layer == get_os_layer(current_os));
}

bool process_detected_host_os_user(os_variant_t detected_os) {
    current_os = detected_os;
    os_detection_complete = true;
    set_single_default_layer(get_os_layer(current_os));

    switch (detected_os) {
        case OS_MACOS:
            break;
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
 * WIN
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
[WIN] = LAYOUT(
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
 * │ BOOT │MACOS │  WIN │ GAME │VLRNT │      │ │   │       │   │  │      │      │      │      │      │ BOOT │
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
    QK_BOOT,  DF(MACOS),DF(WIN),   DF(GAME), DF(VLRNT),XXXXXXX,                          XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  DB_TOGG,  QK_BOOT,
    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,                          XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
    XXXXXXX,  XXXXXXX,  XXXXXXX,  RM_PREV,  RM_TOGG,  RM_NEXT,                          XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
    XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX , XXXXXXX,  XXXXXXX,  XXXXXXX,      XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
                        _______,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,      XXXXXXX,  _______,  XXXXXXX,  KC_OS,    QK_RBT
),
#endif // defined(TRI_LAYER_ENABLE)
};

layer_state_t layer_state_set_user(layer_state_t state) {
    socd_cleaner_enabled = IS_LAYER_ON_STATE(state, GAME);
    #ifdef CONSOLE_ENABLE
        dprintf("Layer: 0x%02X\n", get_highest_layer(layer_state | default_layer_state));
    #endif
    return state;
}

socd_cleaner_t socd_opposing_pairs[] = {
    // {{KC_W, KC_S}, SOCD_CLEANER_LAST},
    {{KC_A, KC_D}, SOCD_CLEANER_NEUTRAL},
};


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    #ifdef CONSOLE_ENABLE
        // dprintf("KL: kc: 0x%04X, col: %2u, row: %2u, pressed: %u, time: %5u, int: %u, count: %u\n", keycode, record->event.key.col, record->event.key.row, record->event.pressed, record->event.time, record->tap.interrupted, record->tap.count);
    #endif
    switch (keycode) {
        case SOCD_CYCL:
            if (record->event.pressed) {
                current_resolution++;
                if (current_resolution == SOCD_CLEANER_NUM_RESOLUTIONS) {
                    current_resolution = SOCD_CLEANER_OFF;
                }
                socd_opposing_pairs[0].resolution = current_resolution;
            }
        break;
    #if defined(OS_DETECTION_ENABLE)
        case KC_OS:
            if (record->event.pressed) {
                if (os_detection_complete) {
                    set_single_default_layer(get_os_layer(current_os));
                }
            }
            // return false;
    #endif // defined(OS_DETECTION_ENABLE)
        break;
    }
    return true;
};

#if defined(OLED_ENABLE)

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
#endif

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

// #if defined(OS_DETECTION_ENABLE)
// bool process_detected_host_os_user(os_variant_t detected_os) {
//     switch (detected_os) {
//         case OS_MACOS:
//         case OS_IOS:
//             break;
//         case OS_WINDOWS:
//             break;
//         case OS_LINUX:
//             break;
//         case OS_UNSURE:
//             break;
//     }
//     return true;
// }
// #endif // defined(OS_DETECTION_ENABLE)

#if defined(COMBO_ENABLE)
const uint16_t PROGMEM combo_corner_quantum_boot[] = {
    KC_LCMD, KC_LSFT, KC_GRV, KC_5, COMBO_END
};

combo_t key_combos[] = {
    COMBO(combo_corner_quantum_boot, QK_BOOT)
};
#endif // defined(COMBO_ENABLE)

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
    // Current Layer - check both layer_state and default_layer_state
    uint8_t layer = get_highest_layer(layer_state | default_layer_state);
    switch (layer) {
        case MACOS:
            oled_write_P(PSTR("macos"), !is_on_correct_os_layer());
            break;
        case WIN:
            oled_write_P(PSTR("pc   "), !is_on_correct_os_layer());
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
}

void render_debug(void) {
    oled_clear();
    oled_write_ln("debug", false);
    oled_write_ln(os_names[current_os], false);
}

bool oled_task_user(void) {
    if (debug_config.enable) {
        render_debug();
        return false;
    }

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
