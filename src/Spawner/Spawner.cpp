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
#include "Spawner.h"
#include "NetHack.h"
#include "ProtocolZero.h"
#include "ProtocolZero.LatencyLevel.h"
#include <Utilities/Debug.h>
#include <Utilities/DumperTypes.h>
#include <Utilities/Patch.h>

#include <GameOptionsClass.h>
#include <GameStrings.h>
#include <HouseClass.h>
#include <IPXManagerClass.h>
#include <LoadOptionsClass.h>
#include <WWMessageBox.h>
#include <MPGameModeClass.h>
#include <ScenarioClass.h>
#include <time.h>
#include <UDPInterfaceClass.h>
#include <Unsorted.h>
#include <WWMouseClass.h>

#include <cassert>

bool Spawner::Enabled = false;
bool Spawner::Active = false;
std::unique_ptr<SpawnerConfig> Spawner::Config = nullptr;
bool Spawner::DoSave = false;
int Spawner::NextAutoSaveFrame = -1;
int Spawner::NextAutoSaveNumber = 0;

void Spawner::Init()
{
	Spawner::Config = std::make_unique<SpawnerConfig>();

	auto pINI = CCINIClass::LoadINIFile("SPAWN.INI");
	Spawner::Config->LoadFromINIFile(pINI);
	CCINIClass::UnloadINIFile(pINI);
}

bool Spawner::StartGame()
{
	if (Spawner::Active)
		return false;

	Spawner::Active = true;
	Game::IsActive = true;
	Game::InitUIStuff();

	char* pScenarioName = Config->ScenarioName;

	if (strstr(pScenarioName, "RA2->"))
		pScenarioName += sizeof("RA2->") - 1;

	if (strstr(pScenarioName, "PlayMovies->"))
	{
		pScenarioName += sizeof("PlayMovies->") - 1;
		char* context = nullptr;
		char* movieName = strtok_s(pScenarioName, Main::readDelims, &context);
		for (; movieName; movieName = strtok_s(nullptr, Main::readDelims, &context))
			Game::PlayMovie(movieName);

		return false;
	}

	Spawner::LoadSidesStuff();

	bool result = StartScenario(pScenarioName);

	if (Main::GetConfig()->DumpTypes)
		DumperTypes::Dump();

	WWMouseClass::PrepareScreen();

	return result;
}

void Spawner::AssignHouses()
{
	ScenarioClass::AssignHouses();

	const int count = std::min(HouseClass::Array.Count, (int)std::size(Spawner::Config->Houses));
	for (int indexOfHouseArray = 0; indexOfHouseArray < count; indexOfHouseArray++)
	{
		const auto pHouse = HouseClass::Array.GetItem(indexOfHouseArray);

		if (pHouse->Type->MultiplayPassive)
			continue;

		const auto pHousesConfig = &Spawner::Config->Houses[indexOfHouseArray];
		const int nSpawnLocations = pHousesConfig->SpawnLocations;
		const bool isObserver = pHouse->IsHumanPlayer && pHousesConfig->IsObserver;

		// Set Alliances
		for (char i = 0; i < (char)std::size(pHousesConfig->Alliances); ++i)
		{
			const int alliesIndex = pHousesConfig->Alliances[i];
			if (alliesIndex != -1)
				pHouse->Allies.Add(alliesIndex);
		}

		// Set AI UIName
		if (Spawner::Config->AINamesByDifficulty && !pHouse->IsHumanPlayer)
		{
			const auto pAIConfig = &Spawner::Config->Players[indexOfHouseArray];

			switch (pAIConfig->Difficulty)
			{
			case 0:
				wcscpy_s(pHouse->UIName, StringTable::LoadString(GameStrings::GUI_AIHard));
				break;

			case 1:
				wcscpy_s(pHouse->UIName, StringTable::LoadString(GameStrings::GUI_AINormal));
				break;

			case 2:
				wcscpy_s(pHouse->UIName, StringTable::LoadString(GameStrings::GUI_AIEasy));
				break;
			}
		}

		// Set SpawnLocations
		if (!isObserver)
		{
			pHouse->StartingPoint = (nSpawnLocations < 0)
				? -2
				: std::clamp(nSpawnLocations, 0, 7);
		}
		else
		{
			if (pHouse->MakeObserver())
				TabClass::Instance.ThumbActive = false;

			{ // Remove SpawnLocations for Observer
				ScenarioClass* pScenarioClass = ScenarioClass::Instance;
				for (char i = 0; i < (char)std::size(pScenarioClass->HouseIndices); ++i)
				{
					if (pHouse->ArrayIndex == pScenarioClass->HouseIndices[i])
						pScenarioClass->HouseIndices[i] = -1;
				}

				pHouse->StartingPoint = -1;
			}
		}

		// Set Bonus Money
		if (pHousesConfig->CreditsFactor != 1.0)
			pHouse->Balance = int(pHouse->Balance * pHousesConfig->CreditsFactor);

		// Set Handicap Difficulty
		if (pHousesConfig->HandicapDifficulty != -1)
			pHouse->AssignHandicap(pHousesConfig->HandicapDifficulty);
	}
}

