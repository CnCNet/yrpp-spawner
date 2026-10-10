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
*
*  Modal "Synchronization Error" dialog shown when a multiplayer game goes out
*  of sync. The game master gets Continue/Quit; everyone else waits for the
*  master's decision. Both variants have a chat box. Game logic is halted while
*  the dialog is open, but the network is serviced so chat, sign-offs and the
*  decision still flow.
*/

#include "DesyncDialog.h"
#include "DesyncDialog.Resource.h"

#include <Ext/Session/Body.h>
#include <Spawner/Spawner.h>
#include <Spawner/GlobalPacketExt.h>
#include <Utilities/Debug.h>

#include <UI.h>
#include <OwnerDraw.h>
#include <SessionClass.h>
#include <HouseClass.h>
#include <EventClass.h>
#include <Surface.h>
#include <PCX.h>
#include <MapClass.h>
#include <IPX.h>
#include <IPXManagerClass.h>
#include <WWMouseClass.h>
#include <StringTable.h>
#include <Unsorted.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <windowsx.h>


DesyncDialogClass DesyncDialog;


namespace
{
	enum
	{
		HEARTBEAT_TIMER = 1,
		QUIT_ENABLE_TIMER = 2,
	};

	constexpr int HEARTBEAT_INTERVAL_MS = 1000;
	constexpr int HEARTBEAT_TIMEOUT_MS = 25000;
	constexpr int QUIT_ENABLE_DELAY_MS = 10000;
	constexpr int CHAT_BACKLOG_MAX = 50;

	// Chat message buffer size; matches the engine's global chat message field.
	constexpr int MAX_MESSAGE_LENGTH = 112;

	// Engine global-packet command for a player signing off.
	constexpr int NET_SIGN_OFF = 0xA;

	// The chat edit's hint text, from the string table (English fallback).
	const wchar_t* Chat_Edit_Placeholder()
	{
		return StringTable::TryFetchString("GUI:DesyncChatHint", L"Type here to chat...");
	}

	/**
	 *  Player list column positions, in pixels within the listbox client
	 *  area (the same units the game's lobby player lists use).
	 */
	constexpr int PLAYER_LIST_HOST_COL_X = 2;
	constexpr int PLAYER_LIST_NAME_COL_X = 20;
	constexpr int PLAYER_LIST_STATUS_COL_WIDTH = 56;

	/**
	 *  The x-position of the status column. Computed from the listbox's
	 *  actual size, because the dialog is not rescaled by the game and so
	 *  its pixel size depends on the system's font metrics.
	 */
	int Player_List_Status_Col_X(HWND list)
	{
		RECT rect {};
		GetClientRect(list, &rect);
		return rect.right - PLAYER_LIST_STATUS_COL_WIDTH;
	}

	/**
	 *  Converts a wide string to ANSI into the given buffer (NUL-terminated).
	 */
	void Wide_To_Ansi(const wchar_t* src, char* dst, int dst_size)
	{
		if (dst_size <= 0)
			return;

		const int written = WideCharToMultiByte(CP_ACP, 0, src ? src : L"", -1, dst, dst_size, nullptr, nullptr);
		if (written <= 0)
			dst[0] = '\0';
		else
			dst[dst_size - 1] = '\0';
	}

	/**
	 *  The local player's display name as ANSI.
	 */
	void Local_Player_Name(char* dst, int dst_size)
	{
		const wchar_t* name = L"";
		if (HouseClass::CurrentPlayer != nullptr)
			name = HouseClass::CurrentPlayer->UIName;
		Wide_To_Ansi(name, dst, dst_size);
	}

	/**
	 *  Sends one of our global packets to every other player in the session.
	 *  Addresses each peer exactly the way the engine's own in-game packets
	 *  (beacons) do: SessionClass::Players[i]->Address, with index 0 being the
	 *  local player.
	 */
	void Broadcast_Global_Packet(ExtGlobalPacketType& packet, int ack_req)
	{
		auto& players = NodeNameType::Array;
		for (int i = 1; i < players.Count; i++) {
			NodeNameType* node = players[i];
			if (node == nullptr)
				continue;

			IPXManagerClass::Instance.Send_Global_Message(
				&packet, sizeof(packet), ack_req,
				reinterpret_cast<IPXAddressClass*>(&node->Address), 0, 0);
		}
		IPXManagerClass::Instance.Service();
	}
}


/**
 *  Shows the dialog and pumps it until a decision has been made.
 *
 *  Game logic is halted for the duration: this function does not return
 *  until the master has decided what to do (or we have decided to quit).
 *  The network is serviced the whole time, so chat, sign-offs and the
 *  master's decision still come through.
 */
