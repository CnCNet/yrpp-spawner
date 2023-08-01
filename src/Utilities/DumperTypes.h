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

#pragma once
#include <fstream>

#include <HouseTypeClass.h>

#include <InfantryTypeClass.h>
#include <UnitTypeClass.h>
#include <AircraftTypeClass.h>
#include <BuildingTypeClass.h>

#include <TerrainTypeClass.h>
#include <SmudgeTypeClass.h>
#include <OverlayTypeClass.h>

#include <AnimTypeClass.h>
#include <VoxelAnimTypeClass.h>
#include <ParticleTypeClass.h>
#include <ParticleSystemTypeClass.h>

#include <WeaponTypeClass.h>
#include <SuperWeaponTypeClass.h>
#include <WarheadTypeClass.h>

#include <BulletTypeClass.h>

class DumperTypes
{
	template <typename T>
	static  __forceinline void LogType(const char* pSectionName, std::ofstream& file)
	{
		file << pSectionName;
		file << "\n";

		int i = 1;
		for (auto pItem : *T::Array)
		{
			file << i++;
			file << "=";
			file << pItem->get_ID();
			if (strcmp(pItem->get_ID(), pItem->Name) != 0)
			{
				file << " ;";
				file << pItem->Name;
			}
			file << "\n";
		}

		file << "\n";
	};

public:
	static void Dump()
	{
		std::ofstream file("DumpRulesTypes.ini");

		if (file.is_open())
		{
			LogType<HouseTypeClass>("[Countries]", file);

			LogType<InfantryTypeClass>("[InfantryTypes]", file);
			LogType<UnitTypeClass>("[VehicleTypes]", file);
			LogType<AircraftTypeClass>("[AircraftTypes]", file);
			LogType<BuildingTypeClass>("[BuildingTypes]", file);

			LogType<TerrainTypeClass>("[TerrainTypes]", file);
			LogType<SmudgeTypeClass>("[SmudgeTypes]", file);
			LogType<OverlayTypeClass>("[OverlayTypes]", file);

			LogType<AnimTypeClass>("[Animations]", file);
			LogType<VoxelAnimTypeClass>("[VoxelAnims]", file);
			LogType<ParticleTypeClass>("[Particles]", file);
			LogType<ParticleSystemTypeClass>("[ParticleSystems]", file);

			LogType<WeaponTypeClass>("[WeaponTypes]", file);
			LogType<SuperWeaponTypeClass>("[SuperWeaponTypes]", file);
			LogType<WarheadTypeClass>("[Warheads]", file);

			LogType<BulletTypeClass>("[Projectiles]", file);

			file.close();
		}
	};
};
