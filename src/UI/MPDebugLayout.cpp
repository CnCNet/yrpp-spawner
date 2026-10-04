/**
*  yrpp-spawner
*
*  Copyright(C) 2026-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include <Utilities/Macro.h>
#include <Unsorted.h>

// Fixes text layout in the MPDebug panel
DEFINE_PATCH_TYPED(DWORD, 0x542A19, 312 /* 311 */ ); // average response time
DEFINE_PATCH_TYPED(DWORD, 0x542AA6, 322 /* 319 */ ); // maximum response time
DEFINE_PATCH_TYPED(DWORD, 0x542B08, 332 /* 327 */ ); // packet resend count
DEFINE_PATCH_TYPED(DWORD, 0x542B72, 342 /* 335 */ ); // lost packet count
DEFINE_PATCH_TYPED(DWORD, 0x542BD4, 352 /* 343 */ ); // packet loss percentage
DEFINE_PATCH_TYPED(DWORD, 0x542C94, 362 /* 351 */ ); // player processing time
DEFINE_PATCH_TYPED(DWORD, 0x542CF7, 372 /* 359 */ ); // player frame number
DEFINE_PATCH_TYPED(DWORD, 0x542D5E, 382 /* 367 */ ); // send/receive queue lengths
DEFINE_PATCH_TYPED(DWORD, 0x542DC2, 392 /* 375 */ ); // missed overall/magic packets
DEFINE_PATCH_TYPED(DWORD, 0x542E52, 402 /* 383 */ ); // routed response time (WOL)
DEFINE_PATCH_TYPED(DWORD, 0x649699, 420 /* 400 */ ); // frame stall (yellow)
DEFINE_PATCH_TYPED(DWORD, 0x64975D, 420 /* 400 */ ); // command count stall (red)
DEFINE_PATCH_TYPED(DWORD, 0x649C60, 420 /* 400 */ ); // clear stall indicators

// Hide the event queue delay together with the multiplayer debug statistics.
DEFINE_HOOK(0x55F1F8, MPDebugPrint_CheckDrawFlag, 0x8)
{
	enum { SkipEventQueueDelay = 0x55F280 };

	return Game::DrawMPDebugStats
		? 0
		: SkipEventQueueDelay;
}
