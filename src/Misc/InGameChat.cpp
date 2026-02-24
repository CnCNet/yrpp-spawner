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

#include <Spawner/Spawner.h>
#include <Utilities/Macro.h>
#include <HouseClass.h>
#include <MessageListClass.h>
#include <Unsorted.h>

// This corrects the processing of Unicode player names
// and prohibits incoming messages from players with whom chat is disabled

#pragma pack(push, 1)
struct GlobalPacket_NetMessage
{
	DEFINE_REFERENCE(GlobalPacket_NetMessage, Instance, 0xA8D638u);

	int Command;
	wchar_t PlayerName[21];
	byte HouseIndex;
	byte ChatID;
	wchar_t Message[112];
	byte Color;
	byte CRC;
};
#pragma pack(pop)

void __fastcall MainLoop_AfterRender_DisableChat(MessageListClass* pMessageList, DWORD)
{
	pMessageList->Manage();

	if (!Spawner::Enabled || !Spawner::GetConfig()->DisableChat)
		return;

	for (int i = 0; i < 8; ++i)
		Game::ChatMask[i] = false;
}

// Don't send message to others when DisableChat is active.
// Mirrors: hack 0x0055EF38, 0x0055EF3E in chat_disable.asm
DEFINE_HOOK(0x55EF38, MessageSend_DisableChat, 0x6)
{
	if (Spawner::Enabled && Spawner::GetConfig()->DisableChat)
		return 0x55F056; // skip the send

	return 0; // execute original: cmp edi, ebx; mov [esp+0x14], ebx
}

// After receiving a message, don't play sound if AddMessage returned NULL
// (i.e. the message was suppressed). Mirrors: hack 0x0048D97E in chat_disable.asm
DEFINE_HOOK(0x48D97E, NetworkCallBack_NetMessage_Sound, 0x5)
{
	if (!Spawner::Enabled)
		return 0;

	if (!R->EAX<void*>())
		return 0x48D99A; // skip sound

	return 0; // execute original: mov eax, [0x8871E0]
}

DEFINE_HOOK(0x48D92B, NetworkCallBack_NetMessage_Print, 0x5)
{
	if (!Spawner::Enabled)
		return 0;

	enum { SkipMessage = 0x48DAD3, PrintMessage = 0x48D937 };

	const int houseIndex = GlobalPacket_NetMessage::Instance.HouseIndex;

	if (houseIndex < 8 && Game::ChatMask[houseIndex])
	{
		if (HouseClass::Array.ValidIndex(houseIndex))
		{
			HouseClass* pHouse = HouseClass::Array.GetItem(houseIndex);

			GlobalPacket_NetMessage::Instance.Color = (byte)pHouse->ColorSchemeIndex;
			R->ESI(pHouse->UIName);
			return PrintMessage;
		}
	}

	return SkipMessage;
}

DEFINE_HOOK(0x48D95B, NetworkCallBack_NetMessage_SetColor, 0x6)
{
	if (!Spawner::Enabled)
		return 0;

	R->EAX(R->ECX());
	return 0x48D966;
}

DEFINE_HOOK(0x55EDD2, MessageInput_Write, 0x5)
{
	if (!Spawner::Enabled)
		return 0;

	HouseClass* pHouse = HouseClass::CurrentPlayer;
	wcscpy_s(GlobalPacket_NetMessage::Instance.PlayerName, pHouse->UIName);
	GlobalPacket_NetMessage::Instance.HouseIndex = (byte)pHouse->ArrayIndex;

	return 0x55EE00;
}

DEFINE_HOOK(0x55F0A8, MessageInput_Print, 0x5)
{
	if (!Spawner::Enabled)
		return 0;

	R->EAX(GlobalPacket_NetMessage::Instance.PlayerName);
	return 0x55F0B2;
}
