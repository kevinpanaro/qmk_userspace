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
    uint8_t app_id;      // mirror of current_app, synced to the slave half
    uint8_t host_flags;  // mirror of host_flags
    uint8_t mod_state;   // mirror of held modifiers, for the slave's knob hint
} encoder_sync_data_t;

enum layer_names {
    /* _M_XYZ = Mac Os, _W_XYZ = Win/Linux */
    MACOS,
    WIN,
    GAME,
    VLRNT,
    WDOGS,
    WDOGS_NUM,
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

// ---- Host app-context state (pushed from the host over Raw HID) ----
// The focused macOS app, resolved by the host daemon. Order MUST match the
// daemon's AppContext enum.
enum app_context {
    APP_DEFAULT = 0,
    APP_VSCODE,
    APP_BROWSER,
    APP_TERMINAL,
    APP_MEDIA,
    APP_SLACK,
    APP_FIGMA,
    APP_PDF,
    APP_MAIL,
    APP_COUNT,
};

// Host signal flags packed into the HID state report's flags byte.
#define HOST_FLAG_DEBUGGING  0x01
#define HOST_FLAG_DND        0x02
#define HOST_FLAG_FULLSCREEN 0x04

static uint8_t current_app = APP_DEFAULT;
static uint8_t host_flags  = 0;

// Held modiiers, mirrored to the slave so its OLED can render a modifier-aware
// knob hint (split-common only syncs layer state, not mods). Master refreshes
// this from get_mods(); the slave receives it via the ENCODER_SYNC packet.
static uint8_t display_mods = 0;

// What a single encoder turn resolves to: a keycode to tap (or OLED-brightness
// adjust) plus the 5-char OLED label. Produced by resolve_knob() so the keypress
// and the on-screen hint can never disagree.
typedef struct {
    uint16_t keycode;       // tapped via tap_code16; KC_NO = nothing to tap
    char     label[6];      // 5 visible chars + NUL
    bool     oled_brightness;  // true: adjust OLED brightness instead of tapping
} knob_action_t;


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
 * in game action w/ LEANING
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │ MENU │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │   6  │   7  │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ MAP  │ PPT  │ LEANL│   W  │ LEANR│ RLOAD│ │   │       │   │  │   Y  │   U  │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │SCRBD │ GNADE│   A  │   S  │   D  │INTRCT├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ MUTE  │    │  MPLY ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │SPRINT│ WALK │PRONE │HLSTER│CROUCH│MELEE ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │      │ MOUNT│FIREMD│      │ JUMP  │    │ Enter │ TRNS │UPPER │ RALT │ TRNS │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/
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
 * WDOGS
 * ╭──────┬──────┬──────┬──────┬──────┬──────╮ ╭───╮       ╭───╮  ╭──────┬──────┬──────┬──────┬──────┬──────╮
 * │ ESC  │   1  │   2  │   3  │   4  │   5  │ │   │       │   │  │   6  │SOCDT │   8  │   9  │   0  │  -   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ │   │       │   │  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │   ~  │   T  │   Q  │   W  │   E  │   R  │ │   │       │   │  │   Y  │SOCDC │   I  │   O  │   P  │ Bspc │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ ╰───╯       ╰───╯  ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │ TAB  │   G  │   A  │   S  │   D  │   F  ├───────╮    ╭───────┤   H  │   J  │   K  │   L  │   ;  │  '   │
 * ├──────┼──────┼──────┼──────┼──────┼──────┤ MUTE  │    │  MPLY ├──────┼──────┼──────┼──────┼──────┼──────┤
 * │LShift│   B  │   Z  │   X  │   C  │   V  ├───────┤    ├───────┤   N  │   M  │   ,  │   .  │   /  │RShift│
 * ╰──────┴──────┼──────┼──────┼──────┼──────┤       │    │       ├──────┼──────┼──────┼──────┼──────┴──────╯
 *               │      │ LALT │  +   │  =   │ Space │    │ Enter │ TRNS │UPPER │ RALT │ TRNS │
 *               ╰──────┴──────┴──────┴──────┤       │    │       ├──────┴──────┴──────┴──────╯
 *                                           ╰───────╯    ╰───────╯
*/
[WDOGS] = LAYOUT(
    KC_ESC,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,                             KC_6,     SOCDTOG,  KC_8,     KC_9,     KC_0,     KC_MINS,
    KC_GRV,   KC_T,     KC_Q,     KC_W,     KC_E,     KC_R,                             KC_Y,     SOCD_CYCL,KC_I,     KC_O,     KC_P,     KC_BSPC,
    KC_TAB,   KC_G,     KC_A,     KC_S,     KC_D,     KC_F,                             KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,
    KC_LSFT,  KC_B,     KC_Z,     KC_X,     KC_C,     KC_V,     KC_MUTE,      KC_MPLY,  KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_RSFT,
                        MO(WDOGS_NUM),KC_LALT,KC_PPLS,KC_EQL,   KC_SPC,       KC_ENT,   _______,  TL_UPPR,  KC_RALT,  _______
),
/*
 * WDOGS_NUM (held from the key left of LALT on WDOGS)
 * A -> 9, D -> 0
*/
[WDOGS_NUM] = LAYOUT(
    _______,  _______,  _______,  _______,  _______,  _______,                          _______,  _______,  _______,  _______,  _______,  _______,
    _______,  _______,  _______,  _______,  _______,  _______,                          _______,  _______,  _______,  _______,  _______,  _______,
    _______,  _______,  KC_9,     _______,  KC_0,     _______,                          _______,  _______,  _______,  _______,  _______,  _______,
    _______,  _______,  _______,  _______,  _______,  _______,  _______,      _______,  _______,  _______,  _______,  _______,  _______,  _______,
                        _______,  _______,  _______,  _______,  _______,      _______,  _______,  _______,  _______,  _______
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
 * │ BOOT │MACOS │  WIN │ GAME │VLRNT │ WDOGS│ │   │       │   │  │      │      │      │      │      │ BOOT │
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
    QK_BOOT,  DF(MACOS),DF(WIN),   DF(GAME), DF(VLRNT),DF(WDOGS),                          XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  DB_TOGG,  QK_BOOT,
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
        // Right-knob click (KC_F20 on the MACOS layer) is app-aware: F20
        // ("Add to Watch") in VSCode, Reply (Cmd+R) in Mail, and inert
        // elsewhere so the click can't fire stray F20s in other apps.
        case KC_F20:
            if (record->event.pressed) {
                if (current_app == APP_VSCODE) {
                    tap_code(KC_F20);
                } else if (current_app == APP_MAIL) {
                    tap_code16(LGUI(KC_R));
                }
            }
            return false;
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
            .brightness_info = brightness_display,
            .app_id = current_app,
            .host_flags = host_flags,
            .mod_state = display_mods
        };
        transaction_rpc_send(ENCODER_SYNC, sizeof(sync_data), &sync_data);
    }
}
#endif

