#pragma once
#include "Spawner.Config.h"
#include <memory>

class Spawner
{
public:
	static bool Enabled;
	static bool IsActive;

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

private:
	static bool StartNewScenario(const char* scenarioName);
	static bool LoadSavedGame(const char* scenarioName);

	static void InitNetwork();
	static void LoadSidesStuff();
	static void PrepareScreen();
};
