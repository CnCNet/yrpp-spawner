/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <Unsorted.h>
#include <GameStrings.h>

/*
	Fixed Blowfish.dll error - ***FATAL*** String Manager failed to initialize properly
	if the game did not find a registered file in the system, it will no longer
	try to register this file, but will load it bypassing registration.
*/

HRESULT __stdcall Blowfish_Loader(
	REFCLSID  rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD     dwClsContext,
	REFIID    riid,
	LPVOID* ppv
)
{
	typedef HRESULT(__stdcall* pDllGetClassObject)(const IID&, const IID&, IClassFactory**);

	auto result = REGDB_E_KEYMISSING;

	// First, let's try to run the vanilla function
	result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	if (SUCCEEDED(result))
		return result;

	HMODULE hDll = LoadLibrary(GameStrings::BLOWFISH_DLL);
	if (hDll)
	{
		auto GetClassObject = (pDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject");
		if (GetClassObject)
		{

			IClassFactory* pIFactory;
			result = GetClassObject(rclsid, IID_IClassFactory, &pIFactory);

			if (SUCCEEDED(result))
			{
				result = pIFactory->CreateInstance(pUnkOuter, riid, ppv);
				pIFactory->Release();
			}
		}
	}

	if (!SUCCEEDED(result))
	{
		FreeLibrary(hDll);

		const char* Message = "File Blowfish.dll was not found\n";
		Imports::MessageBoxA(0, Message, "Fatal error ", MB_ICONERROR);
		Debug::FatalErrorAndExit(Message);
	}

	return result;
}

DEFINE_FUNCTION_JUMP(CALL6, 0x6BEDDD, Blowfish_Loader);
DEFINE_FUNCTION_JUMP(CALL6, 0x437F6E, Blowfish_Loader);