DesyncDialogOutcomeType DesyncDialogClass::Run()
{
	Debug::Log("DesyncDialog: opening on frame %d.\n", Unsorted::CurrentFrame);

	/**
	 *  Freeze game logic while the dialog is open: with Session.Suspended (which
	 *  is the same field the engine calls SystemResponseMessages) non-zero,
	 *  OwnerDraw::DialogMessageHandler services Call_Back() each iteration instead
	 *  of running Main_Loop(), so the world stays put while the network keeps
	 *  flowing. Lock user input so the dialog owns the keyboard and mouse.
	 */
	SessionClass::Instance.Suspended()++;
	const bool was_input_locked = Unsorted::UserInputLocked;
	Unsorted::UserInputLocked = true;

	/**
	 *  Hand the mouse to the dialog the way the engine's own in-game network
	 *  dialogs do (e.g. the reconnect/kick dialog, Wait_For_Players @0x648C7C):
	 *  release the captured software cursor so Windows draws the system cursor
	 *  over the dialog, and show it. Without this the game's software cursor
	 *  stays drawn on the frozen backbuffer (game logic is suspended) - a stale
	 *  duplicate cursor that doesn't move. Re-captured on close below.
	 */
	const bool was_mouse_captured = WWMouseClass::Instance->IsCaptured() != 0;
	if (was_mouse_captured) {
		WWMouseClass::Instance->ReleaseMouse();
	}
	WWMouseClass::Instance->ShowCursor();

	Decision = 0;
	ContinueReceived = false;
	std::fill(std::begin(PlayerLeft), std::end(PlayerLeft), false);
	std::fill(std::begin(LastHeartbeatFrom), std::end(LastHeartbeatFrom), std::chrono::steady_clock::now());
	ChatBacklog.clear();

	Create_Dialog();

	DesyncDialogOutcomeType outcome;

	if (Window == nullptr) {

		/**
		 *  If the dialog could not be created for whatever reason, fall back
		 *  to continuing without the desynced players.
		 */
		Debug::Log("DesyncDialog: failed to create the dialog!\n");
		outcome = DESYNC_OUTCOME_CONTINUE;

	} else while (true) {

		/**
		 *  Pump the dialog the way the engine runs its own in-game dialogs:
		 *  DialogMessageHandler dispatches input, repaints the dialog, and (while
		 *  we are suspended) services the network via Call_Back. If it ever
		 *  reports that the game itself has ended, bail out.
		 */
		if (UI::Updated()) {
			outcome = DESYNC_OUTCOME_QUIT;
			break;
		}

		Check_Heartbeat_Timeouts();
		Update_Chat_Placeholder();

		if (Decision == IDC_DESYNC_QUIT) {
			outcome = DESYNC_OUTCOME_QUIT;
			break;
		}

		if (ContinueReceived || Decision == IDC_DESYNC_CONTINUE) {

			if (Decision == IDC_DESYNC_CONTINUE) {
				Send_Continue();
			}
			outcome = DESYNC_OUTCOME_CONTINUE;
			break;
		}

		Decision = 0;
	}

	Destroy_Dialog();

	Unsorted::UserInputLocked = was_input_locked;
	SessionClass::Instance.Suspended()--;

	// Give the mouse back to the game (re-capture the software cursor), matching
	// the open above.
	if (was_mouse_captured) {
		WWMouseClass::Instance->CaptureMouse();
	}

	MapClass::Instance.RedrawSidebar(2); // GScreenClass::Flag_To_Redraw @0x4F42F0: full redraw

	Debug::Log("DesyncDialog: closed with outcome %d.\n", static_cast<int>(outcome));
	return outcome;
}


/**
 *  Creates the dialog appropriate for the local player - the decision
 *  dialog for the game master, the wait dialog for everyone else.
 */
