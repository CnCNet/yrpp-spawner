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
#include <FPSCounter.h>
#include <HouseClass.h>
#include <PacketClass.h>
#include <ScenarioClass.h>
#include <SessionClass.h>
#include <Unsorted.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>


// MP score global statistics array, populated by the original sub_5C98A0
// during MPScore_InitDialog. The in-game score screen reads from the same
// array, so reading it here guarantees DTA.LOG matches the in-game display
// exactly (including Score's if(>0) filter and the winner bonus).
// Each record is 112 bytes with 16-byte field spacing (Westwood-style align).
struct MPScoreStatsEntry
{
	wchar_t Name[20];   // 0x00 Player name (copy of pHouse->UIName)
	int    Scheme;      // 0x28 Scenario index
	int    Winner;      // 0x2C 1=winner 0=loser
	int    Lost;        // 0x30 Lost = TotalKilledUnits + TotalKilledBuildings
	int    _pad0[3];    // 0x34
	int    Kills;       // 0x40 Kills = sum(KilledUnitsOfHouses) + sum(KilledBuildingsOfHouses)
	int    _pad1[3];    // 0x44
	int    Built;       // 0x50 Built = sum of 4 FactoryProduced*Types.GetTotal()
	int    _pad2[3];    // 0x54
	int    Score;       // 0x60 Score (with if(>0) filter and winner bonus applied)
	int    _pad3[3];    // 0x64
};
static_assert(sizeof(MPScoreStatsEntry) == 112, "MPScoreStatsEntry size mismatch");

DEFINE_POINTER(MPScoreStatsEntry, MPScoreStatsArray, 0xA8D1FCu)
DEFINE_POINTER(int, MPScoreStatsCount, 0xA8D580u)

bool __forceinline IsStatisticsEnabled()
{
	return Spawner::Active
		&& Spawner::GetConfig()->WriteStatistics
		&& !SessionClass::IsCampaign();
}

