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
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <CCINIClass.h>
#include <GameOptionsClass.h>
#include <Unsorted.h>

void MainConfig::LoadFromINIFile()
{
	auto pINI = &CCINIClass::INI_RA2MD;
	if (!pINI)
		return;

	const char* pOptionsSection = "Options";
	if (pINI->GetSection(pOptionsSection))
	{
		this->AllowChat            = pINI->ReadBool(pOptionsSection, "AllowChat", this->AllowChat);
		this->AllowTaunts          = pINI->ReadBool(pOptionsSection, "AllowTaunts", this->AllowTaunts);
		this->DDrawHandlesClose    = pINI->ReadBool(pOptionsSection, "DDrawHandlesClose", this->DDrawHandlesClose);
		this->DisableEdgeScrolling = pINI->ReadBool(pOptionsSection, "DisableEdgeScrolling", this->DisableEdgeScrolling);
		this->QuickExit            = pINI->ReadBool(pOptionsSection, "QuickExit", this->QuickExit);
		this->SingleProcAffinity   = pINI->ReadBool(pOptionsSection, "SingleProcAffinity", this->SingleProcAffinity);
		this->SkipScoreScreen      = pINI->ReadBool(pOptionsSection, "SkipScoreScreen", this->SkipScoreScreen);
		this->SpeedControl         = pINI->ReadBool(pOptionsSection, "SpeedControl", this->SpeedControl);

		// Keep the legacy [Options] MPDEBUG setting for compatibility; [Debug] MPDebug takes precedence.
		this->MPDebug              = pINI->ReadBool(pOptionsSection, "MPDEBUG", this->MPDebug);
	}

	const char* pVideoSection = "Video";
	if (pINI->GetSection(pVideoSection))
	{
		this->DDrawTargetFPS = pINI->ReadInteger(pVideoSection, "DDrawTargetFPS", this->DDrawTargetFPS);
		this->NoWindowFrame  = pINI->ReadBool(pVideoSection, "NoWindowFrame", this->NoWindowFrame);
		this->WindowedMode   = pINI->ReadBool(pVideoSection, "Video.Windowed", this->WindowedMode);
	}

	const char* pDebugSection = "Debug";
	if (pINI->GetSection(pDebugSection))
	{
		this->DumpTypes          = this->DumpTypes || pINI->ReadBool(pDebugSection, "DumpTypes", this->DumpTypes);
		this->ForceMultiplayer   = pINI->ReadBool(pDebugSection, "ForceMultiplayer", this->ForceMultiplayer);
		this->MPDebug            = pINI->ReadBool(pDebugSection, "MPDebug", this->MPDebug);
		this->MPDebugShow        = pINI->ReadBool(pDebugSection, "MPDebug.Show", this->MPDebugShow);
		this->SkipCreateAppMutex = pINI->ReadBool(pDebugSection, "SkipCreateAppMutex", this->SkipCreateAppMutex);
		this->WriteStatistics    = pINI->ReadBool(pDebugSection, "WriteStatistics", this->WriteStatistics);
	}
}

void MainConfig::ApplyStaticOptions()
{
	if (this->MPDebug)
	{
		Game::EnableMPDebug     = true;
		Game::EnableMPSyncDebug = true;
		Game::DrawMPDebugStats  = this->MPDebugShow;
	}

	if (this->SingleProcAffinity)
	{
		auto const process = GetCurrentProcess();
		DWORD_PTR const processAffinityMask = 1;
		SetProcessAffinityMask(process, processAffinityMask);
	}

	if (this->WindowedMode)
	{
		GameOptionsClass::WindowedMode = true;

		if (this->NoWindowFrame)
			Patch::Apply_RAW(0x777CC0, // CreateMainWindow
			{
				0x68, 0x00, 0x00, 0x0A, 0x86 // push    0x860A0000; vs 0x02CA0000
			});
	}

	if (this->SpeedControl)
	{
		auto& speedControl = *reinterpret_cast<bool*>(0xA8EDDCu);
		speedControl = true;
	}

	auto& LANTaunts = *reinterpret_cast<bool*>(0xA8D110u);
	LANTaunts = this->AllowTaunts;

	// Set 3rd party ddraw.dll options
	if (HMODULE hDDraw = LoadLibraryA("ddraw.dll"))
	{
		if (bool* gameHandlesClose = (bool*)GetProcAddress(hDDraw, "GameHandlesClose"))
			*gameHandlesClose = !this->DDrawHandlesClose;

		LPDWORD TargetFPS = (LPDWORD)GetProcAddress(hDDraw, "TargetFPS");
		if (TargetFPS && this->DDrawTargetFPS != -1)
			*TargetFPS = this->DDrawTargetFPS;
	}
}
