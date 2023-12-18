#include "Spawner.h"
#include <Utilities/Macro.h>

namespace QuickMatch
{
	const wchar_t* PlayerString = L"Player";
}

DEFINE_HOOK(0x643AA5, ProgressScreenClass_643720_HideName, 0x8)
{
	if ((Spawner::Enabled && Spawner::GetConfig()->QuickMatch) == false)
		return 0;

	REF_STACK(wchar_t*, pPlayerName, STACK_OFFSET(0x5C, 8));
	pPlayerName = const_cast<wchar_t*>(QuickMatch::PlayerString);

	return 0;
}

DEFINE_HOOK(0x65837A, RadarClass_658330_HideName, 0x6)
{
	if ((Spawner::Enabled && Spawner::GetConfig()->QuickMatch) == false)
		return 0;

	R->ECX(QuickMatch::PlayerString);
	return 0x65837A + 0x6;
}

DEFINE_HOOK(0x64B156, ModeLessDialog_64AE50_HideName, 0x9)
{
	if ((Spawner::Enabled && Spawner::GetConfig()->QuickMatch) == false)
		return 0;

	R->EDX(QuickMatch::PlayerString);
	return 0x64B156 + 0x9;
}

DEFINE_HOOK(0x648EA8, WaitForPlayers_HideName, 0x6)
{
	if ((Spawner::Enabled && Spawner::GetConfig()->QuickMatch) == false)
		return 0;

	R->EAX(QuickMatch::PlayerString);
	return 0x648EB3;
}
