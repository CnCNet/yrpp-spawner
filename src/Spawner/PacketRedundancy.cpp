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

#include "PacketRedundancy.h"

#include <windows.h>
#include <IPXManagerClass.h>
#include <ConnectionClass.h>
#include <Utilities/Debug.h>

bool PacketRedundancy::Enabled  = true;
int  PacketRedundancy::Copies   = 2;
bool PacketRedundancy::Adaptive = false;

namespace
{
	constexpr size_t HeaderOffset = 4;
	constexpr size_t CodeOffset = HeaderOffset + offsetof(CommHeaderType, Code);

	const int  GaugeBump = 1000;
	const int  GaugeCap = 5000;
	const int  GaugeMsPerUnit = 1;

	int   g_gauge[PacketRedundancy::MaxPeers] = {};
	DWORD g_lastTick[PacketRedundancy::MaxPeers] = {};

	bool ValidPeer(int peer)
	{
		return peer >= 0 && peer < PacketRedundancy::MaxPeers;
	}

	void DecayGauge(int peer)
	{
		if (!ValidPeer(peer))
			return;

		DWORD now = GetTickCount();
		if (g_lastTick[peer] == 0)
		{
			g_lastTick[peer] = now;
			return;
		}

		DWORD dt = now - g_lastTick[peer];
		g_lastTick[peer] = now;
		int dec = static_cast<int>(dt) / GaugeMsPerUnit;
		g_gauge[peer] = (dec >= g_gauge[peer]) ? 0 : (g_gauge[peer] - dec);
	}

	void BumpGauge(int peer)
	{
		if (!ValidPeer(peer))
			return;

		DecayGauge(peer);
		g_gauge[peer] += GaugeBump;
		if (g_gauge[peer] > GaugeCap)
			g_gauge[peer] = GaugeCap;
	}

	int PeerIndexForConnection(const ConnectionClass* connection)
	{
		if (!connection)
			return -1;

		int nconn = static_cast<int>(IPXManagerClass::Instance.NumConnections);
		const int arraySize = sizeof(IPXManagerClass::Instance.Connection) / sizeof(IPXManagerClass::Instance.Connection[0]);
		if (nconn > arraySize) nconn = arraySize;

		for (int i = 0; i < nconn; ++i)
			if (reinterpret_cast<const ConnectionClass*>(IPXManagerClass::Instance.Connection[i]) == connection)
				return i;

		return -1;
	}
}

void PacketRedundancy::Reset()
{
	for (int i = 0; i < MaxPeers; ++i)
	{
		g_gauge[i] = 0;
		g_lastTick[i] = 0;
	}
}

int PacketRedundancy::ClampCopies(int copies)
{
	return copies == 1 ? 1 : 2;
}

// Bumps the loss gauge for whichever peer this resend belongs to (or every
// peer if the connection can't be resolved, so the signal is never dropped).
void PacketRedundancy::NoteResend(const ConnectionClass* connection)
{
	int peer = PeerIndexForConnection(connection);
	if (ValidPeer(peer))
	{
		BumpGauge(peer);
		return;
	}

	// Shouldn't happen (connection always belongs to one of the live slots),
	// but fall back to bumping everyone rather than silently dropping the signal.
	for (int i = 0; i < MaxPeers; ++i)
		BumpGauge(i);
}

// Current loss gauge for peer, decayed up to now.
int PacketRedundancy::LossGauge(int peer)
{
	if (!ValidPeer(peer))
		return 0;

	DecayGauge(peer);
	return g_gauge[peer];
}

void PacketRedundancy::NoteExtraSend(int sendResult)
{
	if (sendResult == -1)
		Debug::Log("[PacketRedundancy] extra sendto() for a duplicate copy failed\n");
}

// Decides how many times to send this outbound datagram: only reliable
// (PACKET_DATA_ACK) packets duplicate, and only while enabled and (if
// Adaptive) actual loss is being observed for this peer.
int PacketRedundancy::CopiesFor(const char* buf, size_t len, int peer)
{
	if (!Enabled || !buf || len <= CodeOffset)
		return 1;

	if (Copies < 2)
		return 1;

	const auto* header = reinterpret_cast<const CommHeaderType*>(buf + HeaderOffset);
	if (header->Code != ConnectionEnum::PACKET_DATA_ACK)
		return 1;

	if (Adaptive && LossGauge(peer) <= 0)
		return 1;

	return Copies;
}
