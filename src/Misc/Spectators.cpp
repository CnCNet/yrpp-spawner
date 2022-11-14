#include <Helpers/Macro.h>
#include <SessionClass.h>
#include <Unsorted.h>

// Allow spectator in skirmish
DEFINE_HOOK_AGAIN(0x4FCBD0, HouseClass_FlagToWinLose_SpectatorPatch, 0x6)
DEFINE_HOOK(0x4FC9E0, HouseClass_FlagToWinLose_SpectatorPatch, 0x6)
{
	return (SessionClass::IsSkirmish() && Game::ObserverMode)
		? 0x4FCDBC
		: 0;
}

// Allow skirmish spectators to control gamespeed
DEFINE_HOOK(0x4E20BA, GameControlsClass_SomeDialog_SpectatorPatch, 0x5)
{
	return (SessionClass::IsSkirmish() && Game::ObserverMode)
		? 0x4E211A
		: 0;
}

DEFINE_HOOK(0x5533E0, LoadProgressMgr_Draw_SpectatorPatch_SetBackground, 0x5)
{
	return Game::ObserverMode
		? 0x5533EF
		: 0;
}

DEFINE_HOOK(0x5539E4, LoadProgressMgr_Draw_SpectatorPatch_LoadBriefing, 0x5)
{
	return Game::ObserverMode
		? 0x5539F3
		: 0;
}

DEFINE_HOOK(0x5536A0, LoadProgressMgr_Draw_SpectatorPatch_CountryName, 0x5)
{
	return Game::ObserverMode
		? 0x5536AF
		: 0;
}
