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
#include "Nethack.h"

#include <CCINIClass.h>
#include <GameOptionsClass.h>
#include <GScreenClass.h>
#include <HouseClass.h>
#include <IPXManagerClass.h>
#include <MessageBox.h>
#include <MouseClass.h>
#include <MPGameModeClass.h>
#include <RulesClass.h>
#include <ScenarioClass.h>
#include <Surface.h>
#include <time.h>
#include <UDPInterfaceClass.h>
#include <Unsorted.h>
#include <Utilities/Debug.h>
#include <WWMouseClass.h>

bool Spawner::Enabled = false;
bool Spawner::Active = false;
std::unique_ptr<SpawnerConfig> Spawner::Config = nullptr;

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
		return 0;

	Spawner::Active = true;
	Game::IsActive = true;

	Game::InitUIStuff();
	Spawner::LoadSidesStuff();

	bool result = Config->LoadSaveGame
		? LoadSavedGame(Config->SaveGameName)
		: StartNewScenario(Config->ScenarioName);

	Spawner::PrepareScreen();
	return result;
}

void Spawner::AssignHouses()
{
	const auto pAISlots = &GameModeOptionsClass::Instance->AISlots;

	// Added AI Players
	for (int i = 0; i < 8; i++)
	{
		const auto pPlayerConfig = &Spawner::Config->Players[i];
		if (pPlayerConfig->IsHuman) continue;

		pAISlots->Difficulties[i] = pPlayerConfig->Difficulty;
		pAISlots->Countries[i]    = pPlayerConfig->Country;
		pAISlots->Colors[i]       = pPlayerConfig->Color;
		pAISlots->Allies[i]       = -1;
	}

	ScenarioClass::AssignHouses();

	for (int i = -2 + HouseClass::Array->Count; i >= 0; i--)
	{
		const auto pPlayerConfig = &Spawner::Config->Players[i];
		const auto pHouse = HouseClass::Array->GetItem(i);

		if (pPlayerConfig->IsSpectator && pHouse == HouseClass::CurrentPlayer)
			HouseClass::Observer = pHouse;

		pHouse->StartingPoint = pPlayerConfig->SpawnLocations;

		for (int j = 0; j < 8; j++)
		{
			const int allyIndex = pPlayerConfig->Alliances[j];
			if (allyIndex != -1)
				pHouse->Allies.Add(allyIndex);
		}
	}
}

bool Spawner::StartNewScenario(const char* scenarioName)
{
	if (scenarioName[0] == 0)
	{
		MessageBox::Show(StringTable::LoadString("TXT_UNABLE_READ_SCENARIO"), StringTable::LoadString("TXT_OK"), 0);
		return false;
	}

	const auto pSession = &SessionClass::Instance;
	const auto pGameOptions = &GameOptionsClass::Instance;
	const auto pGameModeOptions = &GameModeOptionsClass::Instance;

	strcpy_s(&Game::ScenarioName, 0x200, scenarioName);
	pSession->ReadScenarioDescriptions();

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
		// pGameModeOptions->CTF
		pGameModeOptions->FogOfWar          = Spawner::Config->FogOfWar;
		pGameModeOptions->MCVRedeploy       = Spawner::Config->MCVRedeploy;
		// pGameModeOptions->MapDescription

		Game::PlayerColor = Spawner::Config->Players[0].Color;

		pGameOptions->GameSpeed = Spawner::Config->GameSpeed;
		Game::Seed = Spawner::Config->Seed;

		if (!MPGameModeClass::Set(Config->MPModeIndex))
			MPGameModeClass::Set(1);
	}

	{ // Added Human Players
		Nethack::PortHack = true;
		const int maxPlayers = Spawner::Config->IsCampaign ? 1 : 8;
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

			if (pPlayer->IsSpectator && maxPlayers > 1)
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
					Nethack::PortHack = false;
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
		return ScenarioClass::StartScenario(scenarioName, 1, 0);
	}
	else if (SessionClass::IsSkirmish())
	{
		return ScenarioClass::StartScenario(scenarioName, 0, -1);
	}
	else
	{
		Spawner::InitNetwork();
		if (!ScenarioClass::StartScenario(scenarioName, 0, -1))
			return false;

		pSession->GameMode = GameMode::LAN;
		pSession->CreateConnections();
		return true;
	}
}

bool Spawner::LoadSavedGame(const char* saveGameName)
{
	auto& InScenario1 = *reinterpret_cast<bool*>(0xA8E378);
	auto& InScenario2 = *reinterpret_cast<bool*>(0xA8ED5C);

	InScenario1 = false;
	InScenario2 = false;

	if (!ScenarioClass::LoadGame(saveGameName))
	{
		MessageBox::Show(StringTable::LoadString("TXT_ERROR_LOADING_GAME"), StringTable::LoadString("TXT_OK"), 0);
		Debug::Log("Failed Load Game [%s]\n", saveGameName);
		return false;
	}

	return true;
}

void Spawner::InitNetwork()
{
	Tunnel::Id = htons((u_short)Spawner::Config->TunnelId);
	Tunnel::Ip = inet_addr(Spawner::Config->TunnelIp);
	Tunnel::Port = htons((u_short)Spawner::Config->TunnelPort);

	auto& ListenPort = *reinterpret_cast<u_short*>(0x841F30u);
	ListenPort = Tunnel::Port ? 0 : (u_short)Spawner::Config->ListenPort;

	UDPInterfaceClass::Instance = GameCreate<UDPInterfaceClass>();
	UDPInterfaceClass::Instance->Init();
	UDPInterfaceClass::Instance->OpenSocket();
	UDPInterfaceClass::Instance->StartListening();
	UDPInterfaceClass::Instance->DiscardInBuffers();
	UDPInterfaceClass::Instance->DiscardOutBuffers();
	IPXManagerClass::Instance->SetTiming(60, -1, 600, 1);

	Game::Network::PlanetWestwoodStartTime = time(NULL);
	Game::Network::GameStockKeepingUnit = 0x2901;

	Game::Network::Tournament       = Spawner::Config->Tournament;
	Game::Network::WOLGameID        = Spawner::Config->WOLGameID;
	Game::Network::ProtocolVersion  = Spawner::Config->Protocol;
	Game::Network::FrameSendRate    = Spawner::Config->FrameSendRate;
	Game::Network::ReconnectTimeout = Spawner::Config->ReconnectTimeout;
	Game::Network::MaxAhead         = Spawner::Config->MaxAhead;
	Game::Network::MaxMaxAhead      = 0;
	Game::Network::LatencyFudge     = 0;
	Game::Network::RequestedFPS     = 60;

	Game::Network::Init();
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

void Spawner::PrepareScreen()
{
	WWMouseClass::Instance->HideCursor();

	DSurface::Hidden->Fill(0);
	GScreenClass::DoBlit(true, DSurface::Hidden);
	DSurface::Temp = DSurface::Hidden;

	WWMouseClass::Instance->ShowCursor();

	MouseClass::Instance->SetCursor(MouseCursorType::NoMove, false);
	MouseClass::Instance->RestoreCursor();

	TabClass::Instance->Activate();
	MouseClass::Instance->RedrawSidebar(0);
}
