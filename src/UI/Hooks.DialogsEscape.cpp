#include <Utilities/Macro.h>
#include <Unsorted.h>
#include <OwnerDraw.h>
#include <windows.h>

#define RETURN_TO_GAME_OPTIONS 6

int inline GetDialogBackButtonID(int dialogID)
{
	switch (dialogID)
	{
	case 115: // Diplomacy
	case 268: // Briefing
		return 1059;

	case 255: // OptionsInternet
		return 1653;

	case 163:  // KeyboardOptions
	case 181:  // SinglePlayerOptions
	case 182:  // AbortMission
	case 183:  // LoadMission
	case 184:  // SoundOptions
	case 692:  // SaveMission
	case 693:  // DeleteMission
	case 3002: // MultiplayerOptions
	case 3003: // GameControls
		return 1670;

	default:
		return 0;
	}
}

// Treat Escape as Back/Resume in in-game dialogs that use a custom button ID.
DEFINE_HOOK(0x622E9A, StandardWndProc_InGameDialog_EscapeAsBack, 0x6)
{
	enum { MessageHandled = 0x622BBC };
	GET(HWND, hDialog, ESI);
	GET(UINT, message, EDX);
	GET_STACK(LPARAM, source, STACK_OFFSET(0x40, 0x8));
	GET_STACK(WPARAM, command, STACK_OFFSET(0x40, 0x4));

	if (!Game::IsActive)
		return 0;

	if (message != WM_COMMAND || command != IDCANCEL || source != 0)
		return 0;

	const auto pDialog = OwnerDraw::Dialogs.try_get(hDialog);
	if (!pDialog)
		return 0;

	const int backButtonID = GetDialogBackButtonID(pDialog->DialogID);
	if (!backButtonID)
		return 0;

	const auto hCancel = Imports::GetDlgItem(hDialog, IDCANCEL);
	if (hCancel && Imports::IsWindowVisible(hCancel) && Imports::IsWindowEnabled(hCancel))
		return 0;

	const auto hBack = Imports::GetDlgItem(hDialog, backButtonID);
	if (!hBack)
		return 0;

	if (Imports::IsWindowVisible(hBack) && Imports::IsWindowEnabled(hBack))
	{
		enum { AbortMissionDialog = 182 };
		if (pDialog->DialogID == AbortMissionDialog)
		{
			// Resume closes both menus; Escape should return to the pause menu instead.
			if (const auto pResult = reinterpret_cast<int*>(Imports::GetWindowLongA(hDialog, DWL_USER)))
			{
				*pResult = RETURN_TO_GAME_OPTIONS;
				return MessageHandled;
			}
		}

		// Queue the normal button command to keep return/apply behavior without re-entering the dialog.
		if (Imports::PostMessageA(hDialog, WM_COMMAND, MAKEWPARAM(backButtonID, BN_CLICKED), reinterpret_cast<LPARAM>(hBack)))
			return MessageHandled;
	}

	return 0;
}

// Reopen the pause menu after Abort Mission is dismissed with Escape.
DEFINE_HOOK(0x48CA21, SpecialDialog_AbortMission_EscapeAsBack, 0x6)
{
	enum { DispatchDialog = 0x48C960 };
	GET(int, result, EAX);

	if (result != RETURN_TO_GAME_OPTIONS)
		return 0;

	Game::SpecialDialog = 1;
	R->EAX(Game::SpecialDialog);
	return DispatchDialog;
}

// Let the dialog handle Escape even when its custom edit control has keyboard focus.
DEFINE_HOOK(0x614B5A, WWUI_NewEditCtrl_GetDlgCode_AllowEscape, 0x5)
{
	enum { ReturnDialogCode = 0x614B5F };
	GET_STACK(const MSG*, pMessage, STACK_OFFSET(0xB4, 0x10));

	if (!Game::IsActive)
		return 0;

	if (pMessage && pMessage->message == WM_KEYDOWN && pMessage->wParam == VK_ESCAPE)
	{
		R->EAX(0);
		return ReturnDialogCode;
	}

	return 0;
}
