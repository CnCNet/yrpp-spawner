#include "NoCD.h"
#include <GetCDClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

// Based on https://github.com/Ares-Developers/Ares/blob/4f1d929920aca31924c6cd4d3dfa849daa65252a/src/Misc/CopyProtection.cpp

// This douchebag blows your base up when it thinks you're cheating
DEFINE_JUMP(LJMP, 0x55CFDF, 0x55D059); // AuxLoop

// Allows run game without the launcher
DEFINE_PATCH(0x49F5C0,    // CopyProtect__IsLauncherRunning
	0xB0, 0x01,           // mov    al, 1
	0xC3);                // retn

DEFINE_PATCH(0x49F620,    // CopyProtect__NotifyLauncher
	0xB0, 0x01,           // mov    al, 1
	0xC3);                // retn

DEFINE_PATCH(0x49F7A0,    // CopyProtect__Validate
	0xB0, 0x01,           // mov    al, 1
	0xC3);                // retn
