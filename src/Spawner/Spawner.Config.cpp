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

#include "Spawner.Config.h"

#include <Main.h>
#include <CCINIClass.h>

void SpawnerConfig::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pSettingsSection = "Settings";
	if (!pINI || !pINI->GetSection(pSettingsSection))
		return;

	// Engine Options
	Ra2Mode         = pINI->ReadBool(pSettingsSection, "Ra2Mode", Ra2Mode); // TODO
	SkipScoreScreen = pINI->ReadBool(pSettingsSection, "SkipScoreScreen", SkipScoreScreen);

	{ // Game Mode Options
		MPModeIndex    = pINI->ReadInteger(pSettingsSection, "GameMode", MPModeIndex);
		Bases          = pINI->ReadBool(pSettingsSection, "Bases", Bases);
		Credits        = pINI->ReadInteger(pSettingsSection, "Credits", Credits);
		BridgeDestroy  = pINI->ReadBool(pSettingsSection, "BridgeDestroy", BridgeDestroy);
		Crates         = pINI->ReadBool(pSettingsSection, "Crates", Crates);
		ShortGame      = pINI->ReadBool(pSettingsSection, "ShortGame", ShortGame);
		SuperWeapons   = pINI->ReadBool(pSettingsSection, "Superweapons", SuperWeapons);
		BuildOffAlly   = pINI->ReadBool(pSettingsSection, "BuildOffAlly", BuildOffAlly);
		GameSpeed      = pINI->ReadInteger(pSettingsSection, "GameSpeed", GameSpeed);
		MultiEngineer  = pINI->ReadBool(pSettingsSection, "MultiEngineer", MultiEngineer);
		UnitCount      = pINI->ReadInteger(pSettingsSection, "UnitCount", UnitCount);
		AIPlayers      = pINI->ReadInteger(pSettingsSection, "AIPlayers", AIPlayers);
		AIDifficulty   = pINI->ReadInteger(pSettingsSection, "AIDifficulty", AIDifficulty);
		AlliesAllowed  = pINI->ReadBool(pSettingsSection, "AlliesAllowed", AlliesAllowed);
		HarvesterTruce = pINI->ReadBool(pSettingsSection, "HarvesterTruce", HarvesterTruce);
		FogOfWar       = pINI->ReadBool(pSettingsSection, "FogOfWar", FogOfWar);
		MCVRedeploy    = pINI->ReadBool(pSettingsSection, "MCVRedeploy", MCVRedeploy);
		/* UIGameMode */ pINI->ReadString(pSettingsSection, "UIGameMode", UIGameMode, UIGameMode, sizeof(UIGameMode));
	}

	// SaveGame Options
	LoadSaveGame     = pINI->ReadBool(pSettingsSection, "LoadSaveGame", LoadSaveGame);
	/* SaveGameName */ pINI->ReadString(pSettingsSection, "SaveGameName", SaveGameName, SaveGameName, sizeof(SaveGameName));

	{ // Scenario Options
		Seed             = pINI->ReadInteger(pSettingsSection, "Seed", Seed);
		TechLevel        = pINI->ReadInteger(pSettingsSection, "TechLevel", TechLevel);
		IsCampaign       = pINI->ReadBool(pSettingsSection, "IsSinglePlayer", IsCampaign);
		Tournament       = pINI->ReadInteger(pSettingsSection, "Tournament", Tournament);
		WOLGameID        = pINI->ReadInteger(pSettingsSection, "GameID", WOLGameID);
		/* ScenarioName */ pINI->ReadString(pSettingsSection, "Scenario", ScenarioName, ScenarioName, sizeof(ScenarioName));
		/* MapHash      */ pINI->ReadString(pSettingsSection, "MapHash", MapHash, MapHash, sizeof(MapHash));
		/* UIMapName    */ pINI->ReadString(pSettingsSection, "UIMapName", UIMapName, UIMapName, sizeof(UIMapName));
	}

	{ // Network Options
		Protocol         = pINI->ReadInteger(pSettingsSection, "Protocol", Protocol);
		FrameSendRate    = pINI->ReadInteger(pSettingsSection, "FrameSendRate", FrameSendRate);
		ReconnectTimeout = pINI->ReadInteger(pSettingsSection, "ReconnectTimeout", ReconnectTimeout);
		ConnTimeout      = pINI->ReadInteger(pSettingsSection, "ConnTimeout", ConnTimeout);
		MaxAhead         = pINI->ReadInteger(pSettingsSection, "MaxAhead", FrameSendRate * 6);
	}

	{ // Tunnel Options
		TunnelId   = pINI->ReadInteger(pSettingsSection, "Port", TunnelId);
		ListenPort = pINI->ReadInteger(pSettingsSection, "Port", ListenPort);

		const char* pTunnelSection = "Tunnel";
		if (pINI->GetSection(pTunnelSection))
		{
			pINI->ReadString(pTunnelSection, "Ip", TunnelIp, TunnelIp, sizeof(TunnelIp));
			TunnelPort = pINI->ReadInteger(pTunnelSection, "Port", TunnelPort);
		}
	}

	// Players Options
	for (int i = 0; i < 8; i++)
		(&Players[i])->LoadFromINIFile(pINI, i);

	// Extended Options
	// TODO:
	// QuickMatch       = pINI->ReadBool(pSettingsSection, "QuickMatch", QuickMatch);
	// RunAutoSS        = pINI->ReadBool(pSettingsSection, "RunAutoSS", RunAutoSS);
}

