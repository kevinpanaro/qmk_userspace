/* Copyright 2021 Kevin Panaro
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

#ifdef RAW_ENABLE
    #include "raw_hid.h"
    #include <string.h>
    #define RAW_EPSIZE 32
    uint8_t pixel_index = 0;
    bool pixel_state = true;
#endif

bool is_hid_connected = false; // is pc connected yet?
bool is_reset = false; // is the board being reset
bool hid_clear_screen = true;

// Defines names for use in layer keycodes and the keymap
// if you add a layer here, make sure to update the num_of_layers definition
enum layer_names {
    _BASE,
    _SPOTIFY,
    _DISCORD,
    _VALORANT,
    _COD,
    _HASS,
    _NUMPAD,
    num_of_layers,
};

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

enum brightness {
    DOWN,
    UP,
};

// Custom keycokes go here.
enum custom_keycodes {
    MOVE_HOME = SAFE_RANGE,
    VAL_YES,
    VAL_NO,
    VAL_ULT,
    VAL_COMMEND,
    VAL_VOTE_YES,
    VAL_VOTE_NO,
    VAL_NO_TIME,
    SPOT_TPB,
    SPOT_TS,
    SPOT_N,
    SPOT_P,
};

// Tap Dance Declarations
enum {
    TD_1_0,
};

// Combos
enum combo_events {
    COMBO_RESET,
    COMBO_LENGTH
};

// COMBO; update these with the bottom right and top left of _BASE layer
const uint16_t PROGMEM reset_combo[] = {KC_RIGHT, KC_A, COMBO_END};

combo_t key_combos[] = {
    [COMBO_RESET] = COMBO_ACTION(reset_combo),
};

void process_combo_event(uint16_t combo_index, bool pressed) {
    switch(combo_index) {
        case COMBO_RESET:
            if (pressed) {
                is_reset = true;
                reset_keyboard();
            }
            break;
    }
}


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base */
    [_BASE] = LAYOUT_ortho_3x3_1(
        MOVE_HOME,
        KC_A    , KC_B    , KC_LCTL  ,
        KC_F21  , KC_UP   , KC_F     ,
        KC_LEFT , KC_DOWN , KC_RIGHT
    ),
    [_DISCORD] = LAYOUT_ortho_3x3_1(
        MOVE_HOME,
        KC_TRNS    , KC_TRNS    , KC_TRNS    ,
        MEH(KC_F1) , MEH(KC_F2) , MEH(KC_F3) ,
        KC_Y       , KC_Z       , KC_R
    ),
    [_VALORANT] = LAYOUT_ortho_3x3_1(
        MOVE_HOME,
        KC_TRNS , KC_TRNS     , KC_TRNS     ,
        KC_TRNS , VAL_NO_TIME , VAL_COMMEND ,
        VAL_ULT , VAL_NO      , VAL_YES
    ),
    [_NUMPAD] = LAYOUT_ortho_3x3_1(
        MOVE_HOME,
        KC_7       , KC_8 , KC_9 ,
        KC_4       , KC_5 , KC_6 ,
        TD(TD_1_0) , KC_2 , KC_3
    ),
    [_HASS] = LAYOUT_ortho_3x3_1(
        MOVE_HOME,
        KC_NO  , KC_NO , KC_NO  ,
        KC_NO  , KC_NO , KC_NO  ,
        KC_F19 , KC_NO , KC_NO
    ),
    [_COD] = LAYOUT_ortho_3x3_1(
        MOVE_HOME,
        KC_TRNS , KC_TRNS , KC_TRNS ,
        KC_TRNS , KC_NO    , KC_NO   ,
        KC_NO   , KC_NO    , KC_F10
    ),
    [_SPOTIFY] = LAYOUT_ortho_3x3_1(
        MOVE_HOME,
        SPOT_P  , SPOT_TPB , SPOT_N ,
        SPOT_TS , KC_NO    , KC_NO  ,
        KC_NO   , KC_NO    , KC_NO
    )
};

