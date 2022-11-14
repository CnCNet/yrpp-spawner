#include <Utilities/Macro.h>
#include <FootClass.h>
#include <EBolt.h>

// Fix crash at 6F9DB6
DEFINE_JUMP(LJMP, 0x5F58CB, 0x5F58D2); // ObjectClass::Mark

// Fix crash at 727B48
DEFINE_HOOK(0x727B44, TriggerTypeClass_ComputeCRC_FixCrash, 0x6)
{
	GET(HouseTypeClass*, pHouseType, EAX);
	if (!pHouseType)
		return 0x727B55;

	return 0;
}

// Fix crash at 4722F0
DEFINE_HOOK(0x70F82A, TechnoClass_GetMyOwner_FixCrash, 0x7)
{
	GET(TechnoClass*, pTechno, EAX);
	if (!pTechno->CaptureManager)
		return 0x70F837;

	return 0;
}

// Fix crash at 6F49DE
DEFINE_HOOK(0x6F49D8, TechnoClass_Revealed_FixCrash, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	if (!pHouse)
		return 0x6F4A31;

	return 0;
}

// Fix crash at 70AF6C
DEFINE_HOOK(0x70AF6C, TechnoClass_70AF50_FixCrash, 0x9)
{
	GET(TechnoClass*, pTechno, EAX);
	if (!pTechno)
		return 0x70B1C7;

	return 0;
}

// Fix crash at 65DC17
DEFINE_HOOK(0x65DC17, DoReinforcements_FixCrash, 0x6)
{
	GET(HouseClass*, pHouse, ECX);
	if (!pHouse)
	{
		R->EAX(0);
		return 0x70B1C7;
	}

	return 0;
}

// Fix crash at 4C2C19
void __fastcall EBolt_SetOwnerAndWeapon_FixCrash(EBolt* pThis, void*, TechnoClass* pTechno, int pWeapon)
{
	// vanilla code
	if (pTechno && pTechno->WhatAmI() == AbstractType::Unit && pTechno->IsAlive && !pTechno->InLimbo)
	{
		pThis->Owner = pTechno;
		pThis->WeaponSlot = pWeapon;
	}
	// correction code
	else
	{
		pThis->Owner = 0;
		pThis->WeaponSlot = 0;
	}
}

DEFINE_JUMP(CALL, 0x6FD606, GET_OFFSET(EBolt_SetOwnerAndWeapon_FixCrash)); // Replace single call
DEFINE_JUMP(LJMP, 0x4C2BD0, GET_OFFSET(EBolt_SetOwnerAndWeapon_FixCrash)); // For in case another module tries to call function
