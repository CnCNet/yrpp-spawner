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
#include <CCFileClass.h>

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
	static void writeLog(CCFileClass& file, const char* pSectionName)
	{
		file.WriteBytes(const_cast<char*>(pSectionName), strlen(pSectionName));
	}

	template <typename T>
	static void LogType(const char* pSectionName, CCFileClass& file)
	{
		writeLog(file, pSectionName);
		writeLog(file, "\n");

		int index = 1;
		char indexStr[10];;

		for (auto pItem : *T::Array)
		{
			sprintf(indexStr, "%d", index++);
			writeLog(file, indexStr);

			writeLog(file, "=");
			char* id = (char*)pItem->get_ID();
			writeLog(file, id);

			if (strcmp(id, pItem->Name) != 0)
			{
				writeLog(file, " ;");
				writeLog(file, pItem->Name);
			}
			writeLog(file, "\n");
		}

		writeLog(file, "\n");
	};

public:
	static void Dump()
	{
		CCFileClass file = CCFileClass("DumpRulesTypes.ini");

		if (file.Open(FileAccessMode::Write))
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

			file.Close();
		}
	};
};
