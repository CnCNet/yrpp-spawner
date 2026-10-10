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
#include <HouseClass.h>

// Every 120 frames, the game hides cells marked with FlagToShroud.
// Revealing the map on defeat leaves these marks intact, so skip their processing
// for the defeated local player while retaining the visibility recalculation pass.
DEFINE_HOOK(0x578148, MapClass_UpdateShroud_SkipDefeatedPlayer, 0x7)
{
	enum { SkipReshroud = 0x5781A9 };

	if (HouseClass::CurrentPlayer->Defeated)
		return SkipReshroud;

	return 0;
}
