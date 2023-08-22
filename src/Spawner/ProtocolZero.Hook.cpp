#include "ProtocolZero.h"
#include "ProtocolZero.LatencyLevel.h"
#include "Spawner.h"

#include <Ext/Event/Body.h>
#include <Helpers/Macro.h>
#include <Utilities/Debug.h>
#include <Unsorted.h>
#include <EventClass.h>

DEFINE_HOOK(0x55DDA0, MainLoop_AfterRender__ProtocolZero, 0x5)
{
	if (ProtocolZero::Enable && Spawner::Active)
		ProtocolZero::SendResponseTime2();

	return 0;
}

DEFINE_HOOK(0x647BEB, QueueAIMultiplayer__ProtocolZero1, 0x9)
{
	if (ProtocolZero::Enable)
		return 0x647BF4;

	return (R->ESI() >= 5)
		? 0x647BF4
		: 0x647F36;
}

DEFINE_HOOK(0x647EB4, QueueAIMultiplayer__ProtocolZero2, 0x8)
{
	if (ProtocolZero::Enable)
	{
		R->AL(LatencyLevel::NewFrameSendRate);
		R->ECX((DWORD)Unsorted::CurrentFrame);

		return 0x647EBE;
	}

	return 0;
}

DEFINE_HOOK(0x647DF2, QueueAIMultiplayer__ProtocolZero3, 0x5)
{
	if (ProtocolZero::Enable)
	{
		R->EDX((DWORD)Game::Network::MaxAhead & 0xffff);

		return 0x647DF2 + 0x5;
	}

	return 0;
}

DEFINE_HOOK(0x4C8011, EventClassExecute__ProtocolZero, 0x8)
{
	if (ProtocolZero::Enable)
		return 0x4C8024;

	return 0;
}

DEFINE_HOOK(0x64C598, ExecuteDoList__ProtocolZero, 0x6)
{
	if (ProtocolZero::Enable)
	{
		auto dl = (uint8_t)R->DL();

		if (dl == (uint8_t)EventType::EMPTY)
			return 0x64C63D;

		if (dl == (uint8_t)EventType::PROCESS_TIME)
			return 0x64C63D;

		if (dl == (uint8_t)EventTypeExt::ResponseTime2)
			return 0x64C63D;
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x64771D, QueueAIMultiplayer__ProtocolZero_SetTiming, 0x5)
DEFINE_HOOK(0x647E6B, QueueAIMultiplayer__ProtocolZero_SetTiming, 0x5)
{
	if (ProtocolZero::Enable)
	{
		GET(int, NewRetryDelta, EBP);
		GET(int, NewRetryTimeout, EAX);

		Debug::Log("[Spawner] NewRetryDelta = %d, NewRetryTimeout = %d, FrameSendRate = %d, CurentLatencyLevel = %d\n"
			, NewRetryDelta
			, NewRetryTimeout
			, (int)Game::Network::FrameSendRate
			, (int)LatencyLevel::CurentLatencyLevel
		);
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6476CB, QueueAIMultiplayer__ProtocolZero_ResponseTime, 0x5)
DEFINE_HOOK(0x647CC5, QueueAIMultiplayer__ProtocolZero_ResponseTime, 0x5)
{
	if (ProtocolZero::Enable)
	{
		R->EAX(ProtocolZero::WorstMaxAhead);
		return R->Origin() + 0x5;
	}

	return 0;
}
