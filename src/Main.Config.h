#pragma once
class MainConfig
{
public:
	bool SingleProcAffinity;
	bool DisableEdgeScrolling;
	bool QuickExit;
	bool SkipScoreScreen;

	bool WindowedMode;
	bool NoWindowFrame;
	int  DDrawTargetFPS;

	void LoadFromINIFile();
	void ApplyStaticOptions();

	MainConfig()
		: SingleProcAffinity { true }
		, DisableEdgeScrolling { false }
		, QuickExit { false }
		, SkipScoreScreen { false }

		, WindowedMode { false }
		, NoWindowFrame { false }
		, DDrawTargetFPS { -1 }
	{ }
};
