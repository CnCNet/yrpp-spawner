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

#include "FastRetransmit.h"
#include "PacketRedundancy.h"

#include <Helpers/Macro.h>
#include <SessionClass.h>
#include <GeneralDefinitions.h>
#include <IPXManagerClass.h>
#include <ConnectionClass.h>
#include <IPXConnClass.h>
#include <Utilities/Debug.h>

bool FastRetransmit::Enabled = true;
bool FastRetransmit::Backoff = true;

namespace
{
	struct PeerEstimator
	{
		const ConnectionClass* connection;
		bool initialized;
		int srtt;
		int rttvar;
		int rto;
		int cleanSamples;
	};

	PeerEstimator Peers[FastRetransmit::MaxPeers] = {};

	int ClampTicks(int value)
	{
		if (value < FastRetransmit::MinTicks)
			return FastRetransmit::MinTicks;
		if (value > FastRetransmit::MaxTicks)
			return FastRetransmit::MaxTicks;
		return value;
	}

	int ActiveConnectionCount()
	{
		const GameMode gm = SessionClass::Instance.GameMode;
		if (gm != GameMode::LAN && gm != GameMode::Internet)
			return 0;

		int nconn = static_cast<int>(IPXManagerClass::Instance.NumConnections);
		if (nconn < 0) nconn = 0;
		if (nconn > FastRetransmit::MaxPeers) nconn = FastRetransmit::MaxPeers;
		return nconn;
	}

	// True if connection is still one of the engine's live connections. A
	// reconnect replaces a peer's ConnectionClass with a new instance, and the
	// old pointer never comes back - without this check a stale slot's frozen
	// RTO sample sits in Peers[] forever and can dominate EffectiveRTO()'s max
	// long after the connection it came from is gone.
	bool IsLiveConnection(const ConnectionClass* connection)
	{
		if (!connection)
			return false;

		int nconn = static_cast<int>(IPXManagerClass::Instance.NumConnections);
		const int arraySize = sizeof(IPXManagerClass::Instance.Connection) / sizeof(IPXManagerClass::Instance.Connection[0]);
		if (nconn > arraySize) nconn = arraySize;

		for (int i = 0; i < nconn; ++i)
			if (IPXManagerClass::Instance.Connection[i] == connection)
				return true;
		return false;
	}

	void PruneDeadSlots()
	{
		for (int i = 0; i < FastRetransmit::MaxPeers; ++i)
			if (Peers[i].connection && !IsLiveConnection(Peers[i].connection))
				Peers[i] = PeerEstimator{};
	}

	// Finds this connection's estimator slot, allocating a free one or evicting
	// slot 0 if all MaxPeers slots are already in use.
	PeerEstimator* FindSlot(const ConnectionClass* connection)
	{
		if (!connection)
			return nullptr;

		PeerEstimator* freeSlot = nullptr;
		for (int i = 0; i < FastRetransmit::MaxPeers; ++i)
		{
			if (Peers[i].connection == connection)
				return &Peers[i];
			if (!Peers[i].connection && !freeSlot)
				freeSlot = &Peers[i];
		}

		if (!freeSlot)
			freeSlot = &Peers[0];

		*freeSlot = PeerEstimator{};
		freeSlot->connection = connection;
		return freeSlot;
	}
}

void FastRetransmit::Reset()
{
	for (int i = 0; i < MaxPeers; ++i)
		Peers[i] = PeerEstimator{};
}

// Feeds one clean (non-retransmitted) RTT sample into that peer's smoothed estimate.
void FastRetransmit::SampleRTT(const ConnectionClass* connection, int delayTicks, int sendCount)
{
	if (!Enabled)
		return;

	// Karn: ignore RTT measurements for retransmitted packets.
	if (sendCount > 1)
		return;
	if (delayTicks < 0 || delayTicks > MaxTicks)
		return;

	PeerEstimator* peer = FindSlot(connection);
	if (!peer)
		return;

	if (!peer->initialized)
	{
		peer->initialized = true;
		peer->srtt = delayTicks;
		peer->rttvar = delayTicks > 1 ? delayTicks / 2 : 1;
	}
	else
	{
		int err = peer->srtt - delayTicks;
		if (err < 0) err = -err;
		peer->rttvar = (peer->rttvar * 3 + err) / 4;
		if (peer->rttvar < 1) peer->rttvar = 1;
		peer->srtt = (peer->srtt * 7 + delayTicks) / 8;
	}

	int margin = peer->rttvar * 4;
	if (margin < MarginTicks)
		margin = MarginTicks;
	peer->rto = ClampTicks(peer->srtt + margin);
	++peer->cleanSamples;
}

