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

// Skip edge scrolling without changing the state stored in the game or save files.
DEFINE_HOOK(0x693034, ScrollClass_AI_DisableEdgeScrolling, 0x6)
{
	enum { SkipEdgeScrolling = 0x693053 };

	return Main::GetConfig()->DisableEdgeScrolling
		? SkipEdgeScrolling
		: 0;
}