// the below are all user defined functions
void raw_hid_send_current_layer(void) {
    if (is_hid_connected) {
        uint8_t layer = get_highest_layer(layer_state);
        uint8_t send_data[RAW_EPSIZE] = {0};
        send_data[0] = layer;
        raw_hid_send(send_data, sizeof(send_data));
    }
}

void activate_layer(uint8_t layer) {
    layer_clear();
    for (uint8_t lower = 0; lower <= layer; lower++) {
        layer_on(lower);
    }
    raw_hid_send_current_layer();
}


void next_layer(void) {
    uint8_t layer = get_highest_layer(layer_state);
    if ( ( layer + 1 ) < num_of_layers ) {
        layer = layer + 1;
    } else {
        layer = 0;
    }
    activate_layer(layer);
}

void prev_layer(void) {
    uint8_t layer = get_highest_layer(layer_state);
    if ( layer == 0 ) {
        layer = num_of_layers - 1;
    } else {
        layer = layer - 1;
    }
    activate_layer(layer);
}

void set_brightness(uint8_t delta) {
    //
    uint8_t current_brightness = oled_get_brightness();
    uint8_t level = 0;
    switch ( delta ) {
        case UP:
            level = current_brightness + 15;
            if ( current_brightness > level ) {
                level = 255;
            }
            break;
        case DOWN:
            level = current_brightness - 15;
            if ( current_brightness < level ) {
                level = 0;
            }
            break;
        default:
            break;
    }
    oled_set_brightness(level);
}
void clockwise_rotary(void) {
    if ( get_mods() & MOD_MASK_CTRL ) {
        set_brightness(UP);
    } else {
        next_layer();
    }
}
void cclockwise_rotary(void) {
    if ( get_mods() & MOD_MASK_CTRL ) {
        set_brightness(DOWN);
    } else {
        prev_layer();
    }
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_1_0] = ACTION_TAP_DANCE_DOUBLE(KC_P1, KC_P0),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case MOVE_HOME:
            if (record->event.pressed) {
                activate_layer(0);
            } else {
            }
            return false;
        case VAL_YES:
            if (record->event.pressed) {
                SEND_STRING(".33");
            } else {
            }
            return false;
        case VAL_NO:
            if (record->event.pressed) {
                SEND_STRING(".34");
            } else {
            }
            return false;
        case VAL_ULT:
            if (record->event.pressed) {
                SEND_STRING(".15");
            } else {
            }
            return false;
        case VAL_COMMEND:
            if (record->event.pressed) {
                SEND_STRING(".32");
            } else {
            }
            return false;
        case VAL_NO_TIME:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_ENT);
                unregister_code(KC_LSFT);
                SEND_STRING("NO TIME");
                tap_code(KC_ENT);
            } else {
            }
            return false;
        case SPOT_TS:
            if (record->event.pressed) {
                if ( is_hid_connected ) {
                    uint8_t data[RAW_EPSIZE] = {'t','o','g','g', 'l','e','s','h','u','f','f','l','e'};
                    raw_hid_send(data, sizeof(data));
                } else {

                }
            }
            return false;
        case SPOT_TPB:
            if (record->event.pressed) {
                if ( is_hid_connected ) {
                    uint8_t data[RAW_EPSIZE] = {'t','o','g','g','l','e','p','l','a','y','b','a','c','k'};
                    raw_hid_send(data, sizeof(data));
                } else {

                }
            }
            return false;
        case SPOT_N:
            if (record->event.pressed) {
                if ( is_hid_connected ) {
                    uint8_t data[RAW_EPSIZE] = {'n','e','x','t','s','o','n','g'};
                    raw_hid_send(data, sizeof(data));
                } else {

                }
            }
            return false;
        case SPOT_P:
            if (record->event.pressed) {
                if ( is_hid_connected ) {
                    uint8_t data[RAW_EPSIZE] = {'p','r','e','v','s','o','n','g'};
                    raw_hid_send(data, sizeof(data));
                } else {

                }
            }
            return false;
        default:
            return true;
    }
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if ( clockwise ) {
        clockwise_rotary();
    } else {
        cclockwise_rotary();
    }
    return false;
}

