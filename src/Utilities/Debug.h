#pragma once
#include <Windows.h>

class Debug
{
public:
	enum class ExitCode : int
	{
		Undefined = -1,
		SLFail = 114514
	};

	static char StringBuffer[0x1000];

	static void Log(const char* pFormat, ...);
	static void LogAndMessage(const char* pFormat, ...);
	static void LogWithVArgs(const char* pFormat, va_list args);
	static void INIParseFailed(const char* section, const char* flag, const char* value, const char* Message = nullptr);
	static void FatalErrorAndExit(const char* pFormat, ...);
	static void FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...);
};
