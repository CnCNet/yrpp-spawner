#ifdef IS_HARDEND_VER
#include <Utilities/Macro.h>
#include "Ra2Mode.h"

HANDLE __fastcall UI_ApplyAppIcon()
{
	char* iconPath = Ra2Mode::IsEnabled()
		? "./Resources/ra2.ico"
		: "./Resources/clienticon.ico";

	return LoadImageA(NULL, iconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
}

DEFINE_PATCH(0x777C41, 0x90, 0x90, 0x90, 0x90); // Disable Phobos hook (0x777C41, UI_ApplyAppIcon, 0x9)
DEFINE_JUMP(CALL, 0x777C45, GET_OFFSET(UI_ApplyAppIcon));
#endif