static void WriteDTALog()
{
	if (!Spawner::Active)
		return;

	if (!Spawner::GetConfig()->GenerateStatistics)
		return;

	CreateDirectoryA("debug", nullptr);

	CCFileClass file = CCFileClass("debug\\Statistic.log");
	if (!file.Open(FileAccessMode::Write))
		return;

	// Read from the global array populated by sub_5C98A0, the same data
	// source as the in-game score screen. The array is filled after
	// MPScore_InitDialog (0x5C9D42) calls sub_5C98A0.
	const int count = *MPScoreStatsCount;

	for (int i = 0; i < count; i++)
	{
		const auto& entry = MPScoreStatsArray[i];

		char name[64] = { 0 };
		WideCharToMultiByte(CP_UTF8, 0, entry.Name, -1, name, sizeof(name), nullptr, nullptr);

		const char* result = entry.Winner ? "Winner" : "Loser";

		char buffer[256] = { 0 };
		sprintf_s(buffer, "%s: %s\n Lost = %d\n Kills = %d\n Built = %d\n Score = %d\n",
			name, result, entry.Lost, entry.Kills, entry.Built, entry.Score);
		file.WriteBytes(buffer, static_cast<int>(strlen(buffer)));
	}

	char fpsBuffer[128] = { 0 };
	sprintf_s(fpsBuffer, "Game loop finished. Average FPS = %d\n",
		static_cast<int>(FPSCounter::GetAverageFrameRate()));
	file.WriteBytes(fpsBuffer, static_cast<int>(strlen(fpsBuffer)));

	file.Close();
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

DEFINE_HOOK(0x5C9D47, MPScore_InitDialog_WriteDTALog, 0x6)
{
	// sub_5C98A0 has just finished (called at 0x5C9D42), so the global
	// statistics array is now populated with the exact same Score shown
	// in-game (including the if(>0) filter and the winner bonus).
	WriteDTALog();
	return 0;
}

// Send AI player
// Dont send observer
DEFINE_HOOK(0x6C73F8, SendStatisticsPacket_HouseFilter, 0x6)
{
	enum { Send = 0x6C7406, DontSend = 0x6C7414 };

	if (IsStatisticsEnabled())
	{
		GET(HouseClass*, pHouse, EAX);

		const bool isMultiplayPassive = (pHouse && pHouse->Type && pHouse->Type->MultiplayPassive);
		const bool isObserver = (pHouse && pHouse->IsInitiallyObserver());

		return (isMultiplayPassive || isObserver)
			? DontSend
			: Send;
	}

	return 0;
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
DEFINE_HOOK(0x6C735E, SendStatisticsPacket_AddField_HASH, 0x5)
{
	if (IsStatisticsEnabled())
	{
		LEA_STACK(PacketClass*, pPacket, STACK_OFFSET(0x83A4, -0x8394));
		pPacket->AddField<wchar_t*>((char*)"SCEN", Spawner::GetConfig()->UIMapName, sizeof(Spawner::GetConfig()->UIMapName));
		pPacket->AddField<char*>((char*)"HASH", Spawner::GetConfig()->MapHash);
		return 0x6C737D;
	}

	return 0;
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
			pPacket->AddField<LONG>((char*)"MYID", id - '0');
			pPacket->AddField<DWORD>((char*)"NKEY", 0);
			pPacket->AddField<DWORD>((char*)"SKEY", 0);
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

		char fieldALY[] = "ALY*";
		fieldALY[3] = id;
		pPacket->AddField<DWORD>(fieldALY, pHouse->Allies.data);

		char fieldBSP[] = "BSP*";
		fieldBSP[3] = id;
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
	enum { Send = 0x44852D, DontSend = 0x448559 };

	return IsStatisticsEnabled() || (SessionClass::Instance.GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x55D0FB, AuxLoop_SendStatistics_1, 0x5)
{
	enum { Send = 0x55D100, DontSend = 0x55D123 };

	return IsStatisticsEnabled() || (SessionClass::Instance.GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x55D189, AuxLoop_SendStatistics_2, 0x5)
{
	enum { Send = 0x55D18E, DontSend = 0x55D1B1 };

	return IsStatisticsEnabled() || (SessionClass::Instance.GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x64C7FA, ExecuteDoList_SendStatistics_1, 0x6)
{
	enum { Send = 0x64C802, DontSend = 0x64C850 };

	return IsStatisticsEnabled() || (SessionClass::Instance.GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x64C81E, ExecuteDoList_SendStatistics_2, 0x6)
{
	enum { Send = 0x64C826, DontSend = 0x64C850 };

	return IsStatisticsEnabled() || (SessionClass::Instance.GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x64C84B, ExecuteDoList_SendStatistics_3, 0x5)
{
	Game::RegisterGameEndTime();
	Game::SendStatisticsPacket();

	return 0x64C84B + 0x5;
}

DEFINE_HOOK(0x647AE8, QueueAIMultiplayer_SendStatistics_1, 0x7)
{
	enum { Send = 0x647AF5, DontSend = 0x6482A6 };

	return IsStatisticsEnabled() || (SessionClass::Instance.GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x64823C, QueueAIMultiplayer_SendStatistics_2, 0x5)
{
	Debug::LogGame(reinterpret_cast<char*>(0x8373BC) /* "Failure executing DoList\n" */);

	enum { Send = 0x648257, DontSend = 0x64825C };

	return IsStatisticsEnabled() || (SessionClass::Instance.GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x64827D, QueueAIMultiplayer_SendStatistics_3, 0x6)
{
	enum { Send = 0x648285, DontSend = 0x6482A6 };

	return IsStatisticsEnabled() || (SessionClass::Instance.GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x648089, QueueAIMultiplayer_SendStatistics_4, 0x5)
{
	enum { Send = 0x64808E, DontSend = 0x648093 };

	return IsStatisticsEnabled() || (SessionClass::Instance.GameMode == GameMode::Internet)
		? Send
		: DontSend;
}

DEFINE_HOOK(0x64B2E4, KickPlayerNow_SendStatistics, 0x7)
{
	enum { Send = 0x64B2ED, DontSend = 0x64B352 };

	return IsStatisticsEnabled() || (SessionClass::Instance.GameMode == GameMode::Internet)
		? Send
		: DontSend;
}
