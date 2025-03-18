#include <CCINIClass.h>

class INIClassExt
{
public:
	// Ares replaced ReadString with its implementations, which is very different from the vanilla.
	// Vanilla implementation reads raw bytes, which makes it possible to read the UTF-8 from INI.
	// Ares implementation reads only valid ANSI symbols - Belonit
	static int ReadString_WithoutAresHook(INIClass* pThis, const char* pSection, const char* pKey, const char* pDefault, char* pBuffer, size_t szBufferSize);
};
