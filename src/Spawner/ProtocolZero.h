#pragma once

class EventExt;

class ProtocolZero
{
private:
	static constexpr int SendResponseTimeInterval = 30;

public:
	static bool Enable;
	static unsigned char MaxLatencyLevel;
	static int WorstMaxAhead;

	static void SendResponseTime2();
	static void HandleResponseTime2(EventExt* event);
};
