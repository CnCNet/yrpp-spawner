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

#include <GameOptionsClass.h>
#include <GameStrings.h>
#include <HouseClass.h>
#include <IPXManagerClass.h>
#include <LoadOptionsClass.h>
#include <MessageBox.h>
#include <MPGameModeClass.h>
#include <ScenarioClass.h>
#include <time.h>
#include <UDPInterfaceClass.h>
#include <Unsorted.h>
#include <WWMouseClass.h>

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

	const int count = std::min(HouseClass::Array->Count, (int)std::size(Spawner::Config->Houses));
	for (int indexOfHouseArray = 0; indexOfHouseArray < count; indexOfHouseArray++)
	{
		const auto pHouse = HouseClass::Array->GetItem(indexOfHouseArray);

		if (pHouse->Type->MultiplayPassive)
			continue;

		const auto pHousesConfig = &Spawner::Config->Houses[indexOfHouseArray];
		const int nSpawnLocations = pHousesConfig->SpawnLocations;
		const bool isObserver = pHouse->IsHumanPlayer && (
			pHousesConfig->IsObserver
			|| nSpawnLocations == -1
			|| nSpawnLocations == 90
		);

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
			pHouse->StartingPoint = (nSpawnLocations != -2)
				? std::clamp(nSpawnLocations, 0, 7)
				: nSpawnLocations;
		}
		else
		{
			if (pHouse->MakeObserver())
				TabClass::Instance->ThumbActive = false;

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
	}
}

