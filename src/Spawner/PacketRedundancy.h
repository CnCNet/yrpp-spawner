/**
*  yrpp-spawner
*
*  Copyright(C) 2023-present CnCNet
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

/**
*  PacketRedundancy - outbound redundancy for reliable command packets.
*
*  Reliable command packets (CommHeader Code == PACKET_DATA_ACK) are sent twice
*  on the wire so an isolated loss can be recovered without waiting for ARQ
*  retransmit. The duplicate is byte-identical and rides below the engine's
*  reliable/in-order layer, which acknowledges duplicate PacketIDs without
*  delivering commands twice.
*/

#pragma once
#include <stddef.h>

class ConnectionClass;

class PacketRedundancy
{
public:
	static const int MaxPeers = 8;

	static bool Enabled;
	static int  Copies;
	// When true, only duplicate to peers where recent packet loss is observed.
	static bool Adaptive;

	static void Reset();
	static int ClampCopies(int copies);

	// Number of times this outbound datagram should be sent to peer (1 == send
	// once, no duplication). buf points at the on-wire game bytes
	// ([CRC(4)][CommHeader...]).
	static int CopiesFor(const char* buf, size_t len, int peer);

	// Loss signal for adaptive mode.
	static void NoteResend(const ConnectionClass* connection);
	static int LossGauge(int peer);

	// Logs a failed extra sendto() call for a duplicate copy.
	static void NoteExtraSend(int sendResult);
};
