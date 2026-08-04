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
*  FrameGate - frame-aware advance gate.
*
*  Vanilla gates frame advance on a raw per-peer command COUNT
*  (their[i].__recv >= their[i].__send). That test carries no frame
*  information, so a single lost command packet stalls the whole lobby even
*  though the missing commands are stamped for a frame nobody has reached yet.
*
*  This replaces the count test with the one it was approximating: we may
*  execute the current frame if, for every peer, all of that peer's commands
*  stamped for frames <= the current frame are already in hand. We prove that
*  from the packet stream: a packet stamped F carrying cumulative count C means
*  "commands 1..C are all stamped <= F"; once our __recv >= C we hold them, so
*  every frame <= F is safe with respect to that peer.
*
*/

#pragma once

#include <TheirSync.h>

class FrameGate
{
public:
	static const int MaxPeers = 8;

	static bool Enabled;

	static TheirSync* Peers()
	{
		return TheirSync::Array;
	}

	static void Reset();

	// Called in place of the vanilla command-count loop. Returns true if every
	// peer's commands for the current frame are in hand (vanilla recv>=send OR
	// the frame-aware relaxation). On false, *gapIndex is the first blocking
	// peer, for the engine's existing stall bookkeeping.
	static bool AllCommandsSatisfied(TheirSync* peers, int* gapIndex);

	// Records the watermark for one received data/framesync packet.
	// theirEntry = &their[index]; ev = the packet header.
	static void OnReceive(unsigned int theirEntry, const unsigned char* ev);
};
