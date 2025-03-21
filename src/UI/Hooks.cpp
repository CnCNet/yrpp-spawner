
#include "Dialogs.h"

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x609299, UI_IsStaticAndOrOwnerDraw_MultiplayerGameOptionsDialog, 0x5)
{
	enum { RetFalse = 0x609664, RetTrue = 0x609693 };

	GET(int, dlgCtrlID, EAX);
	return (dlgCtrlID == 1314 || dlgCtrlID == 1313 || dlgCtrlID == 1311) ? RetTrue : RetFalse;
}