void DesyncDialogClass::Create_Dialog()
{
	IsHostDialog = SessionClass::Instance.Am_I_Master();

	/**
	 *  BeginDialog finds our template via the hooked Fetch_Resource, which
	 *  falls back to this DLL for resources the game's own resources don't
	 *  have, and registers the dialog with the message loop.
	 */
	Window = UI::BeginDialog(MAKEINTRESOURCE(IsHostDialog ? IDD_DESYNC_HOST : IDD_DESYNC_WAIT), &Dialog_Proc, 0);
	Debug::Log("DesyncDialog: BeginDialog returned %p (host=%d).\n", static_cast<void*>(Window), IsHostDialog);
	if (Window == nullptr) {
		return;
	}

	/**
	 *  The owner-draw framework lays the dialog out: because our ids are accepted
	 *  by TrySetDialogLayoutBand1 (hooked in DesyncDialog.Hook.cpp), WM_INITDIALOG
	 *  has already moved us full-screen, scaled the controls, and tagged the
	 *  element so OwnerDraw::Draw_Menu paints the in-game screen (BKGD*.SHP +
	 *  sidebar) behind us - the same treatment the engine's own full-screen
	 *  in-game dialogs get. So there's nothing to size or centre here.
	 *
	 *  Set up the player list columns: player name, host icon, status.
	 *  The name column must be added first: the listbox automatically
	 *  gives every new row a PRIMARY cell (which draws the row's string)
	 *  in the first column that was added, and INVALID cells in the rest.
	 */
	HWND list = GetDlgItem(Window, IDC_DESYNC_PLAYER_LIST);
	if (list != nullptr) {
		const int status_x = Player_List_Status_Col_X(list);
		SendMessage(list, WW_LB_ADDCOLUMN, status_x - PLAYER_LIST_NAME_COL_X - 6, PLAYER_LIST_NAME_COL_X);
		SendMessage(list, WW_LB_ADDCOLUMN, 0, PLAYER_LIST_HOST_COL_X);
		SendMessage(list, WW_LB_ADDCOLUMN, 0, status_x);
	}

	/**
	 *  The chat list is also a cell-based owner-draw listbox, so it needs a
	 *  column for the row text to land in. One full-width column is enough.
	 *  (wParam = column width, lParam = column x; see WW_LB_ADDCOLUMN.)
	 */
	HWND chat_list = GetDlgItem(Window, IDC_DESYNC_CHAT_LIST);
	if (chat_list != nullptr) {
		RECT chat_client {};
		GetClientRect(chat_list, &chat_client);
		// Full-width column so the chat text spans the same width as the chat
		// edit box below it (both are the full control width).
		SendMessage(chat_list, WW_LB_ADDCOLUMN, chat_client.right, 0);
	}

	Update_Player_List();

	if (IsHostDialog) {
		EnableWindow(GetDlgItem(Window, IDC_DESYNC_CONTINUE), TRUE);
	} else {

		/**
		 *  The Quit button starts out disabled (so the player doesn't
		 *  instantly quit out of reflex) and is enabled after a delay.
		 */
		SetTimer(Window, QUIT_ENABLE_TIMER, QUIT_ENABLE_DELAY_MS, nullptr);
	}

	SetTimer(Window, HEARTBEAT_TIMER, HEARTBEAT_INTERVAL_MS, nullptr);

	Refill_Chat_List();

	ShowWindow(Window, SW_SHOWNORMAL);
	SetForegroundWindow(Window);

	/**
	 *  Seed the chat edit's hint text unless it already has focus; thereafter
	 *  Update_Chat_Placeholder (polled from the pump) clears it while the player
	 *  is typing and restores it when the box is left empty.
	 */
	HWND edit = GetDlgItem(Window, IDC_DESYNC_CHAT_EDIT);
	if (edit != nullptr && GetFocus() != edit) {
		SendMessage(edit, WW_SETTEXTW, 0, reinterpret_cast<LPARAM>(Chat_Edit_Placeholder()));
		ChatPlaceholderActive = true;
	}

	Debug::Log("DesyncDialog: Create_Dialog complete.\n");
}


/**
 *  Destroys the dialog.
 */
void DesyncDialogClass::Destroy_Dialog()
{
	if (Window != nullptr) {
		KillTimer(Window, HEARTBEAT_TIMER);
		UI::EndDialog(Window);
		Window = nullptr;
	}
}


/**
 *  If the game master has left and we have been promoted in their place,
 *  replace the wait dialog with the decision dialog.
 */
void DesyncDialogClass::Morph_To_Host_Dialog_If_Needed()
{
	if (!Is_Active() || IsHostDialog) {
		return;
	}

	if (!SessionClass::Instance.Am_I_Master()) {
		return;
	}

	Debug::Log("DesyncDialog: we are the new game master, switching to the decision dialog.\n");

	Destroy_Dialog();
	Create_Dialog();
}


/**
 *  Refills the player list with every player's name and status.
 */
