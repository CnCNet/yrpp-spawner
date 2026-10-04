/**
*  yrpp-spawner
*
*  Copyright(C) 2026-present CnCNet
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

#include <Main.h>
#include <CCINIClass.h>
#include <GameStrings.h>
#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

DEFINE_HOOK(0x6BBE56, WinMain__SkipCreateAppMutex, 0x5)
{
	enum { SkipCreateAppMutex = 0x6BBEE3 };

	// The normal settings load happens after the application mutex check.
	auto pINI = CCINIClass::LoadINIFile(GameStrings::RA2MD_INI);
	auto pConfig = Main::GetConfig();
	pConfig->SkipCreateAppMutex = pINI->ReadBool("Debug", "SkipCreateAppMutex", pConfig->SkipCreateAppMutex);
	CCINIClass::UnloadINIFile(pINI);

	if (pConfig->SkipCreateAppMutex)
	{
		Debug::LogDeferred("Skip create AppMutex.\n");
		return SkipCreateAppMutex;
	}

	return 0;
}
