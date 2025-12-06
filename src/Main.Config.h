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
	bool MPDebug;
	bool QuickExit;
	bool SingleProcAffinity;
	bool SkipScoreScreen;
	bool SpeedControl;

	// Video
	bool NoWindowFrame;
	bool WindowedMode;
	int DDrawTargetFPS;

	// Other
	bool DumpTypes;
	bool NoCD;
	int RA2ModeSaveID;

	MainConfig()
		// Options
		: AllowChat { true }
		, AllowTaunts { true }
		, DDrawHandlesClose { false }
		, DisableEdgeScrolling { false }
		, MPDebug { false }
		, QuickExit { false }
		, SingleProcAffinity { true }
		, SkipScoreScreen { false }
		, SpeedControl { false }

		// Video
		, DDrawTargetFPS { -1 }
		, NoWindowFrame { false }
		, WindowedMode { false }

		// Other
		, DumpTypes { false }
		, NoCD { false }
		, RA2ModeSaveID { 0 }
	{ }

	void LoadFromINIFile();
	void ApplyStaticOptions();
};
