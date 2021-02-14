/*
Copyright 2021 Robin Haberkorn <robin.haberkorn@googlemail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <avr/io.h>
#include <util/delay.h>

#include "print.h"
#include "debug.h"
#include "timer.h"
#include "led.h"
#include "host.h"
#include "pwm.h"
#include "keyclick.h"
#include "matrix.h"

/*
 * NOTE: The "Betriebsdokumentation" mentions that the keyboard matrix
 * must not change for two scan cycles.
 * The setting below actually means, it must not change for 2ms.
 * This has been shown to be sufficient.
 * It is still possible for keypresses to be instable but this has only
 * been observed with a poor power source.
 */
#ifndef DEBOUNCE
#   define DEBOUNCE	2
#endif

/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];

static void init_pins(void);
static void select_col(uint8_t col);
static void unselect_cols(void);
static bool read_row(uint8_t row);

void matrix_init(void)
{
    debug_enable = true;
    debug_matrix = true;

    /* this also configures all LED pins and brings them into defined states */
    led_set(host_keyboard_leds());

    init_pins();

    /* initialize matrix state: all keys off */
    memset(matrix, sizeof(matrix), 0);
    memset(matrix_debouncing, sizeof(matrix_debouncing), 0);
}

/*
 * FIXME: If we rotated the matrix (8 columns and 16 rows)
 * we could simplify and speed up the code below.
 * Unfortunately, we'd also have to rotate the Unimap translation table and
 * it would no longer correspond with the "Serviceschaltplan".
 */
