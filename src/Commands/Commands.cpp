#include "Commands.h"

#include "QuickSave.h"
#include <CCINIClass.h>
#include <InputManagerClass.h>
#include <MouseClass.h>
#include <WWMouseClass.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x533066, CommandClassCallback_Register, 0x6)
{
	// Load it after Ares'
	MakeCommand<QuickSaveCommandClass>();
	return 0;
}
