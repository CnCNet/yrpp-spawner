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

// New, spawner-added global network packet types.
//
// This mirrors Vinifera's ExtGlobalPacketType: the engine's out-of-band global
// packet (GlobalPacketType) is 0x1C7 = 455 bytes, and the IPX layer accepts a
// raw buffer of that size, so an alternative struct of the same size can ride
// the very same channel. Command sits at offset 0 (matching GlobalPacketType),
// which is what Network_Call_Back's dispatch switch reads; commands the engine
// does not know fall through to its `default` case, which is where the receiver
// for these is meant to be hooked in.

#pragma once

// Command values for the spawner's own global packets.
//
// The engine's native command values run up to ~0x2F. These are placed well
// above that range so they never collide and always reach the unknown-command
// (default) dispatch path.
enum ExtNetCommandType : int
{
	EXT_NET_DESYNC_HEARTBEAT = 0xE0, // Periodic keep-alive while the desync dialog is open; also detects departures.
	EXT_NET_DESYNC_CONTINUE  = 0xE1, // The host's decision to continue the game without the desynced players.
	EXT_NET_DESYNC_CHAT      = 0xE2, // A chat line typed in the desync dialog while game logic is halted.
	EXT_NET_HOST_ANNOUNCE    = 0xE3, // The host announcing itself at game start so everyone records the master.
};

#pragma pack(push, 1)
struct ExtGlobalPacketType
{
	// Must alias GlobalPacketType::Command (offset 0) so the engine dispatch
	// reads it correctly.
	int Command;

	// Sender's display name (ANSI; converted from the wide UIName on send).
	char Name[32];

	union
	{
		struct
		{
			char HouseID; // Sender's house (ArrayIndex).
			char IsHost;  // Non-zero if the sender is the game master.
		} Heartbeat;

		struct
		{
			char SenderHouseID;
			char Text[200]; // ANSI chat text.
		} Chat;

		// Forces the whole struct to the engine's GlobalPacketType size so the
		// IPX layer treats it identically.
		char _padding[455 - sizeof(int) - 32];
	};
};
#pragma pack(pop)

static_assert(sizeof(ExtGlobalPacketType) == 455, "ExtGlobalPacketType must match the engine's GlobalPacketType (0x1C7 bytes)");
