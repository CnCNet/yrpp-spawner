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

#include <Spawner/Spawner.h>
#include <Helpers/Macro.h>
#include <HouseClass.h>

// TODO: Don't show an observer in another observer's panel

DEFINE_HOOK(0x5D69CA, MultiplayerGameMode__5D6890_SetObserverSpawn, 0x7)
{
	return Spawner::Enabled
		? 0x5D69CA + 0x7
		: 0;
}

// Skip score field for all spectators
DEFINE_HOOK(0x5C98E5, MultiplayerScore__5C98A0_SkipObserverScore, 0x6)
{
	GET(HouseClass*, pHouse, EDI);
	return pHouse->GetSpawnPosition() == -1
		? 0x5C9A7E
		: 0;
}

// Use correct colors in diplomacy menu for all spectators
DEFINE_HOOK(0x65838B, RadarClass__658330_SetObserverColorScheme, 0x5)
{
	GET(HouseClass*, pHouse, EBX);
	if (pHouse->GetSpawnPosition() == -1)
	{
		R->EAX(pHouse);
		return 0x65838B + 0x5;
	}

	return 0;
}

// Use correct flag icon in diplomacy menu for all spectators
DEFINE_HOOK(0x65846D, RadarClass__658330_SetObserverFlag, 0x6)
{
	GET(HouseClass*, pHouse, EBX);
	return pHouse->GetSpawnPosition() == -1
		? 0x658480
		: 0;
}

// Slip defeated message to all spectators
DEFINE_HOOK(0x4FC343, HouseClass__MPlayer_Defeated, 0x5)
{
	GET(HouseClass*, pHouse, ESI);
	if (pHouse->GetSpawnPosition() == -1)
	{
		R->EAX(pHouse);
		return 0x4FC343 + 0x5;
	}

	return 0;
}

// For Curent player if all human players are Observers
// ===============================

// Do not end skirmish match if player is Observers
// TODO: Do not end the match if all human players are Observers
DEFINE_HOOK_AGAIN(0x4FCBD0, HouseClass__FlagToWinLose_ObserverPatch, 0x6)
DEFINE_HOOK(0x4FC9E0, HouseClass__FlagToWinLose_ObserverPatch, 0x6)
{
	// GET(HouseClass*, pHouse, ECX);
	return (SessionClass::IsSkirmish() && Game::ObserverMode)
		? 0x4FCDBC
		: 0;
}

// Allow skirmish observer to control gamespeed
// TODO: Allow control speed in skirmish if all human players are Observers
DEFINE_HOOK(0x4E20BA, GameControlsClass__SomeDialog, 0x5)
{
	enum { AllowControlSpeed = 0x4E211A, ForbidControlSpeed = 0x4E20C3 };

	return true || (SessionClass::IsSkirmish() && Game::ObserverMode)
		? AllowControlSpeed
		: 0;
}

// For Curent player
// ===============================

// Use correct flag icon for observer on loading screen in skirmish
DEFINE_HOOK(0x6439F4, ProgressScreenClass__643720, 0x6)
{
	enum { AllowObserver = 0x643A04, NotAllowObserver = 0x643A18 };

	return !SessionClass::IsCampaign()
		? AllowObserver
		: NotAllowObserver;
}

// Use correct loading screen colors for observer in skirmish
DEFINE_HOOK(0x642B60, ProgressScreenClass__LoadTextColor3, 0x5)
{
	enum { AllowObserver = 0x642B6F, NotAllowObserver = 0x642B8B };

	return !SessionClass::IsCampaign()
		? AllowObserver
		: NotAllowObserver;
}

// Use correct observer player color on loading screen in skirmish
DEFINE_HOOK(0x642BC3, ProgressScreenClass__GetPlayerColorSchemes, 0x5)
{
	enum { AllowObserver = 0x642BCD, NotAllowObserver = 0x642BF3 };

	return !SessionClass::IsCampaign()
		? AllowObserver
		: NotAllowObserver;
}

DEFINE_HOOK(0x5533E0, LoadProgressMgr__Draw_SetBackground, 0x5)
{
	return Game::ObserverMode
		? 0x5533EF
		: 0;
}

DEFINE_HOOK(0x5539E4, LoadProgressMgr__Draw_LoadBriefing, 0x5)
{
	return Game::ObserverMode
		? 0x5539F3
		: 0;
}

DEFINE_HOOK(0x5536A0, LoadProgressMgr__Draw_CountryName, 0x5)
{
	return Game::ObserverMode
		? 0x5536AF
		: 0;
}
