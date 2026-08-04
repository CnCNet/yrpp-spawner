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
*  MPStatsFix - correct the command-count stall counter in Wait_For_Players.
*
*  Vanilla increments MPStats[v79].CommandCoundStalls (0x6497DC) whenever a
*  peer's command count is behind, but indexes the write with v79 - the
*  lowest-frame peer from an unrelated loop - instead of the actual culprit.
*  When there is still frame runway (the common packet-loss case: a peer's
*  commands lag but MaxAhead has not run out), v79 is forced to -1 just before
*  this write, so the increment lands on MPStats[-1].CommandCoundStalls, which
*  is exactly &ProcessingFrames (0xA8B564) - the frame-timing accumulator used
*  elsewhere for frame-rate negotiation.
*
*/

#include <Helpers/Macro.h>
#include <IPXManagerClass.h>
#include <SessionClass.h>
#include "FrameGate.h"

// Replaces the mis-indexed "++MPStats[v79].CommandCoundStalls" at 0x6497DC.
// Recomputes the real culprit - the first peer whose received command count
// is behind its sent count, instead of trusting v79 (often -1) or a clobbered register.
// Skips the write entirely if no peer is actually behind.
DEFINE_HOOK(0x6497DC, WaitForPlayers_CommandStallStat_Fix, 0x7)
{
	int nconn = static_cast<int>(IPXManagerClass::Instance.NumConnections);
	if (nconn > FrameGate::MaxPeers)
		nconn = FrameGate::MaxPeers;

	const TheirSync* their = FrameGate::Peers();

	int culprit = -1;
	for (int i = 0; i < nconn; ++i)
	{
		if (static_cast<unsigned int>(their[i].__recv) < static_cast<unsigned int>(their[i].__send))
		{
			culprit = i;
			break;
		}
	}

	if (culprit >= 0)
		++SessionClass::Instance.MPStats[culprit].CommandCoundStalls;

	return 0x6497E3;
}
