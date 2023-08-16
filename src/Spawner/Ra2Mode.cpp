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

#include "Ra2Mode.h"

#include <Spawner/Spawner.h>
#include <Utilities/Macro.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <LoadOptionsClass.h>

bool Ra2Mode::Enabled = false;

void Ra2Mode::Apply()
{
	if (Ra2Mode::Enabled)
		return;

	Ra2Mode::Enabled = true;

	// Window title
	Patch::Apply_RAW(0x849F48, "Red Alert 2");

	// Allow buildings to be selected with the type command/key in RA2 mode
	Patch::Apply_RAW(0x732C59, { 0x38 }); // 0x3C, replace call Is_Selectable2 to Is_Selectable1
	Patch::Apply_RAW(0x732C30, { 0x53, 0x8B, 0xDA, 0x55, 0x56 }); // Disable Ares hook TechnoClass_IDMatches

	// Allow units to be selected when they are inside/on a building
	Patch::Apply_LJMP(0x6FC05D, 0x6FC080);

	// Allow Dogs & Flak Troopers to get a speed boost
	// Skip Crawls check on InfantryType
	// https://github.com/CnCNet/yr-patches/issues/15
	Patch::Apply_LJMP(0x51D77A, 0x51D793);

	// game expects there to be 3 baseunits
	Patch::Apply_LJMP(0x4F8EEE, 0x4F8EFE);
	Patch::Apply_LJMP(0x4F8CEC, 0x4F8DB1);

	// Don't show minimap on Load screen
	Patch::Apply_LJMP(0x553686, 0x553686 + 6);

	{ // Use classic border style for UI items
		// GroupBoxCtrl
		Patch::Apply_LJMP(0x61EC64, 0x61EC75);
		Patch::Apply_OFFSET(0x61E8AB + 1, 0xAC4624);
		Patch::Apply_OFFSET(0x61E8B0 + 2, 0xAC4624);

		// DrawUI_DrawBorder
		Patch::Apply_LJMP(0x620A89, 0x620A9B);
		Patch::Apply_LJMP(0x620B06, 0x620B16);
		Patch::Apply_LJMP(0x620B7D, 0x620C82);
		Patch::Apply_RAW(0x620A29,
		{
			0xB9, 0x01, 0x00, 0x00, 0x00, // mov ecx, 1
			0x90, 0x90                    // nops
		});
	}

	{ // Skip mixes
		// EXPANDMD%02d.MIX
		Patch::Apply_LJMP(0x5301BA, 0x530290);

		// CAMEOMD.MIX
		Patch::Apply_LJMP(0x530680, 0x530709);
		Patch::Apply_LJMP(0x5316C9, 0x5316CF);

		// LOADMD.MIX
		Patch::Apply_LJMP(0x552CDE, 0x552D0F);
	}

	{ // Change mixes priority
		Patch::Apply_RAW(0x826638, "LOCALMD.MIX"); // LOCAL.MIX
		Patch::Apply_RAW(0x826644, "LOCAL.MIX");   // LOCALMD.MIX
	}

	{ // Rename files
		Patch::Apply_RAW(0x816280, "AUDIO.MIX");     // AUDIOMD.MIX
		Patch::Apply_RAW(0x81C24C, "THEME.MIX");     // THEMEMD.MIX
		Patch::Apply_RAW(0x81C284, "MULTI.MIX");     // MULTIMD.MIX
		Patch::Apply_RAW(0x81C210, "MOVIES%02d.MIX");// MOVMD%02d.MIX
		Patch::Apply_RAW(0x81C2EC, "MAPS%02d.MIX");  // MAPSMD%02d.MIX

		Patch::Apply_RAW(0x825D94, "THEME.INI");     // THEMEMD.INI
		Patch::Apply_RAW(0x825DF0, "EVA.INI");       // EVAMD.INI
		Patch::Apply_RAW(0x825E50, "SOUND.INI");     // SOUNDMD.INI
		Patch::Apply_RAW(0x826198, "BATTLE.INI");    // BATTLEMD.INI
		Patch::Apply_RAW(0x82621C, "AI.INI");        // AIMD.INI
		Patch::Apply_RAW(0x830A18, "MPModes.ini");   // MPModesMD.ini
		Patch::Apply_RAW(0x839724, "MISSION.INI");   // MISSIONMD.INI
	}
}

bool Ra2Mode::CheckSaveGameID(const char* saveGameName)
{
	if (saveGameName[0])
	{
		SavegameInformation savegameInfo;
		if (SavegameInformation::ReadFromFile(saveGameName, &savegameInfo))
		{
			if (savegameInfo.Version == Main::GetConfig()->RA2ModeSaveID)
				return true;
		}
	}

	return false;
}

// Allow allies to repair on service depot
DEFINE_HOOK(0x700594, TechnoClass_WhatAction__AllowAlliesRepair, 0x5)
{
	if (!Ra2Mode::IsEnabled())
		return 0;

	GET(TechnoClass*, pThis, ESI);
	GET(ObjectClass*, pObject, EDI);

	auto const pBuilding = abstract_cast<BuildingClass* const>(pObject);
	auto const pBuildingOwner = pBuilding ? pBuilding->Owner : nullptr;

	return (pBuildingOwner && pBuildingOwner->IsAlliedWith(pThis))
		? 0x70059D
		: 0x7005E6;
}
