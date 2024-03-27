/**
*  yrpp-spawner
*
*  Copyright(C) 2023-present CnCNet
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

#include "ProtocolZero.h"
#include "ProtocolZero.LatencyLevel.h"

#include <HouseClass.h>
#include <MessageListClass.h>
#include <Utilities/Debug.h>

LatencyLevelEnum LatencyLevel::CurentLatencyLevel = LatencyLevelEnum::LATENCY_LEVEL_INITIAL;
unsigned char LatencyLevel::NewFrameSendRate = 3;

void LatencyLevel::Apply(LatencyLevelEnum newLatencyLevel)
{
	if (newLatencyLevel > LatencyLevelEnum::LATENCY_LEVEL_MAX)
		newLatencyLevel = LatencyLevelEnum::LATENCY_LEVEL_MAX;

	auto maxLatencyLevel = static_cast<LatencyLevelEnum>(ProtocolZero::MaxLatencyLevel);
	if (newLatencyLevel > maxLatencyLevel)
		newLatencyLevel = maxLatencyLevel;

	if (newLatencyLevel <= CurentLatencyLevel)
		return;

	Debug::Log("[Spawner] Player %ls, Loss mode (%d, %d) Frame = %d\n"
		, HouseClass::CurrentPlayer->UIName
		, newLatencyLevel
		, CurentLatencyLevel
		, (int)Unsorted::CurrentFrame
	);

	CurentLatencyLevel = newLatencyLevel;
	NewFrameSendRate = static_cast<unsigned char>(newLatencyLevel);
	Game::Network::PreCalcFrameRate = 60;
	Game::Network::PreCalcMaxAhead = GetMaxAhead(newLatencyLevel);

	MessageListClass::Instance->PrintMessage(GetLatencyMessage(newLatencyLevel), 270, ColorScheme::White, true);
}

int LatencyLevel::GetMaxAhead(LatencyLevelEnum latencyLevel)
{
	const int maxAhead[] =
	{
		/* 0 */ 1

		/* 1 */ ,4
		/* 2 */ ,6
		/* 3 */ ,12
		/* 4 */ ,16
		/* 5 */ ,20
		/* 6 */ ,24
		/* 7 */ ,28
		/* 8 */ ,32
		/* 9 */ ,36
	};

	return maxAhead[(int)latencyLevel];
}

const wchar_t* LatencyLevel::GetLatencyMessage(LatencyLevelEnum latencyLevel)
{
	const wchar_t* message[] =
	{
		/* 0 */ L"CnCNet: Latency mode set to: 0 - Initial" // Players should never see this, if it doesn't then it's a bug

		/* 1 */ ,L"CnCNet: Latency mode set to: 1 - Best"
		/* 2 */ ,L"CnCNet: Latency mode set to: 2 - Super"
		/* 3 */ ,L"CnCNet: Latency mode set to: 3 - Excellent"
		/* 4 */ ,L"CnCNet: Latency mode set to: 4 - Very Good"
		/* 5 */ ,L"CnCNet: Latency mode set to: 5 - Good"
		/* 6 */ ,L"CnCNet: Latency mode set to: 6 - Good"
		/* 7 */ ,L"CnCNet: Latency mode set to: 7 - Default"
		/* 8 */ ,L"CnCNet: Latency mode set to: 8 - Default"
		/* 9 */ ,L"CnCNet: Latency mode set to: 9 - Default"
	};

	return message[(int)latencyLevel];
}

LatencyLevelEnum LatencyLevel::FromResponseTime(unsigned char rspTime)
{
	for (auto i = LatencyLevelEnum::LATENCY_LEVEL_1; i < LatencyLevelEnum::LATENCY_LEVEL_MAX; i = static_cast<LatencyLevelEnum>(1 + static_cast<char>(i)))
	{
		if (rspTime <= GetMaxAhead(i))
			return static_cast<LatencyLevelEnum>(i);
	}

	return LatencyLevelEnum::LATENCY_LEVEL_MAX;
}
