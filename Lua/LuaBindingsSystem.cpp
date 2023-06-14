// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

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
		.property("AnalogCursor", &Controller::GetAnalogCursor, &Controller::SetAnalogCursor)
		.property("Player", &Controller::GetPlayer, &Controller::SetPlayer)
		.property("MouseMovement", &Controller::GetMouseMovement)
		.property("Disabled", &Controller::IsDisabled, &Controller::SetDisabled)

		.def("IsPlayerControlled", &Controller::IsPlayerControlled)
		.def("RelativeCursorMovement", &Controller::RelativeCursorMovement)
		.def("IsMouseControlled", &Controller::IsMouseControlled)
		.def("IsGamepadControlled", &Controller::IsGamepadControlled)
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
			luabind::value("RELEASE_FACEBUTTON", ControlState::RELEASE_FACEBUTTON),
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
		.property("IsFaction", &DataModule::IsFaction)
		.property("IsMerchant", &DataModule::IsMerchant)

		.def_readwrite("Presets", &DataModule::m_EntityList, luabind::return_stl_iterator);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(SystemLuaBindings, Timer) {
		return luabind::class_<Timer>("Timer")

		.def(luabind::constructor<>())
		.def(luabind::constructor<double>())
		.def(luabind::constructor<double, double>())

		.property("StartRealTimeMS", &Timer::GetStartRealTimeMS, &Timer::SetStartRealTimeMS)
		.property("ElapsedRealTimeS", &Timer::GetElapsedRealTimeS, &Timer::SetElapsedRealTimeS)
		.property("ElapsedRealTimeMS", &Timer::GetElapsedRealTimeMS, &Timer::SetElapsedRealTimeMS)
		.property("StartSimTimeMS", &Timer::GetStartSimTimeMS, &Timer::SetStartSimTimeMS)
		.property("ElapsedSimTimeS", &Timer::GetElapsedSimTimeS, &Timer::SetElapsedSimTimeS)
		.property("ElapsedSimTimeMS", &Timer::GetElapsedSimTimeMS, &Timer::SetElapsedSimTimeMS)
		.property("RealTimeLimitProgress", &Timer::RealTimeLimitProgress)
		.property("SimTimeLimitProgress", &Timer::SimTimeLimitProgress)

		.def("Reset", &Timer::Reset)
		.def("SetRealTimeLimitMS", &Timer::SetRealTimeLimitMS)
		.def("SetRealTimeLimitS", &Timer::SetRealTimeLimitS)
		.def("IsPastRealTimeLimit", &Timer::IsPastRealTimeLimit)
		.def("LeftTillRealTimeLimitMS", &Timer::LeftTillRealTimeLimitMS)
		.def("LeftTillRealTimeLimitS", &Timer::LeftTillRealTimeLimitS)
		.def("LeftTillRealMS", &Timer::LeftTillRealMS)
		.def("IsPastRealMS", &Timer::IsPastRealMS)
		.def("AlternateReal", &Timer::AlternateReal)
		.def("GetSimTimeLimitMS", &Timer::GetSimTimeLimitMS)
		.def("SetSimTimeLimitMS", &Timer::SetSimTimeLimitMS)
		.def("GetSimTimeLimitS", &Timer::GetSimTimeLimitS)
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
		.property("SqrMagnitude", &Vector::GetSqrMagnitude)
		.property("Largest", &Vector::GetLargest)
		.property("Smallest", &Vector::GetSmallest)
		.property("Normalized", &Vector::GetNormalized)
		.property("Perpendicular", &Vector::GetPerpendicular)
		.property("AbsRadAngle", &Vector::GetAbsRadAngle, &Vector::SetAbsRadAngle)
		.property("AbsDegAngle", &Vector::GetAbsDegAngle, &Vector::SetAbsDegAngle)

		.def_readwrite("X", &Vector::m_X)
		.def_readwrite("Y", &Vector::m_Y)

		.def("MagnitudeIsGreaterThan", &Vector::MagnitudeIsGreaterThan)
		.def("MagnitudeIsLessThan", &Vector::MagnitudeIsLessThan)
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
		.def("GetRadRotatedCopy", &Vector::GetRadRotatedCopy)
		.def("GetDegRotatedCopy", &Vector::GetDegRotatedCopy)
		.def("AbsRotateTo", &Vector::AbsRotateTo)
		.def("SetXY", &Vector::SetXY);
	}
}