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

#include <Utilities/Macro.h>
#include <GameOptionsClass.h>
#include <BasicStructures.h>
#include <Spawner/Spawner.h>

DEFINE_HOOK(0x6BC14D, WinMain_ReadScreenResolutionFromIni, 0x5)
{
	if (!Spawner::Enabled)
		return 0;

	if (GameOptionsClass::Instance->ShellWidth < 800 || GameOptionsClass::Instance->ShellHeight < 600)
		return 0;

	GameOptionsClass::Instance->ShellHeight = GameOptionsClass::Instance->ScreenHeight;
	GameOptionsClass::Instance->ShellWidth = GameOptionsClass::Instance->ScreenWidth;

	return 0;
}

DEFINE_HOOK(0x640CE2, PreviewClass_DrawMap, 0x5)
{
	if (!Spawner::Enabled)
		return 0;

	if (GameOptionsClass::Instance->ShellWidth < 800 || GameOptionsClass::Instance->ShellHeight < 600)
		return 0;

	int Left = 499;
	int Top = 379;
	int Width = 216;
	int Height = 166;

	Left += (GameOptionsClass::Instance->ShellWidth - 800) >> 1;
	Top += (GameOptionsClass::Instance->ShellHeight - 600) >> 1;

	R->EAX(Left);
	R->ECX(Top);
	R->EDX(Width);
	R->ESI(Height);
	return 0x640D36;
}

DEFINE_HOOK(0x60C43B, EnumChildProc_60C0C0, 0x5)
{
	if (!Spawner::Enabled)
		return 0;

	if (GameOptionsClass::Instance->ShellWidth < 800 || GameOptionsClass::Instance->ShellHeight < 600)
		return 0;

	LEA_STACK(RectangleStruct*, Rect, STACK_OFFSET(0x44, -0x10));
	Rect->X = -((GameOptionsClass::Instance->ShellWidth - 800) >> 1);
	Rect->Y = -((GameOptionsClass::Instance->ShellHeight - 600) >> 1);
	Rect->Width = 640;
	Rect->Height = 480;

	return 0x60C443;
}

// allow hires modes
DEFINE_JUMP(LJMP, 0x56017A, 0x560183) // OptionsDlg_WndProc_RemoveResLimit

// skip the allowhires check entirely - all supported 16bit modes are accepted, should make net resolution limit stfu
DEFINE_JUMP(LJMP, 0x5601E3, 0x5601FC) // OptionsDlg_WndProc_RemoveHiResCheck

// Fixes the layout for some screen resolutions, for example 1152x648
DEFINE_HOOK(0x72ECA4, UIStuff_72EC70, 0x6)
{
	__asm {cmp esi, 600}; // original = 768
	return 0x72ECAA;
}

// Cap the sidebar height to 1376 pixels
DEFINE_HOOK(0x6A518E, SidebarClass_InitGUI, 0x5)
{
	GET(int, heightOfSidebar, EAX);
	if (heightOfSidebar > 1376)
		R->EAX(1376);

	return 0;
}

// This will force the game to always use ddraw's blit function rather than WW blit
// We're avoiding ww blit functions because they are not thread safe
DEFINE_PATCH(0x4BB1FE, 0);    // DSurface::Blit_Clip

// Disables drawing the menu bg which pops up on loading
#pragma region SkipsMenuBackground
DEFINE_HOOK(0x7782B7, Load_Title_Screen, 0x6)
{
	return Spawner::Enabled
		? 0x7782C4
		: 0;
}

DEFINE_HOOK(0x52FF4A, Redraw_Surface, 0x6)
{
	return Spawner::Enabled
		? 0x52FF57
		: 0;
}
#pragma endregion SkipsMenuBackground