void DesyncDialogClass::Update_Player_List()
{
	if (!Is_Active()) {
		return;
	}

	HWND list = GetDlgItem(Window, IDC_DESYNC_PLAYER_LIST);
	if (list == nullptr) {
		return;
	}

	ListBox_ResetContent(list);

	const int status_x = Player_List_Status_Col_X(list);
	const int host_id = SessionClass::Instance.MasterPlayerID();

	for (int i = 0; i < SessionExt::MaxPlayers && i < HouseClass::Array.Count; i++) {

		HouseClass* house = HouseClass::Array[i];
		if (house == nullptr || !house->IsHumanPlayer) {
			continue;
		}

		/**
		 *  The owner-draw listbox is cell-based; rows are added with the engine's
		 *  own WW_LB_ADDSTRINGW (the plain LB_ADDSTRING does not build the cell and
		 *  returns LB_ERR). The new row's PRIMARY cell - the row string - lands in
		 *  the first column added (the name column), and the host/status columns are
		 *  filled in below with WW_LB_SETCELLTEXT.
		 */
		const int row = static_cast<int>(SendMessage(list, WW_LB_ADDSTRINGW, 0, reinterpret_cast<LPARAM>(house->UIName)));
		if (row < 0) {
			continue;
		}

		/**
		 *  The game master gets the same host icon the lobby uses.
		 */
		if (i == host_id) {
			// WWUIListBoxCell holds WideWstring members whose default ctor is
			// explicit, so it cannot be brace-initialized; build it field by
			// field (the unset wide strings keep their null buffer, which the
			// framework's copy treats as empty).
			WWUIListBoxCell host_cell;
			host_cell.Format = WWUIListBoxCellFormat::Image;
			host_cell.TextColor = 0;
			PCX::Instance.LoadFile("wolhost.pcx");
			host_cell.Image = PCX::Instance.GetSurface("wolhost.pcx");
			host_cell.Value = 0;
			SendMessage(list, WW_LB_SETCELLTEXT, MAKEWPARAM(PLAYER_LIST_HOST_COL_X, row), reinterpret_cast<LPARAM>(&host_cell));
		}

		const wchar_t* status;
		COLORREF color;
		if (PlayerLeft[i]) {
			status = StringTable::TryFetchString("GUI:DesyncStatusQuit", L"Quit");
			color = RGB(200, 0, 0);
		} else if (SessionExt::Is_Out_of_Sync(i)) {
			status = StringTable::TryFetchString("GUI:DesyncStatusDesynced", L"Desynced");
			color = RGB(200, 200, 0);
		} else {
			status = StringTable::TryFetchString("GUI:DesyncStatusOK", L"OK");
			color = RGB(0, 200, 0);
		}

		WWUIListBoxCell status_cell;
		status_cell.Format = WWUIListBoxCellFormat::Text;
		status_cell.PrimaryText = status; // WideWstring, deep-copied by the message
		status_cell.TextColor = color;
		status_cell.Image = nullptr;
		status_cell.Value = 0;
		SendMessage(list, WW_LB_SETCELLTEXT, MAKEWPARAM(status_x, row), reinterpret_cast<LPARAM>(&status_cell));
	}

	InvalidateRect(list, nullptr, FALSE);
}


/**
 *  Refills the chat list from the backlog after the dialog is (re-)created.
 */
void DesyncDialogClass::Refill_Chat_List()
{
	if (!Is_Active()) {
		return;
	}

	HWND list = GetDlgItem(Window, IDC_DESYNC_CHAT_LIST);
	if (list == nullptr) {
		return;
	}

	ListBox_ResetContent(list);
	for (const auto& line : ChatBacklog) {
		SendMessage(list, WW_LB_ADDSTRINGA, 0, reinterpret_cast<LPARAM>(line.c_str()));
	}
	ListBox_SetTopIndex(list, ListBox_GetCount(list) - 1);
}


/**
 *  Appends a line to the chat list (and the backlog).
 */
void DesyncDialogClass::Append_Chat_Line(const char* line)
{
	ChatBacklog.emplace_back(line);
	if (static_cast<int>(ChatBacklog.size()) > CHAT_BACKLOG_MAX) {
		ChatBacklog.erase(ChatBacklog.begin());
	}

	if (!Is_Active()) {
		return;
	}

	HWND list = GetDlgItem(Window, IDC_DESYNC_CHAT_LIST);
	if (list == nullptr) {
		return;
	}

	SendMessage(list, WW_LB_ADDSTRINGA, 0, reinterpret_cast<LPARAM>(line));
	while (ListBox_GetCount(list) > CHAT_BACKLOG_MAX) {
		ListBox_DeleteString(list, 0);
	}
	ListBox_SetTopIndex(list, ListBox_GetCount(list) - 1);
}


