/**
*  yrpp-spawner
*
*  Copyright(C) 2024-present CnCNet
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

#ifdef IS_HARDEND_VER
#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
#include <CCINIClass.h>

// Skip log spam "Unable to locate scenario %s - No digest info"
DEFINE_JUMP(LJMP, 0x69A797, 0x69A937);

// Disable Ares hook AbstractTypeClass_CTOR_IDTooLong
// Which checks type name length too aggressively
// Use Debug::Log instead of MessageBox
#pragma region IDTooLong
void __fastcall IDTooLong(char* id)
{
	if (strlen(id) > 24)
		Debug::Log("[Developer error] Tried to create a type with ID '%s' which is longer than the maximum length of 24.\n", id);
}

DEFINE_NAKED_HOOK(0x41088D, AbstractTypeClass_CTOR_IDTooLong)
{
	__asm {
		push 24;
		lea edx, [esp + 24h];
		push eax;
		push edx;

		mov ecx, eax;
		call IDTooLong;

		mov ecx, 0x410895;
		jmp ecx;
	}
}
#pragma endregion IDTooLong


// Revert Ares survivors hook
#pragma region AresSurvivors
DEFINE_PATCH(0x737F97, // DEFINE_HOOK(737F97, UnitClass_ReceiveDamage, 0)
	0x8B, 0x16, 0x8B, 0xCE, 0xFF
);

DEFINE_PATCH(0x41668B, // DEFINE_HOOK(41668B, AircraftClass_ReceiveDamage, 6)
	0x8B, 0x44, 0x24, 0x28, 0x8B, 0x16
);
#pragma endregion AresSurvivors

// Revert Phobos fix "SHP debris shadows now respect the Shadow tag"
// Disable Phobos hook Phobos_BugFixes_SHPShadowCheck
DEFINE_PATCH(0x423365, 0x84, 0xC0, 0x0F, 0x84, 0x81, 0x00, 0x00, 0x00);

// Center Pause Menu Background
#pragma region CenterPauseMenuBackground
#include <Surface.h>
#include <FileFormats/SHP.h>

void __fastcall InitSideRectangles_CenterBackground(
	Surface* Surface, ConvertClass* Palette, SHPStruct* SHP, int FrameIndex,
	const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags,
	int Remap, int ZAdjust, ZGradient ZGradientDescIndex, int Brightness,
	int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset
)
{
	RectangleStruct shpRect = SHP->GetFrameBounds(0);
	RectangleStruct surRect = Surface->GetRect();

	Point2D nPoint {
		(surRect.Width - 168 - shpRect.Width) >> 1 ,
		(surRect.Height - 32 - shpRect.Height) >> 1
	};

	CC_Draw_Shape(Surface, Palette, SHP, FrameIndex, &nPoint, Bounds, Flags, Remap, ZAdjust,
		ZGradientDescIndex, Brightness, TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}

DEFINE_JUMP(CALL, 0x72F624, GET_OFFSET(InitSideRectangles_CenterBackground));
#pragma endregion CenterPauseMenuBackground
#endif
