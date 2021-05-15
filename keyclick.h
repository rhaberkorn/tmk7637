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

#ifndef KEYCLICK_H
#define KEYCLICK_H

#include <stdbool.h>

#include <avr/io.h>

#define KEYCLICK_SOLENOID_EXTENDTIME 15 /* ms */
#define KEYCLICK_BUZZER_TIME 50 /* ms */

enum keyclick_mode {
    KEYCLICK_OFF = 0,
    KEYCLICK_SOLENOID,
    KEYCLICK_BUZZER,
    /** not a real keyclick mode */
    KEYCLICK_MAX
};

extern enum keyclick_mode keyclick_mode;

static inline void
keyclick_solenoid_set(bool state)
{
	if (state)
		PORTB |= (1 << PB3);
	else
	        PORTB &= ~(1 << PB3);
}

#endif
