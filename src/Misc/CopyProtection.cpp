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

#include <Utilities/Macro.h>

// Based on https://github.com/Ares-Developers/Ares/blob/4f1d929920aca31924c6cd4d3dfa849daa65252a/src/Misc/CopyProtection.cpp

// This blows your base up when it thinks you're cheating
DEFINE_JUMP(LJMP, 0x55CFDF, 0x55D059); // AuxLoop

// Allows run game without the launcher
DEFINE_PATCH(0x49F5C0,    // CopyProtect__IsLauncherRunning
	0xB0, 0x01,           // mov    al, 1
	0xC3);                // retn

DEFINE_PATCH(0x49F620,    // CopyProtect__NotifyLauncher
	0xB0, 0x01,           // mov    al, 1
	0xC3);                // retn

DEFINE_PATCH(0x49F7A0,    // CopyProtect__Validate
	0xB0, 0x01,           // mov    al, 1
	0xC3);                // retn
