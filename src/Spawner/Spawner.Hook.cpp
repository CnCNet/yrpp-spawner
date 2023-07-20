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

#include <SessionClass.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x52BA78, InitGame_Before, 0x5)
{
	if (Spawner::Enabled)
	{
		Spawner::Init();

		Patch::Apply_CALL(0x48CDD3, Spawner::StartGame); // Main_Game
		Patch::Apply_CALL(0x48CFAA, Spawner::StartGame); // Main_Game

		{ // HousesStuff
			Patch::Apply_CALL(0x68745E, Spawner::AssignHouses); // Read_Scenario_INI
			Patch::Apply_CALL(0x68ACFF, Spawner::AssignHouses); // ScenarioClass::Read_INI

			Patch::Apply_LJMP(0x5D74A0, 0x5D7570); // MPGameModeClass_AllyTeams
			Patch::Apply_LJMP(0x501721, 0x501736); // HouseClass_ComputerParanoid
			Patch::Apply_LJMP(0x686A9E, 0x686AC6); // RemoveAIPlayers
		}

		{ // Nethack
			Patch::Apply_CALL(0x7B3D75, Nethack::SendTo);   // UDPInterfaceClass::Message_Handler
			Patch::Apply_CALL(0x7B3EEC, Nethack::RecvFrom); // UDPInterfaceClass::Message_Handler
		}

		{ // Skip Intro, EA_WWLOGO and Loadscreen
			Patch::Apply_LJMP(0x52CB50, 0x52CB6E); // InitIntro_Skip
			Patch::Apply_LJMP(0x52C5E0, 0x52C5F8); // InitGame_SkipLogoAndLoadscreen
		}

		{ // Cooperative
			Patch::Apply_LJMP(0x553321, 0x5533C5); // LoadProgressMgr_Draw_CooperativeDescription
			Patch::Apply_LJMP(0x55D0DF, 0x55D0E8); // AuxLoop_Cooperative_EndgameCrashFix
		}

		// Set ConnTimeout
		Patch::Apply_TYPED<int>(0x6843C7, { Spawner::GetConfig()->ConnTimeout }); //  Scenario_Load_Wait

		{ // Add support unicode player name in ingame chat
			Patch::Apply_RAW(0x48D930, { 0x8B, 0xC1, 0x90, 0x90, 0x90 }); // mov eax, ecx
			Patch::Apply_RAW(0x55F0AD, { 0x8B, 0xC1, 0x90, 0x90, 0x90 }); // mov eax, ecx
		}

		// Show GameMode in DiplomacyDialog in Skirmish
		Patch::Apply_LJMP(0x658117, 0x658126);  // RadarClass_DiplomacyDialog
	}

	return 0;
}

// Add support unicode player name in ingame chat
DEFINE_HOOK(0x55EDD2, MessageInput_UnicodePlayerName, 0x5)
{
	if (!Spawner::Enabled)
		return 0;

	wcscpy(reinterpret_cast<wchar_t*>(0xA8D63C), NodeNameType::Array->GetItem(0)->Name);
	return 0x55EE00;
}

// Display UIGameMode if is set
// Otherwise use mode name from MPModesMD.ini
DEFINE_HOOK(0x65813E, RadarClass__DiplomacyDialog_UIGameMode, 0x9)
{
	if (Spawner::Enabled && Spawner::GetConfig()->UIGameMode[0])
		R->EAX(Spawner::GetConfig()->UIGameMode);

	return 0;
}

// Clear UIGameMode on game load
DEFINE_HOOK(0x689669, ScenarioClass_Load_Suffix, 0x6)
{
	if (Spawner::Enabled)
		Spawner::GetConfig()->UIGameMode[0] = 0;

	return 0;
}