bool Spawner::StartScenario(const char* pScenarioName)
{
	if (pScenarioName[0] == 0 && !Config->LoadSaveGame)
	{
		Debug::Log("Failed Read Scenario [%s]\n", pScenarioName);

		WWMessageBox::Instance.Process(
			StringTable::LoadString(GameStrings::TXT_UNABLE_READ_SCENARIO),
			StringTable::LoadString(GameStrings::TXT_OK),
			0);

		return false;
	}

	const auto pSession = &SessionClass::Instance;
	const auto pGameModeOptions = &GameModeOptionsClass::Instance;

	strcpy_s(Game::ScenarioName, 0x200, pScenarioName);
	pSession->ReadScenarioDescriptions();

	{ // Set MPGameMode
		pSession->MPGameMode = MPGameModeClass::Get(Spawner::Config->MPModeIndex);
		if (!pSession->MPGameMode)
			pSession->MPGameMode = MPGameModeClass::Get(1);
	}

	{ // Set Options
		pGameModeOptions->MPModeIndex       = Spawner::Config->MPModeIndex;
		// pGameModeOptions->ScenarioIndex
		pGameModeOptions->Bases             = Spawner::Config->Bases;
		pGameModeOptions->Money             = Spawner::Config->Credits;
		pGameModeOptions->BridgeDestruction = Spawner::Config->BridgeDestroy;
		pGameModeOptions->Crates            = Spawner::Config->Crates;
		pGameModeOptions->ShortGame         = Spawner::Config->ShortGame;
		pGameModeOptions->SWAllowed         = Spawner::Config->SuperWeapons;
		pGameModeOptions->BuildOffAlly      = Spawner::Config->BuildOffAlly;
		pGameModeOptions->GameSpeed         = Spawner::Config->GameSpeed;
		pGameModeOptions->MultiEngineer     = Spawner::Config->MultiEngineer;
		pGameModeOptions->UnitCount         = Spawner::Config->UnitCount;
		pGameModeOptions->AIPlayers         = Spawner::Config->AIPlayers;
		pGameModeOptions->AIDifficulty      = Spawner::Config->AIDifficulty;
		// pGameModeOptions->AISlots
		pGameModeOptions->AlliesAllowed     = Spawner::Config->AlliesAllowed;
		pGameModeOptions->HarvesterTruce    = Spawner::Config->HarvesterTruce;
		// pGameModeOptions->CaptureTheFlag
		pGameModeOptions->FogOfWar          = Spawner::Config->FogOfWar;
		pGameModeOptions->MCVRedeploy       = Spawner::Config->MCVRedeploy;
		wcscpy(pGameModeOptions->MapDescription, Spawner::Config->UIMapName);

		Game::Seed = Spawner::Config->Seed;
		Game::TechLevel = Spawner::Config->TechLevel;
		Game::PlayerColor = Spawner::Config->Players[0].Color;
		GameOptionsClass::Instance.GameSpeed = Spawner::Config->GameSpeed;

		Spawner::NextAutoSaveNumber = Spawner::Config->NextAutoSaveNumber;
	}

	{ // Added AI Players
		const auto pAISlots = &pGameModeOptions->AISlots;
		for (char slotIndex = 0; slotIndex < (char)std::size(pAISlots->Allies); ++slotIndex)
		{
			const auto pPlayerConfig = &Spawner::Config->Players[slotIndex];
			if (pPlayerConfig->IsHuman)
				continue;

			pAISlots->Difficulties[slotIndex] = pPlayerConfig->Difficulty;
			pAISlots->Countries[slotIndex]    = pPlayerConfig->Country;
			pAISlots->Colors[slotIndex]       = pPlayerConfig->Color;
			pAISlots->Allies[slotIndex]       = -1;
		}
	}

	{ // Added Human Players
		NetHack::PortHack = true;
		const char maxPlayers = Spawner::Config->IsCampaign ? 1 : (char)std::size(Spawner::Config->Players);
		for (char playerIndex = 0; playerIndex < maxPlayers; playerIndex++)
		{
			const auto pPlayer = &Spawner::Config->Players[playerIndex];
			if (!pPlayer->IsHuman)
				continue;

			const auto pNode = GameCreate<NodeNameType>();
			NodeNameType::Array.AddItem(pNode);

			wcscpy_s(pNode->Name, pPlayer->Name);
			pNode->Country = pPlayer->Country;
			pNode->Color = pPlayer->Color;
			pNode->Time = -1;

			if (pPlayer->IsObserver && !Spawner::Config->IsCampaign)
			{
				if (pNode->Country < 0)
					pNode->Country = HouseTypeClass::TempObserverID;

				pNode->SpectatorFlag = 0xFFFFFFFF;

				if (playerIndex == 0)
					Game::ObserverMode = true;
			}

			if (playerIndex > 0)
			{
				pNode->Address.sin_addr.s_addr = playerIndex;

				const auto Ip = inet_addr(pPlayer->Ip);
				const auto Port = htons((u_short)pPlayer->Port);
				ListAddress::Array[playerIndex - 1].Ip = Ip;
				ListAddress::Array[playerIndex - 1].Port = Port;
				if (Port != (u_short)Spawner::Config->ListenPort)
					NetHack::PortHack = false;
			}
		}

		Game::PlayerCount = NodeNameType::Array.Count;
	}

	{ // Set SessionType
		if (Spawner::Config->IsCampaign)
			pSession->GameMode = GameMode::Campaign;
		else if (Game::PlayerCount > 1 || Spawner::Config->ForceMultiplayer)
			pSession->GameMode = GameMode::Internet; // HACK: will be set to LAN later
		else
			pSession->GameMode = GameMode::Skirmish;
	}

	Game::InitRandom();

	// StartScenario
	if (SessionClass::IsCampaign())
	{
		pGameModeOptions->Crates = true;

		// Rename MISSIONMD.INI to this
		// because Ares has LoadScreenText.Color and Phobos has Starkku's PR #1145
		// 2025-05-28: Moved to a hook in Spawner.Hook.cpp - Starkku
		// if (Spawner::Config->ReadMissionSection) // before parsing
		//	 Patch::Apply_RAW(0x839724, "Spawn.ini");

		bool result = Config->LoadSaveGame? Spawner::LoadSavedGame(Config->SaveGameName): ScenarioClass::StartScenario(pScenarioName, 1, 0);

		if (Spawner::Config->CustomMissionID != 0) // after parsing
			ScenarioClass::Instance->EndOfGame = true;

		return result;
	}
	else if (SessionClass::IsSkirmish())
	{
		return Config->LoadSaveGame
			? Spawner::LoadSavedGame(Config->SaveGameName)
			: ScenarioClass::StartScenario(pScenarioName, 0, -1);
	}
	else /* if (SessionClass::IsMultiplayer()) */
	{
		Spawner::InitNetwork();
		bool result = Config->LoadSaveGame
			? Spawner::LoadSavedGame(Config->SaveGameName)
			: ScenarioClass::StartScenario(pScenarioName, 0, -1);

		if (!result)
			return false;

		pSession->GameMode = GameMode::LAN;

		if (Config->LoadSaveGame && !Spawner::Reconcile_Players())
			return false;

		if (!pSession->CreateConnections())
			return false;

		// Ares does not support MultiEngineer switching in multiplayer, however
		// we can disable it simply by setting EngineerCaptureLevel to 1 - Belonit

		// https://ares-developers.github.io/Ares-docs/restored/multiengineer.html
		// https://github.com/Phobos-developers/Antares/blob/7241a5ff20f4dbf7153cc77e16edca5c9db473d4/src/Ext/Infantry/Body.cpp#L44-L46

		if (!pGameModeOptions->MultiEngineer)
			RulesClass::Instance->EngineerCaptureLevel = 1.0;

		if (Main::GetConfig()->AllowChat == false)
		{
			Game::ChatMask[0] = false;
			Game::ChatMask[1] = false;
			Game::ChatMask[2] = false;
			Game::ChatMask[3] = false;
			Game::ChatMask[4] = false;
			Game::ChatMask[5] = false;
			Game::ChatMask[6] = false;
			Game::ChatMask[7] = false;
		}

		return true;
	}
}

