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
	bool MPDebug;
	bool DumpTypes;
	int RA2ModeSaveID;

	bool SingleProcAffinity;
	bool DisableEdgeScrolling;
	bool QuickExit;
	bool SkipScoreScreen;
	bool DDrawHandlesClose;
	bool SpeedControl;

	bool WindowedMode;
	bool NoWindowFrame;
	int  DDrawTargetFPS;

	void LoadFromINIFile();
	void ApplyStaticOptions();

	MainConfig()
		: MPDebug { false }
		, DumpTypes { false }

		, SingleProcAffinity { true }
		, DisableEdgeScrolling { false }
		, QuickExit { false }
		, SkipScoreScreen { false }
		, DDrawHandlesClose { false }
		, SpeedControl { false }

		, WindowedMode { false }
		, NoWindowFrame { false }
		, DDrawTargetFPS { -1 }
	{ }
};
