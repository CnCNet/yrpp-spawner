#include <Utilities/Macro.h>
#include <Unsorted.h>
#include <OwnerDraw.h>
#include <WWKeyboardClass.h>
#include <windows.h>

#define RETURN_TO_GAME_OPTIONS 6

bool DialogEscapeHandled = false;

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

// Use the dialog's normal Back command, keeping its apply/return behavior.
bool inline HandleDialogEscape(HWND hDialog, bool isRepeatedKeyDown)
{
	if (!Imports::IsWindowVisible(hDialog) || !Imports::IsWindowEnabled(hDialog))
		return false;

	const auto pDialog = OwnerDraw::Dialogs.try_get(hDialog);
	if (!pDialog)
		return false;

	const int backButtonID = GetDialogBackButtonID(pDialog->DialogID);
	if (!backButtonID)
		return false;

	const auto hCancel = Imports::GetDlgItem(hDialog, IDCANCEL);
	if (hCancel && Imports::IsWindowVisible(hCancel) && Imports::IsWindowEnabled(hCancel))
		return false;

	const auto hBack = Imports::GetDlgItem(hDialog, backButtonID);
	if (!hBack || !Imports::IsWindowVisible(hBack) || !Imports::IsWindowEnabled(hBack))
		return false;

	// Holding Escape should not close successive parent dialogs.
	if (isRepeatedKeyDown)
		return true;

	enum { AbortMissionDialog = 182 };
	if (pDialog->DialogID == AbortMissionDialog)
	{
		// Resume closes both menus; Escape should return to the pause menu instead.
		const auto pResult = reinterpret_cast<int*>(Imports::GetWindowLongA(hDialog, DWL_USER));
		if (!pResult)
			return false;

		*pResult = RETURN_TO_GAME_OPTIONS;
		DialogEscapeHandled = true;
		return true;
	}

	// Queue the button command without re-entering the dialog procedure.
	if (!Imports::PostMessageA(hDialog, WM_COMMAND, MAKEWPARAM(backButtonID, BN_CLICKED), reinterpret_cast<LPARAM>(hBack)))
		return false;

	DialogEscapeHandled = true;
	return true;
}

// Consume Escape before dialog routing or gameplay input, even if focus returned to the main window.
DEFINE_HOOK(0x5D4DE1, WindowsMessageHandler_InGameDialog_EscapeAsBack, 0x5)
{
	if (!Game::IsActive)
		return 0;

	enum { MessageHandled = 0x5D4E4D };
	LEA_STACK(const MSG*, pMessage, STACK_OFFSET(0x2C, -0x1C));
	if (pMessage->message != WM_KEYDOWN || pMessage->wParam != VK_ESCAPE)
		return 0;

	GET(HWND*, pDialogs, EDX);
	GET(int, dialogIndex, ESI);
	const bool isRepeatedKeyDown = (HIWORD(pMessage->lParam) & KF_REPEAT) != 0;
	if (HandleDialogEscape(pDialogs[dialogIndex], isRepeatedKeyDown))
		return MessageHandled;

	return 0;
}

// Reopen the pause menu after Abort Mission is dismissed with Escape.
DEFINE_HOOK(0x48CA21, SpecialDialog_AbortMission_EscapeAsBack, 0x6)
{
	enum { DispatchDialog = 0x48C960 };
	GET(int, result, EAX);

	if (result == RETURN_TO_GAME_OPTIONS)
	{
		Game::SpecialDialog = 1;
		R->EAX(Game::SpecialDialog);
		return DispatchDialog;
	}

	return 0;
}

// Drop queued gameplay keys when Escape finishes the menu session, preventing an immediate reopen.
DEFINE_HOOK(0x48CC37, SpecialDialog_Exit_ClearEscapeInput, 0x5)
{
	if (DialogEscapeHandled)
	{
		DialogEscapeHandled = false;
		WWKeyboardClass::Instance->Clear();
	}

	return 0;
}
