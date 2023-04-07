// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

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

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, Directions) {
		return luabind::class_<directions>("Directions")

		.enum_("Directions")[
			luabind::value("None", Directions::None),
			luabind::value("Up", Directions::Up),
			luabind::value("Down", Directions::Down),
			luabind::value("Left", Directions::Left),
			luabind::value("Right", Directions::Right),
			luabind::value("Any", Directions::Any)
		];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(MiscLuaBindings, DrawBlendMode) {
		return luabind::class_<blend_modes>("DrawBlendMode")

		.enum_("DrawBlendMode")[
			luabind::value("NoBlend", DrawBlendMode::NoBlend),
			luabind::value("Burn", DrawBlendMode::BlendBurn),
			luabind::value("Color", DrawBlendMode::BlendColor),
			luabind::value("Difference", DrawBlendMode::BlendDifference),
			luabind::value("Dissolve", DrawBlendMode::BlendDissolve),
			luabind::value("Dodge", DrawBlendMode::BlendDodge),
			luabind::value("Invert", DrawBlendMode::BlendInvert),
			luabind::value("Luminance", DrawBlendMode::BlendLuminance),
			luabind::value("Multiply", DrawBlendMode::BlendMultiply),
			luabind::value("Saturation", DrawBlendMode::BlendSaturation),
			luabind::value("Screen", DrawBlendMode::BlendScreen),
			luabind::value("Transparency", DrawBlendMode::BlendTransparency),
			luabind::value("BlendModeCount", DrawBlendMode::BlendModeCount)
		];
	}
}