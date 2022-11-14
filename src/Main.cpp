#include "Main.h"

#include <Utilities/Debug.h>
#include <Utilities/Patch.h>

#include <Spawner/Spawner.h>
#include <Misc/NoCD.h>

HANDLE Main::hInstance = nullptr;
std::unique_ptr<MainConfig> Main::Config = std::make_unique<MainConfig>();;

void Main::ExeRun()
{
	Patch::ApplyStatic();

#ifdef DEBUG
	if (Main::DetachFromDebugger())
	{
		MessageBoxW(NULL,
		L"You can now attach a debugger.\n\n"

		L"Press OK to continue YR execution.",
		L"Debugger Notice", MB_OK);
	}
	else
	{
		MessageBoxW(NULL,
		L"You can now attach a debugger.\n\n"

		L"To attach a debugger find the YR process in Process Hacker "
		L"/ Visual Studio processes window and detach debuggers from it, "
		L"then you can attach your own debugger. After this you should "
		L"terminate Syringe.exe because it won't automatically exit when YR is closed.\n\n"

		L"Press OK to continue YR execution.",
		L"Debugger Notice", MB_OK);
	}
#endif
}

void Main::CmdLineParse(char** ppArgs, int nNumArgs)
{
	// > 1 because the exe path itself counts as an argument, too!
	for (int i = 1; i < nNumArgs; i++)
	{
		const char* pArg = ppArgs[i];

		if (0 == _stricmp(pArg, "-CD"))
		{
			NoCD::Apply();
		}
		else if (0 == _stricmp(pArg, "-SPAWN"))
		{
			NoCD::Apply();
			Spawner::Enabled = true;
		}
	}

	Debug::Log("Initialized YR-Spawner " PRODUCT_VERSION "\n");
	NoCD::InitNoCDMode();
}
