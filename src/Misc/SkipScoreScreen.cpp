#include <Main.h>
#include <Spawner/Spawner.h>
#include <Helpers/Macro.h>

DEFINE_HOOK(0x5C9720, MPScore_Present_SkipScoreScreen, 0x8)
{
	const auto pSpawnerConfig = Spawner::Enabled
		? Spawner::GetConfig()
		: nullptr;

	const bool bSkipScoreScreen = pSpawnerConfig
		? pSpawnerConfig->SkipScoreScreen
		: Main::GetConfig()->SkipScoreScreen;

	return bSkipScoreScreen
		? 0x5C97AC
		: 0;
}
