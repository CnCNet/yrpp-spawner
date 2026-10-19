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
#include <Surface.h>
#include <ColorScheme.h>
#include <IPXManagerClass.h>
#include <algorithm>

// Fixes text layout in the MPDebug panel
DEFINE_PATCH_TYPED(DWORD, 0x542A19, 312 /* 311 */); // average response time
DEFINE_PATCH_TYPED(DWORD, 0x542AA6, 322 /* 319 */); // maximum response time
DEFINE_PATCH_TYPED(DWORD, 0x542B08, 332 /* 327 */); // packet resend count
DEFINE_PATCH_TYPED(DWORD, 0x542B72, 342 /* 335 */); // lost packet count
DEFINE_PATCH_TYPED(DWORD, 0x542BD4, 352 /* 343 */); // packet loss percentage
DEFINE_PATCH_TYPED(DWORD, 0x542C94, 362 /* 351 */); // player processing time
DEFINE_PATCH_TYPED(DWORD, 0x542CF7, 372 /* 359 */); // player frame number
DEFINE_PATCH_TYPED(DWORD, 0x542D5E, 382 /* 367 */); // send/receive queue lengths
DEFINE_PATCH_TYPED(DWORD, 0x542DC2, 392 /* 375 */); // missed overall/magic packets
DEFINE_PATCH_TYPED(DWORD, 0x542E52, 402 /* 383 */); // routed response time (WOL)

DEFINE_PATCH_TYPED(DWORD, 0x649699, 420 /* 400 */); // frame stall (yellow)
DEFINE_PATCH_TYPED(DWORD, 0x64975D, 420 /* 400 */); // command count stall (red)

DEFINE_PATCH_TYPED(DWORD, 0x54267C, 432 /* 402 */); // retry delta
DEFINE_PATCH_TYPED(DWORD, 0x5426F2, 442 /* 412 */); // retry timeout
DEFINE_PATCH_TYPED(DWORD, 0x542754, 452 /* 422 */); // latency fudge
DEFINE_PATCH_TYPED(DWORD, 0x542810, 462 /* 432 */); // frame sync packets per second


DEFINE_HOOK(0x55F667, MPDebugPrint_DrawEventQueueDelay, 0x5)
{
	const auto delay = *reinterpret_cast<const int*>(0x8373A0);
	if (delay == -1)
		return 0;

	Point2D result;
	const Point2D location { 0, 392 };
	const auto bounds = DSurface::Composite->GetRect();
	Fancy_Text_Print_Wide(result, L"Queue delay: %.2f", DSurface::Composite, bounds,
		location, ColorScheme::Array[ColorScheme::Grey], nullptr,
		TextPrintType::Efnt | TextPrintType::NoShadow, delay / 1000.0);

	return 0;
}
DEFINE_JUMP(LJMP, 0x55F1F8, 0x55F280); // Draw the event queue delay inside the panel, after its background and LNFT.

// Blend the panel background with the map at 80% opacity.
DEFINE_HOOK(0x55F2E5, MPDebugPrint_TranslucentBackground, 0x6)
{
	GET(DSurface*, pSurface, ESI);
	LEA_STACK(RectangleStruct*, pRect, STACK_OFFSET(0x230, -0x218));

	// Up to 8 columns (7 remote players + 1 general statistics column), 100 pixels each.
	const int columnCount = 1 + IPXManagerClass::Instance.NumConnections;
	pRect->Width = std::min(pSurface->GetWidth(), 100 * std::max(2, columnCount));

	ColorStruct color { 0, 0, 0 };
	pSurface->FillRectTrans(pRect, &color, 80);

	return 0;
}
DEFINE_PATCH(0x55F2E2, 0x83, 0xC4, 0x08) // Replace the original FillRect call with ADD ESP, 8 to discard its two arguments.

// Restore the stall indicator area from the composite instead of filling it black.
DEFINE_HOOK(0x649C43, MPDebug_RestoreStallIndicatorBackground, 0x7)
{
	GET(DSurface*, pSurface, ECX);
	RectangleStruct rect { 100, 420 /* 400 */, 540, 4 };
	pSurface->CopyFromPart(&rect, DSurface::Composite, &rect, false, true);

	return 0x649C7D;
}
