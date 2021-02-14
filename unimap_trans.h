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
#ifndef UNIMAP_TRANS_H
#define UNIMAP_TRANS_H

#include <stdint.h>
#include <avr/pgmspace.h>

#include "unimap.h"

#define UNIMAP_K7637( \
    K29,K3A,K3B,K3C,K3D,K3E,K3F,K40,K41,K42,K43,K44,K45,K68,      K46,K47,K48,      K01,K02,K03,  K6D,K6E, \
    K35,K1E,K1F,K20,K21,K22,K23,K24,K25,K26,K27,K2D,K2E,K2A,      K4A,K4C,      K53,K54,              K6F, \
    K2B,K14,K1A,K08,K15,K17,K1C,K18,K0C,K12,K13,K2F,K30,          K4D,K4B,      K5F,K60,K61,K57,      K70, \
    K39,K04,K16,K07,K09,K0A,K0B,K0D,K0E,K0F,K33,K34,K32,          K28,K4E,      K5C,K5D,K5E,K66,      K71, \
    K79,K64,K1D,K1B,K06,K19,K05,K11,K10,K36,K37,K38,              K51,K52,      K59,K5A,K5B,K58,      K72, \
    K78,    K7A,            K2C,                K7E,    K7C,      K50,K4F,      K62,K55,K63,K67,      K73  \
) UNIMAP( \
            K68, NO, NO, NO, NO,K6D,K6E,K6F,K70,K71,K72,K73,                                     \
    K29,    K3A,K3B,K3C,K3D,K3E,K3F,K40,K41,K42,K43,K44,K45,      K46,K47,K48,      K01,K02,K03, \
    K35,K1E,K1F,K20,K21,K22,K23,K24,K25,K26,K27,K2D,K2E, NO,K2A,   NO,K4A,K4B,  K53,K54,K55, NO, \
    K2B,K14,K1A,K08,K15,K17,K1C,K18,K0C,K12,K13,K2F,K30,     NO,  K4C,K4D,K4E,  K5F,K60,K61,K57, \
    K39,K04,K16,K07,K09,K0A,K0B,K0D,K0E,K0F,K33,K34,    K32,K28,                K5C,K5D,K5E,K66, \
    K79,K64,K1D,K1B,K06,K19,K05,K11,K10,K36,K37,K38,     NO, NO,      K52,      K59,K5A,K5B,K58, \
    K78, NO,K7A, NO,        K2C,         NO, NO,K7E, NO, NO,K7C,  K50,K51,K4F,      K62,K63,K67  \
)

