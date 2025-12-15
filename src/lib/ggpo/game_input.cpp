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

#include "types.h"
#include "game_input.h"
#include "log.h"


void
GameInput::init(int iframe, char *ibits, int isize)
{
   ASSERT(isize);
   ASSERT(isize <= GAMEINPUT_MAX_BYTES * GAMEINPUT_MAX_PLAYERS);
   frame = iframe;
   size = isize;
   memset(bits, 0, sizeof(bits));
   if (ibits) {
      memcpy(bits, ibits, isize);
   }
}

void
GameInput::desc(char *buf, size_t buf_size, bool show_frame) const
{
   ASSERT(size);
   size_t remaining = buf_size;
   if (show_frame) {
      remaining -= sprintf_s(buf, buf_size, "(frame:%d size:%d ", frame, size);
   } else {
      remaining -= sprintf_s(buf, buf_size, "(size:%d ", size);
   }

   for (int i = 0; i < size * 8; i++) {
      char buf2[16];
      if (value(i)) {
         int c = sprintf_s(buf2, ARRAY_SIZE(buf2), "%2d ", i);
         strncat_s(buf, remaining, buf2, ARRAY_SIZE(buf2));
         remaining -= c;
      }
   }
   strncat_s(buf, remaining, ")", 1);
}

void
GameInput::log(char *prefix, bool show_frame) const
{
	char buf[1024];
   size_t c = strlen(prefix);
	strcpy_s(buf, prefix);
	desc(buf + c, ARRAY_SIZE(buf) - c, show_frame);
   strncat_s(buf, ARRAY_SIZE(buf) - strlen(buf), "\n", 1);
	Log(buf);
}

bool
GameInput::equal(GameInput &other, bool bitsonly)
{
   if (!bitsonly && frame != other.frame) {
      Log("frames don't match: %d, %d\n", frame, other.frame);
   }
   if (size != other.size) {
      Log("sizes don't match: %d, %d\n", size, other.size);
   }
   if (memcmp(bits, other.bits, size)) {
      Log("bits don't match\n");
   }
   ASSERT(size && other.size);
   return (bitsonly || frame == other.frame) &&
          size == other.size &&
          memcmp(bits, other.bits, size) == 0;
}

