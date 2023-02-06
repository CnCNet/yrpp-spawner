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

#include "Spawner.h"

#include <CCFileClass.h>
#include <HouseClass.h>
#include <PacketClass.h>
#include <ScenarioClass.h>
#include <SessionClass.h>
#include <Unsorted.h>
#include <Utilities/Macro.h>

bool __forceinline IsStatisticsEnabled()
{
	return Spawner::Active
		&& Spawner::GetConfig()->WriteStatistics
		&& !SessionClass::IsCampaign();
}

// Write stats.dmp
DEFINE_HOOK(0x6C856C, SendStatisticsPacket_WriteStatisticsDump, 0x5)
{
	if (IsStatisticsEnabled())
	{
		GET(void*, buf, EAX);
		int lengthOfPacket = *reinterpret_cast<int*>(0xB0BD90);

		CCFileClass statsFile = CCFileClass("stats.dmp");
		if (statsFile.Open(FileAccessMode::Write))
		{
			statsFile.WriteBytes(buf, lengthOfPacket);
			statsFile.Close();
		}

		bool& bStatisticsPacketSent = *reinterpret_cast<bool*>(0xA8F900);
		bStatisticsPacketSent = true;

		return 0x6C87B8;
	}

	return 0;
}

// AI has stats
DEFINE_HOOK(0x6C73F8, SendStatisticsPacket_AIHasStats, 0x6)
{
	if (IsStatisticsEnabled())
	{
		GET(HouseClass*, pHouse, EAX);
		if (pHouse && pHouse->Type && !pHouse->Type->MultiplayPassive)
			return 0x6C7402;

		return 0x6C7414;
	}
	return  0;
}

// Use GameStockKeepingUnit instead IsWordDominationTour for GSKU Field
DEFINE_HOOK(0x6C7053, SendStatisticsPacket_SaveGameStockKeepingUnit, 0x6)
{
	if (IsStatisticsEnabled())
		return 0x6C7030;

	return 0;
}

// Add Field HASH
// And use UIMapName instead ScenarioName for SCEN Field
DEFINE_HOOK(0x6C735E, SendStatisticsPacket_AddField_HASH, 0xA)
{
	if (IsStatisticsEnabled())
	{
		LEA_STACK(PacketClass*, pPacket, STACK_OFFSET(0x83A4, -0x8394));
		pPacket->AddField<char*>("SCEN", Spawner::GetConfig()->UIMapName);
		pPacket->AddField<char*>("HASH", Spawner::GetConfig()->MapHash);
		return 0x6C737D;
	}

	// vanilla code
	auto pField = GameCreate<FieldClass>();
	R->EAX(pField);
	return 0x6C735A;
}

// Add Field MYID
DEFINE_HOOK(0x6C7921, SendStatisticsPacket_AddField_MyId, 0x6)
{
	if (IsStatisticsEnabled())
	{
		LEA_STACK(PacketClass*, pPacket, STACK_OFFSET(0x83A8, -0x8394));
		GET(HouseClass*, pHouse, ESI);
		GET(char, id, EBX);

		if (pHouse == HouseClass::CurrentPlayer)
		{
			pPacket->AddField<LONG>("MYID", id - '0');
			pPacket->AddField<DWORD>("NKEY", 0);
			pPacket->AddField<DWORD>("SKEY", 0);
		}
	}

	return 0;
}

// Add Player Fields
DEFINE_HOOK(0x6C7989, SendStatisticsPacket_AddField_ALY, 0x6)
{
	if (IsStatisticsEnabled())
	{
		LEA_STACK(PacketClass*, pPacket, STACK_OFFSET(0x83A4, -0x8394));
		GET(HouseClass*, pHouse, ESI);
		const char id = *reinterpret_cast<char*>(0x841F43);

		char fieldALY[5] = "ALYx";
		fieldALY[3] = id;
		pPacket->AddField<DWORD>(fieldALY, pHouse->Allies.data);

		char fieldBSP[5] = "BSPx";
		fieldALY[3] = id;
		pPacket->AddField<DWORD>(fieldBSP, pHouse->GetSpawnPosition());
	}

	return 0;
}

DEFINE_HOOK(0x6C882A, RegisterGameEndTime_CorrectDuration, 0x6)
{
	if (IsStatisticsEnabled())
	{
		const int startTime = ScenarioClass::Instance->ElapsedTimer.StartTime;
		R->ECX(startTime);
		return 0x6C882A + 0x6;
	}

	return 0;
}

DEFINE_HOOK(0x448524, BuildingClass_Captured_SendStatistics, 0x7)
{
	return IsStatisticsEnabled()
		? 0x64B2ED
		: 0;
}

DEFINE_HOOK(0x55D0FB, AuxLoop_SendStatistics_1, 0x5)
{
	return IsStatisticsEnabled()
		? 0x55D100
		: 0;
}

DEFINE_HOOK(0x55D189, AuxLoop_SendStatistics_2, 0x5)
{
	return IsStatisticsEnabled()
		? 0x55D18E
		: 0;
}

DEFINE_HOOK(0x64C7FA, ExecuteDoList_SendStatistics_1, 0x6)
{
	return IsStatisticsEnabled()
		? 0x64C802
		: 0;
}

DEFINE_HOOK(0x64C81E, ExecuteDoList_SendStatistics_2, 0x6)
{
	return IsStatisticsEnabled()
		? 0x64C826
		: 0;
}

DEFINE_HOOK(0x647AE8, QueueAIMultiplayer_SendStatistics_1, 0x6)
{
	return IsStatisticsEnabled()
		? 0x647AF5
		: 0;
}

DEFINE_HOOK(0x64824B, QueueAIMultiplayer_SendStatistics_2, 0x5)
{
	if (IsStatisticsEnabled())
		R->EAX(GameMode::Internet);

	return 0;
}

DEFINE_HOOK(0x64827D, QueueAIMultiplayer_SendStatistics_3, 0x6)
{
	return IsStatisticsEnabled()
		? 0x648285
		: 0;
}

DEFINE_HOOK(0x64AB6A, QueueProposeKickPlayer_SendStatistics, 0x7)
{
	return IsStatisticsEnabled()
		? 0x64AB73
		: 0;
}

DEFINE_HOOK(0x64B2E4, KickPlayerNow_SendStatistics, 0x7)
{
	return IsStatisticsEnabled()
		? 0x64B2ED
		: 0;
}
