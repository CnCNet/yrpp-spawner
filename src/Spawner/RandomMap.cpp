#include "Spawner.h"

#include <GameStrings.h>
#include <CCFileClass.h>
#include <CCINIClass.h>
#include <ScenarioClass.h>
#include <Unsorted.h>
#include <Utilities/Macro.h>

bool __fastcall ReadScenarioINI(CCINIClass* pINI)
{ JMP_STD(0x686730); }

DEFINE_HOOK(0x6849C9, ReadScenario_RandomMap, 0x5)
{
	// Vanilla Instructions
	GET(CCINIClass*, pINI, ECX);
	R->EAX(ReadScenarioINI(pINI));

	if (Spawner::Enabled && ScenarioClass::Instance->IsRandom)
		return 0x68496B;

	return 0x6849C9 + 0x5;
}

DEFINE_HOOK(0x686C48, ReadScenarioINI_RandomMap, 0x8)
{
	if (Spawner::Enabled && !ScenarioClass::Instance->IsRandom)
	{
		GET(CCINIClass*, pINI, EBP);
		ScenarioClass::Instance->IsRandom = pINI->ReadBool(GameStrings::Basic, "RandomMap", ScenarioClass::Instance->IsRandom);

		if (ScenarioClass::Instance->IsRandom)
			return 0x687917;
	}

	return 0;
}

// Hack Get_Starting_locations, it seems to be broken in random maps
// This can be skipped because the starting location in random maps is stored somewhere else
DEFINE_HOOK(0x688564, ScenStruct_ScenStruct_RandomMap, 0x6)
{
	if (Spawner::Enabled && ScenarioClass::Instance->IsRandom)
		return 0x6885D7;

	return 0;
}

// Hack rmp to read the sceanrio for unit mods and stuff
DEFINE_HOOK(0x5997AB, MapGeneratorClass_Init_RandomMap, 0x9)
{
	if (Spawner::Enabled)
	{
		LEA_STACK(CCINIClass*, pINI, STACK_OFFSET(0xB0, -0x64));

		auto pIncludeFile = CCFileClass(&Game::ScenarioName);
		if (pIncludeFile.Exists())
			pINI->ReadCCFile(&pIncludeFile);
	}

	return 0;
}

// Use Seed from spawn.ini rather than from the map.
DEFINE_HOOK(0x597B76, MapSeedClass_LoadMap_RandomMap, 0x6)
{
	if (Spawner::Enabled)
		R->EAX(Spawner::GetConfig()->Seed);

	return 0;
}