bool Spawner::StartScenario(const char* pScenarioName)
{
	if (pScenarioName[0] == 0 && !Config->LoadSaveGame)
	{
		Debug::Log("[Spawner] Failed Read Scenario [%s]\n", pScenarioName);

		MessageBox::Show(
			StringTable::LoadString(GameStrings::TXT_UNABLE_READ_SCENARIO),
			StringTable::LoadString(GameStrings::TXT_OK),
			0);

		return false;
	}

	const auto pSession = &SessionClass::Instance;
	const auto pGameModeOptions = &GameModeOptionsClass::Instance;

	strcpy_s(&Game::ScenarioName, 0x200, pScenarioName);
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
		GameOptionsClass::Instance->GameSpeed = Spawner::Config->GameSpeed;

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
			NodeNameType::Array->AddItem(pNode);

			wcscpy_s(pNode->Name, pPlayer->Name);
			pNode->Country = pPlayer->Country;
			pNode->Color = pPlayer->Color;
			pNode->Time = -1;

			if (pPlayer->IsObserver && !Spawner::Config->IsCampaign)
			{
				if (pNode->Country < 0)
					pNode->Country = -3;

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

		Game::PlayerCount = NodeNameType::Array->Count;
	}

	{ // Set SessionType
		if (Spawner::Config->IsCampaign)
			pSession->GameMode = GameMode::Campaign;
		else if (Game::PlayerCount > 1)
			pSession->GameMode = GameMode::Internet; // HACK: will be set to LAN later
		else
			pSession->GameMode = GameMode::Skirmish;
	}

	Game::InitRandom();

	// StartScenario
	if (SessionClass::IsCampaign())
	{
		pGameModeOptions->Crates = true;
		return Config->LoadSaveGame ? Spawner::LoadSavedGame(Config->SaveGameName) : ScenarioClass::StartScenario(pScenarioName, 1, 0);
	}
	else if (SessionClass::IsSkirmish())
	{
		return Config->LoadSaveGame ? Spawner::LoadSavedGame(Config->SaveGameName) : ScenarioClass::StartScenario(pScenarioName, 0, -1);
	}
	else /* if (SessionClass::IsMultiplayer()) */
	{
		Spawner::InitNetwork();
		bool result = Config->LoadSaveGame ? Spawner::LoadSavedGame(Config->SaveGameName) : ScenarioClass::StartScenario(pScenarioName, 0, -1);

		if (!result)
			return false;

		pSession->GameMode = GameMode::LAN;

		if (Config->LoadSaveGame && !Spawner::Reconcile_Players())
			return false;

		if (!pSession->CreateConnections())
			return false;

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
		Debug::Log("[Spawner] Failed Load Game [%s]\n", saveGameName);

		MessageBox::Show(
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
	IPXManagerClass::Instance->SetTiming(60, -1, 600, 1);

	Game::Network::PlanetWestwoodStartTime = time(NULL);
	Game::Network::GameStockKeepingUnit = 0x2901;

	ProtocolZero::Enable = (pSpawnerConfig->Protocol == 0);
	if (ProtocolZero::Enable)
	{
		Game::Network::FrameSendRate = 2;
		Game::Network::PreCalcMaxAhead = pSpawnerConfig->PreCalcMaxAhead;
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
	HouseClass* housep;

	// Just use this as Playernodes.
	auto Players = SessionClass::Instance->StartSpots;

	/**
	 *  If there are no players, there's nothing to do.
	 */
	if (Players.Count == 0)
		return true;

	/**
	 *  Make sure every name we're connected to can be found in a House.
	 */
	for (i = 0; i < Players.Count; i++)
	{
		found = false;
		for (house = 0; house < Players.Count; house++)
		{
			housep = HouseClass::Array->Items[house];
			if (!housep)
			{
				continue;
			}

			//
			for (wchar_t c : Players.Items[i]->Name)
			{
				Debug::LogAndMessage("%c", (char)c);
			}
			Debug::LogAndMessage("\n");
			for (wchar_t c : housep->UIName)
			{
				Debug::LogAndMessage("%c", (char)c);
			}
			Debug::LogAndMessage("\n");
			//

			if (!wcscmp(Players.Items[i]->Name, housep->UIName))
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
	for (house = 0; house < Players.Count; house++)
	{
		housep = HouseClass::Array->Items[house];
		if (!housep)
		{
			continue;
		}

		/**
		 *  Skip this house if it wasn't human to start with.
		 */
		if (!housep->IsHumanPlayer)
		{
			continue;
		}

		/**
		 *  Try to find this name in the Players vector; if it's found, set
		 *  its ID to this house.
		 */
		found = false;
		for (i = 0; i < Players.Count; i++)
		{
			if (!wcscmp(Players.Items[i]->Name, housep->UIName))
			{
				found = true;
				Players.Items[i]->HouseIndex = house;
				break;
			}
		}

		/**
		 *  If this name wasn't found, remove it
		 */
		if (!found)
		{

			/**
			 *  Turn the player's house over to the computer's AI
			 */
			housep->IsHumanPlayer = false;
			housep->Production = true;
			housep->IQLevel = RulesClass::Instance->MaxIQLevels;

			static wchar_t buffer[21];
			std::swprintf(buffer, sizeof(buffer), L"%s (AI)", housep->UIName);
			std::wcscpy(housep->UIName, buffer);
			//strcpy(housep->IniName, Fetch_String(TXT_COMPUTER));

			SessionClass::Instance->MPlayerCount--;
		}
	}

	/**
	 *  If all went well, our Session.NumPlayers value should now equal the value
	 *  from the saved game, minus any players we removed.
	 */
	if (SessionClass::Instance->MPlayerCount == Players.Count)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Spawner::LoadSidesStuff()
{
	RulesClass* pRules = RulesClass::Instance;
	CCINIClass* pINI = CCINIClass::INI_Rules;

	pRules->Read_Countries(pINI);
	pRules->Read_Sides(pINI);

	for (auto const& pItem : *HouseTypeClass::Array)
		pItem->LoadFromINI(pINI);
}

void Spawner::RespondToSaveGame(EventExt* event)
{
	/**
	 *  Mark that we'd like to save the game.
	 */
	Spawner::DoSave = true;
}

/**
 *  Prints a message that there's an autosave happening.
 *
 *  Original author: Vinifera Project
 *  Migration: TaranDahl
 */
void Print_Saving_Game_Message()
{
	/**
	 *  Calculate the message delay.
	 */
	const int message_delay = RulesClass::Instance->MessageDelay * 900;

	/**
	 *  Send the message.
	 */
	MessageListClass::Instance->AddMessage(nullptr, 0, L"Saving game...", 4, TextPrintType::Point6Grad | TextPrintType::UseGradPal | TextPrintType::FullShadow, message_delay, false);

	/**
	 *  Force a redraw so that our message gets printed.
	 */
	MapClass::Instance->MarkNeedsRedraw(2);
	MapClass::Instance->Render();
}

/**
 *  We do it by ourselves here instead of letting original Westwood code save when
 *  the event is executed, because saving mid-frame before Remove_All_Inactive()
 *  has been called can lead to save corruption
 *  In other words, by doing it here we fix a Westwood bug/oversight
 * 
 *  Original author: Vinifera Project
 *  Migration: TaranDahl
 */
void Spawner::After_Main_Loop()
{
	auto pConfig = Spawner::GetConfig();

	const bool do_campaign_autosaves = SessionClass::Instance->GameMode == GameMode::Campaign  && pConfig->AutoSaveCount > 0 && pConfig->AutoSaveInterval > 0;
	const bool do_mp_autosaves = Spawner::Active && SessionClass::Instance->GameMode == GameMode::LAN && pConfig->AutoSaveInterval > 0;

	/**
	 *  Schedule to make a save if it's time to autosave.
	 */
	if (do_campaign_autosaves || do_mp_autosaves)
	{
		if (Unsorted::CurrentFrame == Spawner::NextAutoSaveFrame)
		{
			Spawner::DoSave = true;
		}
	}

	if (Spawner::DoSave)
	{

		// Print_Saving_Game_Message();

		/**
		 *  Campaign autosave.
		 */
		if (SessionClass::Instance->GameMode == GameMode::Campaign)
		{

			static char save_filename[32];
			static wchar_t save_description[32];

			/**
			 *  Prepare the save name and description.
			 */
			std::sprintf(save_filename, "AUTOSAVE%d.SAV", Spawner::NextAutoSaveNumber + 1);
			std::swprintf(save_description, L"Mission Auto-Save (Slot %d)", Spawner::NextAutoSaveNumber + 1);

			/**
			 *  Pause the mission timer.
			 */
			// Pause_Game();
			reinterpret_cast<void(__fastcall*)()>(0x683EB0)();
			Game::CallBack();

			/**
			 *  Save!
			 */
			ScenarioClass::Instance->SaveGame(save_filename, save_description);

			/**
			 *  Unpause the mission timer.
			 */
			// Resume_Game();
			reinterpret_cast<void(__fastcall*)()>(0x683FB0)();

			/**
			 *  Increment the autosave number.
			 */
			Spawner::NextAutoSaveNumber = (Spawner::NextAutoSaveNumber + 1) % pConfig->AutoSaveCount;

			/**
			 *  Schedule the next autosave.
			 */
			Spawner::NextAutoSaveFrame = Unsorted::CurrentFrame + pConfig->AutoSaveInterval;
		}
		else if (SessionClass::Instance->GameMode == GameMode::LAN)
		{

			 /**
			  *  Save!
			  */
			ScenarioClass::Instance->SaveGame("SAVEGAME.NET", L"Multiplayer Game");

			/**
			 *  Schedule the next autosave.
			 */
			Spawner::NextAutoSaveFrame = Unsorted::CurrentFrame + pConfig->AutoSaveInterval;
		}

		Spawner::DoSave = false;
	}
}
