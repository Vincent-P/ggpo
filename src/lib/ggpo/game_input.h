/**
 * Copyright (C) 2025 Vincent Parizet
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
**/
/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _GAMEINPUT_H
#define _GAMEINPUT_H

#include <stdio.h>
#include <memory.h>

// GAMEINPUT_MAX_BYTES * GAMEINPUT_MAX_PLAYERS * 8 must be less than
// 2^BITVECTOR_NIBBLE_SIZE (see bitvector.h)

#define GAMEINPUT_MAX_BYTES      9
#define GAMEINPUT_MAX_PLAYERS    2

struct GameInput {
   enum Constants {
      NullFrame = -1
   };
   int      frame;
   int      size; /* size in bytes of the entire input for all players */
   char     bits[GAMEINPUT_MAX_BYTES * GAMEINPUT_MAX_PLAYERS];

   void init(int frame, char *bits, int size);
   bool value(int i) const { return (bits[i/8] & (1 << (i%8))) != 0; }
   void set(int i) { bits[i/8] |= (1 << (i%8)); }
   void clear(int i) { bits[i/8] &= ~(1 << (i%8)); }
   void erase() { memset(bits, 0, sizeof(bits)); }
   void desc(char *buf, size_t buf_size, bool show_frame = true) const;
   void log(char *prefix, bool show_frame = true) const;
   bool equal(GameInput &input, bool bitsonly = false);
};

#endif
