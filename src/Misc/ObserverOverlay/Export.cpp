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

// Observer overlay export.
//
// While the local player is observing, periodically writes what the observer can see (credits, power,
// production queues and owned unit / building counts of every player) to observer_overlay.json in the
// game directory. tools/ObserverOverlay serves that file to an OBS browser source.
//
// Enabled with RA2MD.INI:
//   [Options]
//   ObserverOverlay=yes
//   ObserverOverlay.Interval=15 ; game frames between writes

#include "Snapshot.h"

#include <Main.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <AircraftTypeClass.h>
#include <BuildingTypeClass.h>
#include <FactoryClass.h>
#include <HouseClass.h>
#include <HouseTypeClass.h>
#include <InfantryTypeClass.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>
#include <UnitTypeClass.h>
#include <Unsorted.h>

#include <cstdio>
#include <cwchar>
#include <string>

namespace
{
	constexpr const char* OverlayFileName = "observer_overlay.json";
	constexpr const char* OverlayTempFileName = "observer_overlay.json.tmp";

	// FactoryClass::Production is a StageClass hardcoded to 54 steps.
	constexpr int FactoryProductionSteps = 54;

	int LastExportFrame = -1;
	bool LoggedWriteFailure = false;

	std::string ToUtf8(const wchar_t* pText)
	{
		if (!pText || !*pText)
			return {};

		const int length = static_cast<int>(wcslen(pText));
		const int size = WideCharToMultiByte(CP_UTF8, 0, pText, length, nullptr, 0, nullptr, nullptr);
		if (size <= 0)
			return {};

		std::string result(static_cast<size_t>(size), '\0');
		WideCharToMultiByte(CP_UTF8, 0, pText, length, result.data(), size, nullptr, nullptr);
		return result;
	}

	ObserverOverlay::TypeEntry MakeTypeEntry(TechnoTypeClass const* pType)
	{
		return { pType->ID, ToUtf8(pType->UIName) };
	}

	void AddFactory(ObserverOverlay::PlayerState& player, const char* pCategory, FactoryClass* pFactory)
	{
		if (!pFactory)
			return;

		ObserverOverlay::FactoryState factory;
		factory.Category = pCategory;
		factory.OnHold = pFactory->OnHold;
		factory.IsDone = pFactory->IsDone();

		if (pFactory->Object)
		{
			if (auto const pType = pFactory->Object->GetTechnoType())
			{
				factory.HasCurrent = true;
				factory.Current = MakeTypeEntry(pType);
				factory.ProgressPercent = ObserverOverlay::ProgressToPercent(pFactory->GetProgress(), FactoryProductionSteps);
			}
		}

		for (auto const pQueued : pFactory->QueuedObjects)
		{
			if (pQueued)
				factory.Queue.push_back(MakeTypeEntry(pQueued));
		}

		if (factory.HasCurrent || !factory.Queue.empty())
			player.Production.push_back(std::move(factory));
	}

	template <typename TType>
	void AddCounts(ObserverOverlay::PlayerState& player, HouseClass const* pHouse, const char* pCategory)
	{
		for (auto const pType : TType::Array)
		{
			const int count = pHouse->CountOwnedNow(pType);
			if (count > 0)
				player.Counts.push_back({ pCategory, MakeTypeEntry(pType), count });
		}
	}

	bool IsPlayingHouse(HouseClass const* pHouse)
	{
		return pHouse
			&& pHouse->Type
			&& !pHouse->Type->MultiplayPassive
			&& !pHouse->IsObserver()
			&& !pHouse->IsInitiallyObserver();
	}

	ObserverOverlay::PlayerState MakePlayerState(HouseClass* pHouse)
	{
		ObserverOverlay::PlayerState player;
		player.Index = pHouse->ArrayIndex;
		player.Name = ToUtf8(pHouse->UIName);
		if (player.Name.empty())
			player.Name = pHouse->PlainName;
		player.Country = pHouse->Type->ID;
		player.ColorR = pHouse->Color.R;
		player.ColorG = pHouse->Color.G;
		player.ColorB = pHouse->Color.B;
		player.Credits = static_cast<int>(pHouse->Available_Money());
		player.PowerOutput = pHouse->PowerOutput;
		player.PowerDrain = pHouse->PowerDrain;
		player.Defeated = pHouse->Defeated;

		AddFactory(player, "Building", pHouse->Primary_ForBuildings);
		AddFactory(player, "Defense", pHouse->Primary_ForDefenses);
		AddFactory(player, "Infantry", pHouse->Primary_ForInfantry);
		AddFactory(player, "Vehicle", pHouse->Primary_ForVehicles);
		AddFactory(player, "Naval", pHouse->Primary_ForShips);
		AddFactory(player, "Aircraft", pHouse->Primary_ForAircraft);

		AddCounts<BuildingTypeClass>(player, pHouse, "Building");
		AddCounts<InfantryTypeClass>(player, pHouse, "Infantry");
		AddCounts<UnitTypeClass>(player, pHouse, "Vehicle");
		AddCounts<AircraftTypeClass>(player, pHouse, "Aircraft");

		return player;
	}

	void WriteSnapshot(const std::string& json)
	{
		// Write to a temporary file and then swap it in, so the overlay never reads a half-written file.
		FILE* pFile = nullptr;
		if (fopen_s(&pFile, OverlayTempFileName, "wb") != 0 || !pFile)
		{
			if (!LoggedWriteFailure)
			{
				Debug::Log("[Spawner] ObserverOverlay: unable to open %s for writing\n", OverlayTempFileName);
				LoggedWriteFailure = true;
			}
			return;
		}

		const size_t written = fwrite(json.data(), 1, json.size(), pFile);
		fclose(pFile);

		if (written != json.size()
			|| !MoveFileExA(OverlayTempFileName, OverlayFileName, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			if (!LoggedWriteFailure)
			{
				Debug::Log("[Spawner] ObserverOverlay: unable to write %s\n", OverlayFileName);
				LoggedWriteFailure = true;
			}
		}
	}

	void ExportIfDue()
	{
		auto const pConfig = Main::GetConfig();
		if (!pConfig->ObserverOverlay || !HouseClass::IsCurrentPlayerObserver())
			return;

		const int currentFrame = Unsorted::CurrentFrame;
		if (currentFrame < LastExportFrame)
			LastExportFrame = -1; // A new game was started from the same process.

		if (LastExportFrame >= 0 && currentFrame - LastExportFrame < pConfig->ObserverOverlayInterval)
			return;

		LastExportFrame = currentFrame;

		ObserverOverlay::Snapshot snapshot;
		snapshot.Frame = currentFrame;
		for (auto const pHouse : HouseClass::Array)
		{
			if (IsPlayingHouse(pHouse))
				snapshot.Players.push_back(MakePlayerState(pHouse));
		}

		WriteSnapshot(ObserverOverlay::ToJson(snapshot));
	}
}

DEFINE_HOOK(0x55DDA0, MainLoop_AfterRender__ObserverOverlay, 0x5)
{
	ExportIfDue();
	return 0;
}