uint8_t matrix_scan(void)
{
    static uint16_t debouncing_time = 0, keyclick_time = 0;
    /** Number of pressed keys in `matrix_debouncing` */
    uint8_t matrix_debouncing_pressed_keys = 0;

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        select_col(col);
        /*
         * Give the signals some time to settle.
         * This is not mentioned in the "Betriebsdokumentation"
         * but has been tested experimentally.
         * 30us is used by all the other controller firmwares as well.
         */
        _delay_us(30);

        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            matrix_row_t prev_row = matrix_debouncing[row];

            if (read_row(row)) {
                matrix_debouncing[row] |= (1 << col);
                /* the "security key" bits do not count into the pressed keys */
                if (col < MATRIX_COLS-1 || row > 3)
                    matrix_debouncing_pressed_keys++;
            } else {
                matrix_debouncing[row] &= ~(1 << col);
            }

            if (matrix_debouncing[row] != prev_row)
                debouncing_time = timer_read();
        }

        unselect_cols();
    }

    if (debouncing_time && timer_elapsed(debouncing_time) >= DEBOUNCE) {
        /** Number of pressed keys in `matrix` */
        static uint8_t matrix_pressed_keys = 0;

        /*
         * Trigger keyclick whenever the keyboard matrix has actually
         * changed (ie. `debouncing_time` was set).
         * When using the solenoid, the keyclick is supposed to immitate a
         * mechanical click, so there should be one sound when pressing down
         * the key and another when releasing it.
         * If a key is already pressed (ie. you press an additional key),
         * we release the solenoid for 50ms.
         * We consciously do not _delay_ms(50) here since that would delay
         * key event delivery.
         * When using the buzzer, a short 50ms beep is played every time
         * a new key is pressed.
         */
        switch (keyclick_mode) {
            case KEYCLICK_SOLENOID:
                if (matrix_debouncing_pressed_keys > 1 ||
                    (matrix_debouncing_pressed_keys == 1 && matrix_pressed_keys > 1)) {
                    keyclick_solenoid_set(false);
                    keyclick_time = timer_read();
                } else {
                    keyclick_solenoid_set(matrix_debouncing_pressed_keys);
                }
                break;

            case KEYCLICK_BUZZER:
                if (matrix_debouncing_pressed_keys > matrix_pressed_keys) {
                    pwm_pb5_set_tone(550);
                    keyclick_time = timer_read();
                }
                break;

            default:
                break;
        }

        memcpy(matrix, matrix_debouncing, sizeof(matrix));
        matrix_pressed_keys = matrix_debouncing_pressed_keys;

        /*
         * The first 4 bits in the 15th column
         * are sensed like ordinary keypresses but
         * in reality represent a 3-bit "security key"
         * (cf. Betriebsdokumentation, p.13f) with
         * an actual resolution of 6 encoded into a special
         * keylike device that's plugged into the keyboard.
         * It makes no sense to map these original bits into
         * the keyboard matrix.
         * Instead we translate it into one of six key presses
         * via unused fields of the keyboard matrix whenever
         * the "security" key is removed.
         * These pseudo-keypress are mapped to F19-F24 in unimap_trans
         * and could be mapped at the OS level, eg. to lock up the screen.
         * When removing the "security key", F18 is also pressed which
         * will usually be mapped to a modifier but could also be left
         * F18 and be used to lock up the screen, ignoring all the other
         * pseudo-keys.
         *
         * FIXME: It would be more elegant to directly cause
         * a key event to be generated, but this does not seem to be
         * supported if we want to take the configurable keymap into account.
         * Also, it might be more elegant to have all the pseudo-keys
         * in a dedicated matrix row.
         */
        static uint8_t last_security_key = 0;
        uint8_t security_key = (~(matrix[0] >> (MATRIX_COLS-1)) & (1 << 0)) |
                               (~(matrix[1] >> (MATRIX_COLS-2)) & (1 << 1)) |
                               (~(matrix[2] >> (MATRIX_COLS-3)) & (1 << 2));

        for (uint8_t i = 0; i < 4; i++)
            matrix[i] &= ~(1 << (MATRIX_COLS-1)); /* F19-F24 */

        if (last_security_key == 0 && 1 <= security_key && security_key <= 6) {
            dprintf("Security key %u inserted\n", security_key);
            matrix[security_key-1] |= (1 << (MATRIX_COLS-1)); /* F19-F24 */
        } else if (1 <= last_security_key && last_security_key <= 6 && security_key == 0) {
            dprintf("Security key %u removed\n", last_security_key);
            matrix[0] |= (1 << 13); /* F18 */
            matrix[last_security_key-1] |= (1 << (MATRIX_COLS-1)); /* F19-F24 */
        }

        last_security_key = security_key;

        debouncing_time = 0;
    } else {
        /*
         * Physically inserting or removing the "security key" should
         * result in a keypress event immediately followed by a kreyrelease.
         * We therefore clear the matrix slots reserved for it, so that
         * the key press is reported only for one scan cycle.
         */
        matrix[0] &= ~(1 << 13); /* F18 */
        for (uint8_t i = 0; i < 4; i++)
            matrix[i] &= ~(1 << (MATRIX_COLS-1)); /* F19-F24 */
    }

    /*
     * This is responsible for performing a delayed action depending
     * on the keyclick mode (see abovc).
     * For solenoids, it reactivates them since they should be on and
     * are released when the last key is released.
     * When using the buzzer, this is responsible for turning it off
     * after a short while.
     * Polling here makes sure we do not delay any key delivery.
     */
    if (keyclick_time && timer_elapsed(keyclick_time) >= 50) {
        switch (keyclick_mode) {
            case KEYCLICK_SOLENOID:
                keyclick_solenoid_set(true);
                break;

            case KEYCLICK_BUZZER:
                if (!(host_keyboard_leds() & (1 << USB_LED_KANA)))
                    pwm_pb5_set_tone(0);
                break;

            default:
                break;
        }

        keyclick_time = 0;
    }

    return 1;
}

inline
matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

