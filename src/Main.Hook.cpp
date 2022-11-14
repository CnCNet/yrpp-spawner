#include <Main.h>
#include <Helpers/Macro.h>

bool __stdcall DllMain(HANDLE hInstance, DWORD dwReason, LPVOID v)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		Main::hInstance = hInstance;
	}
	return true;
}

DEFINE_HOOK(0x7CD810, ExeRun, 0x9)
{
	Main::ExeRun();
	return 0;
}

DEFINE_HOOK(0x52F639, ParseCommandLine, 0x5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Main::CmdLineParse(ppArgs, nNumArgs);
	return 0;
}

DEFINE_HOOK(0x6BC0D7, WinMain_LoadSettings, 0x5)
{
	Main::GetConfig()->LoadFromINIFile();
	Main::GetConfig()->ApplyStaticOptions();
	return 0;
}
