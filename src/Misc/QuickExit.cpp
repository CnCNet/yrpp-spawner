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

#include <Main.h>
#include <Helpers/Macro.h>
#include <HouseClass.h>
#include <EventClass.h>

bool RageQuit = false;

DEFINE_HOOK(0x77786B, MainWindowProc_HandleRageQuit, 0x5)
{
	if (Main::GetConfig()->QuickExit)
	{
		if (Game::IsActive && HouseClass::CurrentPlayer && SessionClass::IsMultiplayer() && !Game::ScoreStuffLoad)
		{
			RageQuit = true;

			EventClass::OutList.Add(EventClass(HouseClass::CurrentPlayer->ArrayIndex, EventType::Destruct));
			EventClass::OutList.Add(EventClass(HouseClass::CurrentPlayer->ArrayIndex, EventType::Exit));
		}
		else
		{
			ExitProcess(0);
		}
	}

	return 0;
}

DEFINE_HOOK(0x623125, OwnerDrawLoop_HandleRageQuit, 0x5)
{
	return RageQuit
		? 0x623157
		: 0;
}
