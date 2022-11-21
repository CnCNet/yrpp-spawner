#pragma once
#include "version.h"
#include "Main.Config.h"
#include <Windows.h>
#include <memory>

class CCINIClass;
class AbstractClass;

class Main
{
private:
	static std::unique_ptr<MainConfig> Config;

public:
	static void ExeRun();
	static void CmdLineParse(char**, int);

	static MainConfig* GetConfig()
	{
		return Config.get();
	}

	// variables
	static HANDLE hInstance;

	static const size_t readLength = 2048;
	static char readBuffer[readLength];
	static wchar_t wideBuffer[readLength];
	static const char readDelims[4];

#ifdef DEBUG
	static bool DetachFromDebugger();
#endif
};
