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
#include <math.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "debug.h"
#include "pwm.h"

/**
 * Translation table from brightness to PWM setting to allow smooth
 * fadings (8-bit timers).
 *
 * This is calculated by the formula:
 * ```
 * uint16_t resolution = 0xFF;
 * pwm_table8[x] = x ? round(pow((double)x / 255.0, 2.5) * resolution) : 0;
 * ```
 */
static const uint8_t pwm_table8[256] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x07,
    0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0A, 0x0A, 0x0A,
    0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F,
    0x10, 0x10, 0x11, 0x11, 0x12, 0x12, 0x13, 0x13, 0x14, 0x14, 0x15, 0x16,
    0x16, 0x17, 0x17, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1C, 0x1C, 0x1D,
    0x1E, 0x1E, 0x1F, 0x20, 0x21, 0x21, 0x22, 0x23, 0x24, 0x24, 0x25, 0x26,
    0x27, 0x28, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
    0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4B, 0x4C, 0x4D, 0x4E, 0x50, 0x51, 0x52, 0x53, 0x55, 0x56, 0x57, 0x59,
    0x5A, 0x5B, 0x5D, 0x5E, 0x5F, 0x61, 0x62, 0x63, 0x65, 0x66, 0x68, 0x69,
    0x6B, 0x6C, 0x6E, 0x6F, 0x71, 0x72, 0x74, 0x75, 0x77, 0x79, 0x7A, 0x7C,
    0x7D, 0x7F, 0x81, 0x82, 0x84, 0x86, 0x87, 0x89, 0x8B, 0x8D, 0x8E, 0x90,
    0x92, 0x94, 0x96, 0x97, 0x99, 0x9B, 0x9D, 0x9F, 0xA1, 0xA3, 0xA5, 0xA6,
    0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xB4, 0xB6, 0xB8, 0xBA, 0xBD, 0xBF,
    0xC1, 0xC3, 0xC5, 0xC7, 0xC9, 0xCC, 0xCE, 0xD0, 0xD2, 0xD4, 0xD7, 0xD9,
    0xDB, 0xDD, 0xE0, 0xE2, 0xE4, 0xE7, 0xE9, 0xEB, 0xEE, 0xF0, 0xF3, 0xF5,
    0xF8, 0xFA, 0xFD, 0xFF
};

/**
 * Translation table from brightness to PWM setting to allow smooth
 * fadings (16-bit timers).
 */
static const uint16_t pwm_table16[256] PROGMEM = {
    0x0000, 0x0000, 0x0000, 0x0001, 0x0002, 0x0004, 0x0006, 0x0008, 0x000B,
    0x000F, 0x0014, 0x0019, 0x001F, 0x0026, 0x002E, 0x0037, 0x0041, 0x004B,
    0x0057, 0x0063, 0x0071, 0x0080, 0x008F, 0x00A0, 0x00B2, 0x00C5, 0x00DA,
    0x00EF, 0x0106, 0x011E, 0x0137, 0x0152, 0x016E, 0x018B, 0x01A9, 0x01C9,
    0x01EB, 0x020E, 0x0232, 0x0257, 0x027F, 0x02A7, 0x02D2, 0x02FD, 0x032B,
    0x0359, 0x038A, 0x03BC, 0x03EF, 0x0425, 0x045C, 0x0494, 0x04CF, 0x050B,
    0x0548, 0x0588, 0x05C9, 0x060C, 0x0651, 0x0698, 0x06E0, 0x072A, 0x0776,
    0x07C4, 0x0814, 0x0866, 0x08B9, 0x090F, 0x0967, 0x09C0, 0x0A1B, 0x0A79,
    0x0AD8, 0x0B3A, 0x0B9D, 0x0C03, 0x0C6A, 0x0CD4, 0x0D3F, 0x0DAD, 0x0E1D,
    0x0E8F, 0x0F03, 0x0F79, 0x0FF2, 0x106C, 0x10E9, 0x1168, 0x11E9, 0x126C,
    0x12F2, 0x137A, 0x1404, 0x1490, 0x151F, 0x15B0, 0x1643, 0x16D9, 0x1771,
    0x180B, 0x18A7, 0x1946, 0x19E8, 0x1A8B, 0x1B32, 0x1BDA, 0x1C85, 0x1D33,
    0x1DE2, 0x1E95, 0x1F49, 0x2001, 0x20BB, 0x2177, 0x2236, 0x22F7, 0x23BB,
    0x2481, 0x254A, 0x2616, 0x26E4, 0x27B5, 0x2888, 0x295E, 0x2A36, 0x2B11,
    0x2BEF, 0x2CD0, 0x2DB3, 0x2E99, 0x2F81, 0x306D, 0x315A, 0x324B, 0x333F,
    0x3435, 0x352E, 0x3629, 0x3728, 0x3829, 0x392D, 0x3A33, 0x3B3D, 0x3C49,
    0x3D59, 0x3E6B, 0x3F80, 0x4097, 0x41B2, 0x42D0, 0x43F0, 0x4513, 0x463A,
    0x4763, 0x488F, 0x49BE, 0x4AF0, 0x4C25, 0x4D5D, 0x4E97, 0x4FD5, 0x5116,
    0x525A, 0x53A1, 0x54EB, 0x5638, 0x5787, 0x58DA, 0x5A31, 0x5B8A, 0x5CE6,
    0x5E45, 0x5FA7, 0x610D, 0x6276, 0x63E1, 0x6550, 0x66C2, 0x6837, 0x69AF,
    0x6B2B, 0x6CAA, 0x6E2B, 0x6FB0, 0x7139, 0x72C4, 0x7453, 0x75E5, 0x777A,
    0x7912, 0x7AAE, 0x7C4C, 0x7DEF, 0x7F94, 0x813D, 0x82E9, 0x8498, 0x864B,
    0x8801, 0x89BA, 0x8B76, 0x8D36, 0x8EFA, 0x90C0, 0x928A, 0x9458, 0x9629,
    0x97FD, 0x99D4, 0x9BB0, 0x9D8E, 0x9F70, 0xA155, 0xA33E, 0xA52A, 0xA71A,
    0xA90D, 0xAB04, 0xACFE, 0xAEFB, 0xB0FC, 0xB301, 0xB509, 0xB715, 0xB924,
    0xBB37, 0xBD4D, 0xBF67, 0xC184, 0xC3A5, 0xC5CA, 0xC7F2, 0xCA1E, 0xCC4D,
    0xCE80, 0xD0B7, 0xD2F1, 0xD52F, 0xD771, 0xD9B6, 0xDBFE, 0xDE4B, 0xE09B,
    0xE2EF, 0xE547, 0xE7A2, 0xEA01, 0xEC63, 0xEECA, 0xF134, 0xF3A2, 0xF613,
    0xF888, 0xFB02, 0xFD7E, 0xFFFF
};

