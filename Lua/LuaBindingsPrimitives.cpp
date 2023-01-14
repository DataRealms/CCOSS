// Make sure that binding definition files are always set to NOT use pre-compiled headers and conformance mode (/permissive) otherwise everything will be on fire!

#include "LuaBindingRegisterDefinitions.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, GraphicalPrimitive) {
		return luabind::class_<GraphicalPrimitive>("GraphicalPrimitive");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, LinePrimitive) {
		return luabind::class_<LinePrimitive, GraphicalPrimitive>("LinePrimitive")

		.def(luabind::constructor<int, const Vector &, const Vector &, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, ArcPrimitive) {
		return luabind::class_<ArcPrimitive, GraphicalPrimitive>("ArcPrimitive")

		.def(luabind::constructor<int, const Vector &, float, float, int, int, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, SplinePrimitive) {
		return luabind::class_<SplinePrimitive, GraphicalPrimitive>("SplinePrimitive")

		.def(luabind::constructor<int, const Vector &, const Vector &, const Vector &, const Vector &, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, BoxPrimitive) {
		return luabind::class_<BoxPrimitive, GraphicalPrimitive>("BoxPrimitive")

		.def(luabind::constructor<int, const Vector &, const Vector &, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, BoxFillPrimitive) {
		return luabind::class_<BoxFillPrimitive, GraphicalPrimitive>("BoxFillPrimitive")

		.def(luabind::constructor<int, const Vector &, const Vector &, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, RoundedBoxPrimitive) {
		return luabind::class_<RoundedBoxPrimitive, GraphicalPrimitive>("RoundedBoxPrimitive")

		.def(luabind::constructor<int, const Vector &, const Vector &, int , unsigned char >());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, RoundedBoxFillPrimitive) {
		return luabind::class_<RoundedBoxFillPrimitive, GraphicalPrimitive>("RoundedBoxFillPrimitive")

		.def(luabind::constructor<int, const Vector &, const Vector &, int, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, CirclePrimitive) {
		return luabind::class_<CirclePrimitive, GraphicalPrimitive>("CirclePrimitive")

		.def(luabind::constructor<int, const Vector &, int, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, CircleFillPrimitive) {
		return luabind::class_<CircleFillPrimitive, GraphicalPrimitive>("CircleFillPrimitive")

		.def(luabind::constructor<int, const Vector &, int, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, EllipsePrimitive) {
		return luabind::class_<EllipsePrimitive, GraphicalPrimitive>("EllipsePrimitive")

		.def(luabind::constructor<int, const Vector &, int, int, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, EllipseFillPrimitive) {
		return luabind::class_<EllipseFillPrimitive, GraphicalPrimitive>("EllipseFillPrimitive")

		.def(luabind::constructor<int, const Vector &, int, int, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, TrianglePrimitive) {
		return luabind::class_<TrianglePrimitive, GraphicalPrimitive>("TrianglePrimitive")

		.def(luabind::constructor<int, const Vector &, const Vector &, const Vector &, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, TriangleFillPrimitive) {
		return luabind::class_<TriangleFillPrimitive, GraphicalPrimitive>("TriangleFillPrimitive")

		.def(luabind::constructor<int, const Vector &, const Vector &, const Vector &, unsigned char>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, TextPrimitive) {
		return luabind::class_<TextPrimitive, GraphicalPrimitive>("TextPrimitive")

		.def(luabind::constructor<int, const Vector &, const std::string &, bool, int, float>());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	LuaBindingRegisterFunctionDefinitionForType(PrimitiveLuaBindings, BitmapPrimitive) {
		return luabind::class_<BitmapPrimitive, GraphicalPrimitive>("BitmapPrimitive")

		.def(luabind::constructor<int, const Vector &, const MOSprite *, float, int, bool, bool>())
		.def(luabind::constructor<int, const Vector &, const std::string &, float, bool, bool>());
	}
}