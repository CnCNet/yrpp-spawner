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

HANDLE Main::hInstance = nullptr;
std::unique_ptr<MainConfig> Main::Config = std::make_unique<MainConfig>();

void Main::ExeRun()
{
	Patch::ApplyStatic();
#ifdef DEBUG
	Main::DetachFromDebugger();
#endif
}

void Main::CmdLineParse(char** ppArgs, int nNumArgs)
{
	Debug::Log("Initialized " PRODUCT_NAME " " PRODUCT_VERSION "\n");

	// > 1 because the exe path itself counts as an argument, too!
	for (int i = 1; i < nNumArgs; ++i)
	{
		const char* pArg = ppArgs[i];

		if (0 == _stricmp(pArg, "-CD"))
		{
			Main::Config->NoCD = true;
		}
		else if (0 == _stricmp(pArg, "-SPAWN"))
		{
			Spawner::Enabled = true;
		}
		else if (0 == _stricmp(pArg, "-DumpTypes"))
		{
			Main::Config->DumpTypes = true;
		}
		else if (strstr(pArg, "-RA2ModeSaveID="))
		{
			Main::Config->RA2ModeSaveID = strtoul(pArg - 1 + sizeof("-RA2ModeSaveID="), 0, 0);
		}
	}
}
