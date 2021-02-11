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
#include <avr/pgmspace.h>

#include "action_code.h"
#include "unimap_trans.h"

#ifdef KEYMAP_SECTION_ENABLE
const action_t actionmaps[][UNIMAP_ROWS][UNIMAP_COLS] __attribute__ ((section (".keymap.keymaps"))) = {
#else
const action_t actionmaps[][UNIMAP_ROWS][UNIMAP_COLS] PROGMEM = {
#endif
    [0] = UNIMAP_K7637(
        ESC, F1,  F2,  F3,  F4,  F5,  F6,  F7,  F8,  F9,  F10, F11, F12, F13,   PSCR,SLCK,PAUS,       VOLD,VOLU,MUTE,  F19,
        GRV, 1,   2,   3,   4,   5,   6,   7,   8,   9,   0,   MINS,EQL, BSPC,  END, DEL,        NLCK,PSLS,            F20,
        TAB, Q,   W,   E,   R,   T,   Y,   U,   I,   O,   P,   LBRC,RBRC,       HOME,PGUP,       P7,  P8,  P9,  PPLS,  F21,
        CAPS,A,   S,   D,   F,   G,   H,   J,   K,   L,   SCLN,QUOT,NUHS,       ENT, PGDN,       P4,  P5,  P6,  NO,    F22,
        LSFT,NUBS,Z,   X,   C,   V,   B,   N,   M,   COMM,DOT, SLSH,            DOWN,UP,         P1,  P2,  P3,  PENT,  F23,
        NO,       LCTL,               SPC,                     RALT,     NO,    LEFT,RGHT,       P0,  P00, PCMM,NO,    F24),
};
