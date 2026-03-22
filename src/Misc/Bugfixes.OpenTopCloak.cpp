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
#include <FootClass.h>
#include <WeaponTypeClass.h>

// NOTE: Overrides incorrect Ares hook at the same address.
// Simplified: always apply the cloak check without configurable open-topped behavior.
DEFINE_HOOK(0x6FCA26, TechnoClass_CanFire_RevertAresOpenTopCloakFix, 0x6)
{
	enum { Continue = 0x6FCA36, NotApplicable = 0x6FCA5E };

	GET(WeaponTypeClass*, pWeapon, EBX);

	if (!pWeapon->DecloakToFire)
		return NotApplicable;

	GET(TechnoClass*, pThis, ESI);

	R->EAX(pThis->CloakState);
	return Continue;
}

// Prevent the transporter from decloaking when a passenger fires from an open-topped transport.
//	DEFINE_HOOK(0x6FCD1D, TechnoClass_CanFire_OpenTopCloakFix, 0x5)
//	{
//		return 0;
//	}
