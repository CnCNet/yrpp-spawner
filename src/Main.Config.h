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
class MainConfig
{
public:
	// Options
	bool AllowChat;
	bool AllowTaunts;
	bool DDrawHandlesClose;
	bool DisableEdgeScrolling;
	bool QuickExit;
	bool SingleProcAffinity;
	bool SkipScoreScreen;
	bool SpeedControl;

	// Observer overlay for streamers, see src/Misc/ObserverOverlay/Export.cpp
	bool ObserverOverlay;
	int ObserverOverlayInterval;

	// Video
	bool NoWindowFrame;
	bool WindowedMode;
	int DDrawTargetFPS;

	// Debug
	bool DumpTypes;
	bool ForceMultiplayer;
	bool MPDebug;
	bool MPDebugShow;
	bool SkipCreateAppMutex;
	bool WriteStatistics;

	// Other
	bool NoCD;
	int RA2ModeSaveID;

	MainConfig()
		// Options
		: AllowChat { true }
		, AllowTaunts { true }
		, DDrawHandlesClose { false }
		, DisableEdgeScrolling { false }
		, QuickExit { false }
		, SingleProcAffinity { true }
		, SkipScoreScreen { false }
		, SpeedControl { false }

		// Observer overlay
		, ObserverOverlay { false }
		, ObserverOverlayInterval { 15 }

		// Video
		, DDrawTargetFPS { -1 }
		, NoWindowFrame { false }
		, WindowedMode { false }

		// Debug
		, DumpTypes { false }
		, ForceMultiplayer { false }
		, MPDebug { false }
		, MPDebugShow { true }
		, SkipCreateAppMutex { false }
		, WriteStatistics { false }

		// Other
		, NoCD { false }
		, RA2ModeSaveID { 0 }
	{ }

	void LoadFromINIFile();
	void ApplyStaticOptions();
};
