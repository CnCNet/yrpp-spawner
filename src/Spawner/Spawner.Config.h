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

#pragma once
#include <Main.h>

class CCINIClass;

class SpawnerConfig
{

	// Used to create NodeNameType
	// The order of entries may differ from HouseConfig
	struct PlayerConfig
	{
		bool IsHuman;
		wchar_t Name[20];
		int Color;
		int Country;
		int Difficulty;
		bool IsObserver;
		char Ip[0x20];
		int Port;

		PlayerConfig()
			: IsHuman { false }
			, Name { L"" }
			, Color { -1 }
			, Country { -1 }
			, Difficulty { -1 }
			, IsObserver { false }
			, Ip { "0.0.0.0" }
			, Port { -1 }
		{ }

		void LoadFromINIFile(CCINIClass* pINI, int index);
	};

	// Used to augment the generated HouseClass
	// The order of entries may differ from PlayerConfig
	struct HouseConfig
	{
		bool IsObserver;
		int SpawnLocations;
		int Alliances[8];

		HouseConfig()
			: IsObserver { false }
			, SpawnLocations { -2 }
			, Alliances { -1, -1, -1, -1, -1, -1, -1, -1 }
		{ }

		void LoadFromINIFile(CCINIClass* pINI, int index);
	};

public:
	// Game Mode Options
	int  MPModeIndex;
	bool Bases;
	int  Credits;
	bool BridgeDestroy;
	bool Crates;
	bool ShortGame;
	bool SuperWeapons;
	bool BuildOffAlly;
	int  GameSpeed;
	bool MultiEngineer;
	int  UnitCount;
	int  AIPlayers;
	int  AIDifficulty;
	bool AlliesAllowed;
	bool HarvesterTruce;
	bool FogOfWar;
	bool MCVRedeploy;
	wchar_t UIGameMode[60];

	// SaveGame Options
	bool LoadSaveGame;
	char SaveGameName[60];

	// Scenario Options
	int  Seed;
	int  TechLevel;
	bool IsCampaign;
	int  Tournament;
	DWORD WOLGameID;
	char ScenarioName[60];
	char MapHash[255];
	wchar_t UIMapName[45];

	// Network Options
	int Protocol;
	int FrameSendRate;
	int ReconnectTimeout;
	int ConnTimeout;
	int MaxAhead;

	// Tunnel Options
	int  TunnelId;
	char TunnelIp[0x20];
	int  TunnelPort;
	int  ListenPort;

	// Players Options
	PlayerConfig Players[8];

	// Houses Options
	HouseConfig Houses[8];

	// Extended Options
	bool Ra2Mode;
	bool SkipScoreScreen;
	bool WriteStatistics;
	bool AINamesByDifficulty;
	bool ContinueWithoutHumans;
	bool DefeatedBecomesObserver;
	bool Observer_ShowAIOnSidebar;
	// TODO:
	// bool QuickMatch;
	// bool RunAutoSS;

	SpawnerConfig() // default values
		// Game Mode Options
		: MPModeIndex { 1 }
		, Bases { true }
		, Credits { 10000 }
		, BridgeDestroy { true }
		, Crates { false }
		, ShortGame { false }
		, SuperWeapons { true }
		, BuildOffAlly { false }
		, GameSpeed { 0 }
		, MultiEngineer { false }
		, UnitCount { 0 }
		, AIPlayers { 0 }
		, AIDifficulty { 1 }
		, AlliesAllowed { false }
		, HarvesterTruce { false }
		, FogOfWar { false }
		, MCVRedeploy { true }
		, UIGameMode { L"" }

		// SaveGame
		, LoadSaveGame { false }
		, SaveGameName { "" }

		// Scenario Options
		, Seed { 0 }
		, TechLevel { 10 }
		, IsCampaign { false }
		, Tournament { 0 }
		, WOLGameID { 0xDEADBEEF }
		, ScenarioName { "" }
		, MapHash { "" }
		, UIMapName { L"" }

		// Network Options
		, Protocol { 2 }
		, FrameSendRate { 4 }
		, ReconnectTimeout { 2400 }
		, ConnTimeout { 3600 }
		, MaxAhead { FrameSendRate * 6 }

		// Tunnel Options
		, TunnelId { 0 }
		, TunnelIp { "0.0.0.0" }
		, TunnelPort { 0 }
		, ListenPort { 1234 }

		// Players Options
		, Players {
			PlayerConfig(),
			PlayerConfig(),
			PlayerConfig(),
			PlayerConfig(),

			PlayerConfig(),
			PlayerConfig(),
			PlayerConfig(),
			PlayerConfig()
		}

		// Houses Options
		, Houses {
			HouseConfig(),
			HouseConfig(),
			HouseConfig(),
			HouseConfig(),

			HouseConfig(),
			HouseConfig(),
			HouseConfig(),
			HouseConfig()
		}

		// Extended Options
		, Ra2Mode { false }
		, SkipScoreScreen { Main::GetConfig()->SkipScoreScreen }
		, WriteStatistics { false }
		, AINamesByDifficulty { false }
		, ContinueWithoutHumans { false }
		, DefeatedBecomesObserver { false }
		, Observer_ShowAIOnSidebar { false }
		// TODO:
		// , QuickMatch { false }
		// , RunAutoSS { false }
	{ }

	void LoadFromINIFile(CCINIClass* pINI);
};
