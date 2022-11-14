#include <Main.h>
#include <Helpers/Macro.h>

DEFINE_HOOK(0x692DD8, ScrollClass_DisableEdgeScrolling1, 0x7)
{
	return Main::GetConfig()->DisableEdgeScrolling
		? 0x692E07
		: 0;
}

DEFINE_HOOK(0x692DFA, ScrollClass_DisableEdgeScrolling2, 0x5)
{
	return Main::GetConfig()->DisableEdgeScrolling
		? 0x692EA2
		: 0;
}

DEFINE_HOOK(0x692E34, ScrollClass_DisableEdgeScrolling3, 0x6)
{
	return Main::GetConfig()->DisableEdgeScrolling
		? 0x692E40
		: 0;
}
