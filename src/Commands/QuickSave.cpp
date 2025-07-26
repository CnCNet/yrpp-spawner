#include "QuickSave.h"

#include <ScenarioClass.h>
#include <HouseClass.h>
#include <SessionClass.h>
#include <EventClass.h>

const char* QuickSaveCommandClass::GetName() const
{
	return "Quicksave";
}

const wchar_t* QuickSaveCommandClass::GetUIName() const
{
	return StringTable::TryFetchString("TXT_QUICKSAVE", L"Quicksave");
}

const wchar_t* QuickSaveCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* QuickSaveCommandClass::GetUIDescription() const
{
	return StringTable::TryFetchString("TXT_QUICKSAVE_DESC", L"Save the current game.");
}

void QuickSaveCommandClass::Execute(WWKey eInput) const
{
	EventClass event { HouseClass::CurrentPlayer->ArrayIndex,EventType::SaveGame };
	EventClass::AddEvent(event);
}
