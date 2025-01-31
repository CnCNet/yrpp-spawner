/**
*  yrpp-spawner
*
*  Copyright(C) 2025-present CnCNet
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
#include <Spawner/Spawner.h>
#include <DisplayClass.h>
class TacticalClass;

// Fixes glitches if the map size is smaller than the screen resolution
// Author: Belonit

bool __fastcall Tactical_ClampTacticalPos(TacticalClass* pThis, void*, Point2D* tacticalPos)
{
	bool isUpdated = false;

	auto pMapRect = &MapClass::Instance->MapRect;
	auto pMapVisibleRect = &MapClass::Instance->VisibleRect;
	auto pSurfaceViewBounds = &DSurface::ViewBounds;

	{
		const int xMin = Unsorted::CellWidthInPixels * (pMapVisibleRect->X - (pMapRect->Width >> 1)) + (pSurfaceViewBounds->Width >> 1);
		if (tacticalPos->X < xMin)
		{
			tacticalPos->X = xMin;
			isUpdated = true;
		}
		else
		{
			const int xMax = Math::max(Unsorted::CellWidthInPixels * pMapVisibleRect->Width - pSurfaceViewBounds->Width + xMin, xMin);
			if (tacticalPos->X > xMax)
			{
				tacticalPos->X = xMax;
				isUpdated = true;
			}
		}
	}

	{
		const int yMin = Unsorted::CellHeightInPixels * (pMapVisibleRect->Y + (pMapRect->Width >> 1)) + (pSurfaceViewBounds->Height >> 1) - int(2.5 * Unsorted::CellHeightInPixels);
		if (tacticalPos->Y < yMin)
		{
			tacticalPos->Y = yMin;
			isUpdated = true;
		}
		else
		{
			const int yMax = Math::max(Unsorted::CellHeightInPixels * pMapVisibleRect->Height - pSurfaceViewBounds->Height + int(4.5 * Unsorted::CellHeightInPixels) + yMin, yMin);
			if (tacticalPos->Y > yMax)
			{
				tacticalPos->Y = yMax;
				isUpdated = true;
			}
		}
	}

	return isUpdated;
}
DEFINE_JUMP(LJMP, 0x6D8640, GET_OFFSET(Tactical_ClampTacticalPos))

DEFINE_HOOK(0x6D4934, Tactical_Render_OverlapForeignMap, 0x6)
{
	auto pMapVisibleRect = &MapClass::Instance->VisibleRect;
	auto pSurfaceViewBounds = &DSurface::ViewBounds;

	{
		const int maxWidth =
			pSurfaceViewBounds->Width - pMapVisibleRect->Width * Unsorted::CellWidthInPixels;

		if (maxWidth > 0)
		{
			RectangleStruct rect = {
				pSurfaceViewBounds->Width - maxWidth,
				0,
				maxWidth,
				pSurfaceViewBounds->Height};

			DSurface::Composite->FillRect(&rect, COLOR_BLACK);
		}
	}

	{
		const int maxHeight = pSurfaceViewBounds->Height - pMapVisibleRect->Height * Unsorted::CellHeightInPixels - int(4.5 * Unsorted::CellHeightInPixels);

		if (maxHeight > 0)
		{
			RectangleStruct rect = {
				0,
				pSurfaceViewBounds->Height - maxHeight,
				pSurfaceViewBounds->Width,
				maxHeight};

			DSurface::Composite->FillRect(&rect, COLOR_BLACK);
		}
	}

	return 0;
}
