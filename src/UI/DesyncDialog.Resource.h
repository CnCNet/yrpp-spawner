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

// Resource and control IDs for the multiplayer desync dialog.
// Shared between the .rc script and the dialog code.
//
// Per the spawner convention, brand-new control IDs start at 5000 so they never
// collide with the engine's own dialog/control IDs.

#pragma once

// Dialog templates (live in this DLL; found via the FetchResource hook).
#define IDD_DESYNC_HOST            5000 // Game master: Continue/Quit.
#define IDD_DESYNC_WAIT            5001 // Everyone else: wait for the master.

// Controls.
#define IDC_DESYNC_HEADER          5010
#define IDC_DESYNC_PLAYER_LIST     5011
#define IDC_DESYNC_CHAT_LIST       5012
#define IDC_DESYNC_CHAT_EDIT       5013
#define IDC_DESYNC_CONTINUE        5021
#define IDC_DESYNC_QUIT            5022