#if defined(ENCODER_ENABLE)
uint8_t mod_state;

// Record a resolved knob action for the OLED (flash) and sync it to the slave.
void update_encoder_display(uint8_t index, knob_action_t action) {
    last_encoder_action[index].keycode = action.keycode;
    last_encoder_action[index].timestamp = timer_read32();
    last_encoder_action[index].is_active = true;

    // System (monitor) brightness has no readable level; flag it for the slave.
    if (action.keycode == KC_BRMU || action.keycode == KC_BRMD) {
        brightness_display.level = 0;  // Unknown level
        brightness_display.timestamp = timer_read32();
        brightness_display.is_active = true;
        brightness_display.is_oled_brightness = false;
    }

    strncpy(last_encoder_action[index].display_text, action.label,
            sizeof(last_encoder_action[index].display_text));
    last_encoder_action[index].display_text[sizeof(last_encoder_action[index].display_text) - 1] = '\0';

    // Sync encoder actions to slave for display.
    // Note: In split keyboards, all encoder processing happens on master,
    // so we must sync to slave for the slave OLED to display encoder 1 actions.
    if (is_keyboard_master()) {
        encoder_sync_data_t sync_data = {
            .encoder_actions = {last_encoder_action[0], last_encoder_action[1]},
            .brightness_info = brightness_display,
            .app_id = current_app,
            .host_flags = host_flags,
            .mod_state = display_mods
        };
        transaction_rpc_send(ENCODER_SYNC, sizeof(sync_data), &sync_data);
    }
}

