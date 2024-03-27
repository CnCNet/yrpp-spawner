/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
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
#include <SessionClass.h>
#include <Unsorted.h>

// skip error "А mouse is required for playing Yurts Revenge" - remove the GetSystemMetrics check
DEFINE_JUMP(LJMP, 0x6BD8A4, 0x6BD8C2); // WinMain

// Prevents accidental exit when pressing the spacebar while waiting
// Remove focus from the Leave Game button in the player waiting window
DEFINE_HOOK(0x648CCC, WaitForPlayers_RemoveFocusFromLeaveGameBtn, 0x6)
{
	Imports::SetFocus(0);

	return 0;
}

// A patch to prevent framerate drops when a player spams the 'type select' key
// Skip call GScreenClass::FlagToRedraw(1)
DEFINE_JUMP(LJMP, 0x732CED, 0x732CF9); // End_Type_Select_Command

DEFINE_HOOK(0x649851, WaitForPlayers_OnlineOptimizations, 0x5)
{
	Sleep(3); // Sleep yields the remaining CPU cycle time to any other processes

	return 0x6488B0;
}

// Open campaign briefing when pressing Tab
DEFINE_HOOK(0x55E08F, KeyboardProcess_PressTab, 0x5)
{
	Game::SpecialDialog = SessionClass::IsCampaign() ? 9 : 8;

	return 0x55E099;
}
