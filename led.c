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

#include <avr/io.h>

#include "debug.h"
#include "keyclick.h"
#include "led.h"
#include "pwm.h"

void led_set(uint8_t usb_led)
{
    dprintf("Set keyboard LEDs: 0x%02X\n", usb_led);

    /*
     * All LEDs and the buzzer are output pins obviously, even for PWM operation
     */
    DDRD |= 0b00001111;
    DDRB |= 0b11110000;

    /* LED right next to the Capslock key (C99) */
    if (usb_led & (1 << USB_LED_CAPS_LOCK))
        PORTD &= ~(1 << PD3);
    else
        PORTD |= (1 << PD3);

    /* 1st LED on first row (G00). */
    pwm_pb5_set_led(usb_led & (1 << USB_LED_NUM_LOCK) ? 255 : 0);

    /* 2nd LED on first row (G01): Highlight keyclick mode. */
    pwm_pd1_set_led(keyclick_mode*255/(KEYCLICK_MAX-1));

    /* 3rd LED on the first row (G02) */
    pwm_pb7_set_led(usb_led & (1 << USB_LED_COMPOSE) ? 255 : 0);

    /*
     * 4th LED (G03) are currently not triggerable via USB.
     * Could be triggered as the "backlight".
     */
    pwm_pb4_set_led(0);

    /* 5th LED on the first row (G04) */
    pwm_pb6_set_led(usb_led & (1 << USB_LED_SCROLL_LOCK) ? 255 : 0);

    /*
     * 6th LED on the first row (G53).
     *
     * Triggering this LED (PD2) will also enable the buzzer (PD0),
     * so we have got a way to beep from userspace (see ./k7637-beep.sh).
     *
     * The original firmware also had the error display on G53
     * (cf. Betriebsdokumentation).
     */
    if (usb_led & (1 << USB_LED_KANA)) {
        PORTD &= ~(1 << PD2);
        pwm_pd0_set_tone(2200);
    } else {
        PORTD |= (1 << PD2);
        pwm_pd0_set_tone(0);
    }
}
