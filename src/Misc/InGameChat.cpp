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

// This corrects the processing of Unicode player names
// and prohibits incoming messages from players with whom chat is disabled

#pragma pack(push, 1)
struct GlobalPacket_NetMessage
{
	static constexpr reference<GlobalPacket_NetMessage, 0xA8D638u> const Instance {};

	int Command;
	wchar_t PlayerName[21];
	byte HouseIndex;
	byte ChatID;
	wchar_t Message[112];
	byte Color;
	byte CRC;
};
#pragma pack(pop)

DEFINE_HOOK(0x48D92B, NetworkCallBack_NetMessage_Print, 0x5)
{
	if (!Spawner::Enabled)
		return 0;

	enum { SkipMessage = 0x48DAD3, PrintMessage = 0x48D937 };

	const int houseIndex = GlobalPacket_NetMessage::Instance->HouseIndex;

	if (houseIndex < 8 && Game::ChatMask[houseIndex])
	{
		if (HouseClass::Array->ValidIndex(houseIndex))
		{
			HouseClass* pHouse = HouseClass::Array->GetItem(houseIndex);

			GlobalPacket_NetMessage::Instance->Color = (byte)pHouse->ColorSchemeIndex;
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
	wcscpy_s(GlobalPacket_NetMessage::Instance->PlayerName, pHouse->UIName);
	GlobalPacket_NetMessage::Instance->HouseIndex = (byte)pHouse->ArrayIndex;

	return 0x55EE00;
}

DEFINE_HOOK(0x55F0A8, MessageInput_Print, 0x5)
{
	if (!Spawner::Enabled)
		return 0;

	R->EAX(GlobalPacket_NetMessage::Instance->PlayerName);
	return 0x55F0B2;
}