/**
 * Configure a PWM pin of Timer 0 (8-bit resolution).
 *
 * @note Timer 0 is used by tmk's timer module as well.
 * You can therefore only temporarilly PWM-modulate using timer 0
 * (ie. NOT during matrix scanning), cannot use any timer module functions
 * and should call timer_init() afterwards.
 * This can be easily resolved by using pwm_pb5_set_led() for one of the LEDs
 * and redistribute all the LED.
 * The buzzer can then use the no longer used PD0 since it's manually
 * IRQ-triggered anyway.
 *
 * @param channel Channel to initialize.
 *     0 (OC0A/PB7), 1 (OC0B/PD0)
 */
static void pwm_timer0_init(uint8_t channel)
{
    /* Fast PWM on OC0x, inverted duty cycle, TOP = 0xFF */
    TCCR0A |= (0b11 << (3-channel)*2) | 0b11;
    /* no prescaling */
    TCCR0B = 0b00000001;
}

void pwm_pd0_set_led(uint8_t brightness)
{
    switch (brightness) {
        case 0:
            TCCR0A &= ~0b00110000;
            PORTD |= (1 << PD0);
            break;
        case 255:
            TCCR0A &= ~0b00110000;
            PORTD &= ~(1 << PD0);
            break;
        default:
            pwm_timer0_init(1);
            OCR0B = pgm_read_byte(&pwm_table8[brightness]);
            break;
    }
}

/**
 * Configure a PWM pin of Timer 1 (16-bit resolution).
 *
 * @param channel Channel to initialize.
 *     0 (OC1A/PB5), 1 (OC1B/PB6), 2 (OC1C/PB7)
 */
static void pwm_timer1_init(uint8_t channel)
{
    /* Fast PWM on OC1x, inverted duty cycle, TOP = ICR1 */
    TCCR1A |= (0b11 << (3-channel)*2) | 0b10;
    /* stop timer */
    TCCR1B = 0;
    /* TOP for PWM - full 16 Bit */
    ICR1 = 0xFFFF;
    /* no prescaling */
    TCCR1B = 0b00011001;
}

void pwm_pb5_set_led(uint8_t brightness)
{
    switch (brightness) {
        case 0:
            TCCR1A &= ~0b11000000;
            PORTB |= (1 << PB5);
            break;
        case 255:
            TCCR1A &= ~0b11000000;
            PORTB &= ~(1 << PB5);
            break;
        default:
            pwm_timer1_init(0);
            OCR1A = pgm_read_word(&pwm_table16[brightness]);
            break;
    }
}

void pwm_pb6_set_led(uint8_t brightness)
{
    switch (brightness) {
        case 0:
            TCCR1A &= ~0b00110000;
            PORTB |= (1 << PB6);
            break;
        case 255:
            TCCR1A &= ~0b00110000;
            PORTB &= ~(1 << PB6);
            break;
        default:
            pwm_timer1_init(1);
            OCR1B = pgm_read_word(&pwm_table16[brightness]);
            break;
    }
}