static void init_pins(void)
{
    /*
     * All rows are input pins (DDR:0) and need pull-ups.
     * There is a pull-up attached to the EPROM, but I acceidentally
     * destroyed it.
     * Therefore we enable internal pull-ups (PORT:1).
     */
    /* Row 0: PB0 == D7 */
    DDRB  &= ~0b1;
    PORTB |= 0b1;
    /* Row 1-2: PE4-5 == D5-6 (Connected by cable) */
    DDRE  &= ~0b110000;
    PORTE |= 0b110000;
    /* Row 3-4: PF0-1 == D4-3 (NOTE: PF0 == Row 4, PF1 == Row 3) */
    DDRF  &= ~0b11;
    PORTF |= 0b11;
    /* Row 5: PE7 == D2 */
    DDRE  &= ~0b10000000;
    PORTE |= 0b10000000;
    /* Row 6-7: PB1-2 == D0-1 */
    DDRB  &= ~0b110;
    PORTB |= 0b110;

    /*
     * We've got a strobe-sense matrix and we strobe via the column pins.
     * Therefore they must be outputs (DDR:1).
     */
    /* Column 0-3: PF4-7 == A11-14 */
    DDRF  |= 0b11110000;
    /* Column 4-11: PC0-7 == A3-10 */
    DDRC  |= 0b11111111;
    /* Column 12-13: PE0-1 == A1-2 */
    DDRE  |= 0b11;
    /* Column 14: PD7 == A0 */
    DDRD  |= 0b10000000;

    /*
     * PF3 == A15 must be LOW since otherwise some NAND gates (IC13) will interfere with D0-3
     * (half of the matrix rows).
     * We might also draw this to GND by cable or destroy IC13.
     * The latter might even save some power.
     *
     * FIXME: A15 must actually be triggered for some of the modifiers!
     */
    DDRF  |= 0b00001000;
    PORTF &= ~0b00001000;

    /*
     * Solenoid (ie. relay). It's LOW-active.
     */
    DDRB  |= (1 << PB3);
    PORTB |= (1 << PB3);
}

static void select_col(uint8_t col)
{
    /*
     * FIXME: Simplify? Will go at the expense of readability.
     */
    switch (col) {
        case 0:  PORTD |= 0b10000000; break;
        case 1:  PORTE |= 0b00000001; break;
        case 2:  PORTE |= 0b00000010; break;
        case 3:  PORTC |= 0b00000001; break;
        case 4:  PORTC |= 0b00000010; break;
        case 5:  PORTC |= 0b00000100; break;
        case 6:  PORTC |= 0b00001000; break;
        case 7:  PORTC |= 0b00010000; break;
        case 8:  PORTC |= 0b00100000; break;
        case 9:  PORTC |= 0b01000000; break;
        case 10: PORTC |= 0b10000000; break;
        case 11: PORTF |= 0b10000000; break;
        case 12: PORTF |= 0b01000000; break;
        case 13: PORTF |= 0b00100000; break;
        case 14: PORTF |= 0b00010000; break;
        case 15: PORTF |= 0b00001000; break;
    }
}

static void unselect_cols(void)
{
    PORTD &= ~0b10000000;
    PORTE &= ~0b11;
    PORTC &= ~0b11111111;
    PORTF &= ~0b11111000;
}

static bool read_row(uint8_t row)
{
    /*
     * NOTE: The key mechanism pulls the row pin to GND when the key is
     * pressed.
     * However, the rows are not directly connected to the keyboard matrix
     * but through NAND gates. The signal is therefore inverted.
     * The NAND gates no longer serve any purpose and could be skipped
     * altogether by desoldering them and directly connecting the inputs with
     * the outputs. The PIN registers should then be inverted.
     */
    switch (row) {
        case 0: return PINB & 0b00000010;
        case 1: return PINB & 0b00000100;
        case 2: return PINE & 0b10000000;
        case 3: return PINF & 0b00000001;
        case 4: return PINF & 0b00000010;
        case 5: return PINE & 0b00100000;
        case 6: return PINE & 0b00010000;
        case 7: return PINB & 0b00000001;
    }

    /* not reached */
    return false;
}
