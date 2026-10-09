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

#pragma once

// Game-independent description of what an observer sees, written to disk for the streamer overlay.
// This header and Snapshot.cpp must not include YRpp or Windows headers so they can be unit tested
// on any platform (see tools/ObserverOverlay/tests/snapshot_json_test.cpp).

#include <string>
#include <string_view>
#include <vector>

namespace ObserverOverlay
{
	// Bump when the JSON layout changes in a way the overlay page needs to know about.
	constexpr int SnapshotVersion = 1;

	struct TypeEntry
	{
		std::string ID;   // INI ID, e.g. "HTNK"
		std::string Name; // UI name in UTF-8, e.g. "Rhino Tank"
	};

	struct FactoryState
	{
		std::string Category;          // "Building", "Defense", "Infantry", "Vehicle", "Naval" or "Aircraft"
		bool HasCurrent = false;       // false when the factory only has queued items
		TypeEntry Current;             // the item being built right now
		int ProgressPercent = 0;       // 0-100
		bool OnHold = false;           // paused, e.g. out of money or put on hold by the player
		bool IsDone = false;           // finished and waiting (e.g. a building waiting to be placed)
		std::vector<TypeEntry> Queue;  // items queued after the current one
	};

	struct CountEntry
	{
		std::string Category; // "Building", "Infantry", "Vehicle" or "Aircraft"
		TypeEntry Type;
		int Count = 0;
	};

	struct PlayerState
	{
		int Index = -1;          // HouseClass::ArrayIndex
		std::string Name;        // player name in UTF-8
		std::string Country;     // HouseTypeClass ID, e.g. "Russians"
		unsigned char ColorR = 0;
		unsigned char ColorG = 0;
		unsigned char ColorB = 0;
		int Credits = 0;
		int PowerOutput = 0;
		int PowerDrain = 0;
		bool Defeated = false;
		std::vector<FactoryState> Production;
		std::vector<CountEntry> Counts; // only types with a non-zero count
	};

	struct Snapshot
	{
		int Frame = 0;
		std::vector<PlayerState> Players;
	};

	// Appends value as a quoted, escaped JSON string. value must be UTF-8.
	void AppendJsonString(std::string& out, std::string_view value);

	// Serializes the snapshot as a single JSON object.
	std::string ToJson(const Snapshot& snapshot);

	// Converts FactoryClass progress steps (0..totalSteps) to a 0-100 percentage, clamped.
	int ProgressToPercent(int steps, int totalSteps);
}
