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

#ifndef PWM_H
#define PWM_H

#include <stdint.h>

void pwm_pd1_set_led(uint8_t brightness);
void pwm_pb4_set_led(uint8_t brightness);
void pwm_pb5_set_led(uint8_t brightness);
void pwm_pb6_set_led(uint8_t brightness);
void pwm_pb7_set_led(uint8_t brightness);

void pwm_set_led(uint8_t led, uint8_t brightness);

void pwm_pd0_set_tone(uint16_t freq);

#endif
