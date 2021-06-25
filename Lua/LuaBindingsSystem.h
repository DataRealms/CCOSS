#ifndef _RTELUABINDSYSTEM_
#define _RTELUABINDSYSTEM_

#include "LuaMacros.h"

#include "Box.h"
#include "Controller.h"
#include "DataModule.h"
#include "PieSlice.h"
#include "Vector.h"

namespace RTE {

	/// <summary>
	/// 
	/// </summary>
	struct SystemLuaBindings {

		LuaBindingRegisterFunctionForType(Box) {
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

		LuaBindingRegisterFunctionForType(Controller) {
			return luabind::class_<Controller>("Controller")
				.enum_("ControlState")[
					value("PRIMARY_ACTION", ControlState::PRIMARY_ACTION),
					value("SECONDARY_ACTION", ControlState::SECONDARY_ACTION),
					value("MOVE_IDLE", ControlState::MOVE_IDLE),
					value("MOVE_RIGHT", ControlState::MOVE_RIGHT),
					value("MOVE_LEFT", ControlState::MOVE_LEFT),
					value("MOVE_UP", ControlState::MOVE_UP),
					value("MOVE_DOWN", ControlState::MOVE_DOWN),
					value("MOVE_FAST", ControlState::MOVE_FAST),
					value("BODY_JUMPSTART", ControlState::BODY_JUMPSTART),
					value("BODY_JUMP", ControlState::BODY_JUMP),
					value("BODY_CROUCH", ControlState::BODY_CROUCH),
					value("AIM_UP", ControlState::AIM_UP),
					value("AIM_DOWN", ControlState::AIM_DOWN),
					value("AIM_SHARP", ControlState::AIM_SHARP),
					value("WEAPON_FIRE", ControlState::WEAPON_FIRE),
					value("WEAPON_RELOAD", ControlState::WEAPON_RELOAD),
					value("PIE_MENU_ACTIVE", ControlState::PIE_MENU_ACTIVE),
					value("WEAPON_CHANGE_NEXT", ControlState::WEAPON_CHANGE_NEXT),
					value("WEAPON_CHANGE_PREV", ControlState::WEAPON_CHANGE_PREV),
					value("WEAPON_PICKUP", ControlState::WEAPON_PICKUP),
					value("WEAPON_DROP", ControlState::WEAPON_DROP),
					value("ACTOR_NEXT", ControlState::ACTOR_NEXT),
					value("ACTOR_PREV", ControlState::ACTOR_PREV),
					value("ACTOR_BRAIN", ControlState::ACTOR_BRAIN),
					value("ACTOR_NEXT_PREP", ControlState::ACTOR_NEXT_PREP),
					value("ACTOR_PREV_PREP", ControlState::ACTOR_PREV_PREP),
					value("HOLD_RIGHT", ControlState::HOLD_RIGHT),
					value("HOLD_LEFT", ControlState::HOLD_LEFT),
					value("HOLD_UP", ControlState::HOLD_UP),
					value("HOLD_DOWN", ControlState::HOLD_DOWN),
					value("PRESS_PRIMARY", ControlState::PRESS_PRIMARY),
					value("PRESS_SECONDARY", ControlState::PRESS_SECONDARY),
					value("PRESS_RIGHT", ControlState::PRESS_RIGHT),
					value("PRESS_LEFT", ControlState::PRESS_LEFT),
					value("PRESS_UP", ControlState::PRESS_UP),
					value("PRESS_DOWN", ControlState::PRESS_DOWN),
					value("RELEASE_PRIMARY", ControlState::RELEASE_PRIMARY),
					value("RELEASE_SECONDARY", ControlState::RELEASE_SECONDARY),
					value("PRESS_FACEBUTTON", ControlState::PRESS_FACEBUTTON),
					value("SCROLL_UP", ControlState::SCROLL_UP),
					value("SCROLL_DOWN", ControlState::SCROLL_DOWN),
					value("DEBUG_ONE", ControlState::DEBUG_ONE),
					value("CONTROLSTATECOUNT", ControlState::CONTROLSTATECOUNT)
				]
				.enum_("InputMode")[
					value("CIM_DISABLED", Controller::InputMode::CIM_DISABLED),
					value("CIM_PLAYER", Controller::InputMode::CIM_PLAYER),
					value("CIM_AI", Controller::InputMode::CIM_AI),
					value("CIM_NETWORK", Controller::InputMode::CIM_NETWORK),
					value("CIM_INPUTMODECOUNT", Controller::InputMode::CIM_INPUTMODECOUNT)
				]
				.def(luabind::constructor<>())
				.property("InputMode", &Controller::GetInputMode, &Controller::SetInputMode)
				.def("IsPlayerControlled", &Controller::IsPlayerControlled)
				.property("ControlledActor", &Controller::GetControlledActor, &Controller::SetControlledActor)
				.property("Team", &Controller::GetTeam, &Controller::SetTeam)
				.property("AnalogMove", &Controller::GetAnalogMove, &Controller::SetAnalogMove)
				.property("AnalogAim", &Controller::GetAnalogAim, &Controller::SetAnalogAim)
				.property("AnalogCursor", &Controller::GetAnalogCursor)
				.def("RelativeCursorMovement", &Controller::RelativeCursorMovement)
				.property("Player", &Controller::GetPlayer, &Controller::SetPlayer)
				.def("IsMouseControlled", &Controller::IsMouseControlled)
				.property("MouseMovement", &Controller::GetMouseMovement)
				.property("Disabled", &Controller::IsDisabled, &Controller::SetDisabled)
				.def("SetState", &Controller::SetState)
				.def("IsState", &Controller::IsState);
		}

		LuaBindingRegisterFunctionForType(DataModule) {
			return luabind::class_<DataModule>("DataModule")
				//.def_readwrite("Presets", &DataModule::m_EntityList, return_stl_iterator)
				.property("FileName", &DataModule::GetFileName)
				.property("FriendlyName", &DataModule::GetFriendlyName)
				.property("Author", &DataModule::GetAuthor)
				.property("Description", &DataModule::GetDescription)
				.property("Version", &DataModule::GetVersionNumber);
		}

		LuaBindingRegisterFunctionForType(PieSlice) {
			return luabind::class_<PieSlice>("Slice")
				.enum_("Direction")[
					value("NONE", PieSlice::SliceDirection::NONE),
					value("UP", PieSlice::SliceDirection::UP),
					value("RIGHT", PieSlice::SliceDirection::RIGHT),
					value("DOWN", PieSlice::SliceDirection::DOWN),
					value("LEFT", PieSlice::SliceDirection::LEFT)
				]
				.enum_("PieSliceIndex")[
					value("PSI_NONE", PieSlice::PieSliceIndex::PSI_NONE),
					value("PSI_PICKUP", PieSlice::PieSliceIndex::PSI_PICKUP),
					value("PSI_DROP", PieSlice::PieSliceIndex::PSI_DROP),
					value("PSI_NEXTITEM", PieSlice::PieSliceIndex::PSI_NEXTITEM),
					value("PSI_PREVITEM", PieSlice::PieSliceIndex::PSI_PREVITEM),
					value("PSI_RELOAD", PieSlice::PieSliceIndex::PSI_RELOAD),
					value("PSI_BUYMENU", PieSlice::PieSliceIndex::PSI_BUYMENU),
					value("PSI_STATS", PieSlice::PieSliceIndex::PSI_STATS),
					value("PSI_MINIMAP", PieSlice::PieSliceIndex::PSI_MINIMAP),
					value("PSI_FORMSQUAD", PieSlice::PieSliceIndex::PSI_FORMSQUAD),
					value("PSI_CEASEFIRE", PieSlice::PieSliceIndex::PSI_CEASEFIRE),
					value("PSI_SENTRY", PieSlice::PieSliceIndex::PSI_SENTRY),
					value("PSI_PATROL", PieSlice::PieSliceIndex::PSI_PATROL),
					value("PSI_BRAINHUNT", PieSlice::PieSliceIndex::PSI_BRAINHUNT),
					value("PSI_GOLDDIG", PieSlice::PieSliceIndex::PSI_GOLDDIG),
					value("PSI_GOTO", PieSlice::PieSliceIndex::PSI_GOTO),
					value("PSI_RETURN", PieSlice::PieSliceIndex::PSI_RETURN),
					value("PSI_STAY", PieSlice::PieSliceIndex::PSI_STAY),
					value("PSI_DELIVER", PieSlice::PieSliceIndex::PSI_DELIVER),
					value("PSI_SCUTTLE", PieSlice::PieSliceIndex::PSI_SCUTTLE),
					value("PSI_DONE", PieSlice::PieSliceIndex::PSI_DONE),
					value("PSI_LOAD", PieSlice::PieSliceIndex::PSI_LOAD),
					value("PSI_SAVE", PieSlice::PieSliceIndex::PSI_SAVE),
					value("PSI_NEW", PieSlice::PieSliceIndex::PSI_NEW),
					value("PSI_PICK", PieSlice::PieSliceIndex::PSI_PICK),
					value("PSI_MOVE", PieSlice::PieSliceIndex::PSI_MOVE),
					value("PSI_REMOVE", PieSlice::PieSliceIndex::PSI_REMOVE),
					value("PSI_INFRONT", PieSlice::PieSliceIndex::PSI_INFRONT),
					value("PSI_BEHIND", PieSlice::PieSliceIndex::PSI_BEHIND),
					value("PSI_ZOOMIN", PieSlice::PieSliceIndex::PSI_ZOOMIN),
					value("PSI_ZOOMOUT", PieSlice::PieSliceIndex::PSI_ZOOMOUT),
					value("PSI_TEAM1", PieSlice::PieSliceIndex::PSI_TEAM1),
					value("PSI_TEAM2", PieSlice::PieSliceIndex::PSI_TEAM2),
					value("PSI_TEAM3", PieSlice::PieSliceIndex::PSI_TEAM3),
					value("PSI_TEAM4", PieSlice::PieSliceIndex::PSI_TEAM4),
					value("PSI_SCRIPTED", PieSlice::PieSliceIndex::PSI_SCRIPTED),
					value("PSI_COUNT", PieSlice::PieSliceIndex::PSI_COUNT)
				]
				.def(luabind::constructor<>())
				.property("FunctionName", &PieSlice::GetFunctionName)
				.property("Description", &PieSlice::GetDescription)
				.property("Type", &PieSlice::GetType)
				.property("Direction", &PieSlice::GetDirection);
		}

		LuaBindingRegisterFunctionForType(Timer) {
			return luabind::class_<Timer>("Timer")
				.def(luabind::constructor<>())
				.def("Reset", &Timer::Reset)
				.property("StartRealTimeMS", &Timer::GetStartRealTimeMS, &Timer::SetStartRealTimeMS)
				.property("ElapsedRealTimeS", &Timer::GetElapsedRealTimeS, &Timer::SetElapsedRealTimeS)
				.property("ElapsedRealTimeMS", &Timer::GetElapsedRealTimeMS, &Timer::SetElapsedRealTimeMS)
				.def("SetRealTimeLimitMS", &Timer::SetRealTimeLimitMS)
				.def("SetRealTimeLimitS", &Timer::SetRealTimeLimitS)
				.def("IsPastRealTimeLimit", &Timer::IsPastRealTimeLimit)
				.def("LeftTillRealTimeLimitMS", &Timer::LeftTillRealTimeLimitMS)
				.def("LeftTillRealTimeLimitS", &Timer::LeftTillRealTimeLimitS)
				.def("LeftTillRealMS", &Timer::LeftTillRealMS)
				.def("IsPastRealMS", &Timer::IsPastRealMS)
				.def("AlternateReal", &Timer::AlternateReal)
				.property("StartSimTimeMS", &Timer::GetStartSimTimeMS, &Timer::SetStartSimTimeMS)
				.property("ElapsedSimTimeS", &Timer::GetElapsedSimTimeS, &Timer::SetElapsedSimTimeS)
				.property("ElapsedSimTimeMS", &Timer::GetElapsedSimTimeMS, &Timer::SetElapsedSimTimeMS)
				.def("SetSimTimeLimitMS", &Timer::SetSimTimeLimitMS)
				.def("SetSimTimeLimitS", &Timer::SetSimTimeLimitS)
				.def("IsPastSimTimeLimit", &Timer::IsPastSimTimeLimit)
				.def("LeftTillSimTimeLimitMS", &Timer::LeftTillSimTimeLimitMS)
				.def("LeftTillSimTimeLimitS", &Timer::LeftTillSimTimeLimitS)
				.def("LeftTillSimMS", &Timer::LeftTillSimMS)
				.def("IsPastSimMS", &Timer::IsPastSimMS)
				.def("AlternateSim", &Timer::AlternateSim);
		}

		LuaBindingRegisterFunctionForType(Vector) {
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
				.def("SetMagnitude", &Vector::SetMagnitude)
				.property("Largest", &Vector::GetLargest)
				.property("Smallest", &Vector::GetSmallest)
				.property("Normalized", &Vector::GetNormalized)
				.property("Perpendicular", &Vector::GetPerpendicular)
				.def("GetXFlipped", &Vector::GetXFlipped)
				.def("GetYFlipped", &Vector::GetYFlipped)
				.property("AbsRadAngle", &Vector::GetAbsRadAngle)
				.property("AbsDegAngle", &Vector::GetAbsDegAngle)
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
				.def_readwrite("X", &Vector::m_X)
				.def_readwrite("Y", &Vector::m_Y)
				.def("SetXY", &Vector::SetXY);
		}
	};
}
#endif