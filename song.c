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

#include <avr/pgmspace.h>
#include <util/delay.h>

#include "debug.h"
#include "timer.h"
#include "led.h"
#include "host.h"
#include "pwm.h"
#include "keyclick.h"
#include "song.h"

static void delay_long(uint16_t ms)
{
     while (ms >= 10) {
         _delay_ms(10);
         ms -= 10;
     }
     switch (ms) {
         case 9: _delay_ms(9); break;
         case 8: _delay_ms(8); break;
         case 7: _delay_ms(7); break;
         case 6: _delay_ms(6); break;
         case 5: _delay_ms(5); break;
         case 4: _delay_ms(4); break;
         case 3: _delay_ms(3); break;
         case 2: _delay_ms(2); break;
         case 1: _delay_ms(1); break;
    }
}

struct song_note {
    uint16_t freq;
    uint16_t dur;
};

// midicsv anthem_ddr.mid | perl midicsv2frequency.pl
static const struct song_note song_ruinen[] PROGMEM = {
    {440,585},
    {440,459},
    {0,585},
    {0,156},
    {391,500},
    {0,100},
    {349,500},
    {0,100},
    {466,1044},
    {0,156},
    {440,500},
    {0,100},
    {391,500},
    {0,100},
    {523,500},
    {0,100},
    {440,500},
    {0,100},
    {349,500},
    {0,100},
    {523,500},
    {0,100},
    {523,800},
    {0,100},
    {587,294},
    {0,6},
    {466,1044},
    {0,156},
    {440,1044},
    {0,156},
    {391,500},
    {0,100},
    {349,500},
    {0,100},
    {466,1044},
    {0,156},
    {440,500},
    {0,100},
    {391,500},
    {0,100},
    {523,500},
    {0,100},
    {440,500},
    {0,100},
    {587,500},
    {0,100},
    {466,500},
    {0,100},
    {391,800},
    {0,100},
    {329,294},
    {0,6},
    {349,1700},
    {0,100},
    {349,444},
    {0,6},
    {329,144},
    {0,6},
    {293,800},
    {0,100},
    {329,294},
    {0,6},
    {349,800},
    {0,100},
    {440,294},
    {0,6},
    {391,500},
    {0,100},
    {261,1100},
    {0,100},
    {349,444},
    {0,6},
    {329,144},
    {0,6},
    {293,800},
    {0,100},
    {329,294},
    {0,6},
    {349,800},
    {0,100},
    {440,294},
    {0,6},
    {391,1044},
    {0,156},
    {440,500},
    {0,100},
    {440,500},
    {0,100},
    {391,500},
    {0,100},
    {349,500},
    {0,100},
    {466,500},
    {0,100},
    {466,500},
    {0,100},
    {440,500},
    {0,100},
    {391,500},
    {0,100},
    {523,500},
    {0,100},
    {440,500},
    {0,100},
    {349,500},
    {0,100},
    {523,500},
    {0,100},
    {523,800},
    {0,100},
    {587,294},
    {0,6},
    {466,500},
    {0,100},
    {349,294},
    {0,6},
    {391,294},
    {0,6},
    {440,1044},
    {0,156},
    {391,1044},
    {0,156},
    {523,1044},
    {0,156},
    {349,500},
    {0,100},
    {391,500},
    {0,100},
    {440,1044},
    {0,156},
    {391,1044},
    {0,156},
    {349,1044}
};

void song_play_ruinen(void)
{
    keyclick_solenoid_set(false);

    uint8_t i = 0;
    for (long unsigned int cur_note = 0; cur_note < sizeof(song_ruinen)/sizeof(song_ruinen[0]); cur_note++) {
        pwm_pb5_set_tone(pgm_read_word(&song_ruinen[cur_note].freq));

        uint8_t max_brightness = ((uint32_t)pgm_read_word(&song_ruinen[cur_note].freq)*255)/600;
        uint16_t fade_dur = pgm_read_word(&song_ruinen[cur_note].dur)/2/(max_brightness+1);
        for (int16_t brightness = 0; brightness <= max_brightness; brightness++) {
            pwm_set_led(i % 5, brightness);
            delay_long(fade_dur);
        }

        /*
         * The fade duration is probably not very precise, so compensate for it.
         *
         * FIXME: Once timer 0 is freed, we should use timer_read() and timer_elapsed()
         * which will be more precise.
         */
        delay_long(pgm_read_word(&song_ruinen[cur_note].dur) - fade_dur*(max_brightness+1)*2);

        for (int16_t brightness = max_brightness; brightness >= 0; brightness--) {
            pwm_set_led(i % 5, brightness);
            delay_long(fade_dur);
        }

        if (pgm_read_word(&song_ruinen[cur_note].freq))
            i++;
    }

    pwm_pb5_set_tone(0);

    /* we screwed up timer 0 settings and they are also used by the timer module */
    timer_init();

    /* restore the previous lock lights */
    led_set(host_keyboard_leds());
}

