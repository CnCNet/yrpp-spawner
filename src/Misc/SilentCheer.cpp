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

#include <Utilities/Macro.h>
#include <SessionClass.h>

// Disabling taunts also mutes the house cheer sound
DEFINE_HOOK(0x50C8F4, HouseClass_Cheer_RespectTauntSetting, 0x5)
{
	enum { SkipSound = 0x50C910 };

	if (SessionClass::Instance.GameMode == GameMode::LAN && !SessionClass::Instance.LANTaunts)
		return SkipSound;

	if (SessionClass::Instance.GameMode == GameMode::Internet && !SessionClass::Instance.WOLTaunts)
		return SkipSound;

	return 0;
}
