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

// Extend IsoMapPack5 decoding size limit
// (Large map support)

// When big sized maps with high details cross about 9750 + few lines in
// IsoMapPack5 section, game doesn't decode those and fills those (typically
// bottom-left corner of the map) with height level 0 clear tiles.
// This patch raises the buffer usage limit to about 3 times the original.
// From 640 (0x280), 400 (0x190) and value of 512000 (= 640 * 400 * 2)
// To 1024 (0x400), 768 (0x300) and 1572864 (= 1024 * 768 * 2).

// Author: E1 Elite

DEFINE_PATCH_TYPED(DWORD, 0x4AD344, 0x300); // 0x190
DEFINE_PATCH_TYPED(DWORD, 0x4AD349, 0x400); // 0x280
DEFINE_PATCH_TYPED(DWORD, 0x4AD357, (0x300 * 0x400 * 2));
