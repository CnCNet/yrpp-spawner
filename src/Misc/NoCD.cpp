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

#include "NoCD.h"
#include <GetCDClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

// Based on https://github.com/Ares-Developers/Ares/blob/4f1d929920aca31924c6cd4d3dfa849daa65252a/src/Misc/CopyProtection.cpp
//      and https://github.com/Ares-Developers/Ares/blob/4f1d929920aca31924c6cd4d3dfa849daa65252a/src/Ares.cpp#L155
//      and https://github.com/Ares-Developers/Ares/blob/4f1d929920aca31924c6cd4d3dfa849daa65252a/src/Ares.cpp#L323
//      and https://github.com/CnCNet/yr-patches/blob/1f2a00de8226823952763fa411aa5e75dbb4012a/src/no-cd.asm

bool NoCD::Enabled = false;

void NoCD::Apply()
{
	if (NoCD::Enabled)
		return;

	Patch::Apply_RAW(0x4A80D0, // GetCDIndex
	{
		0xB8, 0x02, 0x00, 0x00, 0x00, // mov    eax, 2
		0xC3                          // retn
	});

	Patch::Apply_RAW(0x4790E0, // CD_IsDiskAvailable
	{
		0xB0, 0x01,      // mov    al, 1
		0xC2, 0x04, 0x00 // retn   4
	});

	Patch::Apply_RAW(0x479110, // CD_RequestDisk
	{
		0xB0, 0x01,      // mov    al, 1
		0xC2, 0x04, 0x00 // retn   4
	});

	{ // If NoCD mode is enabled, do not read files from the hard disk mimicking the CD-Rom
		Patch::Apply_LJMP(0x47AE26, 0x47AEF0); // CDFileClass_SetFileName
		Patch::Apply_LJMP(0x47B020, 0x47B0AE); // FileFindOpen
	}

	{ // Skip checking of secondary MIX files if NoCD mode is enabled
		Patch::Apply_LJMP(0x530BF8, 0x530C09); // InitSecondaryMIXes_SkipMulti
		Patch::Apply_LJMP(0x530B67, 0x530B76); // InitSecondaryMIXes_SkipMaps
		Patch::Apply_RAW(0x531289,             // InitSecondaryMIXes_SkipMovies
		{
			0xB0, 0x01, // mov    al, 1
			0x90		// nop
		});
	}

	// CD switch behavior changed to support true No-CD mode with no need to still have a drive
	// Also, this should improve performance considerably when no disc is inserted into any CD drive
	Debug::Log("[Spawner] Optimizing list of CD drives for NoCD mode.\n");
	memset(GetCDClass::Instance->Drives, -1, 26);

	char drv[] = "a:\\";
	for (int i = 0; i < 26; ++i)
	{
		drv[0] = 'a' + (i + 2) % 26;
		if (GetDriveTypeA(drv) == DRIVE_FIXED)
		{
			GetCDClass::Instance->Drives[0] = (i + 2) % 26;
			GetCDClass::Instance->Count = 1;
			break;
		}
	}

	NoCD::Enabled = true;
};

void NoCD::InitNoCDMode()
{
	if (!GetCDClass::Instance->Count)
	{
		Debug::Log("[Spawner] No CD drives detected. Switching to NoCD mode.\n");
		NoCD::Apply();
	}
}
