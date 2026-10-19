#include <Utilities/Macro.h>
#include <GadgetClass.h>
#include <TacticalClass.h>

// Allow right-button camera dragging on the radar using the game's normal camera positioning.
namespace
{
	constexpr auto leftButtonActions = GadgetFlag::LeftPress | GadgetFlag::LeftHeld | GadgetFlag::LeftRelease;
	constexpr auto resetDragFlags = GadgetFlag::RightPress | GadgetFlag::RightRelease | GadgetFlag::RightUp;

	bool IsDraggingCamera = false;

	bool __forceinline ShouldContinueCameraDrag(GadgetFlag flags)
	{
		return IsDraggingCamera && (flags & GadgetFlag::RightHeld) && !(flags & leftButtonActions);
	}
}

// Let an ongoing camera drag pass the radar's filter for held mouse buttons.
DEFINE_HOOK(0x6539D8, RadarClass_RTacticalClass_Action_AllowCameraDrag, 0x8)
{
	GET_STACK(GadgetFlag, flags, STACK_OFFSET(0x48, 0x4));

	// The radar gadget normally does not request right-button hold events.
	if (flags & GadgetFlag::RightPress)
	{
		GET(GadgetClass*, pRadarButton, ECX);
		pRadarButton->Flags |= GadgetFlag::RightHeld;
	}

	// Admit dragging and left clicks during the gesture; later code reads the original stack flags.
	const bool isHoldingRightButton = IsDraggingCamera && (flags & GadgetFlag::RightHeld);
	if (isHoldingRightButton && !(flags & GadgetFlag::LeftHeld))
		R->DL(0);

	return 0;
}

// Continue through native camera positioning after validating the radar cell.
DEFINE_HOOK(0x653ADC, RadarClass_RTacticalClass_Action_ContinueCameraDrag, 0x6)
{
	enum { PanCamera = 0x653D7A };
	GET_STACK(GadgetFlag, flags, STACK_OFFSET(0x54, 0x4));

	if (ShouldContinueCameraDrag(flags))
		return PanCamera;

	return 0;
}

// Queue held-button movement like normal scrolling instead of forcing a full redraw on every input poll.
DEFINE_HOOK(0x653E81, RadarClass_RTacticalClass_Action_QueueCameraDrag, 0x6)
{
	enum { FinishCameraMovement = 0x653EC1 };

	GET_STACK(GadgetFlag, flags, STACK_OFFSET(0x54, 0x4));
	GET(const CoordStruct*, pCameraCoords, EAX);

	if (ShouldContinueCameraDrag(flags))
	{
		TacticalClass::Instance->TacticalCoord2 = TacticalClass::CoordsToScreen(*pCameraCoords);
		return FinishCameraMovement;
	}

	return 0;
}

// Start dragging only when the initial right press actually takes the camera movement path.
DEFINE_HOOK(0x653D7A, RadarClass_RTacticalClass_Action_StartCameraDrag, 0x5)
{
	GET_STACK(GadgetFlag, flags, STACK_OFFSET(0x54, 0x4));
	if (flags & GadgetFlag::RightPress)
		IsDraggingCamera = true;

	return 0;
}

// End the previous gesture before dispatching input, even when the cursor is outside the radar.
DEFINE_HOOK(0x4E1822, GadgetClass_Input_ResetRadarCameraDrag, 0x6)
{
	GET(GadgetFlag, flags, EDI);
	if (flags & resetDragFlags)
		IsDraggingCamera = false;

	return 0;
}

// A camera drag must not carry over into another scenario or a loaded save.
DEFINE_HOOK_AGAIN(0x67E6DA, ResetRadarCameraDrag, 0x6) // LoadGame_AfterInit
DEFINE_HOOK(0x685659, ResetRadarCameraDrag, 0xA) // ScenarioClass_ClearClasses
{
	IsDraggingCamera = false;
	return 0;
}
