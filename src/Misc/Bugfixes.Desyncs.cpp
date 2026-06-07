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

// Multiplayer desync fixes for missing CellClass::RecalcAttributes calls.
//
// CellClass::RecalcAttributes derives LandType/Passability from
// a cell's iso tile and overlay. Several game code paths clear overlays
// (crates, flags, veinholes, bridges) without calling RecalcAttributes,
// leaving LandType stale. This diverges between players when an aircraft
// passes over the cell(s) which later leads to pathfinding differences.

#include <Utilities/Macro.h>
#include <CellClass.h>
#include <FootClass.h>
#include <MapClass.h>
#include <TechnoClass.h>

// ============================================================
// Aircraft shadow desync fix
// ============================================================
// AircraftClass::Draw_It temporarily calls Set_Height(0) before drawing the
// shadow, which triggers MapClass::Place_Down and Pick_Up, which call
// RecalcAttributes. This only runs for players who can see the aircraft,
// causing LandType to diverge based on fog-of-war state.
// Fix: no-op all three Set_Height calls during shadow drawing.
// 0x5F4300 = ObjectClass::Record_The_Kill_House (empty body, safe NOP target).
DEFINE_JUMP(CALL6, 0x4147D5, 0x5F4300); // Set_Height bridge path
DEFINE_JUMP(CALL6, 0x4147F3, 0x5F4300); // Set_Height non-bridge path
DEFINE_JUMP(CALL6, 0x4148AB, 0x5F4300); // Set_Height restore

// ============================================================
// Crate removal RecalcAttributes
// ============================================================

// Single-player crate removal path.
DEFINE_HOOK(0x4A1BEF, CrateClass_Get_Crate_RecalcAttributes, 0x6)
{
	GET(CellClass*, pCell, EBX);
	pCell->RecalcAttributes(DWORD(-1));
	return 0;
}

// Multiplayer crate removal path.
DEFINE_HOOK(0x56C1DA, MapClass_Remove_Crate_RecalcAttributes, 0x6)
{
	GET(CellClass*, pCell, EBX);
	pCell->RecalcAttributes(DWORD(-1));
	return 0;
}

// ============================================================
// Flag removal RecalcAttributes
// ============================================================

// HouseClass::Flag_Remove: clears the flag home cell overlay without RecalcAttributes.
DEFINE_HOOK(0x4FBF3C, HouseClass_Flag_Remove_RecalcAttributes, 0x5)
{
	GET(CellClass*, pCell, EAX);
	pCell->RecalcAttributes(DWORD(-1));
	return 0;
}

// ============================================================
// Veinhole constructor RecalcAttributes
// ============================================================

// VeinholeMonsterClass constructor directly writes SlopeIndex, IsoTileTypeIndex,
// Height, and Level for a 3x3 cell grid without calling RecalcAttributes.
// Hook after the last per-cell write (Level) each iteration to fix all 9 cells.
DEFINE_HOOK(0x74C775, VeinholeMonster_Constructor_RecalcAttributes, 0x6)
{
	GET(CellClass*, pCell, EAX);
	pCell->Level = (char)R->DL(); // write Level-1 before RecalcAttributes
	pCell->RecalcAttributes(DWORD(-1));
	MapClass::Instance.ResetZones(pCell->MapCoords);
	MapClass::Instance.RecalculateSubZones(pCell->MapCoords);
	return 0x74C77B;
}

// ============================================================
// Bridge RecalcAttributes helpers
// ============================================================

// Called when a bridge section overlay is fully removed (OverlayTypeIndex = -1).
static void BridgeCellDestroyed(CellClass* pCell)
{
	pCell->RecalcAttributes(DWORD(-1));
	MapClass::Instance.ResetZones(pCell->MapCoords);
	MapClass::Instance.RecalculateSubZones(pCell->MapCoords);
}

// Called when a bridge cell's damage state changes (OverlayData only, overlay
// type index still valid).
static void BridgeCellDamaged(CellClass* pCell)
{
	pCell->RecalcAttributes(DWORD(-1));
	MapClass::Instance.RecalculateZones(pCell->MapCoords);
	MapClass::Instance.RecalculateSubZones(pCell->MapCoords);
}

// ============================================================
// Bridge hooks - Group A: ESI = CellClass*, OverlayTypeIndex = -1 (7 bytes)
// ============================================================

DEFINE_HOOK(0x56EFF2, MapBridge_56EF50_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x56EFF9;
}

DEFINE_HOOK(0x56F392, MapBridge_56F2F0_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x56F399;
}

DEFINE_HOOK(0x56F956, MapBridge_Destroy_56F8B0_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x56F95D;
}