/**
 *  Sends the message currently in the chat edit box to the other players.
 *
 *  The in-game chat UI is not running while the game is frozen, so the dialog
 *  carries its own chat over a dedicated global packet and echoes it locally.
 */
void DesyncDialogClass::Send_Chat()
{
	if (!Is_Active()) {
		return;
	}

	HWND edit = GetDlgItem(Window, IDC_DESYNC_CHAT_EDIT);
	if (edit == nullptr) {
		return;
	}

	/**
	 *  The box is showing its hint text, not a real message - nothing to send.
	 */
	if (ChatPlaceholderActive) {
		return;
	}

	/**
	 *  The chat edit is an owner-draw NewEdit: it keeps its text in the engine's
	 *  own buffer rather than the Win32 window text, so read and clear it with the
	 *  WW_*TEXT messages, not Get/SetWindowText.
	 */
	char buf[MAX_MESSAGE_LENGTH];
	buf[0] = '\0';
	SendMessage(edit, WW_GETTEXTA, sizeof(buf), reinterpret_cast<LPARAM>(buf));
	if (buf[0] == '\0') {
		return;
	}

	SendMessage(edit, WW_SETTEXTW, 0, reinterpret_cast<LPARAM>(L""));

	/**
	 *  Repaint the edit right away. The owner-draw edit only redraws itself on
	 *  a text change when it has focus, so clearing it from the Send button
	 *  (where the button, not the edit, is focused) would otherwise leave the
	 *  old text on screen until the next pump cycle. Pressing Enter doesn't hit
	 *  this because the edit is already focused.
	 */
	InvalidateRect(edit, nullptr, TRUE);
	UpdateWindow(edit);

	SetFocus(edit);

	ExtGlobalPacketType packet {};
	packet.Command = EXT_NET_DESYNC_CHAT;
	Local_Player_Name(packet.Name, sizeof(packet.Name));
	if (HouseClass::CurrentPlayer != nullptr)
		packet.Chat.SenderHouseID = static_cast<char>(HouseClass::CurrentPlayer->ArrayIndex);
	std::strncpy(packet.Chat.Text, buf, sizeof(packet.Chat.Text) - 1);
	Broadcast_Global_Packet(packet, 1);

	/**
	 *  Show our own message locally (the broadcast only reaches the others).
	 */
	Notify_Chat(packet.Name, buf);
}


/**
 *  Manages the hint text in the chat edit box: the hint is shown while the box
 *  is empty and unfocused, and cleared once the player clicks into it.
 *
 *  Polled from the pump loop: the owner-draw NewEdit keeps its text in the
 *  engine's own buffer (the WW_*TEXT messages, not the Win32 window text) and
 *  sends no focus notifications, so there is nothing to drive this off events.
 *  We track focus with GetFocus instead.
 */
void DesyncDialogClass::Update_Chat_Placeholder()
{
	if (!Is_Active()) {
		return;
	}

	HWND edit = GetDlgItem(Window, IDC_DESYNC_CHAT_EDIT);
	if (edit == nullptr) {
		return;
	}

	if (GetFocus() == edit) {
		if (ChatPlaceholderActive) {
			SendMessage(edit, WW_SETTEXTW, 0, reinterpret_cast<LPARAM>(L""));
			ChatPlaceholderActive = false;
			InvalidateRect(edit, nullptr, TRUE);
		}
	} else if (!ChatPlaceholderActive) {
		char buf[MAX_MESSAGE_LENGTH];
		buf[0] = '\0';
		SendMessage(edit, WW_GETTEXTA, sizeof(buf), reinterpret_cast<LPARAM>(buf));
		if (buf[0] == '\0') {
			SendMessage(edit, WW_SETTEXTW, 0, reinterpret_cast<LPARAM>(Chat_Edit_Placeholder()));
			ChatPlaceholderActive = true;
			InvalidateRect(edit, nullptr, TRUE);
		}
	}
}


/**
 *  Lets the other players know we are still alive while game logic is halted.
 *  This both keeps the connections (and any NAT mappings) warm and lets
 *  everyone detect players that have silently disappeared.
 */
