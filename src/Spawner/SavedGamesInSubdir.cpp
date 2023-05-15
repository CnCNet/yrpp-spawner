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
#include <Main.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>


namespace SavedGames
{
	constexpr auto DirDefaultName = "Saved Games";
	constexpr auto FileNamePrefix = "Saved Games\\%s";

	// Create the directory if it doesn't exist
	inline bool CreateSubdir()
	{
		HANDLE hDir = CreateFileA(DirDefaultName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
		if (hDir == INVALID_HANDLE_VALUE && GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			Debug::Log("\nFolder Saved Games does not exist, creating...");
			if (!CreateDirectoryA(DirDefaultName, nullptr))
			{
				Debug::Log("Cannot create folder Saved Games! WTF!\n");
				CloseHandle(hDir);
				return false;
			}
			Debug::Log("Done.\n");
		}
		CloseHandle(hDir);
		return true;
	}
}

DEFINE_HOOK(0x67E475, LoadGame_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		GET(char*, pFileName, ESI);
		sprintf(Main::readBuffer, SavedGames::FileNamePrefix, pFileName);
		R->ESI(Main::readBuffer);
	}

	return 0;
}

DEFINE_HOOK(0x559EB0, DeleteSave_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		REF_STACK(char*, pFileName, 0x4);
		sprintf(Main::readBuffer, SavedGames::FileNamePrefix, pFileName);
		pFileName = Main::readBuffer;
	}

	return 0;
}

DEFINE_HOOK(0x67CF11, SaveGame_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		if (!SavedGames::CreateSubdir())
			return 0;

		GET(char*, pFileName, EDI);
		sprintf(Main::readBuffer, SavedGames::FileNamePrefix, pFileName);
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
		sprintf(Main::readBuffer, SavedGames::FileNamePrefix, pFileName);
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
		sprintf(Main::readBuffer, SavedGames::FileNamePrefix, pFileName);
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
		sprintf(result, SavedGames::FileNamePrefix, pFileName);
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
		sprintf(pFileName, SavedGames::FileNamePrefix, Main::readBuffer);
		// Always "Saved Games\*.SAV"
	}

	return 0;
}

DEFINE_HOOK(0x67FD26, LoadOptionsClass_ReadSaveInfo_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		GET(char*, pFileName, ECX);
		sprintf(Main::readBuffer, SavedGames::FileNamePrefix, pFileName);
		R->ECX(Main::readBuffer);
	}

	return 0;
}