/* Mapping to Universal keyboard layout
 *         ,-----------------------------------------------.
 *         |F13|F14|F15|F16|F17|F18|F19|F20|F21|F22|F23|F24|
 * ,---.   |-----------------------------------------------|     ,-----------.     ,-----------.
 * |Esc|   |F1 |F2 |F3 |F4 |F5 |F6 |F7 |F8 |F9 |F10|F11|F12|     |PrS|ScL|Pau|     |VDn|VUp|Mut|
 * `---'   `-----------------------------------------------'     `-----------'     `-----------'
 * ,-----------------------------------------------------------. ,-----------. ,---------------.
 * |  `|  1|  2|  3|  4|  5|  6|  7|  8|  9|  0|  -|  =|JPY|Bsp| |Ins|Hom|PgU| |NmL|  /|  *|  -|
 * |-----------------------------------------------------------| |-----------| |---------------|
 * |Tab  |  Q|  W|  E|  R|  T|  Y|  U|  I|  O|  P|  [|  ]|  \  | |Del|End|PgD| |  7|  8|  9|  +|
 * |-----------------------------------------------------------| `-----------' |---------------|
 * |CapsL |  A|  S|  D|  F|  G|  H|  J|  K|  L|  ;|  '|  #|Retn|               |  4|  5|  6|KP,|
 * |-----------------------------------------------------------|     ,---.     |---------------|
 * |Shft|  <|  Z|  X|  C|  V|  B|  N|  M|  ,|  ,|  /| RO|Shift |     |Up |     |  1|  2|  3|KP=|
 * |-----------------------------------------------------------| ,-----------. |---------------|
 * |Ctl|Gui|Alt|MHEN|     Space      |HENK|KANA|Alt|Gui|App|Ctl| |Lef|Dow|Rig| |  0    |  .|Ent|
 * `-----------------------------------------------------------' `-----------' `---------------'
 *
 *         ,-----------------------------------------------.
 *         | 68| 69| 6A| 6B| 6C| 6D| 6E| 6F| 70| 71| 72| 73|
 * ,---.   |-----------------------------------------------|     ,-----------.     ,-----------.
 * | 29|   | 3A| 3B| 3C| 3D| 3E| 3F| 40| 41| 42| 43| 44| 45|     | 46| 47| 48|     | 01| 02| 03|
 * `---'   `-----------------------------------------------'     `-----------'     `-----------'
 * ,-----------------------------------------------------------. ,-----------. ,---------------.
 * | 35| 1E| 1F| 20| 21| 22| 23| 24| 25| 26| 27| 2D| 2E| 74| 2A| | 49| 4A| 4B| | 53| 54| 55| 56|
 * |-----------------------------------------------------------| |-----------| |---------------|
 * |   2B| 14| 1A| 08| 15| 17| 1C| 18| 0C| 12| 13| 2F| 30|   31| | 4C| 4D| 4E| | 5F| 60| 61| 57|
 * |-----------------------------------------------------------| `-----------' |---------------|
 * |    39| 04| 16| 07| 09| 0A| 0B| 0D| 0E| 0F| 33| 34| 32|  28|               | 5C| 5D| 5E| 66|
 * |-----------------------------------------------------------|     ,---.     |---------------|
 * |  79| 64| 1D| 1B| 06| 19| 05| 11| 10| 36| 37| 38| 75|    7D|     | 52|     | 59| 5A| 5B| 58|
 * |-----------------------------------------------------------| ,-----------. |---------------|
 * | 78| 7B| 7A|  77|              2C|  76|  00| 7E| 7F| 65| 7C| | 50| 51| 4F| |     62| 63| 67|
 * `-----------------------------------------------------------' `-----------' `---------------'
 *
 * This is a direct adaption of the table in "Serviceschaltplaene", p.3.
 * I tried to keep the geometries as close as possible and support keys that my particular model
 * does not feature.
 * This may have resulted in countrintuitive keycaps, but makes sure that other models will also
 * be supported and makes it easier to work with the keymap editor.
 *
 * NOTE: The first 6 rows in the last column aren't in the original keyboard matrix - they are the
 * result of mapping the "security" key.
 * unimap_trans[0][13] is also an extension and is used to signal "security key" removal
 * (usually mapped to a modifier).
 */
#define NO UNIMAP_NO
const uint8_t PROGMEM unimap_trans[MATRIX_ROWS][MATRIX_COLS] = {
    {0x02, 0x01, 0x48, 0x45, 0x46, 0x44, 0x40, 0x43, 0x3C, 0x29, 0x3D, 0x42, 0x3E, 0x6D, 0x3A, 0x6E},
    {  NO,   NO,   NO, 0x51, 0x47, 0x34, 0x10, 0x38, 0x06,   NO, 0x05, 0x37, 0x11,   NO, 0x1B, 0x6F},
    {0x61, 0x60, 0x5F, 0x2E, 0x4B, 0x2D, 0x23, 0x12, 0x1F, 0x2B, 0x17, 0x25, 0x22, 0x57, 0x1E, 0x70},
    {0x5B, 0x5A, 0x59, 0x4D, 0x52, 0x30, 0x0D, 0x33, 0x07, 0x64, 0x09, 0x0E, 0x0B, 0x58, 0x14, 0x71},
    {0x5E, 0x5D, 0x5C, 0x2A, 0x4E, 0x2F, 0x18, 0x13, 0x08, 0x04, 0x15, 0x0C, 0x1C, 0x66, 0x1A, 0x72},
    {0x63, 0x55, 0x62, 0x50, 0x4F, 0x7C,   NO, 0x7E, 0x2C, 0x7A,   NO,   NO,   NO, 0x67, 0x78, 0x73},
    {0x03, 0x54, 0x53, 0x68, 0x4C, 0x27, 0x24, 0x26, 0x20,   NO, 0x21, 0x41, 0x3F,   NO, 0x3B, 0x35},
    {  NO,   NO,   NO, 0x28, 0x4A, 0x32, 0x36, 0x39, 0x19, 0x1D, 0x0A, 0x0F,   NO,   NO, 0x16, 0x79}
};
#undef NO

#endif
