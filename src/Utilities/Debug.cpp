#include "Debug.h"

#include <CRT.h>
#include <MessageListClass.h>
#include <Utilities/Macro.h>

char Debug::StringBuffer[0x1000];

void Debug::Log(const char* pFormat, ...)
{
	JMP_STD(0x4068E0);
}

void Debug::LogAndMessage(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	vsprintf_s(StringBuffer, pFormat, args);
	Log("%s", StringBuffer);
	va_end(args);
	wchar_t buffer[0x1000];
	CRT::mbstowcs(buffer, StringBuffer, 0x1000);
	MessageListClass::Instance->PrintMessage(buffer);
}

void Debug::LogWithVArgs(const char* pFormat, va_list args)
{
	vsprintf_s(StringBuffer, pFormat, args);
	Log("%s", StringBuffer);
}

void Debug::INIParseFailed(const char* section, const char* flag, const char* value, const char* Message)
{
	const char* LogMessage = (Message == nullptr)
		? "Failed to parse INI file content: [%s]%s=%s\n"
		: "Failed to parse INI file content: [%s]%s=%s (%s)\n"
		;

	Debug::Log(LogMessage, section, flag, value, Message);
}

void Debug::FatalErrorAndExit(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	LogWithVArgs(pFormat, args);
	va_end(args);
	FatalExit(static_cast<int>(ExitCode::Undefined));
}

void Debug::FatalErrorAndExit(ExitCode nExitCode, const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	LogWithVArgs(pFormat, args);
	va_end(args);
	FatalExit(static_cast<int>(nExitCode));
}

DEFINE_PATCH( // Add new line after "Init Secondary Mixfiles....."
	/* Offset */ 0x825F9B,
	/*   Data */ '\n'
);

DEFINE_PATCH( // Replace SUN.INI with RA2MD.INI in the debug.log
	/* Offset */ 0x8332F4,
	/*   Data */ "-------- Loading RA2MD.INI settings --------\n"
);
