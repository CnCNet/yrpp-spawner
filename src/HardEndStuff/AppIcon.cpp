/**
*  yrpp-spawner
*
*  Copyright(C) 2024-present CnCNet
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

#ifdef IS_HARDEND_VER
#include <Utilities/Macro.h>
#include "Ra2Mode.h"

HANDLE __fastcall UI_ApplyAppIcon()
{
	const char* iconPath = Ra2Mode::IsEnabled()
		? "./Resources/ra2.ico"
		: "./Resources/clienticon.ico";

	return LoadImageA(NULL, iconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
}

DEFINE_PATCH(0x777C41, 0x90, 0x90, 0x90, 0x90); // Disable Phobos hook (0x777C41, UI_ApplyAppIcon, 0x9)
DEFINE_JUMP(CALL, 0x777C45, GET_OFFSET(UI_ApplyAppIcon));
#endif
