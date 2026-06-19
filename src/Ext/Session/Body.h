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

// Re-created bits of Vinifera's SessionClassExtension that the desync dialog
// needs. YR's SessionClass has no equivalent of these (it only has a single
// global OutOfSync bool and no per-player tracking), so this holds the new
// state spawner-side. The game's native master/host fields (MasterPlayerID /
// MasterPlayerName) DO exist and are written through here.

#pragma once

class HouseClass;

namespace SessionExt
{
	// The engine supports up to 8 multiplayer houses.
	constexpr int MaxPlayers = 8;

	// --- Per-player out-of-sync tracking (the engine only has one global flag).
	//     Populated by the desync-detection hook (to be wired in separately) and
	//     read by the dialog to colour each player's status.
	extern bool IsOutOfSync[MaxPlayers];

	// Frame the game first went out of sync, or -1.
	extern int OutOfSyncFrame;

	// Scope flag for an outgoing chat message (true = allies only). Kept for
	// parity with Vinifera; the desync dialog broadcasts to everyone.
	extern bool IsChatToAllies;

	bool Is_Out_of_Sync(int house_id);
	void Mark_Player_As_Out_of_Sync(int house_id);
	void Clear_Out_Of_Sync_Data();

	// True when running under the spawner (the analog of Vinifera's
	// SessionClassExtension::IsSpawnerSession).
	bool Is_Spawner_Session();

	// Assigns the game master/host, writing the engine's native MasterPlayerID
	// and MasterPlayerName so SessionClass::Am_I_Master() agrees.
	void Set_Master(int house_id);

	// Called on the host at game start: records itself as the master and tells
	// every other player who the host is (EXT_NET_HOST_ANNOUNCE), so MasterPlayerID
	// is authoritative on all machines before any desync. Mirrors Vinifera's
	// SessionClassExtension::Announce_Master.
	void Announce_Master();

	// Recomputes the master after a player has been removed: if the current
	// master is gone, promotes the first remaining non-defeated human house.
	void Update_Master_After_Player_Removal();
}
