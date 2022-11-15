#include <Utilities/Macro.h>

#pragma warning(push)
#pragma warning(disable: 4702)

LONG __fastcall PrintException(int exception_id, _EXCEPTION_POINTERS* ExceptionInfo)
{
	JMP_STD(0x4C8FE0);
}

LONG __fastcall TopLevelExceptionFilter(int exception_id, _EXCEPTION_POINTERS* ExceptionInfo)
{
	DWORD* eip = &(ExceptionInfo->ContextRecord->Eip);
	switch (*eip)
	{
	case 0x7BC806:
		*eip = 0x7BC80F;
		return EXCEPTION_CONTINUE_EXECUTION;

	case 0x5D6C21:
		// This bug most likely happens when a map Doesn't have Waypoint 90
		*eip = 0x5D6C36;
		return EXCEPTION_CONTINUE_EXECUTION;

	case 0x7BAEA1:
		// A common crash in DSurface::GetPixel
		*eip = 0x7BAEA8;
		ExceptionInfo->ContextRecord->Ebx = 0;
		return EXCEPTION_CONTINUE_EXECUTION;

	case 0x535DBC:
		// Common crash in keyboard command class
		*eip = 0x535DCE;
		ExceptionInfo->ContextRecord->Esp += 12;
		return EXCEPTION_CONTINUE_EXECUTION;

	case 0x000000:
		if (ExceptionInfo->ContextRecord->Esp && *(DWORD*)ExceptionInfo->ContextRecord->Esp == 0x55E018)
		{
			// A common crash that seems to happen when yuri prime mind controls a building and then dies while the user is pressing hotkeys
			*eip = 0x55E018;
			ExceptionInfo->ContextRecord->Esp += 8;
			return  EXCEPTION_CONTINUE_EXECUTION;
		}
		return PrintException(exception_id, ExceptionInfo);
		break;
	default:
		return PrintException(exception_id, ExceptionInfo);
	}
	return 0;
}


DEFINE_JUMP(LJMP, 0x6BE06F, GET_OFFSET(TopLevelExceptionFilter))

#pragma warning(pop)
