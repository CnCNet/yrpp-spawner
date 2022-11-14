#include <Utilities/Macro.h>

#include <GameOptionsClass.h>
#include <FPSCounter.h>
#include <YRMath.h>

/***
	the following hooks replace the original checks that disable certain visual
	effects when the frame rate drops below a certain limit. the issue with them
	was that they only take into account the settings from the rulesmd.ini, but
	ignore the game speed setting. that means if the frame rate is supposed to
	be 20 frames or less (DetailMinFrameRateNormal=15, DetailBufferZoneWidth=5),
	then the effects are still disabled. lasers draw as ugly lines, non-damaging
	particles don't render, spotlights aren't created, ...
***/

/***
	Ported from Ares
	Since hooks do not return 0, nothing bad will happen even if every dll includes this fix
	https://github.com/Ares-Developers/Ares/blob/master/src/Misc/Bugfixes.Perf.cpp
	http://ares-developers.github.io/Ares-docs/bugfixes/type1/visualeffectsdisabled.html
 ***/

bool __forceinline DetailsCurrentlyEnabled()
{
	// not only checks for the min frame rate from the rules, but also whether
	// the low frame rate is actually desired. in that case, don't reduce.
	const u_int current = FPSCounter::CurrentFrameRate;
	const u_int wanted = static_cast<u_int>(60 / Math::clamp(GameOptionsClass::Instance->GameSpeed, 1, 6));
	return current >= wanted || current >= Detail::GetMinFrameRate();
}

DEFINE_HOOK(0x48A634, FlashbangWarheadAt_Details, 0x5)
{
	return DetailsCurrentlyEnabled()
		? 0x48A64A
		: 0x48A641;
}

DEFINE_HOOK(0x5FF86E, SpotlightClass_Draw_Details, 0x5)
{
	return DetailsCurrentlyEnabled()
		? 0x5FF87F
		: 0x5FFF77;
}

DEFINE_HOOK(0x422FCC, AnimClass_Draw_Details, 0x5)
{
	return DetailsCurrentlyEnabled()
		? 0x422FEC
		: 0x422FD9;
}

DEFINE_HOOK(0x550BCA, LaserDrawClass_Draw_InHouseColor_Details, 0x5)
{
	return DetailsCurrentlyEnabled()
		? 0x550BD7
		: 0x550BE5;
}

DEFINE_HOOK(0x62CEC9, ParticleClass_Draw_Details, 0x5)
{
	return DetailsCurrentlyEnabled()
		? 0x62CEEA
		: 0x62CED6;
}

DEFINE_HOOK(0x6D7847, TacticalClass_DrawPixelEffects_Details, 0x5)
{
	return DetailsCurrentlyEnabled()
		? 0x6D7858
		: 0x6D7BF2;
}
