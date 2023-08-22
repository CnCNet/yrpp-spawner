#pragma once

class EventExt;

class ProtocolZero
{
private:
	static constexpr int SendResponseTimeInterval = 30;

public:
	static bool Enable;
	static int WorstMaxAhead; // = 24

	static void SendResponseTime2();
	static void HandleResponseTime2(EventExt* event);
};
