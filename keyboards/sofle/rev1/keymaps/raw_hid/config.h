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
#pragma once

/* The way how "handedness" is decided (which half is which),
see https://docs.qmk.fm/#/feature_split_keyboard?id=setting-handedness
for more options.
*/

// #define RGB_DI_PIN D3

#if defined(RGB_MATRIX_ENABLE) || defined(RGBLIGHT_ENABLE)
#undef RGBLIGHT_LED_COUNT
#define RGBLIGHT_LED_COUNT 71 // when soldering i destroyed the pad of the right side indicator. which is why this is 71. changes also made in rev 1

#undef RGB_MATRIX_SPLIT
#define RGB_MATRIX_SPLIT {36,35}
#undef RGBLED_SPLIT
#define RGBLED_SPLIT RGB_MATRIX_SPLIT
#define RGBLIGHT_LIMIT_VAL 120
#endif
#ifdef RGB_MATRIX_ENABLE
// #define DRIVER_LED_TOTAL RGBLIGHT_LED_COUNT
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 150 // limits maximum brightness of LEDs to 150 out of 255. Higher may cause the controller to crash.
#define RGB_MATRIX_HUE_STEP 8
#define RGB_MATRIX_SAT_STEP 8
#define RGB_MATRIX_VAL_STEP 8
#define RGB_MATRIX_SPD_STEP 10
#define RGB_MATRIX_KEYPRESSES
//#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#define SPLIT_TRANSPORT_MIRROR
#define ENABLE_RGB_MATRIX_SOLID_MULTISPLASH
#define ENABLE_RGB_MATRIX_GRADIENT_UP_DOWN
#define ENABLE_RGB_MATRIX_SOLID_REACTIVE_SIMPLE
#define ENABLE_RGB_MATRIX_CYCLE_OUT_IN
#endif

#ifdef RGBLIGHT_ENABLE
//   #define RGBLIGHT_SLEEP  /* If defined, the RGB lighting will be switched off when the host goes to sleep */
// /*== all animations enable ==*/
#define RGBLIGHT_ANIMATIONS
// /*== or choose animations ==*/
//   #define RGBLIGHT_EFFECT_BREATHING
   //#define RGBLIGHT_EFFECT_RAINBOW_MOOD
   //#define RGBLIGHT_EFFECT_RAINBOW_SWIRL
//   #define RGBLIGHT_EFFECT_SNAKE
//   #define RGBLIGHT_EFFECT_KNIGHT
//   #define RGBLIGHT_EFFECT_CHRISTMAS
//   #define RGBLIGHT_EFFECT_STATIC_GRADIENT
//   #define RGBLIGHT_EFFECT_RGB_TEST
//   #define RGBLIGHT_EFFECT_ALTERNATING
#endif

#define MEDIA_KEY_DELAY 2

#define USB_POLLING_INTERVAL_MS 1
#define QMK_KEYS_PER_SCAN 12

// potential space saving undefs
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE

// this limits the number of layers the keyboard can use to 8. save some space.
#define LAYER_STATE_16BIT

// oled definitions
#define SPLIT_OLED_ENABLE
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_TRANSPORT_MIRROR
#define SPLIT_LED_STATE_ENABLE
#define SPLIT_MODS_ENABLE

#define OLED_BRIGHTNESS 128
#define OLED_FADE_OUT
#define OLED_TIMEOUT 10000

// Custom Logo
#define OLED_FONT_H "font.c"
#define ENCODER_RESOLUTIONS { 4 }
#define ENCODER_RESOLUTIONS_RIGHT { 4 }