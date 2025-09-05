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

#include "GameSpeedSlider.h"
#include <Spawner/Spawner.h>
#include <Utilities/Macro.h>
#include <SessionClass.h>
#include <HouseClass.h>

bool GameSpeedSlider::IsEnabled()
{
	auto cfg = Spawner::GetConfig();
	return Spawner::Enabled && cfg && !cfg->DisableGameSpeed;
}

bool GameSpeedSlider::IsDisabled()
{
	return !IsEnabled();
}

// Hide the GameSpeed (FPS) slider group only when feature disabled.
// Skirmish observers must always retain the slider regardless of config.
DEFINE_HOOK(0x4E20BA, GameControlsClass__SomeDialog_GameSpeedSlider, 0x5)
{
	bool const isSkirmishObserver = (SessionClass::IsSkirmish() && HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->IsObserver());

	if (GameSpeedSlider::IsDisabled() && !isSkirmishObserver)
	{
		GET(void*, fnGetCtrlById, EDI);
		GET(void*, fnShowWindow, EBP);
		GET(void*, hDlg, ESI);

		using GetCtrlById_t = void* (__stdcall*)(void* hDlg, int id);
		using ShowWindow_t  = void  (__stdcall*)(void* hWnd, int nCmdShow);

		auto getCtrl = reinterpret_cast<GetCtrlById_t>(fnGetCtrlById);
		auto showWnd = reinterpret_cast<ShowWindow_t>(fnShowWindow);

		if (getCtrl && showWnd)
		{
			if (auto ctrl = getCtrl(hDlg, 0x529)) { showWnd(ctrl, 0); }
			if (auto ctrl = getCtrl(hDlg, 0x714)) { showWnd(ctrl, 0); }
			if (auto ctrl = getCtrl(hDlg, 0x671)) { showWnd(ctrl, 0); }
		}

		return 0x4E211A;
	}

	return 0;
}

