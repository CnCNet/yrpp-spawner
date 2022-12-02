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

#include <Helpers/Macro.h>
#include <SessionClass.h>
#include <Unsorted.h>

// Allow spectator in skirmish
DEFINE_HOOK_AGAIN(0x4FCBD0, HouseClass_FlagToWinLose_SpectatorPatch, 0x6)
DEFINE_HOOK(0x4FC9E0, HouseClass_FlagToWinLose_SpectatorPatch, 0x6)
{
	return (SessionClass::IsSkirmish() && Game::ObserverMode)
		? 0x4FCDBC
		: 0;
}

// Allow skirmish spectators to control gamespeed
DEFINE_HOOK(0x4E20BA, GameControlsClass_SomeDialog_SpectatorPatch, 0x5)
{
	return (SessionClass::IsSkirmish() && Game::ObserverMode)
		? 0x4E211A
		: 0;
}

DEFINE_HOOK(0x5533E0, LoadProgressMgr_Draw_SpectatorPatch_SetBackground, 0x5)
{
	return Game::ObserverMode
		? 0x5533EF
		: 0;
}

DEFINE_HOOK(0x5539E4, LoadProgressMgr_Draw_SpectatorPatch_LoadBriefing, 0x5)
{
	return Game::ObserverMode
		? 0x5539F3
		: 0;
}

DEFINE_HOOK(0x5536A0, LoadProgressMgr_Draw_SpectatorPatch_CountryName, 0x5)
{
	return Game::ObserverMode
		? 0x5536AF
		: 0;
}
