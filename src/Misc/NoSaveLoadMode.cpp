#include <Utilities/Macro.h>
#include <Spawner/Spawner.h>
#include <WWMessageBox.h>
#include <LoadProgressManager.h>

const wchar_t* Fetch_CSF_Text(const char* label, const wchar_t* defaultText)
{
	std::wstring_view msg = StringTable::LoadString(label);
	if (msg.empty() || msg.starts_with(L"MISSING:"))
		return defaultText;
	return msg.data();
}

DEFINE_HOOK(0x686089, DoLose_RetryDialogForCampaigns, 0x7)
{
	if (!Spawner::GetConfig()->DisableSaveLoad) return 0;
	enum { OK = 0x6860F6, Cancel = 0x6860EE};
	if (WWMessageBox::Instance.Process(
		Fetch_CSF_Text("TXT_HARDCORE_FAILURE", L"GG"),
		StringTable::LoadString("GUI:Restart"),
		StringTable::LoadString("GUI:Leave"), nullptr) == WWMessageBox::Result::OK)
		return OK;

	return Cancel;
}

// disable load, save and delete buttons on the ingame menu
DEFINE_HOOK(0x4F17F6, sub_4F1720_DisableSaves, 0x6)
{
	if (!Spawner::GetConfig()->DisableSaveLoad) return 0;
	GET(HWND, hDlg, EBP);

	HWND hLoad = GetDlgItem(hDlg, 1310);
	SendMessageA(hLoad, 1202, 0, (LPARAM)Fetch_CSF_Text("GUI:CantLoad", L"Cannot Load"));
	EnableWindow(hLoad, FALSE);
	HWND hSave = GetDlgItem(hDlg, 1311);
	SendMessageA(hSave, 1202, 0, (LPARAM)Fetch_CSF_Text("GUI:CantSave", L"Cannot Save"));
	EnableWindow(hSave, FALSE);
	HWND hDel = GetDlgItem(hDlg, 1312);
	SendMessageA(hDel, 1202, 0, (LPARAM)L"----");
	EnableWindow(hDel, FALSE);

	return 0x4F1834;
}

std::wstring HardCoreText {};
DEFINE_HOOK(0x553076, LoadProgressMgr_Draw_ExtraText, 0x5)
{
	GET(LoadProgressManager*, self, EBP);
	if (!Spawner::GetConfig()->DisableSaveLoad) return 0;

	Point2D pos
	{
		self->TitleBarRect.X + self->TitleBarRect.Width - 100,
		self->TitleBarRect.Y + 10
	};
	if (HardCoreText.empty())
		HardCoreText = Fetch_CSF_Text("TXT_HARDCORE_MODE", L"HardCore");
	LEA_STACK(RectangleStruct*, pBnd, STACK_OFFSET(0x1268, -0x1204));
	if (auto logo = FileSystem::LoadSHPFile("hardcorelogo.shp"))
	{
		self->ProgressSurface->DrawSHP(FileSystem::PALETTE_PAL, logo, 0, &pos, pBnd, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}
	else
	{
		self->ProgressSurface->DrawText(HardCoreText.c_str(), &pos, COLOR_RED);
	}

	return 0;
}

DEFINE_HOOK(0x6D0E20, TabClass_Draw_ccb, 0x6)
{
	if (HardCoreText.empty()) return 0;
	auto rect = DSurface::Temp->GetRect();
	auto wanted = Drawing::GetTextDimensions(HardCoreText.c_str(), { 0,0 }, 0);
	Point2D pt {
		rect.Width - wanted.Width - 40,
		rect.Height - 24
	};
	DSurface::Temp->DrawText(HardCoreText.c_str(), &pt, COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(0x68A1BD, ScenarioCass_ReadMapINI_eog, 0x6)
{
	if (!Spawner::GetConfig()->DisableSaveLoad && Spawner::GetConfig()->CustomMissionID == 0)
		return 0;
	GET(ScenarioClass*, sce, ESI);
	sce->EndOfGame = true;
	sce->SkipScore = false;
	return 0x68A1DB;
}

DEFINE_HOOK(0x6C9357, MissionEndDlg_ExitText, 0x5)
{
	if (ScenarioClass::Instance->EndOfGame)
	{
		GET(HWND, hdlg, EBX);
		auto btnCtn = GetDlgItem(hdlg, 1059);
		SendMessageA(btnCtn, 1202, 0, (LPARAM)StringTable::LoadString("GUI:ExitGame"));
	}
	return 0;
}