DEFINE_HOOK(0x56FD26, MapBridge_56FCC0_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x56FD2D;
}

DEFINE_HOOK(0x5721C2, MapBridge_572100_RecalcCell_A, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x5721C9;
}

DEFINE_HOOK(0x5724E2, MapBridge_572480_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x5724E9;
}

DEFINE_HOOK(0x572882, MapBridge_572820_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x572889;
}

DEFINE_HOOK(0x572E46, MapBridge_572DE0_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x572E4D;
}

DEFINE_HOOK(0x573216, MapBridge_5731B0_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x57321D;
}

DEFINE_HOOK(0x57779F, MapBridge_577740_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x5777A6;
}

DEFINE_HOOK(0x5778BB, MapBridge_577860_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x5778C2;
}

// ============================================================
// Bridge hooks - Group B: EAX = CellClass*, OverlayData change (7 bytes)
// ============================================================

DEFINE_HOOK(0x56F712, MapBridge_56F690_Damaged_EAX_F1, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x56F719;
}

DEFINE_HOOK(0x56F71B, MapBridge_56F690_Damaged_EAX_E1, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xE;
	BridgeCellDamaged(pCell);
	return 0x56F722;
}

DEFINE_HOOK(0x56F822, MapBridge_56F7A0_Damaged_EAX_F2, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x56F829;
}

DEFINE_HOOK(0x56F82B, MapBridge_56F7A0_Damaged_EAX_D2, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xD;
	BridgeCellDamaged(pCell);
	return 0x56F832;
}

DEFINE_HOOK(0x572C02, MapBridge_572BC0_Damaged_EAX_F3, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x572C09;
}

DEFINE_HOOK(0x572C0B, MapBridge_572BC0_Damaged_EAX_E3, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xE;
	BridgeCellDamaged(pCell);
	return 0x572C12;
}

DEFINE_HOOK(0x572D12, MapBridge_572CD0_Damaged_EAX_F4, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x572D19;
}

DEFINE_HOOK(0x572D1B, MapBridge_572CD0_Damaged_EAX_D4, 0x7)
{
	GET(CellClass*, pCell, EAX);
	pCell->OverlayData = 0xD;
	BridgeCellDamaged(pCell);
	return 0x572D22;
}

// ============================================================
// Bridge hooks - Group C: ESI = CellClass*, OverlayData change (7 bytes)
// ============================================================

DEFINE_HOOK(0x572101, MapBridge_572100_Damaged_ESI, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x572108;
}

DEFINE_HOOK(0x5777FC, MapBridge_577740_Damaged_ESI, 0x7)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayData = 0xF;
	BridgeCellDamaged(pCell);
	return 0x577803;
}

// ============================================================
// Bridge hooks - Group D: EBP = CellClass*, OverlayTypeIndex = -1 (7 bytes)
// ============================================================

// VeinholeMonsterClass area bridge cell clear (0x74CBEE).
DEFINE_HOOK(0x74CBEE, VeinholeArea_Bridge_RecalcCell, 0x7)
{
	GET(CellClass*, pCell, EBP);
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x74CBF5;
}

// ============================================================
// Bridge hooks - Group E: ESI = CellClass*, OverlayData + OverlayTypeIndex (10 bytes)
// ============================================================

DEFINE_HOOK(0x576721, MapBridge_576200_RecalcCell, 0xA)
{
	GET(CellClass*, pCell, ESI);
	pCell->OverlayData = 0;
	pCell->OverlayTypeIndex = -1;
	BridgeCellDestroyed(pCell);
	return 0x57672B;
}

// Fix a desync with invisible objects
// see 1) on https://modenc2.markjfox.net/Reconnection_Error
DEFINE_HOOK(0x70F1E3, TechnoClass_DrawBehind_InvisibleDesyncFix, 0x8)
{
    enum { CreateBehindAnim = 0x70F1EB, SkipBehindAnim = 0x70F659 };

    GET(VisualType, visual, EAX);
    GET(TechnoClass*, pThis, ESI);

    if (visual != VisualType::Normal) // original gate: only proceed when visually normal
        return SkipBehindAnim;

    if (pThis->GetTechnoType()->Invisible) // desync fix: owner-dependent VISUAL_NORMAL
        return SkipBehindAnim;

    return CreateBehindAnim;
}

// Fixes a desync caused by a check for shrouding at a specific cell
// Sets Session.MPGameMode->SkipCheatCheck() to true
// TODO: Remove when Phobos Release is greater than 0.4.0.2
// as that has a better fix for this.
DEFINE_PATCH(0x5C0E30, 0xB0, 0x01)
