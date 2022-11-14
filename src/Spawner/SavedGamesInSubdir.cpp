#include "Spawner.h"
#include <Main.h>

#include <Utilities/Macro.h>

// TODO: Recursively create a directory if it doesn't exist

const auto SavedGamesDir = "Saved Games\\%s";

DEFINE_HOOK(0x67E475, LoadGame_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		GET(char*, pFileName, ESI);
		sprintf(Main::readBuffer, SavedGamesDir, pFileName);
		R->ESI(Main::readBuffer);
	}

	return 0;
}

DEFINE_HOOK(0x559EB0, DeleteSave_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		GET_STACK(char*, pFileName, 0x4);
		sprintf(Main::readBuffer, SavedGamesDir, pFileName);
		R->Stack(0x4, Main::readBuffer);
	}

	return 0;
}

DEFINE_HOOK(0x67CF11, SaveGame_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		GET(char*, pFileName, EDI);
		sprintf(Main::readBuffer, SavedGamesDir, pFileName);
		R->EDI(Main::readBuffer);
	}

	return 0;
}

// Create random filename for save
// Not used. But it does not hurt in case of using a third-party library
// WW compiler made inline in LoadOptionsClass_Dialog
DEFINE_HOOK(0x55961C, LoadOptionsClass_RandomFilename_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		GET(char*, pFileName, ESI);
		sprintf(Main::readBuffer, SavedGamesDir, pFileName);
		R->ESI(Main::readBuffer);
	}

	return 0;
}

// Finds free file name
DEFINE_HOOK(0x5592D2, LoadOptionsClass_Dialog_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		GET(char*, pFileName, EDX);
		sprintf(Main::readBuffer, SavedGamesDir, pFileName);
		R->EDX(Main::readBuffer);
	}

	return 0;
}

// Used for disable buttons in a dialogs
DEFINE_HOOK(0x559C98, LoadOptionsClass_HasSaves_SGInSubdir, 0xB)
{
	LEA_STACK(void*, pFindFileData, STACK_OFFSET(0x348, -0x140));
	LEA_STACK(char*, pFileName, STACK_OFFSET(0x348, -0x33C));

	char* result = pFileName;
	if (Spawner::Enabled)
	{
		result = Main::readBuffer;
		sprintf(result, SavedGamesDir, pFileName);
		// Always "Saved Games\*.SAV"
	}

	R->EAX(result);
	R->EDX(pFindFileData);
	return 0x559CA3; // this is 0x559C98 + 0xB
}

// Fill a list of files
DEFINE_HOOK(0x559882, LoadOptionsClass_FillList_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		// TODO: Expand string size
		LEA_STACK(char*, pFileName/*[128]*/, STACK_OFFSET(0x310, -0x2C0));
		strcpy_s(Main::readBuffer, pFileName);
		sprintf(pFileName, SavedGamesDir, Main::readBuffer);
		// Always "Saved Games\*.SAV"
	}

	return 0;
}

DEFINE_HOOK(0x67FD26, LoadOptionsClass_ReadSaveInfo_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		GET(char*, pFileName, ECX);
		sprintf(Main::readBuffer, SavedGamesDir, pFileName);
		R->ECX(Main::readBuffer);
	}

	return 0;
}
