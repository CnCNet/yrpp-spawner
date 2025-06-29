#include "Body.h"
#include <Utilities/Macro.h>

int __fastcall ReadString_WithoutAresHook_Impl(INIClass* pThis, discard_t, const char* pSection, const char* pKey, const char* pDefault, char* pBuffer, size_t szBufferSize)
{
	EPILOG_THISCALL;

	// It's 5 bytes corrupted by the ares hook
	_asm { sub  esp, 0xC };
	_asm { xor  eax, eax };

	_asm { mov edx, 0x528A10 + 5 };
	_asm { jmp edx }
}

int INIClassExt::ReadString_WithoutAresHook(INIClass* pThis, const char* pSection, const char* pKey, const char* pDefault, char* pBuffer, size_t szBufferSize)
{
	return ReadString_WithoutAresHook_Impl(pThis, 0, pSection, pKey, pDefault, pBuffer, szBufferSize);
}