#ifdef OLED_ENABLE
bool oled_task_user(void) {
    //oled_write_P(PSTR("layer: "), false);
    // Here is the spacing of the oled.
    //  abcdefghijKlmnopqrstu
    // {---6--|---7---|--6---}
    //
    // clear all lower lines
    oled_set_cursor(20, 0);
    if ( is_hid_connected ) {
        oled_write_char(0x04, false);
        if ( hid_clear_screen ) {
            for (int line = 1; line < 8; line++) {
                oled_set_cursor(0, line);
                oled_advance_page(true);
            }
            // oled_set_cursor(0, 2);
            // oled_advance_page(true);
            // oled_set_cursor(0, 4);
            // oled_advance_page(true);
            // oled_set_cursor(0, 6);
            // oled_advance_page(true);
        }
        hid_clear_screen = false;
    } else {
        oled_write_char(0x00, false);
        hid_clear_screen = true;
    }

    oled_set_cursor(0, 0);
    switch (get_highest_layer(layer_state)) {
        case _BASE:
            oled_write_P(PSTR("       default    "), false);
            if ( !is_hid_connected ) {
                oled_set_cursor(0, 2);
                oled_write_P(PSTR("   A      B     CTRL "), false);
                oled_set_cursor(0, 4);
                oled_write_P(PSTR(" mute     ^      F   "), false);
                oled_set_cursor(0, 6);
                oled_write_P(PSTR("  <       .      >   "), false);
            }
            break;
        case _DISCORD:
            oled_write_P(PSTR("       discord    "), false);
            if ( !is_hid_connected ) {
                oled_set_cursor(0, 2);
                oled_write_P(PSTR(" vol-    =>     vol+ "), false);
                oled_set_cursor(0, 4);
                oled_write_P(PSTR(" mute  deafen   share"), false);
                oled_set_cursor(0, 6);
                oled_write_P(PSTR("  <>     <>      <>  "), false);
            }
            break;
        case _VALORANT:
            oled_write_P(PSTR("       valorant   "), false);
            if ( !is_hid_connected ) {
                oled_set_cursor(0, 2);
                oled_write_P(PSTR(" vol-    =>     vol+ "), false);
                oled_set_cursor(0, 4);
                oled_write_P(PSTR(" mute  notime   nice!"), false);
                oled_set_cursor(0, 6);
                oled_write_P(PSTR(" utl     no      yes "), false);
            }
            break;
        case _NUMPAD:
            oled_write_P(PSTR("        numpad    "), false);
            if ( !is_hid_connected ) {
                oled_set_cursor(0, 2);
                oled_write_P(PSTR("  7       8       9  "), false);
                oled_set_cursor(0, 4);
                oled_write_P(PSTR("  4       5       6  "), false);
                oled_set_cursor(0, 6);
                oled_write_P(PSTR(" 0/1      2       3  "), false);
            }
            break;
        case _HASS:
            oled_write_P(PSTR("    home assistant"), false);
            if ( !is_hid_connected ) {
                oled_set_cursor(0, 2);
                oled_write_P(PSTR("  <>     <>      <>  "), false);
                oled_set_cursor(0, 4);
                oled_write_P(PSTR("  <>     <>      <>  "), false);
                oled_set_cursor(0, 6);
                oled_write_P(PSTR("  <>     <>      <>  "), false);\
            }
            break;
        case _COD:
            oled_write_P(PSTR("     call of duty "), false);
            if ( !is_hid_connected ) {
                oled_set_cursor(0, 2);
                oled_write_P(PSTR(" vol-    =>     vol+ "), false);
                oled_set_cursor(0, 4);
                oled_write_P(PSTR(" mute    <>      <>  "), false);
                oled_set_cursor(0, 6);
                oled_write_P(PSTR("  <>     <>     mute "), false);
            }
            break;
        case _SPOTIFY:
            oled_write_P(PSTR("       spotify    "), false);
            if ( !is_hid_connected ) {
                if ( get_mods() & MOD_MASK_SHIFT ) {
                    oled_set_cursor(0, 2);
                    oled_write_P(PSTR(" prev  shuffle  next "), false);
                    oled_set_cursor(0, 4);
                    oled_write_P(PSTR(" mod     <>      <>  "), false);
                    oled_set_cursor(0, 6);
                    oled_write_P(PSTR("  <>     <>     test "), false);
                } else {
                    oled_set_cursor(0, 2);
                    oled_write_P(PSTR(" vol-    =>     vol+ "), false);
                    oled_set_cursor(0, 4);
                    oled_write_P(PSTR(" mod     <>      <>  "), false);
                    oled_set_cursor(0, 6);
                    oled_write_P(PSTR("  <>     <>     test "), false);
                }
            }
            break;
        default:
            oled_write_P(PSTR("undefined"), false);
            break;
    }
    // oled_advance_page(true);


    //oled_advance_page(true);
    // Host Keyboard LED Status
    //led_t led_state = host_keyboard_led_state();
    //oled_write_P(led_state.caps_lock ? PSTR("caps") : PSTR("    "), false);
    return false;
}
#endif

