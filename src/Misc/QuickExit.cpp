#include <Main.h>
#include <Helpers/Macro.h>
#include <HouseClass.h>

bool RageQuit = false;

DEFINE_HOOK(0x77786B, MainWindowProc_HandleRageQuit, 0x5)
{
	if (Main::GetConfig()->QuickExit && HouseClass::CurrentPlayer)
	{
		RageQuit = true;
		return 0x48CBAE;
	}

	return 0;
}

DEFINE_HOOK(0x48CC23, SpecialDialog_HandleRageQuit, 0x7)
{
	if (!RageQuit)
		return 0;

	RageQuit = false;
	return 0x777897;
}
