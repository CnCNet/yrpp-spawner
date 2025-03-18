#include "Body.h"

// Ares replaced ReadString with its implementations, which is very different from the vanilla.
// Vanilla implementation reads raw bytes, which makes it possible to read the UTF-8 from INI.
// Ares implementation reads only valid ANSI symbols - Belonit
int INIClassExt::ReadString_WithoutAresHook(INIClass* pThis, const char* pSection, const char* pKey, const char* pDefault, char* pBuffer, size_t szBufferSize)
{
	EPILOG_THISCALL;

	// It's 5 bytes corrupted by the ares hook
	_asm { sub  esp, 0xC };
	_asm { xor  eax, eax };

	_asm { mov edx, 0x528A10 + 5 };
	_asm { jmp edx }
}
