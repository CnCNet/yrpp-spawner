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

#pragma once
#include "version.h"
#include "Main.Config.h"
#include <Windows.h>
#include <memory>

class CCINIClass;
class AbstractClass;

class Main
{
private:
	static std::unique_ptr<MainConfig> Config;

public:
	static void ExeRun();
	static void CmdLineParse(char** ppArgs, int nNumArgs);

	static MainConfig* GetConfig()
	{
		return Config.get();
	}

	static HANDLE hInstance;

	static const size_t readLength = 2048;
	static char readBuffer[readLength];
	static wchar_t wideBuffer[readLength];
	static const char readDelims[4];

#ifdef DEBUG
	static void DetachFromDebugger();
	static bool TryDetachFromDebugger();
#endif
};
