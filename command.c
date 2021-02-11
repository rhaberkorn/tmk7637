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

#include <stdbool.h>
#include <stdint.h>

#include "debug.h"
#include "led.h"
#include "host.h"
#include "keycode.h"
#include "keyclick.h"
#include "pwm.h"
#include "song.h"
#include "command.h"

enum keyclick_mode keyclick_mode = KEYCLICK_OFF;

bool command_extra(uint8_t code)
{
    switch (code) {
        //case KC_AUDIO_MUTE:
        case KC_SPACE:
            keyclick_mode = (keyclick_mode+1) % KEYCLICK_MAX;
            dprintf("new keyclick mode: %u\n", keyclick_mode);
            /* FIXME: Perhaps do this in matrix_scan() */
            keyclick_solenoid_set(false);
            pwm_pb5_set_tone(0);
            /* update the keyclick mode LED */
            led_set(host_keyboard_leds());
            return true;

        /*
         * NOTE: The Fx keys are also used in command_common()
         * but do the same as the number keys (switch layers).
         * It should therefore be safe to repurpose them.
         */
        case KC_F1:
            song_play_ruinen();
            return true;
        case KC_F2:
            song_play_kitt();
            return true;
    }

    return false;
}
