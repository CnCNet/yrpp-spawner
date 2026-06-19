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

// Hooks that wire the desync dialog into the engine's networking.
// Mirrors what Vinifera added to TS's IPX_Call_Back (0x462DC0); here the
// equivalent function is Network_Call_Back (0x48D1E0).

#include "DesyncDialog.h"
#include "DesyncDialog.Resource.h"

#include <Ext/Session/Body.h>
#include <Spawner/GlobalPacketExt.h>
#include <Utilities/Macro.h>

#include <SessionClass.h>
#include <IPX.h>
#include <Timer.h>

/**
 *  Make the desync dialogs full-screen "menu" dialogs.
 *
 *  OwnerDraw::Draw_Menu picks a dialog's backdrop from its element's LayoutBand,
 *  which OwnerDraw's TrySetDialogLayoutBand1 (0x60C540) sets to 1 - and lays the
 *  dialog out full-screen - only for a hard-coded list of dialog ids. Ours are
 *  not in that list, so we would get the plain fallback background instead of the
 *  in-game screen (BKGDSM/MD/LG.SHP + sidebar) the engine draws behind its own
 *  full-screen dialogs (e.g. the "waiting for players" dialog, id 234).
 *
 *  At 0x60C5A1 EAX holds the dialog id (just loaded from element+0x6C). If it is
 *  one of ours, jump straight to the function's success tail at 0x60C7B7 (which
 *  sets LayoutBand = 1 and returns true with ecx = element, edx = 1 still live);
 *  otherwise fall through to the engine's own id checks.
 */
DEFINE_HOOK(0x60C5A1, OwnerDraw_TrySetDialogLayoutBand1_DesyncDialog, 0x5)
{
	GET(int, dialog_id, EAX);

	if (dialog_id == IDD_DESYNC_HOST || dialog_id == IDD_DESYNC_WAIT) {
		return 0x60C7B7;
	}

	return 0;
}

/**
 *  Position our buttons on the full-screen background like the engine's menu
 *  dialogs do.
 *
 *  When the framework scales a full-screen dialog's children (EnumChildProc_60C0C0),
 *  controls of dialogs in another hard-coded id list are sent to UI_60B7A0, which
 *  offsets them to centre the 800x600 layout within the screen. Controls that miss
 *  it (our buttons) fall to the plain default move and land top-left. At 0x60C399
 *  EDI holds the parent dialog id; if it's one of ours, jump to the UI_60B7A0 call.
 */
DEFINE_HOOK(0x60C399, OwnerDraw_CenterDesyncDialogButtons, 0x6)
{
	GET(int, dialog_id, EDI);

	if (dialog_id == IDD_DESYNC_HOST || dialog_id == IDD_DESYNC_WAIT) {
		return 0x60C3F6;
	}

	return 0;
}

/**
 *  Mark the desync dialogs as recognised full-screen menu dialogs.
 *
 *  ResolveIDs_601360 is the engine's "is this a full-screen menu dialog?" predicate
 *  (id == one of a big hard-coded list). It gates, among other things, the special
 *  bottom-anchored placement of the tooltip bar (control 1685) during full-screen
 *  scaling. ECX holds the dialog id; for ours, jump to the function's "return true"
 *  tail (0x6015D5) - it has no stack frame, so this is a clean early-out. Only our
 *  ids are affected; every other dialog still runs the original checks.
 */
DEFINE_HOOK(0x601360, OwnerDraw_ResolveIDs_DesyncDialog, 0x6)
{
	GET(int, dialog_id, ECX);

	if (dialog_id == IDD_DESYNC_HOST || dialog_id == IDD_DESYNC_WAIT) {
		return 0x6015D5;
	}

	return 0;
}

/**
 *  Place our owner-draw buttons on the in-game sidebar button strip, the way the
 *  engine's full-screen menu dialogs do (e.g. the map-generator dialog 261, whose
 *  Load/Save/Delete/Use buttons stack on the strip).
 *
 *  During full-screen child scaling, an owner-draw control is sent to UI_60B000
 *  (which positions it on the sidebar strip, vertical slot from its y) only when
 *  UI_Is_Static_And_Or_OwnerDraw returns true for the (dialog id, control id)
 *  pair. Ours aren't recognised, so the buttons fell to the plain default move.
 *  At 0x608D27 ESI = dialog id and EAX = control id (just fetched); for our
 *  buttons jump to the function's "return true" tail at 0x608F34. Only our button
 *  ids are matched, so static/list controls are unaffected.
 */
DEFINE_HOOK(0x608D27, OwnerDraw_DesyncDialogSidebarButtons, 0x6)
{
	GET(int, dialog_id, ESI);
	GET(int, control_id, EAX);

	if ((dialog_id == IDD_DESYNC_HOST || dialog_id == IDD_DESYNC_WAIT)
		&& (control_id == IDC_DESYNC_LOAD || control_id == IDC_DESYNC_CONTINUE || control_id == IDC_DESYNC_QUIT)) {
		return 0x608F34;
	}

	return 0;
}

