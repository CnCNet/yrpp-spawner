#include <Spawner/Spawner.h>
#include <Utilities/Macro.h>
#include <Unsorted.h>
#include <windows.h>

enum GameOptionsControlID
{
	SaveGameButton = 1311,
	GameControlsButton = 1313,
	AbortMissionButton = 1314
};

// Create the button before StandardWndProc registers and formats the controls.
DEFINE_HOOK(0x4F11D6, MultiplayerGameOptionsDialog_AddSaveButton, 0x6)
{
	enum { MultiplayerGameOptionsDialog = 3002 };

	if (!Spawner::Enabled)
		return 0;

	GET(UINT, message, EBX);
	GET(HWND, hDialog, ESI);
	GET(const WORD*, initData, EBP);

	if (message != WM_INITDIALOG || !initData || *initData != MultiplayerGameOptionsDialog)
		return 0;

	const auto pConfig = Spawner::GetConfig();
	if (pConfig && !pConfig->ShowSaveGameButton)
		return 0;

	const auto hExistingSave = Imports::GetDlgItem(hDialog, SaveGameButton);
	if (hExistingSave)
		return 0;

	const auto hControls = Imports::GetDlgItem(hDialog, GameControlsButton);
	const auto hAbort = Imports::GetDlgItem(hDialog, AbortMissionButton);
	if (!hControls || !hAbort)
		return 0;

	RECT controlsRect;
	RECT abortRect;
	const auto hasControlsRect = Imports::GetWindowRect(hControls, &controlsRect);
	const auto hasAbortRect = Imports::GetWindowRect(hAbort, &abortRect);
	if (!hasControlsRect || !hasAbortRect)
		return 0;

	MapWindowPoints(nullptr, hDialog, reinterpret_cast<POINT*>(&controlsRect), 2);
	MapWindowPoints(nullptr, hDialog, reinterpret_cast<POINT*>(&abortRect), 2);
	const int width = abortRect.right - abortRect.left;
	const int height = abortRect.bottom - abortRect.top;
	const int buttonStepY = abortRect.top - controlsRect.top;
	const auto hInstance = reinterpret_cast<HINSTANCE>(Imports::GetWindowLongA(hDialog, GWL_HINSTANCE));
	const auto hSave = Imports::CreateWindowExA(0, "BUTTON", "GUI:SaveGame",
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		abortRect.left, abortRect.top, width, height, hDialog,
		reinterpret_cast<HMENU>(SaveGameButton), hInstance, nullptr);

	if (hSave)
	{
		const auto font = Imports::SendMessageA(hAbort, WM_GETFONT, 0, 0);
		Imports::SendMessageA(hSave, WM_SETFONT, font, FALSE);
		// Place the save button after game controls in the child window order.
		Imports::SetWindowPos(hSave, hControls, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		Imports::SetWindowPos(hAbort, nullptr, abortRect.left, abortRect.top + buttonStepY,
			0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	return 0;
}

DEFINE_HOOK(0x609299, UI_IsStaticAndOrOwnerDraw_MultiplayerGameOptionsDialog, 0x5)
{
	enum { RetTrue = 0x609693 };

	GET(int, dlgCtrlID, EAX);
	return Spawner::Enabled && dlgCtrlID == SaveGameButton ? RetTrue : 0;
}
