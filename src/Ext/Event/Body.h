#pragma once
#include <cstddef>
#include <stdint.h>

enum class EventTypeExt : uint8_t
{
	// Vanilla game used Events from 0x00 to 0x2F
	// Ares used Events 0x60 and 0x61

	ResponseTime2 = 0x30,

	FIRST = ResponseTime2,
	LAST = ResponseTime2
};

#pragma pack(push, 1)
class EventExt
{
public:
	EventTypeExt Type;
	bool IsExecuted;
	char HouseIndex;
	uint32_t Frame;
	union
	{
		struct ResponseTime2
		{
			char MaxAhead;
			uint8_t LatencyLevel;
		} ResponseTime2;

		char DataBuffer[104];
	};

	bool AddEvent();
	void RespondEvent();

	static size_t GetDataSize(EventTypeExt type);
	static bool IsValidType(EventTypeExt type);
};

static_assert(sizeof(EventExt) == 111);
static_assert(offsetof(EventExt, DataBuffer) == 7);
#pragma pack(pop)
