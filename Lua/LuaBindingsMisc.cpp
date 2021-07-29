#include "LuaBindingRegisterDefinitions.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, AlarmEvent) {
		return luabind::class_<AlarmEvent>("AlarmEvent")

		.def(luabind::constructor<>())
		.def(luabind::constructor<const Vector &, int, float>())

		.def_readwrite("ScenePos", &AlarmEvent::m_ScenePos)
		.def_readwrite("Team", &AlarmEvent::m_Team)
		.def_readwrite("Range", &AlarmEvent::m_Range);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, InputDevice) {
		return luabind::class_<input_device>("InputDevice")

		.enum_("InputDevice")[
			luabind::value("DEVICE_KEYB_ONLY", InputDevice::DEVICE_KEYB_ONLY),
			luabind::value("DEVICE_MOUSE_KEYB", InputDevice::DEVICE_MOUSE_KEYB),
			luabind::value("DEVICE_GAMEPAD_1", InputDevice::DEVICE_GAMEPAD_1),
			luabind::value("DEVICE_GAMEPAD_2", InputDevice::DEVICE_GAMEPAD_2),
			luabind::value("DEVICE_GAMEPAD_3", InputDevice::DEVICE_GAMEPAD_3),
			luabind::value("DEVICE_GAMEPAD_4", InputDevice::DEVICE_GAMEPAD_4),
			luabind::value("DEVICE_COUNT", InputDevice::DEVICE_COUNT)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, InputElements) {
		return luabind::class_<input_elements>("InputElements")

		.enum_("InputElements")[
			luabind::value("INPUT_L_UP", InputElements::INPUT_L_UP),
			luabind::value("INPUT_L_DOWN", InputElements::INPUT_L_DOWN),
			luabind::value("INPUT_L_LEFT", InputElements::INPUT_L_LEFT),
			luabind::value("INPUT_L_RIGHT", InputElements::INPUT_L_RIGHT),
			luabind::value("INPUT_R_UP", InputElements::INPUT_R_UP),
			luabind::value("INPUT_R_DOWN", InputElements::INPUT_R_DOWN),
			luabind::value("INPUT_R_LEFT", InputElements::INPUT_R_LEFT),
			luabind::value("INPUT_R_RIGHT", InputElements::INPUT_R_RIGHT),
			luabind::value("INPUT_FIRE", InputElements::INPUT_FIRE),
			luabind::value("INPUT_AIM", InputElements::INPUT_AIM),
			luabind::value("INPUT_AIM_UP", InputElements::INPUT_AIM_UP),
			luabind::value("INPUT_AIM_DOWN", InputElements::INPUT_AIM_DOWN),
			luabind::value("INPUT_AIM_LEFT", InputElements::INPUT_AIM_LEFT),
			luabind::value("INPUT_AIM_RIGHT", InputElements::INPUT_AIM_RIGHT),
			luabind::value("INPUT_PIEMENU", InputElements::INPUT_PIEMENU),
			luabind::value("INPUT_JUMP", InputElements::INPUT_JUMP),
			luabind::value("INPUT_CROUCH", InputElements::INPUT_CROUCH),
			luabind::value("INPUT_NEXT", InputElements::INPUT_NEXT),
			luabind::value("INPUT_PREV", InputElements::INPUT_PREV),
			luabind::value("INPUT_START", InputElements::INPUT_START),
			luabind::value("INPUT_BACK", InputElements::INPUT_BACK),
			luabind::value("INPUT_COUNT", InputElements::INPUT_COUNT)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, MouseButtons) {
		return luabind::class_<mouse_buttons>("MouseButtons")

		.enum_("MouseButtons")[
			luabind::value("MOUSE_NONE", MouseButtons::MOUSE_NONE),
			luabind::value("MOUSE_LEFT", MouseButtons::MOUSE_LEFT),
			luabind::value("MOUSE_RIGHT", MouseButtons::MOUSE_RIGHT),
			luabind::value("MOUSE_MIDDLE", MouseButtons::MOUSE_MIDDLE),
			luabind::value("MAX_MOUSE_BUTTONS", MouseButtons::MAX_MOUSE_BUTTONS)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, JoyButtons) {
		return luabind::class_<joy_buttons>("JoyButtons")

		.enum_("JoyButtons")[
			luabind::value("JOY_NONE", JoyButtons::JOY_NONE),
			luabind::value("JOY_1", JoyButtons::JOY_1),
			luabind::value("JOY_2", JoyButtons::JOY_2),
			luabind::value("JOY_3", JoyButtons::JOY_3),
			luabind::value("JOY_4", JoyButtons::JOY_4),
			luabind::value("JOY_5", JoyButtons::JOY_5),
			luabind::value("JOY_6", JoyButtons::JOY_6),
			luabind::value("JOY_7", JoyButtons::JOY_7),
			luabind::value("JOY_8", JoyButtons::JOY_8),
			luabind::value("JOY_9", JoyButtons::JOY_9),
			luabind::value("JOY_10", JoyButtons::JOY_10),
			luabind::value("JOY_11", JoyButtons::JOY_11),
			luabind::value("JOY_12", JoyButtons::JOY_12),
			luabind::value("MAX_JOY_BUTTONS", JoyButtons::MAX_JOY_BUTTONS)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, JoyDirections) {
		return luabind::class_<joy_directions>("JoyDirections")

		.enum_("JoyDirections")[
			luabind::value("JOYDIR_ONE", JoyDirections::JOYDIR_ONE),
			luabind::value("JOYDIR_TWO", JoyDirections::JOYDIR_TWO)
		];
	}
}