void DesyncDialogClass::Send_Heartbeat()
{
	/**
	 *  Guard against being called while the world is in an inconsistent state
	 *  (e.g. a save load tearing down and rebuilding the player list under us).
	 */
	if (HouseClass::CurrentPlayer == nullptr || NodeNameType::Array.Count == 0) {
		return;
	}

	ExtGlobalPacketType packet {};
	packet.Command = EXT_NET_DESYNC_HEARTBEAT;
	Local_Player_Name(packet.Name, sizeof(packet.Name));
	packet.Heartbeat.HouseID = static_cast<char>(HouseClass::CurrentPlayer->ArrayIndex);
	packet.Heartbeat.IsHost = SessionClass::Instance.Am_I_Master() ? 1 : 0;

	Broadcast_Global_Packet(packet, 0);
}


/**
 *  Broadcasts the master's decision to continue without the desynced players.
 */
void DesyncDialogClass::Send_Continue()
{
	Debug::Log("DesyncDialog: broadcasting the decision to continue.\n");

	ExtGlobalPacketType packet {};
	packet.Command = EXT_NET_DESYNC_CONTINUE;
	Local_Player_Name(packet.Name, sizeof(packet.Name));

	Broadcast_Global_Packet(packet, 1);
}


/**
 *  Drops players we have not heard from in a long while (e.g. their game has
 *  crashed without sending a sign-off). This keeps the dialog from waiting on
 *  a dead master forever, and removes dead players from the player list so a
 *  subsequent save load reconciles them to the AI cleanly.
 */
void DesyncDialogClass::Check_Heartbeat_Timeouts()
{
	const auto now = std::chrono::steady_clock::now();

	auto& players = NodeNameType::Array;
	for (int i = players.Count - 1; i >= 1; i--) {

		NodeNameType* node = players[i];
		if (node == nullptr) {
			continue;
		}

		const int id = node->HouseIndex;
		if (id < 0 || id >= SessionExt::MaxPlayers) {
			continue;
		}

		if (now - LastHeartbeatFrom[id] > std::chrono::milliseconds(HEARTBEAT_TIMEOUT_MS)) {
			Debug::Log("DesyncDialog: no heartbeat from house %d for %d seconds, dropping them.\n", id, HEARTBEAT_TIMEOUT_MS / 1000);

			/**
			 *  A non-zero error makes Destroy_Connection remove the player
			 *  via a queued remove-player event rather than an immediate AI
			 *  takeover, and print a "connection lost" message.
			 */
			SessionClass::Destroy_Connection(id, 1);
			SessionExt::Update_Master_After_Player_Removal();
			Notify_Player_Left(id);
		}
	}
}


/**
 *  Broadcasts a sign-off so the other players drop our connection.
 */
void DesyncDialogClass::Send_Sign_Off()
{
	Debug::Log("DesyncDialog: signing off.\n");

	ExtGlobalPacketType packet {};
	packet.Command = NET_SIGN_OFF;
	Local_Player_Name(packet.Name, sizeof(packet.Name));

	/**
	 *  Send twice for good measure, since these are not acked.
	 */
	Broadcast_Global_Packet(packet, 0);
	Broadcast_Global_Packet(packet, 0);
}


/**
 *  Detects per-player desync for the current frame and, if any player has
 *  diverged, shows the dialog and applies the chosen outcome. Returns true if
 *  the game should stop (quit), false to resume.
 *
 *  Called from the Execute_DoList entry hook (which gates the engine's
 *  start-of-game CRC-skip window). Replaces the engine's stock
 *  message-box-and-quit: on "continue" the desynced players are dropped and
 *  their events kept out of the frame by the out-of-sync skip hook.
 */
