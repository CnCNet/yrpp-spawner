#include "NoCD.h"
#include <GetCDClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

// Based on https://github.com/Ares-Developers/Ares/blob/4f1d929920aca31924c6cd4d3dfa849daa65252a/src/Misc/CopyProtection.cpp
//      and https://github.com/Ares-Developers/Ares/blob/4f1d929920aca31924c6cd4d3dfa849daa65252a/src/Ares.cpp#L155
//      and https://github.com/Ares-Developers/Ares/blob/4f1d929920aca31924c6cd4d3dfa849daa65252a/src/Ares.cpp#L323
//      and https://github.com/CnCNet/yr-patches/blob/1f2a00de8226823952763fa411aa5e75dbb4012a/src/no-cd.asm

DEFINE_DYNAMIC_PATCH(GetCDIndex, 0x4A80D0,
	0xB8, 0x02, 0x00, 0x00, 0x00,    // mov    eax, 2
	0xC3);                           // retn

DEFINE_DYNAMIC_PATCH(CD_IsDiskAvailable, 0x4790E0,
	0xB0, 0x01,                      // mov    al, 1
	0xC2, 0x04, 0x00);               // retn   4

DEFINE_DYNAMIC_PATCH(CD_RequestDisk, 0x479110,
	0xB0, 0x01,                      // mov    al, 1
	0xC2, 0x04, 0x00);               // retn   4

// If NoCD mode is enabled, do not read files from the hard disk mimicking the CD-Rom
DEFINE_DYNAMIC_JUMP(LJMP, CDFileClass_SetFileName, 0x47AE26, 0x47AEF0);
DEFINE_DYNAMIC_JUMP(LJMP, FileFindOpen, 0x47B020, 0x47B0AE);

// Skip checking of secondary MIX files NoCD mode is enabled
DEFINE_DYNAMIC_PATCH(InitSecondaryMIXes_SkipMovies, 0x531289, 0xB0, 0x01 /* mov al, 1 */, NOP_LETTER);
DEFINE_DYNAMIC_JUMP(LJMP, InitSecondaryMIXes_SkipMulti, 0x530BF8, 0x530C09);
DEFINE_DYNAMIC_JUMP(LJMP, InitSecondaryMIXes_SkipMaps, 0x530B67, 0x530B76);

bool NoCD::Enabled = false;

void NoCD::Apply()
{
	if (NoCD::Enabled)
		return;

	GetCDIndex->Apply();
	CD_IsDiskAvailable->Apply();
	CD_RequestDisk->Apply();

	CDFileClass_SetFileName->Apply();
	FileFindOpen->Apply();

	InitSecondaryMIXes_SkipMovies->Apply();
	InitSecondaryMIXes_SkipMulti->Apply();
	InitSecondaryMIXes_SkipMaps->Apply();

	// CD switch behavior changed to support true No-CD mode with no need to still have a drive
	// Also, this should improve performance considerably when no disc is inserted into any CD drive
	Debug::Log("Optimizing list of CD drives for NoCD mode.\n");
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
		Debug::Log("No CD drives detected. Switching to NoCD mode.\n");
		NoCD::Apply();
	}
}
