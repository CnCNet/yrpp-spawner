#include <Utilities/Macro.h>

// skip error "А mouse is required for playing Yurts Revenge" - remove the GetSystemMetrics check
DEFINE_JUMP(LJMP, 0x6BD8A4, 0x6BD8C2); // WinMain

// A patch to prevent framerate drops when a player spams the 'type select' key
// Skip call GScreenClass::FlagToRedraw(1)
DEFINE_JUMP(LJMP, 0x732CED, 0x732CF9); // End_Type_Select_Command

DEFINE_HOOK(0x649851, WaitForPlayers_OnlineOptimizations, 0x5)
{
	Sleep(3); // Sleep yields the remaining CPU cycle time to any other processes
	return 0x6488B0;
}
