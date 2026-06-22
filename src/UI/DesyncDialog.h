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
*  Dialog shown to the players when a multiplayer game goes out of sync.
*  Ported from Vinifera (Tiberian Sun) to Yuri's Revenge.
*/

#pragma once

#include <windows.h>

#include <Ext/Session/Body.h>
#include <Spawner/GlobalPacketExt.h>

#include <chrono>
#include <string>
#include <vector>

class IPXAddressClass;

/**
 *  The decision the desync dialog was closed with.
 */
enum DesyncDialogOutcomeType
{
	DESYNC_OUTCOME_CONTINUE,    // Continue playing without the desynced players.
	DESYNC_OUTCOME_QUIT,        // The local player wants to exit the game.
};

/**
 *  Manages the modal "Synchronization Error" dialog that is shown when a
 *  multiplayer game goes out of sync. The game master gets a dialog with
 *  Continue/Quit options; everyone else gets a dialog asking them
 *  to wait for the master's decision. Both variants have a chat box.
 *
 *  While the dialog is open, game logic is halted, but the network is
 *  serviced and connections are kept alive with periodic heartbeats.
 */
class DesyncDialogClass
{
public:
	DesyncDialogClass() = default;
	~DesyncDialogClass() = default;

	/**
	 *  Shows the dialog and pumps it until a decision has been made.
	 */
	DesyncDialogOutcomeType Run();

	bool Is_Active() const { return Window != nullptr; }

	/**
	 *  Notifications from the incoming global packet processor.
	 *  All of these are no-ops while the dialog is not open.
	 */
	void Notify_Chat(const char* name, const char* text);
	void Notify_Player_Left(int house_id);
	void Notify_Continue();
	void Notify_Heartbeat(int house_id, bool is_host);

	/**
	 *  Routes a received spawner global packet to the right notification.
	 *  Returns true if the packet was one of ours and was consumed.
	 *  Meant to be called from the global-packet receive hook (to be wired
	 *  in separately); see DesyncDialog.cpp.
	 */
	bool Handle_Global_Packet(const ExtGlobalPacketType* packet, const IPXAddressClass* address);

	/**
	 *  Detects per-player desync for the current frame; if any player has
	 *  diverged, shows the dialog and applies the chosen outcome. Returns true
	 *  if the game should stop (quit), false to resume. Called from the
	 *  Execute_DoList hook.
	 */
	bool Check_And_Handle_Desync();

private:
	void Create_Dialog();
	void Destroy_Dialog();
	void Morph_To_Host_Dialog_If_Needed();
	void Update_Player_List();
	void Refill_Chat_List();
	void Append_Chat_Line(const char* line);
	void Send_Chat();
	void On_Chat_Edit_Focus(bool gained);
	void Send_Heartbeat();
	void Send_Continue();
	void Send_Sign_Off();
	void Check_Heartbeat_Timeouts();

	static BOOL CALLBACK Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

private:
	/**
	 *  The dialog window, null while the dialog is not open.
	 */
	HWND Window = nullptr;

	/**
	 *  Is the currently open dialog the host (game master) variant?
	 */
	bool IsHostDialog = false;

	/**
	 *  Control ID of the button the player pressed, consumed by the pump loop.
	 */
	int Decision = 0;

	/**
	 *  Has the game master decided to continue without the desynced players?
	 */
	bool ContinueReceived = false;

	/**
	 *  Is the chat edit box currently showing its hint text?
	 */
	bool ChatPlaceholderActive = false;

	/**
	 *  Players that have left the game while the dialog was open, by house ID.
	 */
	bool PlayerLeft[SessionExt::MaxPlayers] = {};

	/**
	 *  Heartbeat bookkeeping for detecting players that silently disappear.
	 */
	std::chrono::steady_clock::time_point LastHeartbeatFrom[SessionExt::MaxPlayers] = {};

	/**
	 *  All chat lines shown so far, so the list can be refilled when the
	 *  dialog is re-created (e.g. when a waiting player becomes the master).
	 */
	std::vector<std::string> ChatBacklog;
};

extern DesyncDialogClass DesyncDialog;