bool DesyncDialogClass::Check_And_Handle_Desync()
{
	if (!SessionClass::IsMultiplayer()) {
		return false;
	}

	/**
	 *  Never re-enter while the dialog is already open. If the pump's Call_Back
	 *  ever re-ran the frame logic and reached Execute_DoList again, this would
	 *  otherwise recurse into Run() and overflow the stack.
	 */
	if (Is_Active()) {
		Debug::Log("DesyncDialog: re-entrant desync check ignored (dialog already open).\n");
		return false;
	}

	/**
	 *  Detect desync the same way the engine does: for each FRAMEINFO event
	 *  scheduled this frame, compare the frame's reported CRC against ours and
	 *  mark every player that diverged.
	 */
	bool newly_desynced = false;
	EventClass* offending_event = nullptr;
	const unsigned int current_frame = static_cast<unsigned int>(Unsorted::CurrentFrame);

	for (int i = 0; i < EventClass::DoList.Count; i++) {

		EventClass& event = EventClass::DoList[i];

		if (event.Type != EventType::FrameInfo || event.Frame != current_frame) {
			continue;
		}

		const int id = event.HouseIndex;
		if (id < 0 || id >= SessionExt::MaxPlayers || SessionExt::Is_Out_of_Sync(id)) {
			continue;
		}

		const int index = static_cast<int>((event.Frame - event.FrameInfo.Delay) & 0xFF);
		if (EventClass::LatestFramesCRC[index] != event.FrameInfo.CRC) {
			SessionExt::Mark_Player_As_Out_of_Sync(id);
			newly_desynced = true;
			if (offending_event == nullptr) {
				offending_event = &event;
			}
		}
	}

	if (!newly_desynced) {
		return false;
	}

	Debug::Log("DesyncDialog: desync detected on frame %u.\n", current_frame);

	/**
	 *  Write the engine's sync-debug dump (SYNC*.TXT) for the offending event,
	 *  exactly as the stock out-of-sync handler does (Execute_DoList @0x64CC68).
	 *  Our entry hook intercepts before that handler runs, so reproduce it here
	 *  or desyncs become impossible to diagnose.
	 */
	if (offending_event != nullptr) {
		if (Game::EnableMPSyncDebug) {
			for (int slot = 0; slot < 256; slot++) {
				EventClass::Print_CRCs_All_Players(slot, offending_event);
			}
		} else {
			EventClass::Print_CRCs_Current_Player(offending_event);
		}
	}

	const DesyncDialogOutcomeType outcome = Run();

	switch (outcome) {

	case DESYNC_OUTCOME_CONTINUE:
		/**
		 *  Continue without the out-of-sync players. Destroy_Connection queues
		 *  their removal; the out-of-sync skip hook keeps their events (and
		 *  FRAMEINFO) out of the executing DoList this frame so the game resumes
		 *  cleanly. Symmetric: the desynced players drop us the same way.
		 */
		for (int id = 0; id < SessionExt::MaxPlayers; id++) {
			if (SessionExt::Is_Out_of_Sync(id)) {
				SessionClass::Destroy_Connection(id, -1);
			}
		}
		SessionExt::Update_Master_After_Player_Removal();
		return false;

	case DESYNC_OUTCOME_QUIT:
	default:
		/**
		 *  Sign off so the others drop us, then let the caller stop the game
		 *  (the Execute_DoList hook returns failure to it).
		 */
		Send_Sign_Off();
		return true;
	}
}


/**
 *  Appends a chat message to the dialog's chat list.
 */
void DesyncDialogClass::Notify_Chat(const char* name, const char* text)
{
	if (!Is_Active()) {
		return;
	}

	char buf[256];
	std::snprintf(buf, std::size(buf), "%s: %s", name, text);
	Append_Chat_Line(buf);
}


/**
 *  Called when a player has left the game (signed off or timed out)
 *  while the dialog was open.
 */
void DesyncDialogClass::Notify_Player_Left(int house_id)
{
	if (!Is_Active()) {
		return;
	}

	if (house_id >= 0 && house_id < SessionExt::MaxPlayers) {
		PlayerLeft[house_id] = true;
	}

	if (house_id >= 0 && house_id < HouseClass::Array.Count && HouseClass::Array[house_id] != nullptr) {
		char name[64];
		Wide_To_Ansi(HouseClass::Array[house_id]->UIName, name, sizeof(name));
		char buf[128];
		std::snprintf(buf, std::size(buf), "%s has left the game.", name);
		Append_Chat_Line(buf);
	}

	Update_Player_List();

	/**
	 *  If the master is the one who left, SessionExt::Update_Master_After_Player_Removal
	 *  (run by our callers before this) has already promoted a replacement; if it
	 *  promoted us, switch to the host dialog.
	 */
	Morph_To_Host_Dialog_If_Needed();
}


/**
 *  Called when the master has decided to continue the game without
 *  the desynced players.
 */
void DesyncDialogClass::Notify_Continue()
{
	if (!Is_Active()) {
		return;
	}

	Debug::Log("DesyncDialog: the game master has chosen to continue.\n");
	ContinueReceived = true;
}


/**
 *  Called when a heartbeat has been received from another player.
 */
void DesyncDialogClass::Notify_Heartbeat(int house_id, bool is_host)
{
	if (!Is_Active()) {
		return;
	}

	if (house_id < 0 || house_id >= SessionExt::MaxPlayers) {
		return;
	}

	LastHeartbeatFrom[house_id] = std::chrono::steady_clock::now();

	/**
	 *  Self-heal the master fields in case we missed the announcement,
	 *  and move the host icon in the player list to the right row.
	 */
	if (is_host && SessionClass::Instance.MasterPlayerID() == -1) {
		SessionExt::Set_Master(house_id);
		Update_Player_List();
	}
}


