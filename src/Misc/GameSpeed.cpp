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

#include <Spawner/Spawner.h>
#include <Utilities/Macro.h>
#include <HouseClass.h>
#include <Unsorted.h>

DEFINE_HOOK(0x4E20BA, GameControlsDialog__Init_SetGameSpeedAvailability, 0x5)
{
	enum { AllowControlSpeed = 0x4E211A, ForbidControlSpeed = 0x4E20C3 };

	if (!Spawner::Enabled)
		return 0;

	if (Spawner::GetConfig()->DisableGameSpeed)
		return ForbidControlSpeed;

	const auto pCurrentPlayer = HouseClass::CurrentPlayer;
	if (Game::ObserverMode || (pCurrentPlayer && pCurrentPlayer->IsObserver()))
	{
		// Allow skirmish observers to control game speed.
		if (SessionClass::IsSkirmish())
			return AllowControlSpeed;

		// Defeated humans may control the speed once no human is still playing.
		if (SessionClass::IsMultiplayer())
		{
			bool bIsAllHumanDefeated = true;
			for (const auto pHouse : HouseClass::Array)
			{
				if (pHouse && pHouse->IsHumanPlayer && !pHouse->Defeated)
				{
					bIsAllHumanDefeated = false;
					break;
				}
			}

			if (bIsAllHumanDefeated)
				return AllowControlSpeed;
		}
	}

	return 0;
}