int FastRetransmit::ActivePeers()
{
	return ActiveConnectionCount();
}

int FastRetransmit::InitializedPeers()
{
	int count = 0;
	for (int i = 0; i < MaxPeers; ++i)
		if (Peers[i].connection && Peers[i].initialized)
			++count;
	return count;
}

// Worst (max) RTO among peers sampled so far, or 0 if none have a clean sample yet.
int FastRetransmit::EffectiveRTO()
{
	if (ActiveConnectionCount() <= 0)
		return 0;

	PruneDeadSlots();

	int rto = 0;
	bool any = false;
	for (int i = 0; i < MaxPeers; ++i)
	{
		if (Peers[i].connection && Peers[i].initialized)
		{
			any = true;
			if (Peers[i].rto > rto)
				rto = Peers[i].rto;
		}
	}
	return any ? ClampTicks(rto) : 0;
}

int FastRetransmit::CleanSamples()
{
	int total = 0;
	for (int i = 0; i < MaxPeers; ++i)
		total += Peers[i].cleanSamples;
	return total;
}

// ConnectionClass::Service_Send_Queue, at the point an ACK'd PACKET_DATA_ACK
// entry's round-trip is about to be folded into the queue's response time.
DEFINE_HOOK(0x48C436, ServiceSendQueue_RTTSample_FastRetransmit, 0x8)
{
	if (FastRetransmit::Enabled)
	{
		const auto* entry = reinterpret_cast<const SendQueueType*>(R->EBX());
		int delay = static_cast<int>(R->EBP()) - entry->FirstTime;
		FastRetransmit::SampleRTT(reinterpret_cast<const ConnectionClass*>(R->EDI()), delay, entry->SendCount);
	}
	return 0;
}

// IPXManagerClass::Set_Timing entry. We overwrite retrydelta in place from the
// maximum clean per-peer RTO. Only shortens, never lengthens.
DEFINE_HOOK(0x540C60, IPXSetTiming_FastRetransmit, 0x8)
{
	if (!FastRetransmit::Enabled)
		return 0;

	int retryDelta = FastRetransmit::EffectiveRTO();
	if (retryDelta <= 0)
		return 0;

	int original = R->Stack<int>(0x4);
	if (retryDelta < original)
	{
		R->Stack<int>(0x4, retryDelta);

		static int lastLogged = -1;
		if (retryDelta != lastLogged)
		{
			lastLogged = retryDelta;
			Debug::Log("[FastRetransmit] RetryDelta %d -> %d ticks (peers=%d/%d clean=%d)\n",
				original, retryDelta,
				FastRetransmit::InitializedPeers(), FastRetransmit::ActivePeers(), FastRetransmit::CleanSamples());
		}
	}

	return 0;
}

// ConnectionClass::Service_Send_Queue, at the per-entry retransmit decision -
// also the one place a real retransmit is detected, so it feeds
// PacketRedundancy's loss signal even when FastRetransmit/Backoff are off.
DEFINE_HOOK(0x48C4AE, ServiceSendQueue_Backoff_FastRetransmit, 0x5)
{
	const auto* conn  = reinterpret_cast<const ConnectionClass*>(R->EDI());
	const auto* entry = reinterpret_cast<const SendQueueType*>(R->ESI());
	const int retryDelta = static_cast<int>(conn->RetryDelta);
	const int sendCount   = entry->SendCount;
	const int elapsed = static_cast<int>(R->EBP()) - R->ECX<int>();

	if (!FastRetransmit::Enabled || !FastRetransmit::Backoff)
	{
		if (elapsed > retryDelta)
			PacketRedundancy::NoteResend(reinterpret_cast<const ConnectionClass*>(R->EDI()));
		return 0;
	}

	int prior = sendCount - 1;
	if (prior < 0) prior = 0;
	if (prior > FastRetransmit::BackoffCap) prior = FastRetransmit::BackoffCap;

	int base = retryDelta;
	if (base < FastRetransmit::MinTicks)
		base = FastRetransmit::MinTicks;

	long long backedOff = static_cast<long long>(base)
		+ (static_cast<long long>(base) * prior * FastRetransmit::BackoffStepHalves) / 2;
	if (backedOff > 0x7FFFFFFF)
		backedOff = 0x7FFFFFFF;
	int eff = static_cast<int>(backedOff);

	if (elapsed > eff)
		PacketRedundancy::NoteResend(reinterpret_cast<const ConnectionClass*>(R->EDI()));

	R->EAX(eff);
	R->EDX(R->EBP());
	return 0x48C4B3;
}