/**
 *  Receive dispatch. Network_Call_Back routes received global packets through a
 *  switch on GPacket.Command; commands outside the engine's range (our desync
 *  commands are 0xE0+) fall to the default case at 0x48DAC4, which would call
 *  Process_Global_Packet. Intercept there: if it is one of ours, hand it to the
 *  dialog and skip the engine's default handling; otherwise let it run.
 *
 *  0x48DAC4 is `mov edx, offset GAddress` (5 bytes, absolute), so `return 0`
 *  safely restores it and continues into the original mov/call.
 */
DEFINE_HOOK(0x48DAC4, NetworkCallBack_DesyncPacket, 0x5)
{
	enum { SkipDefaultHandler = 0x48DAD3 }; // continue past Process_Global_Packet

	auto* const packet = reinterpret_cast<ExtGlobalPacketType*>(&SessionClass::GlobalReceivePacket);
	auto* const address = reinterpret_cast<IPXAddressClass*>(&SessionClass::GlobalReceiveAddress);

	if (DesyncDialog.Handle_Global_Packet(packet, address))
		return SkipDefaultHandler;

	return 0; // not one of ours: run the engine's Process_Global_Packet
}

/**
 *  Sign-off -> player-left. On NET_SIGN_OFF the engine calls
 *  Destroy_Connection(id, 0) at 0x48D859; tell the dialog so the departing
 *  player shows as "Quit". (Heartbeat timeouts already notify from
 *  DesyncDialogClass::Check_Heartbeat_Timeouts. Mirrors Vinifera's sign-off
 *  path: Destroy_Connection -> Update_Master_After_Player_Removal -> notify.)
 *
 *  We perform the drop ourselves and jump past the original call so the notify
 *  runs *after* the connection is gone and the master reassigned: if the host
 *  is the one leaving, Notify_Player_Left -> Morph_To_Host_Dialog_If_Needed must
 *  see the new master to promote a waiting player. `return 0` would run before
 *  the restored call (wrong order). ECX = id, EDX = error (0).
 */
DEFINE_HOOK(0x48D859, NetworkCallBack_SignOff_NotifyPlayerLeft, 0x5)
{
	enum { AfterDestroyConnection = 0x48D85E };

	GET(int, id, ECX);

	SessionClass::Destroy_Connection(id, 0);
	SessionExt::Update_Master_After_Player_Removal();
	DesyncDialog.Notify_Player_Left(id);

	return AfterDestroyConnection;
}

/**
 *  Trigger (Hook A). Mirrors Vinifera replacing Execute_DoList: we detect
 *  per-player desync ourselves and run our dialog instead of the engine's
 *  stock "out of sync" message box + quit.
 *
 *  Returns 0 from Execute_DoList to the caller (Queue_AI_Multiplayer), whose
 *  failure path then stops the game. Execute_DoList cleans 3 stack args (0xC);
 *  this stub runs in place of the function, so the prologue never executed and
 *  there are no saved registers to restore.
 */
static __declspec(naked) void Execute_DoList_Quit()
{
	__asm
	{
		xor eax, eax
		retn 0Ch
	}
}

/**
 *  Execute_DoList entry: detect desync and, if any, run the dialog. The 4th arg
 *  (skip_crc) is the engine's initial CRC-skip window; respect it so a new game
 *  does not falsely desync. On "continue" let the original run (out-of-sync
 *  players are skipped by the hook below); on "quit" return 0 so the caller
 *  stops the game.
 */
DEFINE_HOOK(0x64C380, Execute_DoList_DesyncDialog, 0xA)
{
	GET_STACK(CDTimerClass*, skip_crc, 0x8);

	const bool in_skip_window = skip_crc && skip_crc->GetTimeLeft() != 0;

	if (!in_skip_window && DesyncDialog.Check_And_Handle_Desync())
		return reinterpret_cast<DWORD>(&Execute_DoList_Quit);

	return 0; // run the original Execute_DoList
}

/**
 *  Execute_DoList outer loop: skip houses we are out of sync with, so their
 *  FRAMEINFO is not re-checked (no stock message box) and their events are not
 *  executed after a "continue". EAX holds the house index here; 0x64CC3D is the
 *  loop's "advance to the next house" path.
 */
DEFINE_HOOK(0x64C4EC, Execute_DoList_SkipOutOfSyncHouse, 0x5)
{
	enum { SkipToNextHouse = 0x64CC3D };

	GET(int, house_index, EAX);

	if (SessionExt::Is_Out_of_Sync(house_index))
		return SkipToNextHouse;

	return 0; // process this house normally
}
