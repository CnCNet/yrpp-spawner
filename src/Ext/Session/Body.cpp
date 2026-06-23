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

#include "Body.h"

#include <Spawner/GlobalPacketExt.h>

#include <SessionClass.h>
#include <HouseClass.h>
#include <IPX.h>
#include <IPXManagerClass.h>

#include <Utilities/Macro.h>

#include <cwchar>

bool SessionExt::IsOutOfSync[SessionExt::MaxPlayers] = {};

bool SessionExt::Is_Out_of_Sync(int house_id)
{
	if (house_id < 0 || house_id >= MaxPlayers)
		return false;

	return IsOutOfSync[house_id];
}

void SessionExt::Mark_Player_As_Out_of_Sync(int house_id)
{
	if (house_id < 0 || house_id >= MaxPlayers)
		return;

	IsOutOfSync[house_id] = true;
}

void SessionExt::Set_Master(int house_id)
{
	SessionClass::Instance.MasterPlayerID() = house_id;

	wchar_t* const master_name = SessionClass::Instance.MasterPlayerName();
	master_name[0] = L'\0';

	if (HouseClass::Array.Count > house_id && house_id >= 0)
	{
		if (HouseClass* house = HouseClass::Array[house_id])
		{
			// MasterPlayerName is wchar_t[21]; UIName is the same width.
			wcsncpy(master_name, house->UIName, 20);
			master_name[20] = L'\0';
		}
	}
}

void SessionExt::Announce_Master()
{
	HouseClass* const me = HouseClass::CurrentPlayer;
	if (me == nullptr)
		return;

	// Record ourselves as the master locally; we never receive our own packet.
	Set_Master(me->ArrayIndex);

	ExtGlobalPacketType packet {};
	packet.Command = EXT_NET_HOST_ANNOUNCE;
	packet.Heartbeat.HouseID = static_cast<char>(me->ArrayIndex);
	packet.Heartbeat.IsHost = 1;

	// Send to every other player. As in the engine's own beacons, index 0 is the
	// local player, so start at 1. (Addresses come from the connections that were
	// just created.)
	auto& players = NodeNameType::Array;
	for (int i = 1; i < players.Count; i++)
	{
		NodeNameType* const node = players[i];
		if (node == nullptr)
			continue;

		IPXManagerClass::Instance.Send_Global_Message(
			&packet, sizeof(packet), 1,
			reinterpret_cast<IPXAddressClass*>(&node->Address), 0, 0);
	}
	IPXManagerClass::Instance.Service();
}

void SessionExt::Update_Master_After_Player_Removal()
{
	// Mirrors Vinifera: decide the master from the connected-players list
	// (NodeNameType::Array == SessionClass::Players, 0xA8DA74), which
	// Destroy_Connection shrinks immediately - even while the desync dialog has
	// game logic suspended. We must NOT use the houses' Defeated flag here: a
	// departed house is only flagged defeated when its queued E_REMOVEPLAYER
	// event runs, which never happens while suspended, so the old master would
	// keep looking valid and never get replaced. node->HouseIndex is the house
	// id (the same value Set_Master/MasterPlayerID use).
	const int current = SessionClass::Instance.MasterPlayerID();
	auto& players = NodeNameType::Array;

	// If the current master is still connected, there is nothing to do.
	if (current != -1)
	{
		for (int i = 0; i < players.Count; i++)
		{
			NodeNameType* const node = players[i];
			if (node != nullptr && node->HouseIndex == current)
				return;
		}
	}

	// Otherwise promote the connected player with the lowest house id. This is
	// deterministic, so every remaining client agrees without negotiation.
	int new_master = -1;
	for (int i = 0; i < players.Count; i++)
	{
		NodeNameType* const node = players[i];
		if (node == nullptr)
			continue;

		const int id = node->HouseIndex;
		if (id >= 0 && (new_master == -1 || id < new_master))
			new_master = id;
	}

	if (new_master != -1 && new_master != current)
		Set_Master(new_master);
}

/**
 *  Replacement for SessionClass::Am_I_Master (0x697E70).
 *
 *  The vanilla implementation only consults MasterPlayerID/MasterPlayerName in
 *  GameMode::Internet (WOL) sessions, falling back to "the first non-defeated
 *  human house is the master" otherwise. Spawner multiplayer games run as
 *  GameMode::LAN, so the host/master we record through Set_Master (from the
 *  EXT_NET_HOST_ANNOUNCE at game start) would be ignored. Extend the check to LAN
 *  so the announced master is honoured - including after host migration. Mirrors
 *  Vinifera's SessionClassExt::_Am_I_Master.
 *
 *  __fastcall(ECX=this, EDX unused, stack: who) reproduces the original __thiscall
 *  (retn 4); the whole function is replaced via DEFINE_FUNCTION_JUMP below.
 */
static bool __fastcall SessionClass_Am_I_Master(SessionClass* pThis, void*, HouseClass* who)
{
	if (who == nullptr)
		who = HouseClass::CurrentPlayer;

	if ((pThis->GameMode == GameMode::Internet || pThis->GameMode == GameMode::LAN) && who != nullptr)
	{
		const int master = pThis->MasterPlayerID();
		if (master != -1)
			return who->ArrayIndex == master;

		if (_wcsicmp(who->UIName, pThis->MasterPlayerName()) == 0)
			return true;
	}

	// Fallback: the first non-defeated human house is the master.
	for (int i = 0; i < HouseClass::Array.Count; i++)
	{
		HouseClass* const house = HouseClass::Array[i];
		if (house && house->IsHumanPlayer && !house->Defeated)
			return who == house;
	}

	return false;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x697E70, SessionClass_Am_I_Master)
