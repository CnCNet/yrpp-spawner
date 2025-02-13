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
#include "Spawner.Config.h"
#include <Ext/Event/Body.h>
#include <memory>

class Spawner
{
public:
	static bool Enabled;
	static bool Active;
	static bool DoSave;
	static int NextAutoSaveFrame;
	static int NextAutoSaveNumber;

private:
	static std::unique_ptr<SpawnerConfig> Config;

public:
	static SpawnerConfig* GetConfig()
	{
		return Config.get();
	}

	static void Init();
	static bool StartGame();
	static void AssignHouses();
	static void After_Main_Loop();
	static void RespondToSaveGame(EventExt* event);

private:
	static bool StartScenario(const char* scenarioName);
	static bool LoadSavedGame(const char* scenarioName);

	static void InitNetwork();
	static bool Reconcile_Players();
	static void LoadSidesStuff();
};