bool Spawner::LoadSavedGame(const char* saveGameName)
{
	if (!saveGameName[0] || !LoadOptionsClass::LoadMission(saveGameName))
	{
		Debug::Log("Failed Load Game [%s]\n", saveGameName);

		WWMessageBox::Instance.Process(
			StringTable::LoadString(GameStrings::TXT_ERROR_LOADING_GAME),
			StringTable::LoadString(GameStrings::TXT_OK),
			0);

		return false;
	}

	return true;
}

void Spawner::InitNetwork()
{
	const auto pSpawnerConfig = Spawner::GetConfig();

	Tunnel::Id = htons((u_short)pSpawnerConfig->TunnelId);
	Tunnel::Ip = inet_addr(pSpawnerConfig->TunnelIp);
	Tunnel::Port = htons((u_short)pSpawnerConfig->TunnelPort);

	auto& ListenPort = *reinterpret_cast<u_short*>(0x841F30u);
	ListenPort = Tunnel::Port ? 0 : (u_short)pSpawnerConfig->ListenPort;

	UDPInterfaceClass::Instance = GameCreate<UDPInterfaceClass>();
	UDPInterfaceClass::Instance->Init();
	UDPInterfaceClass::Instance->OpenSocket();
	UDPInterfaceClass::Instance->StartListening();
	UDPInterfaceClass::Instance->DiscardInBuffers();
	UDPInterfaceClass::Instance->DiscardOutBuffers();
	IPXManagerClass::Instance.SetTiming(60, -1, 600, 1);

	Game::Network::PlanetWestwoodStartTime = time(NULL);
	Game::Network::GameStockKeepingUnit = 0x2901;

	ProtocolZero::Enable = (pSpawnerConfig->Protocol == 0);
	if (ProtocolZero::Enable)
	{
		Game::Network::FrameSendRate = 2;
		Game::Network::PreCalcMaxAhead = pSpawnerConfig->PreCalcMaxAhead;

		ProtocolZero::NextSendFrame = -1;
		ProtocolZero::WorstMaxAhead = LatencyLevel::GetMaxAhead(LatencyLevelEnum::LATENCY_LEVEL_6);

		ProtocolZero::MaxLatencyLevel = std::clamp(
			pSpawnerConfig->MaxLatencyLevel,
			(byte)LatencyLevelEnum::LATENCY_LEVEL_1,
			(byte)LatencyLevelEnum::LATENCY_LEVEL_MAX
		);
	}
	else
	{
		Game::Network::FrameSendRate = pSpawnerConfig->FrameSendRate;
	}

	Game::Network::MaxAhead = pSpawnerConfig->MaxAhead == -1
		? Game::Network::FrameSendRate * 6
		: pSpawnerConfig->MaxAhead;

	Game::Network::MaxMaxAhead      = 0;
	Game::Network::ProtocolVersion  = 2;
	Game::Network::LatencyFudge     = 0;
	Game::Network::RequestedFPS     = 60;
	Game::Network::Tournament       = pSpawnerConfig->Tournament;
	Game::Network::WOLGameID        = pSpawnerConfig->WOLGameID;
	Game::Network::ReconnectTimeout = pSpawnerConfig->ReconnectTimeout;

	if (pSpawnerConfig->QuickMatch)
	{
		Game::EnableMPDebug     = false;
		Game::DrawMPDebugStats  = false;
		Game::EnableMPSyncDebug = false;
	}

	Game::Network::Init();
}

