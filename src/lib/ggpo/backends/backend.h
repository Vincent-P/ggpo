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

#ifndef _BACKEND_H
#define _BACKEND_H

#include "ggponet.h"
#include "types.h"

struct GGPOSession {
	virtual ~GGPOSession() {}
	virtual GGPOErrorCode DoPoll(int timeout) { return GGPO_OK; }
	virtual GGPOErrorCode AddPlayer(GGPOPlayer* player, GGPOPlayerHandle* handle) = 0;
	virtual GGPOErrorCode AddLocalInput(GGPOPlayerHandle player, void* values, int size) = 0;
	virtual GGPOErrorCode SyncInput(void* values, int size, int* disconnect_flags) = 0;
	virtual GGPOErrorCode IncrementFrame(void) { return GGPO_OK; }
	virtual GGPOErrorCode DisconnectPlayer(GGPOPlayerHandle handle) { return GGPO_OK; }
	virtual GGPOErrorCode GetNetworkStats(GGPONetworkStats* stats, GGPOPlayerHandle handle) { return GGPO_OK; }
	virtual GGPOErrorCode Logv(const char* fmt, va_list list) { ::Logv(fmt, list); return GGPO_OK; }

	virtual GGPOErrorCode SetFrameDelay(GGPOPlayerHandle player, int delay) { return GGPO_ERRORCODE_UNSUPPORTED; }
	virtual GGPOErrorCode SetDisconnectTimeout(int timeout) { return GGPO_ERRORCODE_UNSUPPORTED; }
	virtual GGPOErrorCode SetDisconnectNotifyStart(int timeout) { return GGPO_ERRORCODE_UNSUPPORTED; }
};

#endif