static const struct song_note song_knight_rider[] PROGMEM = {
    // KnightRider:d=4, o=5, b=125:16e, 16p, 16f, 16e, 16e, 16p, 16e, 16e, 16f, 16e, 16e, 16e,
    // 16d#, 16e, 16e, 16e, 16e, 16p, 16f, 16e, 16e, 16p, 16f, 16e, 16f, 16e, 16e, 16e, 16d#,
    // 16e, 16e, 16e, 16d, 16p, 16e, 16d, 16d, 16p, 16e, 16d, 16e, 16d, 16d, 16d, 16c, 16d, 16d,
    // 16d, 16d, 16p, 16e, 16d, 16d, 16p, 16e, 16d, 16e, 16d, 16d, 16d, 16c, 16d, 16d, 16d
    {0, 480},
    {330, 480},
    {0, 120},
    {349, 120},
    {330, 120},
    {330, 120},
    {0, 120},
    {330, 120},
    {330, 120},
    {349, 120},
    {330, 120},
    {330, 120},
    {330, 120},
    {311, 120},
    {330, 120},
    {330, 120},
    {330, 120},
    {330, 120},
    {0, 120},
    {349, 120},
    {330, 120},
    {330, 120},
    {0, 120},
    {349, 120},
    {330, 120},
    {349, 120},
    {330, 120},
    {330, 120},
    {330, 120},
    {311, 120},
    {330, 120},
    {330, 120},
    {330, 120},
    {294, 120},
    {0, 120},
    {330, 120},
    {294, 120},
    {294, 120},
    {0, 120},
    {330, 120},
    {294, 120},
    {330, 120},
    {294, 120},
    {294, 120},
    {294, 120},
    {262, 120},
    {294, 120},
    {294, 120},
    {294, 120},
    {294, 120},
    {0, 120},
    {330, 120},
    {294, 120},
    {294, 120},
    {0, 120},
    {330, 120},
    {294, 120},
    {330, 120},
    {294, 120},
    {294, 120},
    {294, 120},
    {262, 120},
    {294, 120},
    {294, 120},

    // KnightRider:d=4, o=5, b=63:16e, 32f, 32e, 8b, 16e6, 32f6, 32e6, 8b, 16e, 32f, 32e, 16b,
    // 16e6, d6, 8p, p, 16e, 32f, 32e, 8b, 16e6, 32f6, 32e6, 8b, 16e, 32f, 32e, 16b, 16e6, f6, p
    {0, 952},
    {330, 952},
    {349, 119},
    {330, 119},
    {494, 476},
    {659, 238},
    {698, 119},
    {659, 119},
    {494, 476},
    {330, 238},
    {349, 119},
    {330, 119},
    {494, 238},
    {659, 238},
    {587, 952},
    {0, 476},
    {0, 952},
    {330, 238},
    {349, 119},
    {330, 119},
    {494, 476},
    {659, 238},
    {698, 119},
    {659, 119},
    {494, 476},
    {330, 238},
    {349, 119},
    {330, 119},
    {494, 238},
    {659, 238},
    {698, 952}

#if 0
    {0, 952},
    {659, 952},
    {698, 119},
    {659, 119},
    {988, 476},
    {1319, 238},
    {1397, 119},
    {1319, 119},
    {988, 476},
    {659, 238},
    {698, 119},
    {659, 119},
    {988, 238},
    {1319, 238},
    {1175, 952},
    {0, 476},
    {0, 952},
    {659, 238},
    {698, 119},
    {659, 119},
    {988, 476},
    {1319, 238},
    {1397, 119},
    {1319, 119},
    {988, 476},
    {659, 238},
    {698, 119},
    {659, 119},
    {988, 238},
    {1319, 238},
    {1397, 952}
#endif
};