#ifdef RAW_ENABLE
void raw_hid_receive(uint8_t *data, uint8_t length) {
    //oled_scroll_set_area(start_line, end_line);
    // if we are here, the pc is connected
    is_hid_connected = true;
    const char *oled_data = (char*)data;
    uint8_t send_data[RAW_EPSIZE] = {0};
    uint8_t command = data[0];

    switch( command ) {
        case WRITE:
            oled_set_cursor(0, data[1]);
            oled_write(oled_data + 2, false);
            break;
        case PIXEL:
            if ( data[1] ) {
                pixel_state = true;
            } else {
                pixel_state = false;
            }
            pixel_index = 2;
            while(pixel_index < RAW_EPSIZE && data[pixel_index] != 0xff){
                oled_write_pixel(data[pixel_index], data[pixel_index + 1], pixel_state);
                pixel_index += 2;
            }
            break;
        case SCROLL:
        // this section is so buggy, please use at own risk
            switch( data[1] ) {
                case 1:
                    oled_scroll_off();
                    break;
                case 2:
                    oled_scroll_left();
                    break;
                case 3:
                    oled_scroll_right();
                    break;
                case 4:
                    oled_scroll_left();
                    break;
                case 5:
                    oled_scroll_set_speed(data[2]);
                    break;
                case 6:
                    oled_scroll_set_area(data[2], data[3]);
                    break;
                default:
                    break;
            }
            break;
        case BRIGHTNESS:
            oled_set_brightness(data[1]);
            break;
        case QUERY:
            switch( data[1] ) {
                case 1:
                    if ( is_oled_on() ) {
                        send_data[0] = 1;
                    } else {
                        send_data[0] = 0;
                    }
                    raw_hid_send(send_data, length);
                    break;
                case 2:
                    oled_on();
                    break;
                case 3:
                    oled_off();
                    break;
                case 4:
                    // current layer
                    send_data[0] = get_highest_layer(layer_state);
                    raw_hid_send(send_data, length);
                case 5:
                    // current brightness
                    send_data[0] = oled_get_brightness();
                    raw_hid_send(send_data, length);
                    break;
                case 6:
                    send_data[0] = oled_max_chars();
                    raw_hid_send(send_data, length);
                    break;
                case 7:
                    send_data[0] = oled_max_lines();
                    raw_hid_send(send_data, length);
                    break;
                default:
                    break;
            }
            break;
        case LAYER:
            activate_layer(data[1]);
            break;
        case EXIT:
            is_hid_connected = false;
            break;
        case CLEAR:
            switch( data[1] ) {
                case 8:
                    oled_clear();
                    break;
                default:
                    oled_set_cursor(0, data[1]);
                    oled_advance_page(true);
                    break;
            }
            break;
        default:
            break;
        }
    //raw_hid_send(data, length);
}
#endif
