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

#pragma region HouseClass is Observer

// Skip set spawn point for all observers
DEFINE_HOOK(0x5D69BF, MPGameMode__AssignStartingPositions_SetObserverSpawn, 0x5)
{
	return Spawner::Enabled
		? 0x5D69D1
		: 0;
}

// Skip score field for all observers
DEFINE_HOOK(0x5C98E5, MultiplayerScore__5C98A0_SkipObserverScore, 0x6)
{
	GET(HouseClass*, pHouse, EDI);
	return pHouse->IsInitiallyObserver()
		? 0x5C9A7E
		: 0;
}

// Use correct colors in diplomacy menu for all observers
DEFINE_HOOK(0x65838B, RadarClass__658330_SetObserverColorScheme, 0x5)
{
	GET(HouseClass*, pHouse, EBX);
	if (pHouse->IsInitiallyObserver())
	{
		R->EAX(pHouse);
		return 0x65838B + 0x5;
	}

	return 0;
}

// Use correct flag icon in diplomacy menu for all observers
DEFINE_HOOK(0x65846D, RadarClass__658330_SetObserverFlag, 0x6)
{
	GET(HouseClass*, pHouse, EBX);
	return pHouse->IsInitiallyObserver()
		? 0x658480
		: 0;
}

// Skip defeated message to all observers
DEFINE_HOOK(0x4FC343, HouseClass__MPlayer_Defeated, 0x5)
{
	GET(HouseClass*, pHouse, ESI);
	if (pHouse->IsInitiallyObserver())
	{
		R->EAX(pHouse);
		return 0x4FC343 + 0x5;
	}

	return 0;
}

#pragma endregion HouseClass is Observer

// Do not end skirmish match if player is Observers
// TODO: Do not end the match if all human players is Observer
DEFINE_HOOK_AGAIN(0x4FCBD0, HouseClass__FlagToWinLose_ObserverPatch, 0x6)
DEFINE_HOOK(0x4FC9E0, HouseClass__FlagToWinLose_ObserverPatch, 0x6)
{
	// GET(HouseClass*, pHouse, ECX);
	return (SessionClass::IsSkirmish() && Game::ObserverMode)
		? 0x4FCDBC
		: 0;
}

// Allow skirmish observer to control gamespeed
// TODO: Allow control speed in skirmish if all human players is Observer
DEFINE_HOOK(0x4E20BA, GameControlsClass__SomeDialog, 0x5)
{
	enum { AllowControlSpeed = 0x4E211A, ForbidControlSpeed = 0x4E20C3 };

	return (SessionClass::IsSkirmish() && Game::ObserverMode)
		? AllowControlSpeed
		: 0;
}

#pragma region Curent player is Observer

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

#pragma endregion Curent player is Observer

#pragma region Observer in Skirmish

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

// Enable observer sidebar bar in skirmish
DEFINE_HOOK(0x6A557A, SidebarClass__InitIO, 0x5)
{
	enum { AllowObserver = 0x6A558D, NotAllowObserver = 0x6A5830 };

	return !SessionClass::IsCampaign()
		? AllowObserver
		: NotAllowObserver;
}

#pragma endregion Curent player is Observer

#pragma region Show house on Observer sidebar
bool inline ShowHouseOnObserverSidebar(HouseClass* pHouse)
{
	if (pHouse->Type->MultiplayPassive)
		return false;

	const bool bShowAI = (Spawner::Enabled && Spawner::GetConfig()->ObserverSidebar_ShowAI);
	if (!bShowAI && !pHouse->IsHumanPlayer)
		return false;

	if (pHouse->IsInitiallyObserver())
		return false;

	return true;
}

DEFINE_HOOK(0x6A55AB, SidebarClass__InitIO_ShowHouseOnObserverSidebar1, 0xA)
{
	enum { Draw = 0x6A55C8, DontDraw = 0x6A55CF };
	GET(HouseClass*, pHouse, EAX);

	return ShowHouseOnObserverSidebar(pHouse)
		? Draw
		: DontDraw;
}

DEFINE_HOOK(0x6A57E2, SidebarClass__InitIO_ShowHouseOnObserverSidebar2, 0xA)
{
	enum { Draw = 0x6A57FF, DontDraw = 0x6A580E };
	GET(HouseClass*, pHouse, EAX);

	return ShowHouseOnObserverSidebar(pHouse)
		? Draw
		: DontDraw;
}

#pragma endregion Show house on Observer sidebar
