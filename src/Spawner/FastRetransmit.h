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
*  FastRetransmit - RTT-adaptive retransmit timer.
*
*  The engine recomputes RetryDelta as Response_Time() + 10. Response_Time()
*  includes ACK delays for retransmitted packets, so loss inflates the estimate
*  that controls the next retransmit wait. This keeps a clean per-connection RTT
*  estimator using Karn's rule and only shortens the engine's global timer once
*  any peer has a clean sample. Because the engine has one RetryDelta for the
*  whole match, we use the maximum peer RTO.
* https://en.wikipedia.org/wiki/Karn's_algorithm
*/

#pragma once

class ConnectionClass;

class FastRetransmit
{
public:
	static const int MaxPeers = 8;

	static bool Enabled;

	// Added to the smoothed round trip to absorb ordinary jitter.
	static const int MarginTicks = 2;
	// Never arm the timer shorter than this (guards the near-zero-RTT/LAN case).
	static const int MinTicks = 2;
	// Drop absurd / clock-glitch samples and cap computed RTOs to a sane range.
	static const int MaxTicks = 250;

	// Gentle retransmit backoff. Requires both FastRetransmit and
	// RetransmitBackoff to be enabled before it changes retransmit decisions.
	static bool Backoff;
	static const int BackoffStepHalves = 1; // +1/2 of base per prior retry
	static const int BackoffCap = 4;        // max prior-retries counted (up to 3x base)

	static void Reset();
	static void SampleRTT(const ConnectionClass* connection, int delayTicks, int sendCount);
	static int EffectiveRTO();
	static int InitializedPeers();
	static int ActivePeers();
	static int CleanSamples();
};
