#include "LuaBindingRegisterDefinitions.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Box) {
		return luabind::class_<Box>("Box")

		.def(luabind::constructor<>())
		.def(luabind::constructor<const Vector &, const Vector &>())
		.def(luabind::constructor<float, float, float, float>())
		.def(luabind::constructor<const Vector &, float, float>())
		.def(luabind::constructor<const Box &>())
		.def(luabind::self == luabind::other<const Box &>())

		.property("ClassName", &Box::GetClassName)
		.property("Corner", &Box::GetCorner, &Box::SetCorner)
		.property("Width", &Box::GetWidth, &Box::SetWidth)
		.property("Height", &Box::GetHeight, &Box::SetHeight)
		.property("Center", &Box::GetCenter, &Box::SetCenter)
		.property("Area", &Box::GetArea)

		.def("GetRandomPoint", &Box::GetRandomPoint)
		.def("Unflip", &Box::Unflip)
		.def("IsWithinBox", &Box::IsWithinBox)
		.def("IsWithinBoxX", &Box::IsWithinBoxX)
		.def("IsWithinBoxY", &Box::IsWithinBoxY)
		.def("GetWithinBoxX", &Box::GetWithinBoxX)
		.def("GetWithinBoxY", &Box::GetWithinBoxY)
		.def("GetWithinBox", &Box::GetWithinBox)
		.def("IntersectsBox", &Box::IntersectsBox);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Controller) {
		return luabind::class_<Controller>("Controller")

		.def(luabind::constructor<>())

		.property("InputMode", &Controller::GetInputMode, &Controller::SetInputMode)
		.property("ControlledActor", &Controller::GetControlledActor, &Controller::SetControlledActor)
		.property("Team", &Controller::GetTeam, &Controller::SetTeam)
		.property("AnalogMove", &Controller::GetAnalogMove, &Controller::SetAnalogMove)
		.property("AnalogAim", &Controller::GetAnalogAim, &Controller::SetAnalogAim)
		.property("AnalogCursor", &Controller::GetAnalogCursor)
		.property("Player", &Controller::GetPlayer, &Controller::SetPlayer)
		.property("MouseMovement", &Controller::GetMouseMovement)
		.property("Disabled", &Controller::IsDisabled, &Controller::SetDisabled)

		.def("IsPlayerControlled", &Controller::IsPlayerControlled)
		.def("RelativeCursorMovement", &Controller::RelativeCursorMovement)
		.def("IsMouseControlled", &Controller::IsMouseControlled)
		.def("SetState", &Controller::SetState)
		.def("IsState", &Controller::IsState)

		.enum_("ControlState")[
			luabind::value("PRIMARY_ACTION", ControlState::PRIMARY_ACTION),
			luabind::value("SECONDARY_ACTION", ControlState::SECONDARY_ACTION),
			luabind::value("MOVE_IDLE", ControlState::MOVE_IDLE),
			luabind::value("MOVE_RIGHT", ControlState::MOVE_RIGHT),
			luabind::value("MOVE_LEFT", ControlState::MOVE_LEFT),
			luabind::value("MOVE_UP", ControlState::MOVE_UP),
			luabind::value("MOVE_DOWN", ControlState::MOVE_DOWN),
			luabind::value("MOVE_FAST", ControlState::MOVE_FAST),
			luabind::value("BODY_JUMPSTART", ControlState::BODY_JUMPSTART),
			luabind::value("BODY_JUMP", ControlState::BODY_JUMP),
			luabind::value("BODY_CROUCH", ControlState::BODY_CROUCH),
			luabind::value("AIM_UP", ControlState::AIM_UP),
			luabind::value("AIM_DOWN", ControlState::AIM_DOWN),
			luabind::value("AIM_SHARP", ControlState::AIM_SHARP),
			luabind::value("WEAPON_FIRE", ControlState::WEAPON_FIRE),
			luabind::value("WEAPON_RELOAD", ControlState::WEAPON_RELOAD),
			luabind::value("PIE_MENU_ACTIVE", ControlState::PIE_MENU_ACTIVE),
			luabind::value("WEAPON_CHANGE_NEXT", ControlState::WEAPON_CHANGE_NEXT),
			luabind::value("WEAPON_CHANGE_PREV", ControlState::WEAPON_CHANGE_PREV),
			luabind::value("WEAPON_PICKUP", ControlState::WEAPON_PICKUP),
			luabind::value("WEAPON_DROP", ControlState::WEAPON_DROP),
			luabind::value("ACTOR_NEXT", ControlState::ACTOR_NEXT),
			luabind::value("ACTOR_PREV", ControlState::ACTOR_PREV),
			luabind::value("ACTOR_BRAIN", ControlState::ACTOR_BRAIN),
			luabind::value("ACTOR_NEXT_PREP", ControlState::ACTOR_NEXT_PREP),
			luabind::value("ACTOR_PREV_PREP", ControlState::ACTOR_PREV_PREP),
			luabind::value("HOLD_RIGHT", ControlState::HOLD_RIGHT),
			luabind::value("HOLD_LEFT", ControlState::HOLD_LEFT),
			luabind::value("HOLD_UP", ControlState::HOLD_UP),
			luabind::value("HOLD_DOWN", ControlState::HOLD_DOWN),
			luabind::value("PRESS_PRIMARY", ControlState::PRESS_PRIMARY),
			luabind::value("PRESS_SECONDARY", ControlState::PRESS_SECONDARY),
			luabind::value("PRESS_RIGHT", ControlState::PRESS_RIGHT),
			luabind::value("PRESS_LEFT", ControlState::PRESS_LEFT),
			luabind::value("PRESS_UP", ControlState::PRESS_UP),
			luabind::value("PRESS_DOWN", ControlState::PRESS_DOWN),
			luabind::value("RELEASE_PRIMARY", ControlState::RELEASE_PRIMARY),
			luabind::value("RELEASE_SECONDARY", ControlState::RELEASE_SECONDARY),
			luabind::value("PRESS_FACEBUTTON", ControlState::PRESS_FACEBUTTON),
			luabind::value("SCROLL_UP", ControlState::SCROLL_UP),
			luabind::value("SCROLL_DOWN", ControlState::SCROLL_DOWN),
			luabind::value("DEBUG_ONE", ControlState::DEBUG_ONE),
			luabind::value("CONTROLSTATECOUNT", ControlState::CONTROLSTATECOUNT)
		]
		.enum_("InputMode")[
			luabind::value("CIM_DISABLED", Controller::InputMode::CIM_DISABLED),
			luabind::value("CIM_PLAYER", Controller::InputMode::CIM_PLAYER),
			luabind::value("CIM_AI", Controller::InputMode::CIM_AI),
			luabind::value("CIM_NETWORK", Controller::InputMode::CIM_NETWORK),
			luabind::value("CIM_INPUTMODECOUNT", Controller::InputMode::CIM_INPUTMODECOUNT)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, DataModule) {
		return luabind::class_<DataModule>("DataModule")

		.property("FileName", &DataModule::GetFileName)
		.property("FriendlyName", &DataModule::GetFriendlyName)
		.property("Author", &DataModule::GetAuthor)
		.property("Description", &DataModule::GetDescription)
		.property("Version", &DataModule::GetVersionNumber)

		.def_readwrite("Presets", &DataModule::m_EntityList, luabind::return_stl_iterator);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, PieSlice) {
		return luabind::class_<PieSlice>("Slice")

		.def(luabind::constructor<>())

		.property("FunctionName", &PieSlice::GetFunctionName)
		.property("Description", &PieSlice::GetDescription)
		.property("Type", &PieSlice::GetType)
		.property("Direction", &PieSlice::GetDirection)

		.enum_("Direction")[
			luabind::value("NONE", PieSlice::SliceDirection::NONE),
			luabind::value("UP", PieSlice::SliceDirection::UP),
			luabind::value("RIGHT", PieSlice::SliceDirection::RIGHT),
			luabind::value("DOWN", PieSlice::SliceDirection::DOWN),
			luabind::value("LEFT", PieSlice::SliceDirection::LEFT)
		]
		.enum_("PieSliceIndex")[
			luabind::value("PSI_NONE", PieSlice::PieSliceIndex::PSI_NONE),
			luabind::value("PSI_PICKUP", PieSlice::PieSliceIndex::PSI_PICKUP),
			luabind::value("PSI_DROP", PieSlice::PieSliceIndex::PSI_DROP),
			luabind::value("PSI_NEXTITEM", PieSlice::PieSliceIndex::PSI_NEXTITEM),
			luabind::value("PSI_PREVITEM", PieSlice::PieSliceIndex::PSI_PREVITEM),
			luabind::value("PSI_RELOAD", PieSlice::PieSliceIndex::PSI_RELOAD),
			luabind::value("PSI_BUYMENU", PieSlice::PieSliceIndex::PSI_BUYMENU),
			luabind::value("PSI_STATS", PieSlice::PieSliceIndex::PSI_STATS),
			luabind::value("PSI_MINIMAP", PieSlice::PieSliceIndex::PSI_MINIMAP),
			luabind::value("PSI_FORMSQUAD", PieSlice::PieSliceIndex::PSI_FORMSQUAD),
			luabind::value("PSI_CEASEFIRE", PieSlice::PieSliceIndex::PSI_CEASEFIRE),
			luabind::value("PSI_SENTRY", PieSlice::PieSliceIndex::PSI_SENTRY),
			luabind::value("PSI_PATROL", PieSlice::PieSliceIndex::PSI_PATROL),
			luabind::value("PSI_BRAINHUNT", PieSlice::PieSliceIndex::PSI_BRAINHUNT),
			luabind::value("PSI_GOLDDIG", PieSlice::PieSliceIndex::PSI_GOLDDIG),
			luabind::value("PSI_GOTO", PieSlice::PieSliceIndex::PSI_GOTO),
			luabind::value("PSI_RETURN", PieSlice::PieSliceIndex::PSI_RETURN),
			luabind::value("PSI_STAY", PieSlice::PieSliceIndex::PSI_STAY),
			luabind::value("PSI_DELIVER", PieSlice::PieSliceIndex::PSI_DELIVER),
			luabind::value("PSI_SCUTTLE", PieSlice::PieSliceIndex::PSI_SCUTTLE),
			luabind::value("PSI_DONE", PieSlice::PieSliceIndex::PSI_DONE),
			luabind::value("PSI_LOAD", PieSlice::PieSliceIndex::PSI_LOAD),
			luabind::value("PSI_SAVE", PieSlice::PieSliceIndex::PSI_SAVE),
			luabind::value("PSI_NEW", PieSlice::PieSliceIndex::PSI_NEW),
			luabind::value("PSI_PICK", PieSlice::PieSliceIndex::PSI_PICK),
			luabind::value("PSI_MOVE", PieSlice::PieSliceIndex::PSI_MOVE),
			luabind::value("PSI_REMOVE", PieSlice::PieSliceIndex::PSI_REMOVE),
			luabind::value("PSI_INFRONT", PieSlice::PieSliceIndex::PSI_INFRONT),
			luabind::value("PSI_BEHIND", PieSlice::PieSliceIndex::PSI_BEHIND),
			luabind::value("PSI_ZOOMIN", PieSlice::PieSliceIndex::PSI_ZOOMIN),
			luabind::value("PSI_ZOOMOUT", PieSlice::PieSliceIndex::PSI_ZOOMOUT),
			luabind::value("PSI_TEAM1", PieSlice::PieSliceIndex::PSI_TEAM1),
			luabind::value("PSI_TEAM2", PieSlice::PieSliceIndex::PSI_TEAM2),
			luabind::value("PSI_TEAM3", PieSlice::PieSliceIndex::PSI_TEAM3),
			luabind::value("PSI_TEAM4", PieSlice::PieSliceIndex::PSI_TEAM4),
			luabind::value("PSI_SCRIPTED", PieSlice::PieSliceIndex::PSI_SCRIPTED),
			luabind::value("PSI_COUNT", PieSlice::PieSliceIndex::PSI_COUNT)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Timer) {
		return luabind::class_<Timer>("Timer")

		.def(luabind::constructor<>())

		.property("StartRealTimeMS", &Timer::GetStartRealTimeMS, &Timer::SetStartRealTimeMS)
		.property("ElapsedRealTimeS", &Timer::GetElapsedRealTimeS, &Timer::SetElapsedRealTimeS)
		.property("ElapsedRealTimeMS", &Timer::GetElapsedRealTimeMS, &Timer::SetElapsedRealTimeMS)
		.property("StartSimTimeMS", &Timer::GetStartSimTimeMS, &Timer::SetStartSimTimeMS)
		.property("ElapsedSimTimeS", &Timer::GetElapsedSimTimeS, &Timer::SetElapsedSimTimeS)
		.property("ElapsedSimTimeMS", &Timer::GetElapsedSimTimeMS, &Timer::SetElapsedSimTimeMS)

		.def("Reset", &Timer::Reset)
		.def("SetRealTimeLimitMS", &Timer::SetRealTimeLimitMS)
		.def("SetRealTimeLimitS", &Timer::SetRealTimeLimitS)
		.def("IsPastRealTimeLimit", &Timer::IsPastRealTimeLimit)
		.def("LeftTillRealTimeLimitMS", &Timer::LeftTillRealTimeLimitMS)
		.def("LeftTillRealTimeLimitS", &Timer::LeftTillRealTimeLimitS)
		.def("LeftTillRealMS", &Timer::LeftTillRealMS)
		.def("IsPastRealMS", &Timer::IsPastRealMS)
		.def("AlternateReal", &Timer::AlternateReal)
		.def("SetSimTimeLimitMS", &Timer::SetSimTimeLimitMS)
		.def("SetSimTimeLimitS", &Timer::SetSimTimeLimitS)
		.def("IsPastSimTimeLimit", &Timer::IsPastSimTimeLimit)
		.def("LeftTillSimTimeLimitMS", &Timer::LeftTillSimTimeLimitMS)
		.def("LeftTillSimTimeLimitS", &Timer::LeftTillSimTimeLimitS)
		.def("LeftTillSimMS", &Timer::LeftTillSimMS)
		.def("IsPastSimMS", &Timer::IsPastSimMS)
		.def("AlternateSim", &Timer::AlternateSim);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Vector) {
		return luabind::class_<Vector>("Vector")

		.def(luabind::constructor<>())
		.def(luabind::constructor<float, float>())
		.def(luabind::self == luabind::other<const Vector &>())
		.def(luabind::const_self + luabind::other<const Vector &>())
		.def(luabind::const_self - luabind::other<const Vector &>())
		.def(luabind::const_self * float())
		.def(luabind::const_self / float())
		.def(luabind::tostring(luabind::const_self))

		.property("ClassName", &Vector::GetClassName)
		.property("RoundedX", &Vector::GetRoundIntX)
		.property("RoundedY", &Vector::GetRoundIntY)
		.property("Rounded", &Vector::GetRounded)
		.property("FlooredX", &Vector::GetFloorIntX)
		.property("FlooredY", &Vector::GetFloorIntY)
		.property("Floored", &Vector::GetFloored)
		.property("CeilingedX", &Vector::GetCeilingIntX)
		.property("CeilingedY", &Vector::GetCeilingIntY)
		.property("Ceilinged", &Vector::GetCeilinged)
		.property("Magnitude", &Vector::GetMagnitude)
		.property("Largest", &Vector::GetLargest)
		.property("Smallest", &Vector::GetSmallest)
		.property("Normalized", &Vector::GetNormalized)
		.property("Perpendicular", &Vector::GetPerpendicular)
		.property("AbsRadAngle", &Vector::GetAbsRadAngle)
		.property("AbsDegAngle", &Vector::GetAbsDegAngle)

		.def_readwrite("X", &Vector::m_X)
		.def_readwrite("Y", &Vector::m_Y)

		.def("SetMagnitude", &Vector::SetMagnitude)
		.def("GetXFlipped", &Vector::GetXFlipped)
		.def("GetYFlipped", &Vector::GetYFlipped)
		.def("CapMagnitude", &Vector::CapMagnitude)
		.def("ClampMagnitude", &Vector::ClampMagnitude)
		.def("FlipX", &Vector::FlipX)
		.def("FlipY", &Vector::FlipY)
		.def("IsZero", &Vector::IsZero)
		.def("IsOpposedTo", &Vector::IsOpposedTo)
		.def("Dot", &Vector::Dot)
		.def("Cross", &Vector::Cross)
		.def("Round", &Vector::Round)
		.def("ToHalf", &Vector::ToHalf)
		.def("Floor", &Vector::Floor)
		.def("Ceiling", &Vector::Ceiling)
		.def("Normalize", &Vector::Normalize)
		.def("Perpendicularize", &Vector::Perpendicularize)
		.def("Reset", &Vector::Reset)
		.def("RadRotate", &Vector::RadRotate)
		.def("DegRotate", &Vector::DegRotate)
		.def("GetRadRotated", &Vector::GetRadRotated)
		.def("GetDegRotated", &Vector::GetDegRotated)
		.def("AbsRotateTo", &Vector::AbsRotateTo)
		.def("SetXY", &Vector::SetXY);
	}
}