/**
 *  Reconciles loaded data with the "Players" vector.
 *
 *  This function is for supporting loading a saved multiplayer game.
 *  When the game is loaded, we have to figure out which house goes with
 *  which entry in the Players vector. We also have to figure out if
 *  everyone who was originally in the game is still with us, and if not,
 *  turn their stuff over to the computer.
 *
 *  Original author: Vinifera Project
 *  Migration: TaranDahl
 */
bool Spawner::Reconcile_Players()
{
	int i;
	bool found;
	int house;
	HouseClass* pHouse;

	// Just use this as Playernodes.
	auto players = SessionClass::Instance.StartSpots;

	// If there are no players, there's nothing to do.
	if (players.Count == 0)
		return true;

	// Make sure every name we're connected to can be found in a House.
	for (i = 0; i < players.Count; i++)
	{
		found = false;

		for (house = 0; house < players.Count; house++)
		{
			pHouse = HouseClass::Array.Items[house];
			if (!pHouse)
				continue;

			for (wchar_t c : players.Items[i]->Name)
				Debug::LogAndMessage("%c", (char)c);

			Debug::LogAndMessage("\n");

			for (wchar_t c : pHouse->UIName)
				Debug::LogAndMessage("%c", (char)c);

			Debug::LogAndMessage("\n");

			if (!wcscmp(players.Items[i]->Name, pHouse->UIName))
			{
				found = true;
				break;
			}
		}

		if (!found)
			return false;
	}

	/**
	 *  Loop through all Houses; if we find a human-owned house that we're
	 *  not connected to, turn it over to the computer.
	 */
	for (house = 0; house < players.Count; house++)
	{
		pHouse = HouseClass::Array.Items[house];

		if (!pHouse)
			continue;

		// Skip this house if it wasn't human to start with.
		if (!pHouse->IsHumanPlayer)
			continue;

		/**
		 *  Try to find this name in the Players vector; if it's found, set
		 *  its ID to this house.
		 */
		found = false;
		for (i = 0; i < players.Count; i++)
		{
			if (!wcscmp(players.Items[i]->Name, pHouse->UIName))
			{
				found = true;
				players.Items[i]->HouseIndex = house;
				break;
			}
		}

		// If this name wasn't found, remove it
		if (!found)
		{
			/**
			 *  Turn the player's house over to the computer's AI
			 */
			pHouse->IsHumanPlayer = false;
			pHouse->Production = true;
			pHouse->IQLevel = RulesClass::Instance->MaxIQLevels;

			static wchar_t buffer[21];
			std::swprintf(buffer, sizeof(buffer), L"%s (AI)", pHouse->UIName);
			std::wcscpy(pHouse->UIName, buffer);
			//strcpy(pHouse->IniName, Fetch_String(TXT_COMPUTER));

			SessionClass::Instance.MPlayerCount--;
		}
	}

	/**
	 *  If all went well, our Session.NumPlayers value should now equal the value
	 *  from the saved game, minus any players we removed.
	 */
	return SessionClass::Instance.MPlayerCount == players.Count;
}

