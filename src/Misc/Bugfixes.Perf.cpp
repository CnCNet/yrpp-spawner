/*
*  Copyright (c) 2008+, All Ares Contributors
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  1. Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*  3. All advertising materials mentioning features or use of this software
*     must display the following acknowledgement:
*     This product includes software developed by the Ares Contributors.
*  4. Neither the name of Ares nor the
*     names of its contributors may be used to endorse or promote products
*     derived from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY ITS CONTRIBUTORS ''AS IS'' AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE ARES CONTRIBUTORS BE LIABLE FOR ANY
*  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
*  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
	const u_int wanted = static_cast<u_int>(60 / Math::clamp(GameOptionsClass::Instance.GameSpeed, 1, 6));
	return current >= wanted || current >= Detail::GetMinFrameRate();
}

DEFINE_HOOK(0x48A634, FlashBangWarheadAt_Details, 0x5)
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