const char* PlayerSectionArray[8] = {
	"Settings",
	"Other1",
	"Other2",
	"Other3",
	"Other4",
	"Other5",
	"Other6",
	"Other7"
};

const char* MultiTagArray[8] = {
	"Multi1",
	"Multi2",
	"Multi3",
	"Multi4",
	"Multi5",
	"Multi6",
	"Multi7",
	"Multi8"
};

const char* AlliancesSectionArray[8] = {
	"Multi1_Alliances",
	"Multi2_Alliances",
	"Multi3_Alliances",
	"Multi4_Alliances",
	"Multi5_Alliances",
	"Multi6_Alliances",
	"Multi7_Alliances",
	"Multi8_Alliances"
};

const char* AlliancesTagArray[8] = {
	"HouseAllyOne",
	"HouseAllyTwo",
	"HouseAllyThree",
	"HouseAllyFour",
	"HouseAllyFive",
	"HouseAllySix",
	"HouseAllySeven",
	"HouseAllyEight"
};

void PlayerConfig::LoadFromINIFile(CCINIClass* pINI, int index)
{
	if (!pINI || index >= 8)
		return;

	const char* pSection = PlayerSectionArray[index];
	const char* pAlliancesSection = AlliancesSectionArray[index];
	const char* pMultiTag = MultiTagArray[index];

	if (pINI->GetSection(pSection))
	{
		this->IsHuman = true;
		this->Difficulty = -1;

		if (pINI->ReadString(pSection, "Name", "", Main::readBuffer, sizeof(Main::readBuffer)))
			mbstowcs(this->Name, Main::readBuffer, sizeof(this->Name));

		this->Country     = pINI->ReadInteger(pSection, "Side", this->Country);
		this->Color       = pINI->ReadInteger(pSection, "Color", this->Color);
		this->IsSpectator = pINI->ReadBool(pSection, "IsSpectator", this->IsSpectator);
		pINI->ReadString(pSection, "Ip", this->Ip, this->Ip, sizeof(this->Ip));
		this->Port        = pINI->ReadInteger(pSection, "Port", this->Port);
	}
	else if (!IsHuman)
	{
		this->Difficulty  = pINI->ReadInteger("HouseHandicaps", pMultiTag, this->Difficulty);
		this->Country     = pINI->ReadInteger("HouseCountries", pMultiTag, this->Country);
		this->Color       = pINI->ReadInteger("HouseColors", pMultiTag, this->Color);
	}

	SpawnLocations = pINI->ReadInteger("SpawnLocations", pMultiTag, SpawnLocations);
	if (pINI->GetSection(pAlliancesSection))
	{
		for(int i = 0; i < 8; i++)
			this->Alliances[i] = pINI->ReadInteger(pAlliancesSection, AlliancesTagArray[i], this->Alliances[i]);
	}
}