/**
 * Light curve of the Larsen light.
 * This is half a period of a sine curve now.
 */
static const uint8_t song_larsen_curve[] PROGMEM = {
    3, 6, 9, 12, 15, 18, 21, 24, 28, 31, 34, 37, 40, 43, 46, 49, 52, 55, 58, 61, 64, 68,
    71, 74, 77, 79, 82, 85, 88, 91, 94, 97, 100, 103, 106, 109, 111, 114, 117, 120, 122,
    125, 128, 131, 133, 136, 139, 141, 144, 146, 149, 151, 154, 156, 159, 161, 164, 166,
    168, 171, 173, 175, 178, 180, 182, 184, 186, 188, 191, 193, 195, 197, 199, 201, 202,
    204, 206, 208, 210, 212, 213, 215, 217, 218, 220, 221, 223, 224, 226, 227, 229, 230,
    231, 233, 234, 235, 236, 237, 239, 240, 241, 242, 243, 244, 244, 245, 246, 247, 248,
    248, 249, 250, 250, 251, 251, 252, 252, 253, 253, 253, 254, 254, 254, 254, 254, 254,
    254, 255, 254, 254, 254, 254, 254, 254, 254, 253, 253, 253, 252, 252, 251, 251, 250,
    250, 249, 248, 248, 247, 246, 245, 244, 244, 243, 242, 241, 240, 239, 237, 236, 235,
    234, 233, 231, 230, 229, 227, 226, 224, 223, 221, 220, 218, 217, 215, 213, 212, 210,
    208, 206, 204, 202, 201, 199, 197, 195, 193, 191, 188, 186, 184, 182, 180, 178, 175,
    173, 171, 168, 166, 164, 161, 159, 156, 154, 151, 149, 146, 144, 141, 139, 136, 133,
    131, 128, 125, 122, 120, 117, 114, 111, 109, 106, 103, 100, 97, 94, 91, 88, 85, 82,
    79, 77, 74, 71, 68, 64, 61, 58, 55, 52, 49, 46, 43, 40, 37, 34, 31, 28, 24, 21, 18,
    15, 12, 9, 6, 3, 0
};

/**
 * Step length for the Larsen curve animation.
 * This will result in 1024ms per sweep.
 */
#define LARSEN_CURVE_STEP 4 /* ms */

static void song_larsen_light(int16_t pos)
{
    for (uint8_t led = 0; led < 5; led++) {
        int16_t offset = sizeof(song_larsen_curve)*(2+led)/4 - pos;
        pwm_set_led(led, 0 <= offset && offset < sizeof(song_larsen_curve)
                             ? pgm_read_byte(&song_larsen_curve[offset]) : 0);
    }

    _delay_ms(LARSEN_CURVE_STEP);
}

void song_play_kitt(void)
{
    keyclick_solenoid_set(false);

    int16_t i;

    /* fade in larsen light */
    for (i = (int16_t)sizeof(song_larsen_curve)/-2; i < 0; i++)
        song_larsen_light(i);

    long unsigned int cur_note = 0;
    uint16_t cur_note_dur = 0;

    int8_t dir = 1;
    for (i = 0; cur_note < sizeof(song_knight_rider)/sizeof(song_knight_rider[0]); i += dir) {
        /*
         * FIXME: Once we freed up timer 0 by rearranging the LED pins,
         * it might be more precise to use timer_read()/timer_elapsed() to wait for the next note.
         */
        if (cur_note_dur < LARSEN_CURVE_STEP) {
            cur_note_dur = pgm_read_word(&song_knight_rider[cur_note].dur);
            pwm_pb5_set_tone(pgm_read_word(&song_knight_rider[cur_note].freq));
        }

        song_larsen_light(i);

        cur_note_dur -= LARSEN_CURVE_STEP;
        if (cur_note_dur < LARSEN_CURVE_STEP)
            cur_note++;

        if (i == sizeof(song_larsen_curve)-1 || (dir < 0 && i == 0))
            dir *= -1;
    }

    pwm_pb5_set_tone(0);

    /* fade out larsen light */
    while (i < sizeof(song_larsen_curve)*3/2)
        song_larsen_light(i++);

    /* we screwed up timer 0 settings and they are also used by the timer module */
    timer_init();

    /* restore the previous lock lights */
    led_set(host_keyboard_leds());
}