/**
 *  Routes a received spawner global packet to the right notification.
 *  Returns true if the packet was one of ours and was consumed.
 *
 *  This is the receive side. It is ready to be called from the engine's global
 *  packet dispatch (Network_Call_Back, 0x48D1E0): commands the engine does not
 *  recognise fall through to its default case (~0x48DACE -> Process_Global_Packet),
 *  which is where this should be spliced in. The hook itself is left to be wired
 *  in once the exact splice point/registers are confirmed, e.g.:
 *
 *      DEFINE_HOOK(0x48DACE, NetworkCallBack_DesyncPacket, <size>)
 *      {
 *          GET(ExtGlobalPacketType*, packet, <reg for &GPacket>);
 *          GET(IPXAddressClass*, address, <reg for &GAddress>);
 *          if (DesyncDialog.Handle_Global_Packet(packet, address))
 *              return <address after the dispatch switch>;
 *          return 0;
 *      }
 *
 *  Player departures (sign-offs) are likewise routed by hooking the engine's
 *  Destroy_Connection (0x5DA750) to also call DesyncDialog.Notify_Player_Left(id).
 */
bool DesyncDialogClass::Handle_Global_Packet(const ExtGlobalPacketType* packet, const IPXAddressClass* address)
{
	(void)address;

	if (packet == nullptr) {
		return false;
	}

	switch (packet->Command) {
	case EXT_NET_HOST_ANNOUNCE:
		// Sent by the host at game start (before any dialog is open), so record
		// the master unconditionally; this is what makes MasterPlayerID
		// authoritative everywhere - including for the dialog's host icon.
		SessionExt::Set_Master(packet->Heartbeat.HouseID);
		return true;

	case EXT_NET_DESYNC_HEARTBEAT:
		Notify_Heartbeat(packet->Heartbeat.HouseID, packet->Heartbeat.IsHost != 0);
		return true;

	case EXT_NET_DESYNC_CONTINUE:
		Notify_Continue();
		return true;

	case EXT_NET_DESYNC_CHAT:
		Notify_Chat(packet->Name, packet->Chat.Text);
		return true;

	default:
		return false;
	}
}


/**
 *  The window procedure for both dialog variants.
 */
BOOL CALLBACK DesyncDialogClass::Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	/**
	 *  Let the owner-draw framework do all of its standard handling first: it
	 *  subclasses the controls and lays out the dialog on WM_INITDIALOG, paints
	 *  the dialog background, draws the owner-draw controls (WM_DRAWITEM),
	 *  colours the controls, and tears the dialog down on WM_DESTROY. If it
	 *  consumed the message, we are done. (This is exactly how the engine's own
	 *  in-game dialogs, e.g. the diplomacy dialog, are written.)
	 */
	if (LRESULT handled = UI::StandardWndProc(window, message, wparam, lparam)) {
		return static_cast<BOOL>(handled);
	}

	switch (message) {

		case WM_MOVING:
			// Game::OnWindowMoving clamps the drag rect (lParam) to the screen.
			Game::OnWindowMoving(reinterpret_cast<tagRECT*>(lparam));
			return TRUE;

		case WM_TIMER:

			/**
			 *  Heartbeats are sent from a timer rather than the pump loop,
			 *  so that they keep flowing while a nested dialog runs its own
			 *  message loop.
			 */
			if (wparam == HEARTBEAT_TIMER) {
				DesyncDialog.Send_Heartbeat();
			} else if (wparam == QUIT_ENABLE_TIMER) {
				EnableWindow(GetDlgItem(window, IDC_DESYNC_QUIT), TRUE);
				KillTimer(window, QUIT_ENABLE_TIMER);
			}
			break;

		case WW_EDIT_ENTERPRESSED:
			/**
			 *  The owner-draw chat edit reports Enter through this message
			 *  (lParam = the edit's HWND), not as an IDOK WM_COMMAND.
			 */
			if (reinterpret_cast<HWND>(lparam) == GetDlgItem(window, IDC_DESYNC_CHAT_EDIT)) {
				DesyncDialog.Send_Chat();
				return TRUE;
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wparam)) {

				case IDC_DESYNC_CHAT_SEND:
					DesyncDialog.Send_Chat();
					return TRUE;

				case IDC_DESYNC_CONTINUE:
				case IDC_DESYNC_QUIT:
					DesyncDialog.Decision = LOWORD(wparam);
					break;
			}
			break;
	}

	return FALSE;
}
