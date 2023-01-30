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

#include "Spawner.h"
#include "Nethack.h"

#include <Utilities/Macro.h>

DEFINE_DYNAMIC_JUMP(CALL, MainGame_SelectGame1, 0x48CDD3, GET_OFFSET(Spawner::StartGame));
DEFINE_DYNAMIC_JUMP(CALL, MainGame_SelectGame2, 0x48CFAA, GET_OFFSET(Spawner::StartGame));

DEFINE_DYNAMIC_JUMP(CALL, AssignHouses_DoSpawnerStuff1, 0x68745E, GET_OFFSET(Spawner::AssignHouses));
DEFINE_DYNAMIC_JUMP(CALL, AssignHouses_DoSpawnerStuff2, 0x68ACFF, GET_OFFSET(Spawner::AssignHouses));

DEFINE_DYNAMIC_JUMP(CALL, UDPInterfaceClass_MessageHandler_SendTo, 0x7B3D75, GET_OFFSET(Nethack::SendTo));
DEFINE_DYNAMIC_JUMP(CALL, UDPInterfaceClass_MessageHandler_RecvFrom, 0x7B3EEC, GET_OFFSET(Nethack::RecvFrom));

// Skip Intro, EA_WWLOGO and Loadscreen
DEFINE_DYNAMIC_JUMP(LJMP, InitIntro_Skip, 0x52CB50, 0x52CB6E);
DEFINE_DYNAMIC_JUMP(LJMP, InitGame_SkipLogoAndLoadscreen, 0x52C5E0, 0x52C5F8);

DEFINE_HOOK(0x52BA78, InitGame_Before, 0x5)
{
	if (Spawner::Enabled)
	{
		Spawner::Init();

		MainGame_SelectGame1->Apply();
		MainGame_SelectGame2->Apply();

		AssignHouses_DoSpawnerStuff1->Apply();
		AssignHouses_DoSpawnerStuff2->Apply();

		UDPInterfaceClass_MessageHandler_SendTo->Apply();
		UDPInterfaceClass_MessageHandler_RecvFrom->Apply();

		InitIntro_Skip->Apply();
		InitGame_SkipLogoAndLoadscreen->Apply();
	}

	return 0;
}

#pragma region HousesStuff
DEFINE_HOOK(0x5D74A0, MPGameModeClass_AllyTeams, 0x7)
{
	return Spawner::Enabled
		? 0x5D7570
		: 0;
}

DEFINE_HOOK(0x501721, HouseClass_ComputerParanoid, 0x5)
{
	return Spawner::Enabled
		? 0x501736
		: 0;
}

DEFINE_HOOK(0x686A9E, RemoveAIPlayers, 0x6)
{
	return Spawner::Enabled
		? 0x686AC6
		: 0;
}
#pragma endregion HousesStuff

#pragma region Cooperative
DEFINE_HOOK(0x553317, LoadProgressMgr_Draw_CooperativeDescription, 0x6)
{
	if (Spawner::Enabled)
		R->ECX(0);

	return 0;
}

#include <SessionClass.h>
DEFINE_HOOK(0x55D0CB, AuxLoop_Cooperative_EndgameCrashFix, 0x6)
{
	if (!Spawner::Enabled)
		return 0;

	auto pGameMod = R->ECX<void*>();
	if (pGameMod && SessionClass::IsMultiplayer())
		return 0x55D0E8; // original = 0x55D0DF

	return 0x55D0ED;
}
#pragma endregion Cooperative

DEFINE_HOOK(0x6843C6, Scenario_LoadWait_SetConnTimeout, 0x5)
{
	if (!Spawner::Enabled)
		return 0;

	R->ECX(Spawner::GetConfig()->ConnTimeout);
	return 0x6843CB;
}

DEFINE_HOOK(0x658117, DiplomacyDialog_ModeScenarioDescriptions, 0x5)
{
	GET(HWND, hDlg, ESI);

	if (SessionClass::Instance->IsSkirmish() || SessionClass::Instance->IsMultiplayer())
	{
		HWND hItem = GetDlgItem(hDlg, 1062);
		wchar_t modeName[256];
		wsprintfW(modeName, L"%hs", Spawner::GetConfig()->UIGameMode);
		SendMessageA(hItem, WW_STATIC_SETTEXT, 0, reinterpret_cast<LPARAM>(modeName));
	}

	HWND hItem = GetDlgItem(hDlg, 1819);
	wchar_t scenarioName[256];
	wsprintfW(scenarioName, L"%hs", Spawner::GetConfig()->UIMapName);
	SendMessageA(hItem, WW_STATIC_SETTEXT, 0, reinterpret_cast<LPARAM>(scenarioName));

	return 0x658168;
}
