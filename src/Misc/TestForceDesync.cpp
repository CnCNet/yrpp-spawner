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

// =============================================================================
//  TEST ONLY -- deliberate desync, for exercising the desync dialog.
//  DELETE THIS FILE (and its <ClCompile> entry in Spawner.vcxproj) when done.
// =============================================================================
//
//  Over frames 100-200, exactly one machine -- the one controlling the human
//  player with the highest array index (every machine agrees on which that is,
//  so it picks a single machine regardless of which slots the humans occupy) --
//  burns an extra synchronized random number each frame. That permanently
//  offsets its in-match RNG, so its game-state CRC diverges from everyone
//  else's and the desync dialog is triggered.
//
//  Hooked in Main_Loop just after the LogicClass::AI call (0x55DCA3), so the
//  extra draw happens during the frame's logic, before its CRC is computed.

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <Unsorted.h>
#include <SessionClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>

namespace
{
	// True on exactly one machine: the one whose local player is the human with
	// the highest array index. Houses are synchronized, so every machine agrees
	// on which house that is, and only its owner returns true.
	bool Is_Designated_Desync_Machine()
	{
		HouseClass* const me = HouseClass::CurrentPlayer;
		if (me == nullptr || !me->IsHumanPlayer)
			return false;

		int highest = -1;
		for (int i = 0; i < HouseClass::Array.Count; i++) {
			HouseClass* const house = HouseClass::Array[i];
			if (house != nullptr && house->IsHumanPlayer && !house->Defeated && house->ArrayIndex > highest)
				highest = house->ArrayIndex;
		}

		return me->ArrayIndex == highest;
	}
}

DEFINE_HOOK(0x55DCA3, MainLoop_TestForceDesync, 0x5)
{
	static int last_frame = -1;
	static bool announced = false;

	const int frame = Unsorted::CurrentFrame;

	// Re-arm for each new game (the frame counter restarts from zero).
	if (frame < last_frame)
		announced = false;
	last_frame = frame;

	if (!SessionClass::IsMultiplayer() || frame < 100 || frame > 200)
		return 0;

	if (!Is_Designated_Desync_Machine())
		return 0;

	if (!announced) {
		announced = true;
		Debug::Log("TEST: desyncing this machine (local house %d) over frames 100-200.\n",
			HouseClass::CurrentPlayer->ArrayIndex);
	}

	// Burn an extra synced random number each frame so this machine's state
	// diverges from everyone else's; the offset persists and is detected.
	ScenarioClass::Instance->Random.Random();
	return 0;
}
