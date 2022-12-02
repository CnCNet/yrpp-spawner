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
#include <Spawner/Spawner.h>
#include <Helpers/Macro.h>

DEFINE_HOOK(0x5C9720, MPScore_Present_SkipScoreScreen, 0x8)
{
	const auto pSpawnerConfig = Spawner::Enabled
		? Spawner::GetConfig()
		: nullptr;

	const bool bSkipScoreScreen = pSpawnerConfig
		? pSpawnerConfig->SkipScoreScreen
		: Main::GetConfig()->SkipScoreScreen;

	return bSkipScoreScreen
		? 0x5C97AC
		: 0;
}
