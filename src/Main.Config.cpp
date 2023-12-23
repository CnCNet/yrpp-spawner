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
		this->MPDebug              = pINI->ReadBool(pOptionsSection, "MPDEBUG", this->MPDebug);
		this->SingleProcAffinity   = pINI->ReadBool(pOptionsSection, "SingleProcAffinity", this->SingleProcAffinity);
		this->DisableEdgeScrolling = pINI->ReadBool(pOptionsSection, "DisableEdgeScrolling", this->DisableEdgeScrolling);
		this->QuickExit            = pINI->ReadBool(pOptionsSection, "QuickExit", this->QuickExit);
		this->SkipScoreScreen      = pINI->ReadBool(pOptionsSection, "SkipScoreScreen", this->SkipScoreScreen);
		this->DDrawHandlesClose    = pINI->ReadBool(pOptionsSection, "DDrawHandlesClose", this->DDrawHandlesClose);
		this->SpeedControl         = pINI->ReadBool(pOptionsSection, "SpeedControl", this->SpeedControl);
	}

	const char* pVideoSection = "Video";
	if (pINI->GetSection(pVideoSection))
	{
		this->WindowedMode   = pINI->ReadBool(pVideoSection, "Video.Windowed", this->WindowedMode);
		this->NoWindowFrame  = pINI->ReadBool(pVideoSection, "NoWindowFrame", this->NoWindowFrame);
		this->DDrawTargetFPS = pINI->ReadInteger(pVideoSection, "DDrawTargetFPS", this->DDrawTargetFPS);
	}
}

void MainConfig::ApplyStaticOptions()
{
	if (this->MPDebug)
	{
		Game::EnableMPDebug     = true;
		Game::DrawMPDebugStats  = true;
		Game::EnableMPSyncDebug = true;

		// Fixes text maket in the MPDebug panel
		Patch::Apply_TYPED<DWORD>(0x542A19, { 312 });
		Patch::Apply_TYPED<DWORD>(0x542AA6, { 322 });
		Patch::Apply_TYPED<DWORD>(0x542B08, { 332 });
		Patch::Apply_TYPED<DWORD>(0x542B72, { 342 });
		Patch::Apply_TYPED<DWORD>(0x542BD4, { 352 });
		Patch::Apply_TYPED<DWORD>(0x542C94, { 362 });
		Patch::Apply_TYPED<DWORD>(0x542CF7, { 372 });
		Patch::Apply_TYPED<DWORD>(0x542D5E, { 382 });
		Patch::Apply_TYPED<DWORD>(0x542DC2, { 392 });
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
