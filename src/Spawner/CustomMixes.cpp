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
#include "Spawner.h"
#include "Ra2Mode.h"

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <MixFileClass.h>

#include <list>

static std::list<MixFileClass*> CustomMixes		= {  };

inline void FreeMixes(std::list<MixFileClass*>& mixes)
{
	for (auto pMix : CustomMixes)
		GameDelete(pMix);
	CustomMixes.clear();
}

DEFINE_HOOK(0x6BE9BD, ProgEnd_CustomMixes, 0x6)
{
	FreeMixes(CustomMixes);
	return 0;
}

DEFINE_HOOK(0x5301AC, InitBootstrapMixfiles_CustomMixes_Preload, 0x5)
{
	FreeMixes(CustomMixes);

	auto	config	= Spawner::GetConfig();
	for (auto& pair : config->PreloadMixes)
	{
		CustomMixes.push_back(GameCreate<MixFileClass>(pair.second.c_str()));
		Debug::Log(" %s ", pair.second.c_str());
	}

	// Any 'mode' mixes should be loaded after user custom mixes to prevent overload it.
	if (Ra2Mode::IsEnabled())
		CustomMixes.push_back(GameCreate<MixFileClass>(Ra2Mode::MixFileName));

	return 0;
}

DEFINE_HOOK_AGAIN(0x5302E4, InitBootstrapMixfiles_CustomMixes_Postload, 0x9)
DEFINE_HOOK(0x53044A, InitBootstrapMixfiles_CustomMixes_Postload, 0x9)
{
	auto	config	= Spawner::GetConfig();
	for (auto& pair : config->PostloadMixes)
	{
		CustomMixes.push_back(GameCreate<MixFileClass>(pair.second.c_str()));
		Debug::Log(" %s ", pair.second.c_str());
	}

	return 0;
}
