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

#include "FrameGate.h"

#include <Helpers/Macro.h>
#include <Fundamentals.h>
#include <Unsorted.h>
#include <SessionClass.h>
#include <GeneralDefinitions.h>
#include <IPXManagerClass.h>
#include <EventClass.h>
#include <Utilities/Debug.h>

#include <climits>

bool FrameGate::Enabled = true;

namespace
{
	int  SafeThrough[FrameGate::MaxPeers];
	bool Inited = false;

	int  lastMaxAhead = -1;
	int  lastFSR = -1;
	int  guardUntilFrame = 0;

	int  lateDataLogged = 0;

	void ClearWatermarks()
	{
		for (int i = 0; i < FrameGate::MaxPeers; ++i)
			SafeThrough[i] = INT_MIN;
	}

	void InitOnce()
	{
		if (Inited)
			return;
		ClearWatermarks();
		Inited = true;
	}

	void CheckEpoch(int frame, int ma, int fsr)
	{
		bool shrink = (lastMaxAhead >= 0 && ma < lastMaxAhead)
		           || (lastFSR      >= 0 && fsr < lastFSR);
		if (shrink)
		{
			ClearWatermarks();
			guardUntilFrame = frame + lastMaxAhead + fsr + 4;
			Debug::Log("[FrameGate] timing shrink at frame %d: MaxAhead %d->%d FSR %d->%d, guard until frame %d\n",
				frame, lastMaxAhead, ma, lastFSR, fsr, guardUntilFrame);
		}
		lastMaxAhead = ma;
		lastFSR = fsr;
	}
}

void FrameGate::Reset()
{
	InitOnce();
	ClearWatermarks();
	lastMaxAhead = -1;
	lastFSR = -1;
	guardUntilFrame = 0;
	lateDataLogged = 0;
}

// True if peer i either meets vanilla's raw command-count test, or its commands
// through the current frame are provably all in hand per SafeThrough[i].
bool FrameGate::AllCommandsSatisfied(TheirSync* peers, int* gapIndex)
{
	InitOnce();
	*gapIndex = -1;

	int nconn = static_cast<int>(IPXManagerClass::Instance.NumConnections);
	if (nconn < 0) nconn = 0;
	if (nconn > MaxPeers) nconn = MaxPeers;

	if (!peers)
		peers = Peers();

	const int frame = Unsorted::CurrentFrame;
	const int ma    = Game::Network::MaxAhead;
	const int fsr   = Game::Network::FrameSendRate;

	CheckEpoch(frame, ma, fsr);
	const bool relaxOK = Enabled && frame >= guardUntilFrame;

	bool allSat = true;

	for (int i = 0; i < nconn; ++i)
	{
		if (static_cast<unsigned int>(peers[i].__recv) >= static_cast<unsigned int>(peers[i].__send))
			continue;

		if (relaxOK && SafeThrough[i] >= frame)
			continue;

		allSat = false;
		if (*gapIndex < 0)
			*gapIndex = i;
	}

	return allSat;
}

// Updates SafeThrough[peer] from one received FRAMEINFO/FRAMESYNC packet's own
// Frame + cumulative CommandCount.
void FrameGate::OnReceive(unsigned int theirEntry, const unsigned char* evBytes)
{
	InitOnce();
	if (!Enabled || !evBytes)
		return;

	// Derive the array base from the YRpp binding rather than repeating its
	// address, so the two cannot drift apart.
	const auto theirBase = reinterpret_cast<unsigned>(Peers());
	if (theirEntry < theirBase)
		return;

	const unsigned offset = theirEntry - theirBase;
	if ((offset % sizeof(TheirSync)) != 0)
		return;

	const unsigned idx = offset / sizeof(TheirSync);
	if (idx >= MaxPeers)
		return;

	const EventClass* ev = reinterpret_cast<const EventClass*>(evBytes);
	if (ev->Type != EventType::FrameInfo && ev->Type != EventType::FrameSync)
		return;

	if (Unsorted::CurrentFrame < guardUntilFrame)
		return;

	const int F = static_cast<int>(ev->Frame);

	const int C = ev->FrameInfo.CommandCount;
	const int recv = reinterpret_cast<const TheirSync*>(theirEntry)->__recv;


	if (ev->Type == EventType::FrameInfo && F < Unsorted::CurrentFrame && lateDataLogged < 32)
	{
		Debug::Log("[FrameGate] late data: peer=%u stamp=%d current=%d cum=%d recv=%d safeThrough=%d\n",
			idx, F, Unsorted::CurrentFrame, C, recv, SafeThrough[idx]);
		++lateDataLogged;
	}

	if (C <= recv && (F - 1) > SafeThrough[idx])
		SafeThrough[idx] = F - 1;
}

// Replaces vanilla's command-count loop in Wait_For_Players (the raw
// recv>=sent test) with FrameGate::AllCommandsSatisfied.
DEFINE_HOOK(0x6495D5, WaitForPlayers_FrameAwareGate, 0x7)
{
	enum { Satisfied = 0x6495F9, Gapped = 0x649610 };

	const GameMode gm = SessionClass::Instance.GameMode;
	if (gm != GameMode::LAN && gm != GameMode::Internet)
		return 0;
	if (!FrameGate::Enabled)
		return 0;

	GET_STACK(TheirSync*, peers, 0x748);

	int gap = -1;
	if (FrameGate::AllCommandsSatisfied(peers, &gap))
		return Satisfied;

	R->ESI(gap);
	return Gapped;
}

// Feeds every received data/framesync packet to FrameGate::OnReceive so
// SafeThrough stays current.
DEFINE_HOOK(0x64A3F9, ProcessReceivePacket_FrameGateRecord, 0x9)
{
	GET(unsigned int, theirEntry, EBP);
	GET(const unsigned char*, evBytes, EDI);

	FrameGate::OnReceive(theirEntry, evBytes);
	return 0;
}
