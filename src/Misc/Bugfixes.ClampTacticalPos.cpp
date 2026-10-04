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
#include <TacticalClass.h>

// Fixes glitches if the map size is smaller than the screen resolution.
// Author: Belonit, ZivDero
static constexpr float paddingTopInCell = 5;
static constexpr float paddingBottomInCell = 4.5;

// Keep the raw limits for drawing the map edges.
// On a small map, maximum is below minimum.
// The camera uses their midpoint.
static void Tactical_PositionLimits(Point2D& minimum, Point2D& maximum)
{
	const auto& mapRect = MapClass::Instance.MapRect;
	const auto& visibleRect = MapClass::Instance.VisibleRect;
	const auto& view = DSurface::ViewBounds;

	minimum.X = view.Width / 2 + (Unsorted::CellWidthInPixels / 2) * (visibleRect.X * 2 - mapRect.Width);
	maximum.X = minimum.X + Unsorted::CellWidthInPixels * visibleRect.Width - view.Width;

	minimum.Y = view.Height / 2 + (Unsorted::CellHeightInPixels / 2) * (visibleRect.Y * 2 + mapRect.Width - int(paddingTopInCell));
	maximum.Y = minimum.Y + Unsorted::CellHeightInPixels * visibleRect.Height - view.Height
		+ int(Unsorted::CellHeightInPixels * paddingBottomInCell);
}

bool __fastcall Tactical_ClampTacticalPos(TacticalClass* pThis, void*, Point2D* tacticalPos)
{
	Point2D minimum;
	Point2D maximum;
	Tactical_PositionLimits(minimum, maximum);

	if (maximum.X < minimum.X)
		minimum.X = maximum.X = (minimum.X + maximum.X) / 2;

	if (maximum.Y < minimum.Y)
		minimum.Y = maximum.Y = (minimum.Y + maximum.Y) / 2;

	const auto previous = *tacticalPos;
	tacticalPos->X = Math::max(minimum.X, Math::min(tacticalPos->X, maximum.X));
	tacticalPos->Y = Math::max(minimum.Y, Math::min(tacticalPos->Y, maximum.Y));
	return tacticalPos->X != previous.X || tacticalPos->Y != previous.Y;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6D8640, Tactical_ClampTacticalPos)

DEFINE_HOOK(0x6D4934, Tactical_Render_OverlapForeignMap, 0x6)
{
	Point2D minimum;
	Point2D maximum;
	Tactical_PositionLimits(minimum, maximum);

	const auto& view = DSurface::ViewBounds;
	const auto& position = TacticalClass::Instance->TacticalPos;
	const int left = view.X + minimum.X - view.Width / 2 - position.X;
	const int right = view.X + maximum.X + (view.Width - view.Width / 2) - position.X;
	const int top = view.Y + minimum.Y - view.Height / 2 - position.Y;
	const int bottom = view.Y + maximum.Y + (view.Height - view.Height / 2) - position.Y;

	if (left > view.X)
	{
		RectangleStruct rect = { view.X, view.Y, left - view.X, view.Height };
		DSurface::Composite->FillRect(&rect, COLOR_BLACK);
	}

	if (right < view.X + view.Width)
	{
		RectangleStruct rect = { right, view.Y, view.X + view.Width - right, view.Height };
		DSurface::Composite->FillRect(&rect, COLOR_BLACK);
	}

	if (top > view.Y)
	{
		RectangleStruct rect = { view.X, view.Y, view.Width, top - view.Y };
		DSurface::Composite->FillRect(&rect, COLOR_BLACK);
	}

	if (bottom < view.Y + view.Height)
	{
		RectangleStruct rect = { view.X, bottom, view.Width, view.Y + view.Height - bottom };
		DSurface::Composite->FillRect(&rect, COLOR_BLACK);
	}

	return 0;
}
