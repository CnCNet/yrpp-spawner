#pragma once

#include <UI.h>
/*
enum VanillaDialogs : uint16_t
{
	SingleplayerGameOptionsDialog = 181,
	MultiplayerGameOptionsDialog = 3002
};
*/
// For now this can only contain copies of the original dialogs with same IDs
enum SpawnerCustomDialogs : uint16_t
{
	MultiplayerGameOptionsDialog = 3002, // added a save button

	First = 3002,
	Last = 3002
};

class Dialogs {
public:

};
