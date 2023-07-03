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

#include "Main.h"

#include <Utilities/Debug.h>
#include <Utilities/Patch.h>

#include <Spawner/Spawner.h>
#include <Misc/NoCD.h>

HANDLE Main::hInstance = nullptr;
std::unique_ptr<MainConfig> Main::Config = std::make_unique<MainConfig>();

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