// Build a knob action that taps a keycode.
static knob_action_t knob_tap(uint16_t keycode, const char *label) {
    knob_action_t a = {keycode, "     ", false};
    strncpy(a.label, label, sizeof(a.label));
    a.label[sizeof(a.label) - 1] = '\0';
    return a;
}

// Build a knob action that adjusts the OLED brightness instead of tapping.
static knob_action_t knob_oled(const char *label) {
    knob_action_t a = knob_tap(KC_NO, label);
    a.oled_brightness = true;
    return a;
}

// Single source of truth for what each encoder turn does, given the active
// layer, focused host app and held modifiers. Pure: no side effects, so the
// OLED can call it to preview the CW/CCW actions before a turn happens.
//   index 0 = left/master encoder, index 1 = right/slave encoder.
static knob_action_t resolve_knob(uint8_t layer, uint8_t app, uint8_t mods,
                                  uint8_t index, bool clockwise) {
    const knob_action_t none = {KC_NO, "     ", false};
    bool cmd = mods & MOD_MASK_GUI;
    bool opt = mods & MOD_MASK_ALT;
    bool sft = mods & MOD_MASK_SHIFT;

    /* Left / master encoder */
    if (index == 0) {
        switch (layer) {
            case MACOS:
                if (cmd) return knob_tap(clockwise ? KC_BRMU : KC_BRMD, clockwise ? "BRT+ " : "BRT- ");
                return knob_tap(clockwise ? KC_VOLU : KC_VOLD, clockwise ? "VOL+ " : "VOL- ");
            case LOWER:
                return knob_oled(clockwise ? "OLD+ " : "OLD- ");
            case WDOGS:
            case WDOGS_NUM:
                return knob_tap(clockwise ? KC_PGUP: KC_PGDN, clockwise ? "PGUP ": "PGDN ");
            default:
                return none;
        }
    }

    /* Right / slave encoder */
    switch (layer) {
        case MACOS:
            // Option alone (no Shift/Command) is a global word-nav modifier,
            // except in Media where it scrubs the track (handled below).
            if (opt && !cmd && !sft && app != APP_MEDIA) {
                return knob_tap(clockwise ? A(KC_RIGHT) : A(KC_LEFT), clockwise ? "WRD> " : "<WRD ");
            }
            switch (app) {
                case APP_VSCODE:
                    if (sft && opt) return knob_tap(clockwise ? A(KC_DOWN) : A(KC_UP), clockwise ? "LN v " : "LN ^ ");  // move line down/up
                    if (cmd) return knob_tap(clockwise ? KC_F5 : S(KC_F11), clockwise ? "CONT " : "OUT  ");
                    return knob_tap(clockwise ? KC_F10 : KC_F11, clockwise ? "OVER " : "INTO ");
                case APP_BROWSER:
                    if (cmd) return knob_tap(clockwise ? LGUI(KC_RBRC) : LGUI(KC_LBRC), clockwise ? "FWD> " : "<BCK ");
                    return knob_tap(clockwise ? C(KC_TAB) : C(S(KC_TAB)), clockwise ? "TAB> " : "<TAB ");
                case APP_TERMINAL:
                    if (cmd) return knob_tap(clockwise ? LGUI(S(KC_RBRC)) : LGUI(S(KC_LBRC)), clockwise ? "TAB> " : "<TAB ");
                    return knob_tap(clockwise ? LGUI(KC_DOWN) : LGUI(KC_UP), clockwise ? "CMDv " : "CMD^ ");
                case APP_MEDIA:
                    if (cmd) return knob_tap(clockwise ? LGUI(KC_RIGHT) : LGUI(KC_LEFT), clockwise ? "NEXT " : "PREV ");
                    if (opt) return knob_tap(clockwise ? LGUI(S(KC_RIGHT)) : LGUI(S(KC_LEFT)), clockwise ? "SEEK>" : "<SEEK");
                    return knob_tap(clockwise ? LGUI(KC_UP) : LGUI(KC_DOWN), clockwise ? "VOL+ " : "VOL- ");
                case APP_SLACK:
                    if (cmd) return knob_tap(clockwise ? A(S(KC_DOWN)) : A(S(KC_UP)), clockwise ? "UNR v" : "UNR ^");
                    return knob_tap(clockwise ? A(KC_DOWN) : A(KC_UP), clockwise ? "CH v " : "CH ^ ");
                case APP_FIGMA:
                    if (cmd) return knob_tap(clockwise ? LGUI(S(KC_Z)) : LGUI(KC_Z), clockwise ? "REDO " : "UNDO ");
                    return knob_tap(clockwise ? LGUI(KC_EQL) : LGUI(KC_MINS), clockwise ? "ZOM+ " : "ZOM- ");
                case APP_PDF:
                    if (cmd) return knob_tap(clockwise ? LGUI(KC_EQL) : LGUI(KC_MINS), clockwise ? "ZOM+ " : "ZOM- ");
                    return knob_tap(clockwise ? KC_PGDN : KC_PGUP, clockwise ? "PG v " : "PG ^ ");
                case APP_MAIL:
                    if (cmd) return knob_tap(clockwise ? C(G(KC_A)) : S(G(KC_U)), clockwise ? "ARCH " : "UNRD ");  // archive / mark unread
                    return knob_tap(clockwise ? KC_DOWN : KC_UP, clockwise ? "DOWN " : "UP   ");  // next / prev message
                case APP_DEFAULT:
                default:
                    if (cmd) return knob_tap(clockwise ? KC_BRMU : KC_BRMD, clockwise ? "BRT+ " : "BRT- ");
                    return knob_tap(clockwise ? KC_VOLU : KC_VOLD, clockwise ? "VOL+ " : "VOL- ");
            }
        case UPPER:
            return knob_oled(clockwise ? "OLD+ " : "OLD- ");
        default:
            return none;
    }
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    mod_state = get_mods();
    display_mods = mod_state;
    clear_mods();

    knob_action_t action = resolve_knob(get_highest_layer(layer_state | default_layer_state), current_app,
                                        mod_state, index, clockwise);
    if (action.oled_brightness) {
        #if defined(OLED_ENABLE)
        set_brightness(clockwise ? UP : DOWN);
        update_encoder_display(index, action);
        #endif
    } else if (action.keycode != KC_NO) {
        tap_code16(action.keycode);
        update_encoder_display(index, action);
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

    // Mirror host-pushed app context / flags so the slave OLED + RGB match
    current_app = sync_data->app_id;
    host_flags  = sync_data->host_flags;
    display_mods = sync_data->mod_state;
}

void keyboard_post_init_user(void) {
    transaction_register_rpc(ENCODER_SYNC, encoder_sync_slave_handler);
}

void housekeeping_task_user(void) {
    if (is_keyboard_master()) {
        // Track last synced state to avoid redundant updates
        static encoder_action_t last_synced_encoders[2] = {{KC_NO, 0, "----", false}, {KC_NO, 0, "----", false}};
        static brightness_info_t last_synced_brightness = {0, 0, false, false};
        static uint8_t last_synced_app = 0xFF;
        static uint8_t last_synced_flags = 0xFF;
        static uint8_t last_synced_mods = 0xFF;
        static uint32_t last_sync = 0;

        // Keep the modifier mirror fresh so a held mod (with no turn) still
        // updates the slave's knob hint.
        display_mods = get_mods();

        // Expire the encoder/brightness displays on the master's own clock
        // (valid here), then let the change detection below sync the cleared
        // flag to the slave — the slave can't time these out itself.
        for (uint8_t i = 0; i < 2; i++) {
            if (last_encoder_action[i].is_active &&
                timer_elapsed32(last_encoder_action[i].timestamp) > ENCODER_DISPLAY_TIMEOUT) {
                last_encoder_action[i].is_active = false;
            }
        }
        if (brightness_display.is_active &&
            timer_elapsed32(brightness_display.timestamp) > ENCODER_DISPLAY_TIMEOUT) {
            brightness_display.is_active = false;
        }

        // Check if encoder actions or brightness changed
        bool encoder_changed = (memcmp(last_encoder_action, last_synced_encoders, sizeof(last_encoder_action)) != 0);
        bool brightness_changed = (brightness_display.level != last_synced_brightness.level ||
                                   brightness_display.is_active != last_synced_brightness.is_active ||
                                   brightness_display.is_oled_brightness != last_synced_brightness.is_oled_brightness);
        bool state_changed = (current_app != last_synced_app) || (host_flags != last_synced_flags) ||
                             (display_mods != last_synced_mods);

        // Sync if anything changed, or fallback sync every 1 second
        if (encoder_changed || brightness_changed || state_changed || timer_elapsed32(last_sync) > 1000) {
            encoder_sync_data_t sync_data = {
                .encoder_actions = {last_encoder_action[0], last_encoder_action[1]},
                .brightness_info = brightness_display,
                .app_id = current_app,
                .host_flags = host_flags,
                .mod_state = display_mods
            };

            if (transaction_rpc_send(ENCODER_SYNC, sizeof(sync_data), &sync_data)) {
                memcpy(last_synced_encoders, last_encoder_action, sizeof(last_encoder_action));
                last_synced_brightness = brightness_display;
                last_synced_app = current_app;
                last_synced_flags = host_flags;
                last_synced_mods = display_mods;
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
        case WDOGS:
        case WDOGS_NUM:
            oled_write_P(PSTR("wdogs "), false);
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
void oled_app_context(void) {
    switch (current_app) {
        case APP_VSCODE:   oled_write_P(PSTR("code "), false); break;
        case APP_BROWSER:  oled_write_P(PSTR("web  "), false); break;
        case APP_TERMINAL: oled_write_P(PSTR("term "), false); break;
        case APP_MEDIA:    oled_write_P(PSTR("media"), false); break;
        case APP_SLACK:    oled_write_P(PSTR("slack"), false); break;
        case APP_FIGMA:    oled_write_P(PSTR("figma"), false); break;
        case APP_PDF:      oled_write_P(PSTR("pdf  "), false); break;
        case APP_MAIL:     oled_write_P(PSTR("mail "), false); break;
        case APP_DEFAULT:
        default:           oled_write_P(PSTR("     "), false); break;
    }
    oled_write_P(PSTR("\n"), false);
    // Do-not-disturb indicator on its own line
    oled_write_P((host_flags & HOST_FLAG_DND) ? PSTR("dnd  ") : PSTR("     "), false);
    oled_write_P(PSTR("\n"), false);
}

void oled_render_encoder_status(void) {
    // Each side shows only its own encoder
    uint8_t encoder_index = is_keyboard_master() ? 0 : 1;

    // Position cursor at second to last line
    uint8_t max_lines = oled_max_lines();
    uint8_t target_line = max_lines >= 2 ? max_lines - 2 : 0;
    oled_set_cursor(0, target_line);

    // Display this side's encoder action. Expiry is owned by the master (see
    // housekeeping_task_user); the halves don't share a clock, so we render
    // purely on the synced is_active flag rather than comparing timestamps.
    if (last_encoder_action[encoder_index].is_active) {
        oled_write(last_encoder_action[encoder_index].display_text, false);
    } else {
        oled_write_P(PSTR("     "), false);  // Clear with 5 spaces
    }

    // Brightness line (expiry also owned by the master)
    oled_set_cursor(0, max_lines - 1);
    if (brightness_display.is_active) {
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
        oled_write_P(PSTR("     "), false);
    }
}

// Predictive hint: what a clockwise (top) vs counter-clockwise (bottom) turn of
// THIS side's encoder will do right now, given the layer, focused app and held
// modifiers. Resolved by the same resolve_knob() the encoder taps, so the hint
// always matches the keypress. Mods come from the live state on master and from
// the synced mirror (display_mods) on the slave.
void oled_render_knob_hint(void) {
#if defined(ENCODER_ENABLE)
    uint8_t encoder_index = is_keyboard_master() ? 0 : 1;
    uint8_t layer = get_highest_layer(layer_state | default_layer_state);
    uint8_t mods  = is_keyboard_master() ? get_mods() : display_mods;

    knob_action_t cw  = resolve_knob(layer, current_app, mods, encoder_index, true);
    knob_action_t ccw = resolve_knob(layer, current_app, mods, encoder_index, false);

    uint8_t max_lines = oled_max_lines();
    uint8_t base = max_lines >= 5 ? max_lines - 5 : 0;

    oled_set_cursor(0, base);
    oled_write(cw.label, false);   // clockwise / right turn
    oled_set_cursor(0, base + 1);
    oled_write(ccw.label, false);  // counter-clockwise / left turn
    oled_set_cursor(0, base + 2);
    oled_write_P(PSTR("     "), false);  // gap before the flash line
#endif
}

// Shared OLED layout so master and slave line up identically.
// (oled_render_encoder_status() already shows this side's own encoder.)
void oled_render_status(void) {
    oled_current_layer();
    oled_write_P(PSTR("\n"), false);

    // SOCD state line (blank unless on a game layer)
    uint8_t current_layer = get_highest_layer(layer_state | default_layer_state);
    if ((current_layer == GAME) || (current_layer == VLRNT)) {
        oled_current_socd_state();
    } else {
        oled_write_P(PSTR("     "), false);  // Clear with 5 spaces
    }
    oled_write_P(PSTR("\n"), false);

    oled_app_context();
    oled_render_knob_hint();
    oled_render_encoder_status();
}

void oled_master(void) {
    oled_render_status();
}

void oled_slave(void) {
    oled_render_status();
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

#if defined(RGB_MATRIX_ENABLE)
// Accent LED: the upper-left key of the LEFT half (matrix [0,0]). Only the left
// half physically drives this index, so the accent shows on one key there.
#define ACCENT_LED_INDEX 28

// Accent color for the focused app / host flags.
static void accent_color(uint8_t *r, uint8_t *g, uint8_t *b) {
    if (host_flags & HOST_FLAG_DEBUGGING) {
        // Pulsing amber (triangle wave) while a host debug session is active.
        uint8_t phase = (uint8_t)(timer_read32() >> 2);
        uint8_t v = phase < 128 ? phase : (uint8_t)(255 - phase);  // 0..127 ramp
        *r = (uint8_t)(0x40 * v / 127); *g = (uint8_t)(0x18 * v / 127); *b = 0x00;
        return;
    }
    switch (current_app) {
        case APP_VSCODE:   *r = 0x00; *g = 0x00; *b = 0x40; break;  // blue
        case APP_BROWSER:  *r = 0x00; *g = 0x40; *b = 0x00; break;  // green
        case APP_TERMINAL: *r = 0x30; *g = 0x30; *b = 0x30; break;  // white
        case APP_MEDIA:    *r = 0x40; *g = 0x00; *b = 0x40; break;  // magenta
        case APP_SLACK:    *r = 0x20; *g = 0x00; *b = 0x40; break;  // purple
        case APP_FIGMA:    *r = 0x40; *g = 0x14; *b = 0x00; break;  // orange
        case APP_PDF:      *r = 0x00; *g = 0x20; *b = 0x20; break;  // teal
        case APP_MAIL:     *r = 0x00; *g = 0x18; *b = 0x38; break;  // sky blue
        case APP_DEFAULT:
        default:           *r = 0x00; *g = 0x00; *b = 0x00; break;  // off
    }
}

// Show ONLY the accent key: blank the rest of the matrix so no effect shows
// underneath. (Matrix must still be enabled via RM_TOGG for indicators to run.)
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, 0x00, 0x00, 0x00);
    }
    if (ACCENT_LED_INDEX >= led_min && ACCENT_LED_INDEX < led_max) {
        uint8_t r, g, b;
        accent_color(&r, &g, &b);
        rgb_matrix_set_color(ACCENT_LED_INDEX, r, g, b);
    }
    return false;
}
#endif // defined(RGB_MATRIX_ENABLE)


#if defined(RAW_ENABLE)

// The host pushes a single idempotent state snapshot; the keyboard treats it
// as the full truth (no incremental commands, no drift).
//   data[0] = report id (HOST_STATE)
//   data[1] = app id     (enum app_context)
//   data[2] = host flags (HOST_FLAG_* bitfield)
//   data[3] = reserved
enum report_ids {
    HOST_STATE = 1,
};

void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (data[0] == HOST_STATE) {
        if (data[1] < APP_COUNT) {
            current_app = data[1];
        }
        host_flags = data[2];
    }
    raw_hid_send(data, length);  // echo back so the host can verify receipt
}
#endif // defined(RAW_ENABLE)