void Spawner::LoadSidesStuff()
{
	RulesClass* pRules = RulesClass::Instance;
	CCINIClass* pINI = CCINIClass::INI_Rules;

	pRules->Read_Countries(pINI);
	pRules->Read_Sides(pINI);

	for (auto const& pItem : HouseTypeClass::Array)
		pItem->LoadFromINI(pINI);
}

void Spawner::RespondToSaveGame()
{
	// Mark that we'd like to save the game.
	Spawner::DoSave = true;
}

/**
 *  We do it by ourselves here instead of letting original Westwood code save when
 *  the event is executed, because saving mid-frame before Remove_All_Inactive()
 *  has been called can lead to save corruption
 *  In other words, by doing it here we fix a Westwood bug/oversight
 *
 *  Original author: Rampastring, ZivDero
 *  Migration: TaranDahl
 *  Further changes: Kerbiter
 */
void Spawner::After_Main_Loop()
{
	auto pConfig = Spawner::GetConfig();

	const bool doSaveSP =
		SessionClass::IsSingleplayer()
		&& pConfig->AutoSaveCount > 0
		&& pConfig->AutoSaveInterval > 0;

	const bool doSaveMP =
		Spawner::Active
		&& SessionClass::Instance.GameMode == GameMode::LAN
		&& pConfig->AutoSaveInterval > 0;

	const bool isAutoSaving = (doSaveSP || doSaveMP)
		&& Unsorted::CurrentFrame == Spawner::NextAutoSaveFrame;

	// Schedule to make a save if it's time to autosave.
	// The save might be triggered manually, so we have to OR it.
	Spawner::DoSave |= isAutoSaving;

	if (Spawner::DoSave)
	{
		auto PrintMessage = [](const wchar_t* pMessage)
		{
			MessageListClass::Instance.PrintMessage(
				pMessage,
				RulesClass::Instance->MessageDelay,
				HouseClass::CurrentPlayer->ColorSchemeIndex,
				/* bSilent: */ true
			);

			// Force a redraw so that our message gets printed.
			if (Game::SpecialDialog == 0)
			{
				MapClass::Instance.MarkNeedsRedraw(2);
				MapClass::Instance.Render();
			}
		};

		auto SaveGame = [PrintMessage](const char* fName, const wchar_t* description)
		{
			if (ScenarioClass::SaveGame(fName, description))
				PrintMessage(StringTable::LoadString(GameStrings::TXT_GAME_WAS_SAVED));
			else
				PrintMessage(StringTable::LoadString(GameStrings::TXT_ERROR_SAVING_GAME));
		};

		// Send the message.
		PrintMessage(StringTable::LoadString(GameStrings::TXT_SAVING_GAME));

		std::wstring saveGameDescription;
		if (SessionClass::IsCampaign())
			saveGameDescription = ScenarioClass::Instance->UINameLoaded;
		else
			saveGameDescription = ScenarioClass::Instance->Name;
		saveGameDescription += L" - ";

		// This whole situation is a mess, but basically there's a myriad of ways to save
		// scattered across Phobos (quicksave hotkey and save trigger action) and spawner
		// (autosave), all in different conditions (multi- or singleplayer).

		// Previously everything only supported singleplayer, so Phobos didn't have to
		// account for multiplayer. Now we have to support both singleplayer and multiplayer,
		// but only spawner can do proper multiplayer saves on-demand, *and* also without
		// spawner there is no point in doing multiplayer saves at all.

		// What I came up with is: for synced situations (trigger action) we save on Phobos
		// side only if save event code (0x4C7A14) is patched (heuristic, any better ideas
		// are welcome), and for unsynced situations (quicksave) we also check for that patch
		// and emit the event that uses it.

		// If anyone wants to untangle that mess in a nice way - be my guest.
		// - Kerbiter

		// Singleplayer autosave.
		if (SessionClass::Instance.IsSingleplayer())
		{
			// ASSUMPTION: There will be no save events emitted in singleplayer
			// situations, and the only other way for the spawner to save is
			// through the autosave, which is what we are doing here.

			// If you want to fixup this - again, be my guest.
			// - Kerbiter

			assert(isAutoSaving);

			static char saveFileName[32];
			static wchar_t saveDescription[128];

			saveGameDescription += StringTable::TryFetchString("TXT_AUTOSAVE_SUFFIX", L"Autosave (slot %d)");
			std::sprintf(saveFileName, "AUTOSAVE%d.SAV", Spawner::NextAutoSaveNumber + 1);
			std::swprintf(saveDescription, saveGameDescription.c_str(), Spawner::NextAutoSaveNumber + 1);

			SaveGame(saveFileName, saveDescription);

			Spawner::NextAutoSaveNumber = (Spawner::NextAutoSaveNumber + 1) % pConfig->AutoSaveCount;
			Spawner::NextAutoSaveFrame = Unsorted::CurrentFrame + pConfig->AutoSaveInterval;
		}
		else if (SessionClass::Instance.GameMode == GameMode::LAN)
		{
			// CnCNet client follows the legacy approach of fixed save name and copies it
			// over to it's own directory. The description isn't read now, but we write it
			// regardless as it shouldn't impact anything. The suffix for it is unavailable
			// though as it would require a custom event (seems overkill for such).
			saveGameDescription += StringTable::LoadString(GameStrings::TXT_MULTIPLAYER_GAME);
			SaveGame(GameStrings::SAVEGAME_NET, saveGameDescription.c_str());

			Spawner::NextAutoSaveFrame = Unsorted::CurrentFrame + pConfig->AutoSaveInterval;
		}

		Spawner::DoSave = false;
	}
}
