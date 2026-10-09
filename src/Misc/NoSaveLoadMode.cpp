#include <Utilities/Macro.h>
#include <Spawner/Spawner.h>
#include <WWMessageBox.h>
#include <LoadProgressManager.h>

DEFINE_HOOK(0x686089, DoLose_NoSaveLoadModeRetryDialog, 0x7)
{
	if (!Spawner::GetConfig()->DisableSaveLoad)
		return 0;

	enum { Restart = 0x6860F6, Leave = 0x6860EE };

	const auto result = WWMessageBox::Instance.Process(
		StringTable::TryFetchString("TXT_HARDCORE_FAILURE", L"GG"),
		StringTable::LoadString("GUI:Restart"),
		StringTable::LoadString("GUI:Leave"), nullptr);

	if (result == WWMessageBox::Result::OK)
		return Restart;

	return Leave;
}

// disable load, save and delete buttons on the ingame menu
DEFINE_HOOK(0x4F17F6, GameOptionsDialog_DisableSaveLoadButtons, 0x6)
{
	if (!Spawner::GetConfig()->DisableSaveLoad)
		return 0;

	GET(HWND, hDialog, EBP);

	HWND hLoadButton = GetDlgItem(hDialog, 1310);
	SendMessageA(hLoadButton, 1202, 0, (LPARAM)StringTable::TryFetchString("GUI:CantLoad", L"Cannot Load"));
	EnableWindow(hLoadButton, FALSE);

	HWND hSaveButton = GetDlgItem(hDialog, 1311);
	SendMessageA(hSaveButton, 1202, 0, (LPARAM)StringTable::TryFetchString("GUI:CantSave", L"Cannot Save"));
	EnableWindow(hSaveButton, FALSE);

	HWND hDeleteButton = GetDlgItem(hDialog, 1312);
	SendMessageA(hDeleteButton, 1202, 0, (LPARAM)L"----");
	EnableWindow(hDeleteButton, FALSE);

	return 0x4F1834;
}

std::wstring NoSaveLoadModeText { };
DEFINE_HOOK(0x553076, LoadProgressManager_Draw_NoSaveLoadModeIndicator, 0x5)
{
	if (!Spawner::GetConfig()->DisableSaveLoad)
		return 0;

	GET(LoadProgressManager*, pLoadProgressManager, EBP);
	if (NoSaveLoadModeText.empty())
		NoSaveLoadModeText = StringTable::TryFetchString("TXT_HARDCORE_MODE", L"HardCore");

	Point2D position
	{
		pLoadProgressManager->TitleBarRect.X + pLoadProgressManager->TitleBarRect.Width - 100,
		pLoadProgressManager->TitleBarRect.Y + 10
	};

	LEA_STACK(RectangleStruct*, pBounds, STACK_OFFSET(0x1268, -0x1204));
	if (auto pLogoSHP = FileSystem::LoadSHPFile("hardcorelogo.shp"))
		pLoadProgressManager->ProgressSurface->DrawSHP(FileSystem::PALETTE_PAL, pLogoSHP, 0, &position, pBounds, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	else
		pLoadProgressManager->ProgressSurface->DrawText(NoSaveLoadModeText.c_str(), &position, COLOR_RED);

	return 0;
}

DEFINE_HOOK(0x6D0E20, TabClass_Draw_NoSaveLoadModeText, 0x6)
{
	if (NoSaveLoadModeText.empty())
		return 0;

	auto surfaceRect = DSurface::Temp->GetRect();
	auto textDimensions = Drawing::GetTextDimensions(NoSaveLoadModeText.c_str(), { 0,0 }, 0);
	Point2D position {
		surfaceRect.Width - textDimensions.Width - 40,
		surfaceRect.Height - 24
	};
	DSurface::Temp->DrawText(NoSaveLoadModeText.c_str(), &position, COLOR_WHITE);

	return 0;
}

DEFINE_HOOK(0x68A1BD, ScenarioClass_ReadMapINI_SetEndOfGame, 0x6)
{
	if (!Spawner::GetConfig()->DisableSaveLoad && Spawner::GetConfig()->CustomMissionID == 0)
		return 0;

	GET(ScenarioClass*, pScenario, ESI);
	pScenario->EndOfGame = true;
	pScenario->SkipScore = false;

	return 0x68A1DB;
}

DEFINE_HOOK(0x6C9357, MissionEndDialog_SetExitGameText, 0x5)
{
	if (ScenarioClass::Instance->EndOfGame)
	{
		GET(HWND, hDialog, EBX);
		auto hContinueButton = GetDlgItem(hDialog, 1059);
		SendMessageA(hContinueButton, 1202, 0, (LPARAM)StringTable::LoadString("GUI:ExitGame"));
	}

	return 0;
}
