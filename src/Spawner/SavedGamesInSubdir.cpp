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

#include <filesystem>
namespace SavedGames
{
	constexpr auto DirName = "Saved Games"; // Nested paths are also supported, e.g. "Saved Games\\Yuri's Revenge"
	constexpr auto FileNamePrefixFormat = "%s\\%s";

	inline bool CreateSubdir()
	{
		if (!std::filesystem::exists(SavedGames::DirName))
		{
			Debug::Log("\nFolder Saved Games does not exist, creating...");
			if (!std::filesystem::create_directories(SavedGames::DirName))
			{
				Debug::Log("Cannot create folder Saved Games! WTF!\n");
				return false;
			}
			Debug::Log("Done.\n");
		}
		return true;
	}

	inline void FormatPath(char* buffer, const char* pFileName)
	{
		sprintf(buffer, SavedGames::FileNamePrefixFormat, SavedGames::DirName, pFileName);
	}
}

DEFINE_HOOK(0x67E475, LoadGame_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		GET(char*, pFileName, ESI);
		SavedGames::FormatPath(Main::readBuffer, pFileName);
		R->ESI(Main::readBuffer);
	}

	return 0;
}

DEFINE_HOOK(0x559EB0, DeleteSave_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		REF_STACK(char*, pFileName, 0x4);
		SavedGames::FormatPath(Main::readBuffer, pFileName);
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
		SavedGames::FormatPath(Main::readBuffer, pFileName);
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
		SavedGames::FormatPath(Main::readBuffer, pFileName);
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
		SavedGames::FormatPath(Main::readBuffer, pFileName);
		R->EDX(Main::readBuffer);
	}

	return 0;
}

// Used for disable buttons in a dialogs
DEFINE_HOOK(0x559C98, LoadOptionsClass_HasSaves_SGInSubdir, 0xB)
{
	LEA_STACK(void*, pFindFileData, STACK_OFFSET(0x348, -0x140));
	LEA_STACK(char*, pFileName, STACK_OFFSET(0x348, -0x33C));

	if (Spawner::Enabled)
	{
		SavedGames::FormatPath(Main::readBuffer, pFileName);
		pFileName = Main::readBuffer; // Always "Saved Games\*.SAV"
	}

	R->EAX(pFileName);
	R->EDX(pFindFileData);

	return 0x559CA3; // this is 0x559C98 + 0xB
}

// Fill a list of files
DEFINE_HOOK(0x559886, LoadOptionsClass_FillList_SGInSubdir, 0x8)
{
	GET(struct _WIN32_FIND_DATAA*, pFind, EDX);
	GET(char*, pFileName, EAX);

	if (Spawner::Enabled)
	{
		SavedGames::FormatPath(Main::readBuffer, pFileName);
		pFileName = Main::readBuffer; // Always "Saved Games\*.SAV"
	}

	HANDLE result = FindFirstFileA(pFileName, pFind);
	R->EAX(result);

	return 0x55988E; // this is 0x559886 + 0x8
}

DEFINE_HOOK(0x67FD26, LoadOptionsClass_ReadSaveInfo_SGInSubdir, 0x5)
{
	if (Spawner::Enabled)
	{
		GET(char*, pFileName, ECX);
		SavedGames::FormatPath(Main::readBuffer, pFileName);
		R->ECX(Main::readBuffer);
	}

	return 0;
}
