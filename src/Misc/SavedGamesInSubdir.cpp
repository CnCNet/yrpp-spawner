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

#include <Main.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Spawner/Spawner.h>

#include <HouseClass.h>
#include <LoadOptionsClass.h>

#include <filesystem>
#include <optional>

namespace SavedGames
{
	inline bool CreateSubdir()
	{
		if (!std::filesystem::exists(Spawner::GetConfig()->SavedGameDir))
		{
			Debug::Log("\n[Spawner] Folder Saved Games does not exist, creating...\n");
			if (!std::filesystem::create_directories(Spawner::GetConfig()->SavedGameDir))
			{
				Debug::Log("\tCannot create folder Saved Games!\n");
				return false;
			}
			Debug::Log("\tDone.\n");
		}
		return true;
	}

	inline void FormatPath(char* buffer, const char* pFileName)
	{
		sprintf(buffer, "%s\\%s", Spawner::GetConfig()->SavedGameDir, pFileName);
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

	return 0x559C98 + 0xB;
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

	return 0x559886 + 0x8;
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


//issue #18 : Save game filter for 3rd party campaigns
namespace SavedGames
{
	struct CustomMissionID
	{
		static constexpr wchar_t* SaveName = L"CustomMissionID";

		int Number;

		CustomMissionID() : Number { Spawner::GetConfig()->CustomMissionID } { }

		CustomMissionID(int num) : Number { num } { }

		operator int() const { return Number; }
	};

	// More fun

	struct ExtraTestInfo
	{
		static constexpr wchar_t* SaveName = L"Spawner test info";

		int CurrentFrame;
		int TechnoCount;

		explicit ExtraTestInfo()
			:CurrentFrame { Unsorted::CurrentFrame }
			, TechnoCount { TechnoClass::Array->Count }
		{
		}
	};

	template<typename T>
	bool AppendToStorage(IStorage* pStorage)
	{
		IStream* pStream = nullptr;
		bool ret = false;
		HRESULT hr = pStorage->CreateStream(
			T::SaveName,
			STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
			0,
			0,
			&pStream
		);

		if (SUCCEEDED(hr) && pStream != nullptr)
		{
			T info {};
			ULONG written = 0;
			hr = pStream->Write(&info, sizeof(info), &written);
			ret = SUCCEEDED(hr) && written == sizeof(info);
			pStream->Release();
		}

		return ret;
	}


	template<typename T>
	std::optional<T> ReadFromStorage(IStorage* pStorage)
	{
		IStream* pStream = nullptr;
		bool hasValue = false;
		HRESULT hr = pStorage->OpenStream(
			T::SaveName,
			NULL,
			STGM_READ | STGM_SHARE_EXCLUSIVE,
			0,
			&pStream
		);

		T info;

		if (SUCCEEDED(hr) && pStream != nullptr)
		{
			ULONG read = 0;
			hr = pStream->Read(&info, sizeof(info), &read);
			hasValue = SUCCEEDED(hr) && read == sizeof(info);

			pStream->Release();
		}

		return hasValue ? std::make_optional(info) : std::nullopt;
	}

}

DEFINE_HOOK(0x559921, LoadOptionsClass_FillList_FilterFiles, 0x6)
{
	GET(FileEntryClass*, pEntry, EBP);
	enum { NullThisEntry = 0x559959 };
	/*
	// there was a qsort later and filters out these but we could have just removed them right here
	if (pEntry->IsWrongVersion || !pEntry->IsValid)
	{
		GameDelete(pEntry);
		return NullThisEntry;
	};
	*/
	OLECHAR wNameBuffer[0x100] {};
	SavedGames::FormatPath(Main::readBuffer, pEntry->Filename.data());
	MultiByteToWideChar(CP_UTF8, 0, Main::readBuffer, -1, wNameBuffer, std::size(wNameBuffer));
	IStorage* pStorage = nullptr;
	bool shouldDelete = false;
	if (SUCCEEDED(StgOpenStorage(wNameBuffer, NULL,
		STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStorage)
	))
	{
		auto id = SavedGames::ReadFromStorage<SavedGames::CustomMissionID>(pStorage);

		if (Spawner::GetConfig()->CustomMissionID != id.value_or(0))
			shouldDelete = true;
	}

	if (pStorage)
		pStorage->Release();

	if (shouldDelete)
	{
		GameDelete(pEntry);
		return NullThisEntry;
	}

	return 0;
}

// Write : A la fin
DEFINE_HOOK(0x67D2E3, SaveGame_AdditionalInfoForClient, 0x6)
{
	GET_STACK(IStorage*, pStorage, STACK_OFFSET(0x4A0, -0x490));
	using namespace SavedGames;

	if (pStorage)
	{
		if (SessionClass::IsCampaign() && Spawner::GetConfig()->CustomMissionID)
			AppendToStorage<CustomMissionID>(pStorage);
		if (AppendToStorage<ExtraTestInfo>(pStorage))
			Debug::Log("[Spawner] Extra meta info appended on sav file\n");
	}

	return 0;
}

// Read : Au debut
DEFINE_HOOK(0x67E4DC, LoadGame_AdditionalInfoForClient, 0x7)
{
	LEA_STACK(const wchar_t*, filename, STACK_OFFSET(0x518, -0x4F4));
	IStorage* pStorage = nullptr;
	using namespace SavedGames;

	if (SUCCEEDED(StgOpenStorage(filename, NULL,
		STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStorage)
	))
	{
		if (auto id = ReadFromStorage<CustomMissionID>(pStorage))
		{
			int num = id->Number;
			Debug::Log("[Spawner] sav file CustomMissionID = %d\n", num);
			Spawner::GetConfig()->CustomMissionID = num;
			ScenarioClass::Instance->EndOfGame = true;
		}
		else
		{
			Spawner::GetConfig()->CustomMissionID = 0;
		}

		if (auto info = ReadFromStorage<ExtraTestInfo>(pStorage))
		{
			Debug::Log("[Spawner] CurrentFrame = %d, TechnoCount = %d\n"
				, info->CurrentFrame
				, info->TechnoCount
			);
		}
	}
	if (pStorage)
		pStorage->Release();

	return 0;
}
