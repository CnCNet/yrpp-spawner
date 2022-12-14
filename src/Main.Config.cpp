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

#include "Main.Config.h"

#include <CCINIClass.h>
#include <GameOptionsClass.h>
#include <Utilities/Macro.h>

void MainConfig::LoadFromINIFile()
{
	auto pINI = &CCINIClass::INI_RA2MD;
	if (!pINI)
		return;

	const char* pOptionsSection = "Options";
	if (pINI->GetSection(pOptionsSection))
	{
		this->SingleProcAffinity   = pINI->ReadBool(pOptionsSection, "SingleProcAffinity", this->SingleProcAffinity);
		this->DisableEdgeScrolling = pINI->ReadBool(pOptionsSection, "DisableEdgeScrolling", this->DisableEdgeScrolling);
		this->QuickExit            = pINI->ReadBool(pOptionsSection, "QuickExit", this->QuickExit);
		this->SkipScoreScreen      = pINI->ReadBool(pOptionsSection, "SkipScoreScreen", this->SkipScoreScreen);
	}

	const char* pVideoSection = "Video";
	if (pINI->GetSection(pOptionsSection))
	{
		this->WindowedMode   = pINI->ReadBool(pVideoSection, "Video.Windowed", this->WindowedMode);
		this->NoWindowFrame  = pINI->ReadBool(pVideoSection, "NoWindowFrame", this->NoWindowFrame);
		this->DDrawTargetFPS = pINI->ReadInteger(pVideoSection, "DDrawTargetFPS", this->DDrawTargetFPS);
	}
}

DEFINE_DYNAMIC_PATCH(NoWindowFrame_PATCH, 0x777CC0,
	0x68, 0x00, 0x00, 0x0A, 0x86 // push    0x860A0000; vs 0x02CA0000
);

void MainConfig::ApplyStaticOptions()
{
	if (this->SingleProcAffinity)
	{
		auto const process = GetCurrentProcess();
		DWORD_PTR const processAffinityMask = 1;
		SetProcessAffinityMask(process, processAffinityMask);
	}

	if(this->WindowedMode)
	{
		GameOptionsClass::WindowedMode = true;

		if(this->NoWindowFrame)
			NoWindowFrame_PATCH->Apply();
	}

	// Set 3rd party ddraw.dll options
	if (HMODULE hDDraw = LoadLibraryA("ddraw.dll"))
	{
		if (bool* handleClose = (bool*)GetProcAddress(hDDraw, "GameHandlesClose"))
			*handleClose = true;

		LPDWORD TargetFPS = (LPDWORD)GetProcAddress(hDDraw, "TargetFPS");
		if (TargetFPS && this->DDrawTargetFPS != -1)
			*TargetFPS = this->DDrawTargetFPS;
	}
}