void pwm_pb7_set_led(uint8_t brightness)
{
    switch (brightness) {
        case 0:
            TCCR1A &= ~0b00001100;
            PORTB |= (1 << PB7);
            break;
        case 255:
            TCCR1A &= ~0b00001100;
            PORTB &= ~(1 << PB7);
            break;
        default:
            pwm_timer1_init(2);
            OCR1C = pgm_read_word(&pwm_table16[brightness]);
            break;
    }
}

/**
 * Configure a PWM pin of Timer 2 (8-bit resolution).
 *
 * @param channel Channel to initialize.
 *     0 (OC2A/PB4), 1 (OC2B/PD1)
 */
static void pwm_timer2_init(uint8_t channel)
{
    /* Fast PWM on OC2x, inverted duty cycle, TOP = 0xFF */
    TCCR2A |= (0b11 << (3-channel)*2) | 0b11;
    /* no prescaling */
    TCCR2B = 0b00000001;
}

void pwm_pb4_set_led(uint8_t brightness)
{
    switch (brightness) {
        case 0:
            TCCR2A &= ~0b11000000;
            PORTB |= (1 << PB4);
            break;
        case 255:
            TCCR2A &= ~0b11000000;
            PORTB &= ~(1 << PB4);
            break;
        default:
            pwm_timer2_init(0);
            OCR2A = pgm_read_byte(&pwm_table8[brightness]);
            break;
    }
}

void pwm_pd1_set_led(uint8_t brightness)
{
    switch (brightness) {
        case 0:
            TCCR2A &= ~0b00110000;
            PORTD |= (1 << PD1);
            break;
        case 255:
            TCCR2A &= ~0b00110000;
            PORTD &= ~(1 << PD1);
            break;
        default:
            pwm_timer2_init(1);
            OCR2B = pgm_read_byte(&pwm_table8[brightness]);
            break;
    }
}

/**
 * Set a LED by position in the first row.
 *
 * @bug The pins used should be rescrambled after we freed PD0.
 *
 * @param led LED to set (0-4).
 * @param brightness Brightness level to set.
 */
void pwm_set_led(uint8_t led, uint8_t brightness)
{
     switch (led) {
         case 0: pwm_pd0_set_led(brightness); break;
         case 1: pwm_pb7_set_led(brightness); break;
         case 2: pwm_pd1_set_led(brightness); break;
         case 3: pwm_pb6_set_led(brightness); break;
         case 4: pwm_pb4_set_led(brightness); break;
     }
}

/**
 * Play a tone on PB5 (the buzzer).
 *
 * @note This uses timer 3 with an IRQ, even though we cannot
 * use one of the Timer 3 PWM pins -- they are required for matrix
 * strobing.
 * It was problematic to use one of the accessible Timer 0-2 pins as
 * a frequency generator for the buzzer, though:
 * - Timer 0 (PD0) does not support toggle mode and regular PWM mode would
 *   leave only a 7-bit frequency resolution.
 * - Timer 1 is shared with two other LED pins.
 *   It is therefore not possible to use toggle mode, but the remaining 15
 *   bit are sufficient anyway. Unfortunately, changing the frequency changes
 *   the timer's TOP value, which requires recalculating the duty cycles of the
 *   remaining two pins. This calculation is slow and cannot be cached.
 *   Also, changing the frequency would always introduce some flickering.
 * - The same is true for timer 2, just with even less resolution.
 * Using timer 3 exclusively for sound has the advantage that we can also
 * some day use an IRQ for playing back (bit banging) audio samples eg.
 * for a keyclick sounds.
 *
 * @todo It would be nice, if we could specify the tone's volume.
 * This will reduce the effective resolution to 15 bit, but that should be
 * sufficient anyway.
 *
 * @param freq Frequency to play. If 0, disables the buzzer.
 */
void pwm_pb5_set_tone(uint16_t freq)
{
    if (!freq) {
        /*
         * There should be a way to turn off the buzzer as we would otherwise
         * always have some kind of tone.
         */
        TIMSK3 = 0;
        return;
    }

    /*
     * CTC mode: Effectively allows us to toggle the pin after OCR3A counts.
     * This allows us to use the full 16-bit resolution.
     * On the other hand, if we'd like to control volume, we'd have to
     * control the cycle length independently, reducing resolution to 15 bit.
     */
    TCCR3A = 0b00;
    /*
     * Prescaling: 8 (0b010).
     * This allows for frequencies between 15 Hz and 1 MHz.
     *
     * FIXME: It may be even better to use 1 if we have frequencies below 122 Hz.
     */
    TCCR3B = 0b00001010;

    /*
     * The frequency of the resulting signal is calculated as follows:
     * F_CPU / PRESCALER / (CYCLE_LENGTH+1) / 2
     */
    OCR3A = (F_CPU/2/8) / freq - 1;

    /* enable TIMER3_COMPA_vect() interrupt */
    TIMSK3 = (1 << OCIE3A);
}

ISR(TIMER3_COMPA_vect, ISR_NOBLOCK)
{
    PORTB ^= (1 << PB5);
}
